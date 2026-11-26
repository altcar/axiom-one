# Tests to run on the robot, and what each one updates

Every command below is ready to run. The robot must be on and joined to its own
WiFi. `ROBOT` defaults to `192.168.4.1`.

## 0. Before you start

```sh
cd "SEMTM0043_42_report"          # this folder
export ROBOT=192.168.4.1
curl -s http://$ROBOT:8080/status # should return JSON
```

If that fails, the laptop is not on the robot's WiFi, or the dashboard firmware is
not uploaded.

## 1. Baseline scans, the number that closes the biggest gap

Report gap: no real scan is scored, so the 5.4-point model-to-model gap cannot be
compared with the bench. Three repeats at the operating point fix that.

```sh
mkdir -p data
./tools/capture_scan.sh data/real_01.csv     # 17 rows, 160 mm, 7 mm, command 30
./tools/capture_scan.sh data/real_02.csv     # lift and replace the robot between runs
./tools/capture_scan.sh data/real_03.csv

for f in data/real_0*.csv; do
  python3 tools/score_scan.py "$f" --sheet surface.png --out "${f%.csv}.json"
done
```

Each run prints image error, coverage and edge width. Then insert the mean into the
report:

```sh
python3 tools/write_real_values.py data/real_01.json
```

Updates: the Results paragraph inside the `REAL-SCAN` markers, and with three runs
you can quote a spread instead of one number.

## 2. Knife edge at three speeds, separates footprint from delay

This is the experiment the report proposes. Print a solid black band on white
paper, drive across it at three speeds, and measure the width of the transition.

```sh
for s in 20 40 60; do
  ./tools/capture_scan.sh "data/edge_${s}.csv"     # drive manually at about $s mm/s
  python3 tools/knife_edge.py "data/edge_${s}.csv" --speed "$s" --out "data/edge_${s}.json"
done
```

Decision rule: if `width_mm` is the same at all three speeds, the footprint is
around 8 mm and the 3 mm sigma stands. If it grows with speed, delay dominates and
the twin needs a timing term.

Updates: the knife-edge sentence in Limitations, and the sigma check in the
individual report.

## 3. Step wedge, replaces the two-point response

Print a strip of eight grey patches from 0 to 100 percent. Measure each with the
robot stationary.

```sh
./tools/capture_scan.sh data/wedge.csv            # move the robot over each patch, 3 s each
python3 tools/knife_edge.py data/wedge.csv --speed 0 --out data/wedge_check.json
```

For the response itself, plot `dn1` to `dn5` against patch number. The slope gives
gain, the intercept at zero gives offset, and a flat top gives saturation. This is
the measurement the individual report says has not been done.

Updates: the channel-matching paragraph in the individual report, and Table 1 of
the group report if you add a per-channel row.

## 4. Wheel calibration, already done, repeat if the robot is dropped

```sh
# 400 mm straight run on a ruler, then a 360 degree spin
./tools/capture_scan.sh data/straight.csv
./tools/capture_scan.sh data/spin.csv
```

Set the wheel radius from the measured distance, and the separation from the spin.
This is what gives the 82.5 mm in the report.

## 5. Rebuild the PDF

```sh
tectonic -X compile main.tex      # or: pdflatex main.tex, twice
```

## What each test changes in the report

| Test | Report claim it supports | Where |
|---|---|---|
| Three baseline scans | model-to-data gap, replaces "no real scan" | Results, Limitations |
| Knife edge | sigma = 3 mm, or delay dominates | Results Fig 5, Limitations |
| Step wedge | linearity, offset and gain per channel | individual report |
| Wheel calibration | 82.5 mm separation, 9.1 percent belief error | Method, Fig 3 |

Until a test is run, the report keeps the current wording: the gap is described as
model to model, and the footprint as an estimate.
