#!/usr/bin/env python3
"""Emit the supplementary condition table from all_runs.csv (two side-by-side halves)."""
import csv, pathlib
import numpy as np
from collections import defaultdict

rows = list(csv.DictReader(open("/tmp/opencode/dt_build/all_runs.csv")))
G = defaultdict(list)
for r in rows:
    G[r["condition"]].append(r)

def s(c, k):
    v = np.array([float(r[k]) for r in G[c]])
    return v.mean(), v.std(ddof=1)

spec = [
    ("Nominal, 7 mm", "model_nominal"),
    ("Calibrated, 7 mm", "model_calibrated"),
    ("Pitch 5 mm", "pitch_5"),
    ("Pitch 7 mm", "pitch_7"),
    ("Pitch 10 mm", "pitch_10"),
    ("Pitch 13.5 mm", "pitch_13.5"),
    ("Pitch 20 mm", "pitch_20"),
    ("Separation err. 0\\,\\%", "sep_82.5"),
    ("Separation err. 1.8\\,\\%", "sep_84.0"),
    ("Separation err. 3.6\\,\\%", "sep_85.5"),
    ("Separation err. 5.5\\,\\%", "sep_87.0"),
    ("Separation err. 7.3\\,\\%", "sep_88.5"),
    ("Separation err. 9.1\\,\\%", "sep_90.0"),
    (r"$\sigma=0$ mm", "sig3_0.0"),
    (r"$\sigma=1.5$ mm", "sig3_1.5"),
    (r"$\sigma=3$ mm", "sig3_3.0"),
    (r"$\sigma=5$ mm", "sig3_5.0"),
    ("U-turn, eff.\\ 1.00", "uturn_eff_1.0"),
    ("U-turn, eff.\\ 0.98", "uturn_eff_0.98"),
    ("U-turn, eff.\\ 0.96", "uturn_eff_0.96"),
    ("U-turn, eff.\\ 0.95", "uturn_eff_0.95"),
    ("U-turn, eff.\\ 0.94", "uturn_eff_0.94"),
]

body = []
for label, cond in spec:
    e, es = s(cond, "error_vs_sheet_pct")
    c, _ = s(cond, "coverage_pct")
    p, _ = s(cond, "pose_err_mean_mm")
    t, _ = s(cond, "duration_s")
    n = len(G[cond])
    body.append(f"{label} & {e:.1f}$\\pm${es:.1f} & {c:.1f} & {p:.1f} & {t:.0f} \\\\")

head = (r"\toprule" "\n"
        r"Condition & Err. & Cov. & Pose & $t$ \\" "\n"
        r"\midrule" "\n")

tex = r"""\begin{minipage}{\linewidth}
\captionof{table}{Every twin condition in the run set (\texttt{all\_runs.csv}),
$n=10$ runs per row. Columns are mean $\pm$1\,s.d.\ image error (\%), coverage
(\%), mean pose error (mm) and scan time (s). The first group uses 7\,mm pitch;
the footprint rows use 3\,mm so the path does not limit the edge.}
\label{tab:supp}
\small
\renewcommand{\arraystretch}{1.05}
\setlength{\tabcolsep}{4pt}
\begin{tabular}{@{}p{40mm}rrrr@{}}
""" + head + "\n".join(body) + "\n" + r"\bottomrule" "\n" + r"\end{tabular}" + "\n" + r"\end{minipage}" + "\n"
pathlib.Path("/tmp/opencode/report/supp_table.tex").write_text(tex)
print("ok", len(body))
