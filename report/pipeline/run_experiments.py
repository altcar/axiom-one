#!/usr/bin/env python3
"""Run the ImageRaster Digital Twin headlessly for a matrix of conditions.

Every result here is produced by the twin's own physics and controller; nothing
is synthesised. Each condition gets its own folder so runs/summary.csv and the
reconstruction PNGs stay separate.
"""
import json, os, shutil, subprocess, sys, pathlib, csv, concurrent.futures as cf

ROOT = pathlib.Path("/tmp/opencode/dt_build")
JAVA = "/tmp/opencode/env/jdk-21.0.13.1+2/bin/java"
P = "/tmp/opencode/env/Processing/lib/app/resources"
CP = ":".join([
    str(ROOT / "out/classes"),
    f"{P}/core/library/core-4.5.7.jar",
    f"{P}/modes/java/libraries/net/library/net.jar",
    f"{P}/modes/java/libraries/pdf/library/pdf.jar",
    f"{P}/modes/java/libraries/serial/library/serial.jar",
])
ASSETS = ["surface.png", "surface.json", "sheets"]

def prepare(tag):
    d = ROOT / "exp" / tag
    if d.exists():
        shutil.rmtree(d)
    (d / "runs").mkdir(parents=True)
    for a in ASSETS:
        src = ROOT / "src" / a
        if src.is_dir():
            shutil.copytree(src, d / a)
        else:
            shutil.copy2(src, d / a)
    return d

def run(cond):
    tag, props = cond
    d = prepare(tag)
    args = [JAVA, "-cp", CP,
            f"-Dbatch.tag={tag}",
            f"-Dbatch.out={d}",
            f"-Dbatch.runs={props.pop('runs', 1)}"]
    for k, v in props.items():
        args.append(f"-Dbatch.{k}={v}")
    args.append("DigitalTwin")
    r = subprocess.run(["xvfb-run", "-a"] + args, cwd=d, capture_output=True, text=True, timeout=900)
    rows = []
    s = d / "runs" / "summary.csv"
    if s.exists():
        with open(s) as fh:
            rows = list(csv.DictReader(l for l in fh if not l.startswith("#")))
    for row in rows:
        row["condition"] = tag
    return tag, rows, r.stdout[-400:], r.stderr[-400:]

CONDS = []
def add(tag, **p):
    CONDS.append((tag, dict(p)))

# 1. Nominal vs calibrated twin
add("model_nominal", model="nominal", runs=10, beliefSep=90.0, sdxy=1.0, sdth=0.5)
add("model_calibrated", model="calibrated", runs=10, beliefSep=82.5)

# 2. Row pitch, same ~112 mm tall area (rows adjusted)
for pitch, rows in [(5, 23), (7, 17), (10, 12), (13.5, 9), (20, 7)]:
    add(f"pitch_{pitch}", model="calibrated", runs=10, pitch=pitch, rows=rows)

# 3. Controller belief about wheel separation vs the true 82.5 mm
for belief in [82.5, 84.0, 85.5, 87.0, 88.5, 90.0]:
    add(f"sep_{belief}", model="calibrated", runs=10, beliefSep=belief)

# 4. Sensor footprint sigma
for sigma in [0.0, 1.5, 3.0, 5.0]:
    add(f"sigma_{sigma}", model="calibrated", runs=10, sigma=sigma)

# 3b. Interaction cell: no footprint, wrong wheel belief
add("interact_s0_b90", model="calibrated", runs=10, beliefSep=90.0, sigma=0.0)

# 4b. Sensor footprint with a path fine enough not to limit the edge itself
for sigma in [0.0, 1.5, 3.0, 5.0]:
    add(f"sig3_{sigma}", model="calibrated", runs=10, pitch=3, rows=38, sigma=sigma)

# 5. U-turn path with modelled wheel slip (rotation efficiency)
for eff in [1.0, 0.98, 0.97, 0.96, 0.95, 0.94, 0.9, 0.8]:
    add(f"uturn_eff_{eff}", model="calibrated", runs=10, pattern=1, pitch=14, rows=9,
        rowlen=120, efficiency=eff)

if __name__ == "__main__":
    only = sys.argv[1] if len(sys.argv) > 1 else None
    todo = [c for c in CONDS if only is None or only in c[0]]
    allrows = []
    with cf.ThreadPoolExecutor(max_workers=4) as ex:
        for tag, rows, out, err in ex.map(run, todo):
            print(f"{tag:24s} runs={len(rows):2d} " + (rows[-1]["error_vs_sheet_pct"] if rows else "FAILED"))
            if not rows:
                print(out, err)
            allrows += rows
    out = ROOT / "all_runs.csv"
    master = []
    if only is not None and out.exists():
        master = list(csv.DictReader(open(out)))
        done = {r["condition"] for r in allrows}
        master = [r for r in master if r["condition"] not in done]
    master += allrows
    with open(out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(master[0].keys()))
        w.writeheader(); w.writerows(master)
    print("wrote", out, len(master), "rows")
