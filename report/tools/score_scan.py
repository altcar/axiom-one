#!/usr/bin/env python3
"""Score one scan with the same rebuild the digital twin uses.

Usage:
    python3 tools/score_scan.py scan.csv --sheet DigitalTwin/surface.png \
        --offset 85 58.5 --rows 17 --rowlen 160 --pitch 7

Accepts the robot dashboard CSV, the twin CSV, or the raw /data log.
Writes metrics.json and prints a summary. No values are invented: every
number comes from the scan and the sheet you give it.
"""
import argparse, csv, json, math, pathlib
import numpy as np
from PIL import Image


def load_rows(path):
    """Return list of dicts with x, y, theta, dn[5], state, row."""
    rows = []
    with open(path, newline="") as fh:
        lines = [l for l in fh if not l.startswith("#")]
    rdr = csv.reader(lines)
    head = next(rdr, None)
    if head is None:
        return rows
    # Dashboard export has a settings header row before the column names
    if not any(h.strip().startswith("seq") or h.strip().startswith("t_ms") for h in head):
        head = next(rdr, head)
    idx = {h.strip(): i for i, h in enumerate(head)}
    def col(*names):
        for n in names:
            if n in idx:
                return idx[n]
        return None
    cx = col("x", "x_mm"); cy = col("y", "y_mm"); ct = col("theta", "theta_rad")
    cd = [col(f"dn{i}") for i in range(1, 6)]
    cs = col("state"); cr = col("row")
    for r in rdr:
        if len(r) < 6:
            continue
        try:
            rows.append(dict(
                x=float(r[cx]), y=float(r[cy]),
                theta=float(r[ct]) if ct is not None else 0.0,
                dn=[float(r[c]) for c in cd],
                state=int(float(r[cs])) if cs is not None else 2,
                row=int(float(r[cr])) if cr is not None else 0,
            ))
        except (TypeError, ValueError):
            continue
    return rows


