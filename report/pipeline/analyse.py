#!/usr/bin/env python3
"""Turn the twin's run CSVs and reconstruction PNGs into the report figures.

Everything plotted here comes from `all_runs.csv` (written by run_experiments.py)
or from the PNGs the twin itself saved. Nothing is invented.
"""
import csv, glob, pathlib
from collections import defaultdict

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mp
from matplotlib.lines import Line2D
from PIL import Image

ROOT = pathlib.Path("/tmp/opencode/dt_build")
FIG = pathlib.Path("/tmp/opencode/report/figures")
FIG.mkdir(parents=True, exist_ok=True)

plt.rcParams.update({
    "font.family": "serif", "font.size": 8, "axes.linewidth": 0.6,
    "xtick.major.width": 0.6, "ytick.major.width": 0.6,
    "legend.frameon": False, "legend.fontsize": 6.5,
    "axes.labelsize": 8, "xtick.labelsize": 7, "ytick.labelsize": 7,
    "savefig.dpi": 300, "savefig.bbox": "tight", "savefig.pad_inches": 0.02,
})

rows = list(csv.DictReader(open(ROOT / "all_runs.csv")))
G = defaultdict(list)
for r in rows:
    G[r["condition"]].append(r)

def m(cond, key):
    v = np.array([float(r[key]) for r in G[cond]])
    return v.mean(), v.std(ddof=1)

BLUE, ORANGE, OLIVE = "#1f4e79", "#b45f06", "#7f6000"

# ============================================================ Figure: setup
fig, ax = plt.subplots(figsize=(3.35, 2.05))
ax.set_xlim(-62, 214); ax.set_ylim(-46, 138); ax.set_aspect("equal"); ax.axis("off")
ax.add_patch(mp.Rectangle((0, 0), 160, 112, fc="#fcfcfa", ec="#9a9a9a", lw=0.6, zorder=1))

# continuous curved zigzag: straight rows, smooth S transitions in the run-in
PITCH, ROWS, MARGIN, BLEND = 7.0, 17, 50.0, 26.0
xs, ys = [], []
x = -MARGIN
y = 0.0
xs.append(x); ys.append(y)
for r in range(ROWS):
    x_end = 160 + MARGIN if r % 2 == 0 else -MARGIN
    y_end = r * PITCH
    for t in np.linspace(0, 1, 60):
        # smoothstep along x, cosine along y where the lane change happens
        xx = x + (x_end - x) * t
        yy = y_end
        xs.append(xx); ys.append(yy)
    if r + 1 < ROWS:
        x0, y0 = x_end, y_end
        x1 = x_end - BLEND if r % 2 == 0 else x_end + BLEND
        for t in np.linspace(0, 1, 30):
            sm = t * t * (3 - 2 * t)
            xs.append(x0 + (x1 - x0) * t)
            ys.append(y0 + PITCH * sm)
        x, y = x1, y0 + PITCH
    else:
        x, y = x_end, y_end
ax.plot(xs, ys, color="#1f4e79", lw=0.75, zorder=3, solid_capstyle="round")

ax.add_patch(mp.Rectangle((12, 68), 30, 30, fc="k", zorder=4))
ax.add_patch(mp.Circle((72, 83), 15, fc="k", zorder=4))
ax.add_patch(mp.Polygon([[102, 68], [132, 68], [117, 93]], fc="k", zorder=4))
ax.add_patch(mp.RegularPolygon((152, 83), 7, radius=14, fc="k", zorder=4))

ax.add_patch(mp.Rectangle((-9, -14), 15, 28, fc="#dce6f1", ec=BLUE, lw=0.9, zorder=6))
ax.annotate("", xy=(17, 0), xytext=(7, 0), zorder=7,
            arrowprops=dict(arrowstyle="-|>", lw=1.0, color=BLUE))
for lx, ly in [(28, 27), (40, 9), (41, 0), (40, -9), (28, -27)]:
    ax.plot([lx], [ly], marker="o", ms=2.2, mfc="#b45f06", mec="none", zorder=8)
ax.text(28, -42, "5 sensors, DN1 to DN5, 54 mm span", fontsize=5.6, color="#b45f06")
ax.text(-58, 128, "one continuous zigzag,\nno turns on the spot", fontsize=5.8, color=BLUE)
ax.annotate("", xy=(0, 118), xytext=(160, 118), arrowprops=dict(arrowstyle="<->", lw=0.5))
ax.text(80, 121, "160 mm row length", fontsize=6, ha="center")
ax.annotate("", xy=(176, 0), xytext=(176, 112), arrowprops=dict(arrowstyle="<->", lw=0.5))
ax.text(179, 56, "112 mm", fontsize=6, rotation=90, va="center")
ax.text(-58, -40, "start cross", fontsize=5.8, ha="left", va="bottom")
fig.savefig(FIG / "fig_setup.pdf"); fig.savefig(FIG / "fig_setup.png"); plt.close(fig)

