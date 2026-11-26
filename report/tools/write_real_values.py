#!/usr/bin/env python3
"""Insert a measured real-scan result into main.tex between two markers."""
import json, pathlib, sys, re

BEGIN, END = "% REAL-SCAN-BEGIN", "% REAL-SCAN-END"
tex = pathlib.Path("main.tex")
m = json.loads(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "metrics.json").read_text())

para = (
    f"One scan of the robot at the same settings, scored with the same rebuild, "
    f"gives $E={m['image_error_pct']:.1f}$ percent over covered pixels and "
    f"{m['image_error_full_target_pct']:.1f} percent over the whole target, with "
    f"coverage of {m['coverage_pct']:.1f} percent. The gap between the calibrated "
    f"twin and the robot is therefore "
    f"{m['image_error_pct'] - 20.6:+.1f} points on covered pixels.\n"
)
s = tex.read_text()
if BEGIN not in s:
    sys.exit("markers not found in main.tex")
s = re.sub(re.escape(BEGIN) + r".*?" + re.escape(END),
           BEGIN + "\n" + para + END, s, flags=re.S)
tex.write_text(s)
print("inserted:", para.strip())
