#!/usr/bin/env python3
"""Sim-vs-real figure: model sensitivity with the robot's measured point, and
the row-spacing spread the twin cannot reproduce."""
import csv, glob, pathlib
from collections import defaultdict
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = pathlib.Path("/tmp/opencode/dt_build")
FIG = pathlib.Path("/tmp/opencode/report/figures")
plt.rcParams.update({
    "font.family": "sans-serif", "font.sans-serif": ["DejaVu Sans"],
    "font.size": 7, "axes.linewidth": 0.6, "xtick.major.width": 0.6,
    "ytick.major.width": 0.6, "legend.frameon": False, "legend.fontsize": 6,
    "axes.labelsize": 7, "xtick.labelsize": 6.5, "ytick.labelsize": 6.5,
    "savefig.dpi": 400, "savefig.bbox": "tight", "savefig.pad_inches": 0.02,
})

rows = list(csv.DictReader(open(ROOT / "all_runs.csv")))
G = defaultdict(list)
for r in rows:
    G[r["condition"]].append(r)

def m(cond, key):
    v = np.array([float(r[key]) for r in G[cond]])
    return v.mean(), v.std(ddof=1)

BLUE, RED, OLIVE = "#1f4e79", "#a00000", "#7f6000"

fig, (a, b) = plt.subplots(2, 1, figsize=(3.35, 3.0),
                           gridspec_kw=dict(height_ratios=[1.15, 1], hspace=0.62))

# ---------------- (a) sensitivity + measured point ----------------
bel = [82.5, 84.0, 85.5, 87.0, 88.5, 90.0]
x = [(v - 82.5) / 82.5 * 100 for v in bel]
conds = [f"sep_{v}" for v in bel]
pm = [m(c, "pose_err_mean_mm")[0] for c in conds]
ps = [m(c, "pose_err_mean_mm")[1] for c in conds]
px = [m(c, "pose_err_max_mm")[0] for c in conds]

a.fill_between(x, np.subtract(pm, ps), np.add(pm, ps), color=OLIVE, alpha=0.20, lw=0)
a.plot(x, pm, "-o", ms=3.0, lw=1.0, color=OLIVE)
a.plot(x, px, "--^", ms=2.8, lw=0.8, color=OLIVE)
a.axvline(9.09, color=RED, lw=0.8, ls=(0, (3, 2)))
a.annotate("robot measured\n7.5 mm", (9.09, 8.2), xytext=(6.0, 9.4),
           fontsize=5.8, color=RED,
           arrowprops=dict(arrowstyle="-", lw=0.5, color=RED))
a.set_xlabel("Separation error, %")
a.set_ylabel("Pose error, mm")
a.set_xlim(-0.6, 11.4); a.set_ylim(0, 12)
a.text(0.03, 0.94, "a, model sensitivity", transform=a.transAxes,
       fontsize=6.3, va="top", fontweight="bold")
a.text(9.45, 4.85, "mean", fontsize=5.8, color=OLIVE, va="top")
a.text(9.45, 10.0, "max", fontsize=5.8, color=OLIVE, va="top")

# ---------------- (b) row spacing: twin lines vs robot band ----------------
def per_row(cond, n=10):
    out = defaultdict(list)
    for f in sorted(glob.glob(str(ROOT / f"exp/{cond}/runs/*_twin_*.csv")))[:n]:
        d = defaultdict(list)
        for r in csv.DictReader(l for l in open(f) if not l.startswith("#")):
            if int(r["state"]) in (2, 8):
                d[int(r["row"])].append(float(r["true_y_mm"]))
        ys = [np.mean(v) for k, v in sorted(d.items())]
        for i, g in enumerate(np.diff(ys)):
            out[i].append(g)
    ks = sorted(out)
    return np.array(ks), np.array([np.mean(out[k]) for k in ks]), \
        np.array([np.std(out[k], ddof=1) for k in ks])

b.axhspan(5, 32, color=RED, alpha=0.12, lw=0)
b.text(4.0, 5.7, "robot\n5 to 32 mm", fontsize=5.8, color=RED, va="center", ha="left")
for cond, lab, col, mk in [("uturn_eff_1.0", "twin, no slip", BLUE, "o"),
                           ("uturn_eff_0.95", "twin, 5 % slip", OLIVE, "s")]:
    k, mu, sd = per_row(cond)
    b.errorbar(mu, k, xerr=sd, fmt=mk, ms=2.8, lw=0.9, color=col,
               capsize=1.5, elinewidth=0.6, label=lab)

b.set_xlabel("Row-centre spacing, mm, commanded 14")
b.set_ylabel("Row index")
b.set_xlim(0, 40); b.set_ylim(-0.8, 10.4)
b.set_yticks([0, 2, 4, 6, 8])
b.text(0.02, 0.97, "b, twin vs robot", transform=b.transAxes,
       fontsize=6.3, va="top", fontweight="bold")
b.text(14.6, 8.15, "no slip", fontsize=5.8, color=BLUE, ha="center")
b.text(31.5, 8.15, "5 % slip", fontsize=5.8, color=OLIVE, ha="center")

fig.savefig(FIG / "fig_simreal.pdf")
fig.savefig(FIG / "fig_simreal.png")
print("ok")