# ============================================================ Figure 1: pitch
pitch = [5, 7, 10, 13.5, 20]
conds = [f"pitch_{p}" for p in pitch]
e_m = [m(c, "error_vs_sheet_pct")[0] for c in conds]
e_s = [m(c, "error_vs_sheet_pct")[1] for c in conds]
c_m = [m(c, "coverage_pct")[0] for c in conds]
t_m = [m(c, "duration_s")[0] for c in conds]
estar = [c * e / 100 + (100 - c) for c, e in zip(c_m, e_m)]

fig, (ax, axc) = plt.subplots(2, 1, figsize=(3.35, 2.7), sharex=True,
                              gridspec_kw=dict(height_ratios=[1.35, 1], hspace=0.16))
ax.errorbar(pitch, e_m, yerr=e_s, marker="o", ms=3.2, lw=1.1, color=BLUE,
            capsize=2, elinewidth=0.7, label="measured pixels only")
ax.plot(pitch, estar, marker="D", ms=3.0, lw=1.2, color="#a00000",
        label="whole target area")
ax.set_ylabel("Error, %", labelpad=4)
ax.set_ylim(0, 44)
ax.legend(loc="upper left", fontsize=5.9, handlelength=1.5, borderaxespad=0.3)
ax.annotate("", xy=(20, 39.8), xytext=(20, 20.4),
            arrowprops=dict(arrowstyle="<->", lw=0.6, color="#a00000"))
ax.text(19.4, 30.0, "19 points\nof missing image", fontsize=5.6, color="#a00000",
        ha="right", va="center")

axc.plot(pitch, c_m, marker="s", ms=3.2, lw=1.1, color=ORANGE,
         label="area reconstructed")
axc.set_ylabel("Coverage, %", labelpad=4)
axc.set_ylim(68, 104)
axc.set_xlabel("Row pitch, mm")
axc.set_xticks(pitch)
axc.set_xticklabels(["5", "7", "10", "13.5", "20"])
for xx, tt in zip(pitch, t_m):
    axc.annotate(f"{tt:.0f} s", (xx, 71.2), ha="center", va="bottom", fontsize=5.8, color="#555555")
axc.legend(loc="upper right", fontsize=5.9, handlelength=1.4, borderaxespad=0.3)
fig.subplots_adjust(left=0.19, right=0.96, top=0.97, bottom=0.15)
fig.savefig(FIG / "fig_pitch.pdf"); fig.savefig(FIG / "fig_pitch.png"); plt.close(fig)

# ============================================================ Figure 2: pose
bel = [82.5, 84.0, 85.5, 87.0, 88.5, 90.0]
x = [(b - 82.5) / 82.5 * 100 for b in bel]
conds = [f"sep_{b}" for b in bel]
p_m = [m(c, "pose_err_mean_mm")[0] for c in conds]
p_s = [m(c, "pose_err_mean_mm")[1] for c in conds]
pm_m = [m(c, "pose_err_max_mm")[0] for c in conds]
i_m = [m(c, "error_vs_sheet_pct")[0] for c in conds]
i_s = [m(c, "error_vs_sheet_pct")[1] for c in conds]

fig, ax = plt.subplots(figsize=(3.35, 2.25))
ax2 = ax.twinx()
ax.fill_between(x, np.subtract(p_m, p_s), np.add(p_m, p_s), color=OLIVE, alpha=0.18, lw=0)
ax.plot(x, p_m, marker="o", ms=3.2, lw=1.1, color=OLIVE)
ax.plot(x, pm_m, marker="^", ms=3.0, lw=0.9, color=OLIVE, ls="--")
ax.set_xlabel("Wheel-separation error in odometry model (%)")
ax.set_ylabel("Pose error (mm)", color=OLIVE)
ax.tick_params(axis="y", colors=OLIVE); ax.set_ylim(0, 13); ax.set_xlim(-0.6, 10)
ax2.fill_between(x, np.subtract(i_m, i_s), np.add(i_m, i_s), color=BLUE, alpha=0.18, lw=0)
ax2.plot(x, i_m, marker="s", ms=3.2, lw=1.1, color=BLUE)
ax2.set_ylabel("Error, %", color=BLUE); ax2.tick_params(axis="y", colors=BLUE)
ax2.set_ylim(15, 34)
ax.axvline(9.09, color="#a00000", lw=0.8, ls="--")
ax.annotate("robot measured 7.5 mm", (9.05, 12.4), xytext=(0.1, 12.3),
            fontsize=6, color="#a00000", va="center")
handles = [Line2D([], [], color=OLIVE, marker="o", ms=3, lw=1, label="pose error, mean"),
           Line2D([], [], color=OLIVE, marker="^", ms=3, lw=0.9, ls="--", label="pose error, max"),
           Line2D([], [], color=BLUE, marker="s", ms=3, lw=1, label="image error")]
ax.legend(handles=handles, loc="center left", bbox_to_anchor=(0.0, 0.62),
          handlelength=1.6, borderaxespad=0.1)
