#!/usr/bin/env python3
"""Plot the path the twin actually drove, from one run CSV."""
import csv, glob, pathlib
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mp

ROOT = pathlib.Path("/tmp/opencode/dt_build")
plt.rcParams.update({"font.family": "sans-serif", "font.sans-serif": ["DejaVu Sans"],
                     "font.size": 6, "axes.linewidth": 0.5,
                     "xtick.labelsize": 5.5, "ytick.labelsize": 5.5,
                     "axes.labelsize": 6, "savefig.dpi": 400,
                     "savefig.bbox": "tight", "savefig.pad_inches": 0.02})

f = sorted(glob.glob(str(ROOT / "exp/model_calibrated/runs/*_twin_*.csv")))[0]
rows = list(csv.DictReader(l for l in open(f) if not l.startswith("#")))
x = [float(r["true_x_mm"]) for r in rows]
y = [float(r["true_y_mm"]) for r in rows]
row = [int(r["row"]) for r in rows]

fig, ax = plt.subplots(figsize=(3.35, 1.55))
ax.add_patch(mp.Rectangle((0, 0), 160, 112, fc="#fcfcfa", ec="#b0b0b0", lw=0.5, zorder=0))
cmap = plt.get_cmap("viridis")
n = max(row) + 1
pts = ax.scatter(x, y, c=row, cmap=cmap, s=0.6, lw=0, vmin=0, vmax=n - 1, zorder=2)
ax.plot(x, y, color="#888888", lw=0.25, zorder=1)
ax.set_aspect("equal")
ax.set_xlim(-55, 215); ax.set_ylim(-30, 135)
ax.set_xlabel("x, mm"); ax.set_ylabel("y, mm")
ax.tick_params(length=2, pad=1.5)
ax.annotate("straight rows", xy=(85, 56), xytext=(45, 92), fontsize=5.6,
            arrowprops=dict(arrowstyle="-", lw=0.5))
ax.annotate("curved transition\nin the run-in", xy=(178, 8), xytext=(120, -26),
            fontsize=5.6, arrowprops=dict(arrowstyle="-", lw=0.5))
fig.savefig("/tmp/opencode/report/figures/fig_path.pdf")
fig.savefig("/tmp/opencode/report/figures/fig_path.png")
print("ok")
