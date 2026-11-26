#!/usr/bin/env python3
"""State diagram with one action per box."""
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mp
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Polygon

plt.rcParams.update({"font.family": "sans-serif",
                     "font.sans-serif": ["DejaVu Sans"],
                     "savefig.dpi": 400, "savefig.pad_inches": 0.02})

BLUE = "#1f4e79"
GREY = "#4d4d4d"
FILL = "#eef2f7"

fig, ax = plt.subplots(figsize=(3.35, 3.05))
fig.subplots_adjust(0, 0, 1, 1)
ax.set_xlim(0, 100); ax.set_ylim(14, 100); ax.axis("off")

def box(x, y, w, h, text, fc=FILL, ec=BLUE, fs=5.2, dashed=False):
    ax.add_patch(FancyBboxPatch((x - w/2, y - h/2), w, h,
                                boxstyle="round,pad=0,rounding_size=1.2",
                                fc=fc, ec=ec, lw=0.7, zorder=2,
                                linestyle=(0, (2, 1.4)) if dashed else "-"))
    ax.text(x, y, text, ha="center", va="center", fontsize=fs, zorder=3,
            linespacing=1.3)

def diamond(x, y, w, h, text, fs=5.1):
    ax.add_patch(Polygon([[x, y + h/2], [x + w/2, y], [x, y - h/2], [x - w/2, y]],
                         closed=True, fc="#fdf6e8", ec="#8a5a00", lw=0.7, zorder=2))
    ax.text(x, y, text, ha="center", va="center", fontsize=fs, zorder=3)

def arrow(p1, p2, rad=0.0, color=GREY, ls="-"):
    ax.add_patch(FancyArrowPatch(p1, p2, arrowstyle="-|>", mutation_scale=4.4,
                                 connectionstyle=f"arc3,rad={rad}", lw=0.6,
                                 color=color, linestyle=ls, zorder=1,
                                 shrinkA=0, shrinkB=0))

def line(p1, p2, color=GREY):
    ax.add_patch(FancyArrowPatch(p1, p2, arrowstyle="-", lw=0.6, color=color,
                                 zorder=1, shrinkA=0, shrinkB=0))

def lab(x, y, t, fs=4.8):
    ax.text(x, y, t, fontsize=fs, color=GREY, ha="center", va="center", zorder=4,
            bbox=dict(fc="white", ec="none", pad=0.4))

X = 44
ax.add_patch(mp.Circle((X, 97), 2.0, fc=BLUE, ec="none", zorder=2))
ax.text(X + 4, 97, "button pressed", fontsize=4.8, va="center", color=GREY)

box(X, 90, 34, 5.6, "zero the odometry")
box(X, 82, 34, 5.6, "settle 0.3 s")
box(X, 74, 34, 5.6, "ramp the PWM")
box(X, 63, 42, 9.0, "drive along the row\nand keep to the line")
diamond(X, 49, 28, 8.4, "end of row?")
diamond(X, 35, 28, 8.4, "last row?")
box(X, 21, 26, 5.6, "stop")

box(86, 63, 24, 8.4, "record five\nchannels at 50 Hz", fs=5.0, dashed=True)
arrow((65, 63), (73.5, 63), color="#7a7a7a", ls=(0, (2, 1.4)))

for y0, y1 in [(94.8, 92.8), (87.2, 84.8), (79.2, 76.8), (71.2, 67.5)]:
    arrow((X, y0), (X, y1))
arrow((X, 58.5), (X, 53.2))
lab(X + 3.0, 44.2, "yes")
arrow((X, 44.8), (X, 39.2))
lab(X + 3.0, 30.2, "yes")
arrow((X, 30.8), (X, 23.8))

# stay on this row
arrow((X + 14, 49), (X + 21, 63), rad=-0.4)
lab(X + 23, 55, "no")

# next row
box(12, 35, 22, 9.0, "reverse, then\nsteer to the next row", fs=5.0)
arrow((30, 35), (23, 35))
lab(26.5, 36.8, "no")
line((12, 39.5), (12, 63))
arrow((12, 63), (23, 63))

fig.savefig("/tmp/opencode/report/figures/fig_flow.pdf")
fig.savefig("/tmp/opencode/report/figures/fig_flow.png")
print("ok")
