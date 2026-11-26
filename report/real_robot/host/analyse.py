#!/usr/bin/env python3
"""Analyse SensorTests CSV files: offset, noise, step, edge, straight, spin.

    python3 host/analyse.py offset data/offset.csv
    python3 host/analyse.py edge   data/edge.csv --speed 30

Every number printed comes from the file; nothing is assumed.
"""
import argparse, csv, sys
import numpy as np

CH = [f"dn{i}" for i in range(1, 6)]


def load(path):
    rows = []
    with open(path) as fh:
        for line in fh:
            if line.startswith("#") or line.startswith("mode,"):
                continue
            p = line.strip().split(",")
            if len(p) != 12:
                continue
            rows.append(dict(mode=p[0], trial=int(p[1]), sample=int(p[2]),
                             t=float(p[3]), x=float(p[4]), y=float(p[5]),
                             th=float(p[6]), dn=[float(v) for v in p[7:12]]))
    return rows


def per_channel(rows):
    return {c: np.array([r["dn"][i] for r in rows]) for i, c in enumerate(CH)}


def offset(rows, label):
    d = per_channel(rows)
    print(f"{label}: n={len(rows)} snapshots")
    print("channel   median_us    IQR_us   sd_us")
    for c in CH:
        v = d[c]
        print(f"{c:7s} {np.median(v):9.0f} {np.percentile(v,75)-np.percentile(v,25):8.0f} {np.std(v):7.1f}")
    med = [np.median(d[c]) for c in CH]
    print(f"spread across channels: {max(med)-min(med):.0f} us")


def step(rows):
    trials = sorted({r["trial"] for r in rows})
    print("trial  n   median per channel (us)")
    meds = []
    for t in trials:
        sub = [r for r in rows if r["trial"] == t]
        d = per_channel(sub)
        m = [np.median(d[c]) for c in CH]
        meds.append(m)
        print(f"{t:5d} {len(sub):4d}  " + " ".join(f"{v:6.0f}" for v in m))
    if len(meds) > 2:
        meds = np.array(meds)
        print("\nchange between neighbouring trials (us):")
        for i in range(1, len(meds)):
            print(f"  {trials[i-1]} to {trials[i]}: " +
                  " ".join(f"{v:6.0f}" for v in meds[i] - meds[i-1]))


def edge(rows):
    d = per_channel(rows)
    x = np.array([r["x"] for r in rows])
    print(f"n={len(rows)}  travel={x.max()-x.min():.1f} mm")
    for c in CH:
        v = d[c]
        lo, hi = np.percentile(v, 5), np.percentile(v, 95)
        if hi - lo < 50:
            print(f"{c}: no contrast")
            continue
        t10, t90 = lo + 0.1*(hi-lo), lo + 0.9*(hi-lo)
        i10 = np.argmax(v >= t10); i90 = np.argmax(v >= t90)
        w = abs(x[i90] - x[i10])
        print(f"{c}: range {lo:.0f} to {hi:.0f} us, 10-90 width {w:.1f} mm")


def straight(rows):
    x = np.array([r["x"] for r in rows])
    print(f"odometry distance over the run: {x.max()-x.min():.1f} mm")
    print("measure the real distance with a rule, then:")
    print("  new_radius = 16.0 * measured_mm / printed_mm")


def spin(rows):
    th = abs(rows[-1]["th"] - rows[0]["th"])
    print(f"odometry heading change: {np.degrees(th):.1f} deg")
    print("measure the real heading change with a protractor, then:")
    print("  new_separation = 90.0 * odometry_deg / measured_deg")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("kind", choices=["offset", "noise", "step", "edge", "straight", "spin"])
    ap.add_argument("csv")
    a = ap.parse_args()
    rows = load(a.csv)
    if not rows:
        sys.exit("no rows")
    {"offset": lambda: offset(rows, "offset"),
     "noise": lambda: offset(rows, "noise"),
     "step": lambda: step(rows),
     "edge": lambda: edge(rows),
     "straight": lambda: straight(rows),
     "spin": lambda: spin(rows)}[a.kind]()
