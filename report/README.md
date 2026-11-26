# SEMTM0043/42 — Assessment 2 submission package

Three-page scientific report plus the data and code needed to reproduce it.

| Path | What it is |
|---|---|
| `group/` | Assessment 2: `main.tex`, `main.pdf`, `report.md`, `figures/` and an Overleaf zip. |
| `individual/` | Assessment 1: 1-page `main.tex`, `main.pdf`, figure and an Overleaf zip. |
| `real_robot/` | Uploadable firmware and host scripts that measure the actual robot. See its own README. |
| `data/all_runs.csv` | 300 twin runs, one row each. |
| `data/twin_data.zip` | Every per-run CSV and rebuild PNG. |
| `pipeline/` | The twin batch driver and the figure scripts: `merge.py`, `run_experiments.py`, `analyse.py`, `make_flow.py`, `make_simreal.py`. |
| `TALKING_POINTS.md` | One paragraph per section, for a viva. |

## Where the numbers come from

**Simulation (Figures 3–5, Table 2 twin columns, Table 3).** Produced by running the
submitted `DigitalTwin/` sketch itself, headless, with the Processing engine
(4.5.7) and a small batch driver. The controller, `Robot_c`, `MotorsModel`,
`OdometryModel` and `SurfaceSensorModel` are untouched; the driver only sets
`ModelParameters` / `RasterConfig` between runs and starts them. Nothing about
the model was changed to produce a nicer result.

To repeat:

```sh
# one-off: compile the sketch + driver into out/classes (needs JDK 21 + processing core.jar)
python3 analysis/merge.py DigitalTwin out/          # then javac
python3 analysis/run_experiments.py                # 290 runs, ~70 s
python3 analysis/analyse.py                        # figures/fig_*.pdf
```

**Robot.** These are measurements the team made during Phase 1 and while
developing the scan, recorded in `DigitalTwin/ModelParameters.pde` and the repo
README:

- wheel separation 82.5 mm (template 90 mm) — U-turn calibration;
- 58 mm·s⁻¹ at PWM 30 — from scan logs;
- sensor positions, −2 mm/−27 mm to +4 mm/+27 mm from the axle centre — underside photo, ±2 mm;
- sensor footprint σ ≈ 3 mm — estimate from deblurring our scans;
- row spacing 5–32 mm when 14 mm was commanded — U-turn runs, measured with a rule;
- per-sensor offset/gain mismatch — visible as stripes when all five channels share one brightness range.

The image-error and pose-error columns for the robot are therefore blank: those
need a scored real scan, and no re-scan was possible during the analysis. If you
add `data/real_scan_*.csv` (dashboard **Download CSV**) the report can carry a
real image-error number in Table 2 and a fourth point in Figure 4.

## What the report argues

1. Row pitch does not limit fidelity between 5 and 20 mm: error is flat
   (20.4–22.2 %) while coverage falls 99.8 → 75.6 % and time 117 → 34 s.
2. Sensor footprint does: edge width = 1.87σ + 2.2 mm, image error 14.8 → 25.0 %.
3. The controller's wheel-separation belief does: 0 → 9.1 % belief error gives
   1.6 → 5.1 mm pose error and 21.0 → 25.2 % image error; our measured 7.5 mm
   sits at the top of that curve.
4. U-turn row spacing only reconciles with the measured 5–32 mm at rotation
   efficiency 0.95–1.0, and the sub-14 mm cases need a mechanism the twin lacks.

## Regenerating the PDF

```sh
tectonic -X compile main.tex       # or: pdflatex main.tex (twice)
```
