#!/usr/bin/env python3
"""Measure the 10-90 % edge width of each channel from a straight edge scan.

Push the robot at a constant speed across a printed black/white edge, log the
dashboard CSV, then run this at each speed. A width that stays constant with
speed is the footprint. A width that grows is timing delay.

    python3 tools/knife_edge.py scan.csv --speed 20 --out edge.json
"""
import argparse, csv, json, pathlib
import numpy as np


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("scan")
    ap.add_argument("--speed", type=float, required=True, help="mm per second")
    ap.add_argument("--out", default="edge.json")
    a = ap.parse_args()

    with open(a.scan, newline="") as fh:
        lines = [l for l in fh if not l.startswith("#")]
    rdr = csv.reader(lines)
    head = next(rdr)
    if "seq" not in head[0] and "t_ms" not in head[0]:
        head = next(rdr)
    idx = {h.strip(): i for i, h in enumerate(head)}
    t = [float(r[idx["t_ms"]]) for r in rdr]
    rdr2 = list(csv.reader(lines))[1:]
    dn = [[float(r[idx[f"dn{i}"]]) for i in range(1, 6)] for r in rdr2]

    out = {"speed_mm_s": a.speed, "channels": []}
    for s in range(5):
        v = np.array([row[s] for row in dn])
        lo, hi = np.percentile(v, 2), np.percentile(v, 98)
        if hi - lo < 50:
            out["channels"].append(None); continue
        t10, t90 = lo + 0.1 * (hi - lo), lo + 0.9 * (hi - lo)
        # crossing times, first crossing only
        def cross(level):
            for i in range(1, len(v)):
                if (v[i - 1] - level) * (v[i] - level) <= 0:
                    return t[i - 1] + (level - v[i - 1]) / (v[i] - v[i - 1]) * (t[i] - t[i - 1])
            return None
        c10, c90 = cross(t10), cross(t90)
        width = abs(c90 - c10) if (c10 is not None and c90 is not None) else None
        out["channels"].append({
            "sensor": s + 1,
            "width_mm": round(width / 1000 * a.speed, 2) if width else None,
            "range_us": [round(lo, 0), round(hi, 0)],
        })
    pathlib.Path(a.out).write_text(json.dumps(out, indent=2))
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