def build(rows, sheet, ox, oy, kernel_mm=1.0):
    """Rebuild at 1 px/mm with per-sensor 2-98 percentile and a Gaussian splat.

    Returns darkness, weight and the world offsets of the grid origin.
    """
    x0 = math.floor(min(r["x"] for r in rows)) - 40
    y0 = math.floor(min(r["y"] for r in rows)) - 40
    x1 = math.ceil(max(r["x"] for r in rows)) + 40
    y1 = math.ceil(max(r["y"] for r in rows)) + 40
    w, h = x1 - x0, y1 - y0
    dark = np.zeros((h, w)); wt = np.zeros((h, w))

    lo = [0.0] * 5; hi = [1.0] * 5
    for s in range(5):
        v = np.sort(np.array([r["dn"][s] for r in rows]))
        if len(v) > 10:
            lo[s] = v[int(0.02 * (len(v) - 1))]
            hi[s] = max(v[int(0.98 * (len(v) - 1))], lo[s] + 50)

    k = 2  # 5x5 neighbourhood, sigma 1 mm

    dark.fill(0.0); wt.fill(0.0)
    ahead = [28.0, 40.0, 41.0, 40.0, 28.0]   # measured, mm
    lefts = [27.0, 9.0, 0.0, -9.0, -27.0]
    for r in rows:
        for s in range(5):
            sx = r["x"] + ahead[s] * math.cos(r["theta"]) - lefts[s] * math.sin(r["theta"])
            sy = r["y"] + ahead[s] * math.sin(r["theta"]) + lefts[s] * math.cos(r["theta"])
            gx, gy = sx - x0, sy - y0
            cxi, cyi = int(round(gx)), int(round(gy))
            val = min(max((r["dn"][s] - lo[s]) / (hi[s] - lo[s]), 0.0), 1.0)
            for j in range(cyi - k, cyi + k + 1):
                if j < 0 or j >= h:
                    continue
                for i in range(cxi - k, cxi + k + 1):
                    if i < 0 or i >= w:
                        continue
                    d2 = (i - gx) ** 2 + (j - gy) ** 2
                    wgt = math.exp(-d2 / (2 * kernel_mm ** 2))
                    dark[j, i] += wgt * val
                    wt[j, i] += wgt
    img = np.where(wt > 0.05, dark / np.maximum(wt, 1e-9), np.nan)
    return img, wt, x0, y0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("scan")
    ap.add_argument("--sheet", default="DigitalTwin/surface.png")
    ap.add_argument("--offset", nargs=2, type=float, default=[85.0, 58.5])
    ap.add_argument("--rows", type=int, default=17)
    ap.add_argument("--rowlen", type=float, default=160.0)
    ap.add_argument("--pitch", type=float, default=7.0)
    ap.add_argument("--out", default="metrics.json")
    a = ap.parse_args()

    rows = load_rows(a.scan)
    if not rows:
        raise SystemExit("no samples found")

    img, wt, x0, y0 = build(rows, a.sheet, *a.offset)

    sheet = np.asarray(Image.open(a.sheet).convert("L")).astype(float) / 255.0
    sh, sw = sheet.shape
    truth = np.full_like(img, np.nan)  # darkness, 1 = black
    for j in range(img.shape[0]):
        for i in range(img.shape[1]):
            px = int(math.floor(sw / 2 + (x0 + i + 0.5) - a.offset[0]))
            py = int(math.floor(sh / 2 - (y0 + j + 0.5) + a.offset[1]))
            if 0 <= px < sw and 0 <= py < sh:
                truth[j, i] = 1.0 - sheet[py, px]

    # target rectangle: image area, from the scan settings
    lead = 41.0  # DN3 ahead of the axle, as in the twin
    xin = (np.arange(img.shape[1]) + x0 >= lead) & \
          (np.arange(img.shape[1]) + x0 <= lead + a.rowlen)
    yin = (np.arange(img.shape[0]) + y0 >= -9) & \
          (np.arange(img.shape[0]) + y0 <= (a.rows - 1) * a.pitch + 9)
    target = yin[:, None] & xin[None, :]
    covered = target & (wt > 0.05) & ~np.isnan(truth)
    err = np.abs(img[covered] - truth[covered]).mean() * 100 if covered.any() else float("nan")

    # whole-target score, unmeasured area counted as fully wrong
    n_target = target.sum()
    n_cov = covered.sum()
    err_all = (err * n_cov / 100 + (n_target - n_cov)) / n_target * 100 if n_target else float("nan")

    # edge width across the square's left edge, along rows through its middle
    widths = []
    for j in range(img.shape[0]):
        wy = y0 + j + 0.5
        if not (78 < wy < 99):
            continue
        i0 = int(round(30 - x0)); i1 = int(round(70 - x0))
        seg = img[j, max(i0, 0):min(i1, img.shape[1])]
        seg = seg[~np.isnan(seg)]
        if len(seg) < 30:
            continue
        lo, hi = np.percentile(seg, 5), np.percentile(seg, 95)
        if hi - lo < 0.2:
            continue
        t10, t90 = lo + 0.1 * (hi - lo), lo + 0.9 * (hi - lo)
        above = np.where(seg >= t90)[0]
        below = np.where(seg <= t10)[0]
        if len(above) == 0 or len(below) == 0:
            continue
        # left edge: last low sample before the first high sample
        first_high = above.min()
        lows = below[below < first_high]
        if len(lows) == 0:
            continue
        widths.append(first_high - lows.max())
    width = float(np.median(widths)) if widths else float("nan")

    out = dict(samples=len(rows), image_error_pct=round(float(err), 2),
               image_error_full_target_pct=round(float(err_all), 2),
               coverage_pct=round(100.0 * n_cov / n_target, 1) if n_target else None,
               edge_width_mm=round(width, 2), sheet=a.sheet,
               settings=dict(rows=a.rows, row_length_mm=a.rowlen, pitch_mm=a.pitch))
    pathlib.Path(a.out).write_text(json.dumps(out, indent=2))
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