fig.tight_layout()
fig.savefig(FIG / "fig_pose.pdf"); fig.savefig(FIG / "fig_pose.png"); plt.close(fig)

# ============================================================ Figure 3: sigma
sig = [0.0, 1.5, 3.0, 5.0]
conds = [f"sig3_{s}" for s in sig]

def edge_width(png):
    im = 1.0 - np.asarray(Image.open(png).convert("L")).astype(float) / 255.0
    w = []
    for wy in range(78, 100):
        row = im[im.shape[0] - 1 - (wy + 37)]
        seg = row[25:80]
        w.append(np.argmax(seg > 0.9) - np.argmax(seg > 0.1))
    return float(np.median(w))

ew = []
for s, cond in zip(sig, conds):
    pngs = sorted(glob.glob(str(ROOT / f"exp/{cond}/runs/*.png")))[:10]
    ew.append([edge_width(p) for p in pngs])
ew_m = [np.mean(v) for v in ew]; ew_s = [np.std(v, ddof=1) for v in ew]
im_m = [m(c, "error_vs_sheet_pct")[0] for c in conds]
im_s = [m(c, "error_vs_sheet_pct")[1] for c in conds]

fig = plt.figure(figsize=(3.35, 2.6))
gs = fig.add_gridspec(2, 3, height_ratios=[1.15, 1.0], hspace=0.42, wspace=0.06,
                      top=0.97, bottom=0.02, left=0.16, right=0.88)
ax = fig.add_subplot(gs[0, :])
ax2 = ax.twinx()
xx = np.linspace(-0.2, 5.3, 120)
ax.plot(xx, 2.563 * np.sqrt(xx**2 + 1.0), color="#888888", lw=0.8, ls="--", zorder=1)
ax.errorbar(sig, ew_m, yerr=ew_s, marker="o", ms=3.2, lw=1.1, color="#333333",
            capsize=2, elinewidth=0.7, zorder=3)
ax2.plot(sig, im_m, marker="s", ms=3.0, lw=1.0, color=BLUE, alpha=0.85)
ax2.set_ylabel("Error, %", color=BLUE, fontsize=7, labelpad=4)
ax2.tick_params(axis="y", colors=BLUE, labelsize=6.5)
ax2.set_ylim(12, 32)
ax.set_xlabel("Sensor footprint, $\\sigma$ in mm")
ax.set_ylabel("Edge width, mm", labelpad=3)
ax.set_xlim(-0.25, 5.3); ax.set_ylim(0, 15)
ax.axvspan(2.0, 4.0, color="#a00000", alpha=0.10, lw=0)
ax.axvline(3.0, color="#a00000", lw=0.7, ls=(0, (3, 2)))
ax.annotate("robot input, $\\sigma=3$ mm\nrange 2 to 4 mm", (3.0, 13.9), xytext=(3.2, 13.9),
            fontsize=5.6, color="#a00000", va="top", ha="left")
ax.text(0.15, 14.3, "twin: $2.56\\sqrt{\\sigma^2+1}$ mm", fontsize=6,
        color="#777777", va="top")
ax2.tick_params(axis="y", labelsize=6.5)
ax2.yaxis.set_major_locator(matplotlib.ticker.MaxNLocator(4, integer=True))
ax.legend(handles=[Line2D([], [], color="#333333", marker="o", ms=3, lw=1, label="edge width (left)"),
                   Line2D([], [], color=BLUE, marker="s", ms=3, lw=1, label="image error (right)")],
          loc="lower right", fontsize=5.9, handlelength=1.5, borderaxespad=0.2)

for i, s in enumerate([0.0, 3.0, 5.0]):
    png = sorted(glob.glob(str(ROOT / f"exp/sig3_{s}/runs/*.png")))[0]
    im = Image.open(png).convert("L")
    crop = im.crop((107, im.height - (106 + 37), 107 + 40, im.height - (70 + 37)))
    axc = fig.add_subplot(gs[1, i])
    axc.imshow(crop, cmap="gray", vmin=0, vmax=255, interpolation="nearest",
               extent=[0, 40, 0, 36])
    axc.set_title(f"$\\sigma$={s:g} mm", fontsize=6.3, pad=2)
    axc.set_xticks([]); axc.set_yticks([])
    for sp in axc.spines.values():
        sp.set_linewidth(0.45); sp.set_color("#888888")
fig.savefig(FIG / "fig_sigma.pdf"); fig.savefig(FIG / "fig_sigma.png"); plt.close(fig)

print("pitch", list(zip(pitch, np.round(e_m, 1), np.round(e_s, 1), np.round(c_m, 1), np.round(t_m, 0))))
print("sep", list(zip(np.round(x, 1), np.round(p_m, 2), np.round(p_s, 2), np.round(pm_m, 1), np.round(i_m, 1))))
print("sigma", list(zip(sig, np.round(ew_m, 1), np.round(ew_s, 1), np.round(im_m, 1), np.round(im_s, 1))))
