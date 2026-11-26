# Talking points — what each section says

Use these if the TA asks "what have you written?". One or two sentences per
section, with the number that carries the section.

## Group report (Assessment 2)

**Title.** *Footprint or geometry? What limits a raster-scanned image in the digital twin.*
The title names the two candidate explanations we test against each other.

**1. Introduction.** A 3Pi+ scans a printed sheet row by row and rebuilds the
picture; the twin runs the same controller against the same sheet. Because we
also use the twin to choose scan settings, its fidelity is a practical question.
The question: at 7 mm pitch and 30 PWM, is the limit the sensor's spot, the row
spacing, or the controller's belief about wheel separation? Prediction: the spot
sets a resolution floor, pitch below it buys nothing, and geometry error costs
more than either.

**2. Implementation.** (Figure 2 is the state diagram, not pseudocode.) The
controller is a 100 Hz state machine driving
back-and-forth rows with a heading hold (60 PWM/rad, ±15 clamp, 35 mm lookahead),
so lane changes happen in the 50 mm run-in instead of as turns — turns were
abandoned because wheel scrub moved rows 5–32 mm apart when 14 mm was commanded.
Two parameter sets: *nominal* (template ideal robot) and *calibrated* (our
measured 82.5 mm separation, 58 mm/s at PWM 30, measured sensor offsets, 3 mm
spot). The controller's wheel *belief* is separate from the physics — that is how
real odometry error is reproduced rather than assumed.

**3. Method.** Five independent variables (pitch, separation belief, spot size,
path pattern, rotation efficiency), ten repeats each, everything else fixed.
Metrics: image error over covered pixels, coverage, mean/max pose error, 10–90 %
edge width, row spacing. The robot side is characterisation values (82.5 mm
separation, 58 mm/s, 5–32 mm row spacing), not scan trials.

**4. Results.** Figure 3 is the sim-vs-real comparison. Left, model sensitivity
with our measured 7.5 mm marked. Right, the same 14 mm command collapsing to a
line in the twin but spanning 5 to 32 mm on the robot. Figure 4 is the one that
changes the answer. On covered pixels pitch is flat. On the whole target it costs
19 points, because a coarse path removes a quarter of the image. Figure 5 shows
the spot: 8 mm edges at our 3 mm footprint. The last paragraph says the U-turn
runs break the metric, and why.
- **Pitch does not limit fidelity**: error is flat at 20.4–22.2 % from 5 to 20 mm
  while coverage falls 99.8 → 75.6 % and time 117 → 34 s. Finer rows add samples,
  not fidelity; and the metric can't see the gaps, so error and coverage must be
  read together.
- **Geometry does**: 0 → 9.1 % separation-belief error gives 1.6 → 5.1 mm mean
  pose error, 21.0 → 25.2 % image error, and moves the square's centre 6.1 mm.
  Our measured 7.5 mm sits at the top of that curve — bigger than the 7 mm row
  pitch, so samples land on the wrong row.
- **Spot size explains the soft triangle**: edge width = 1.84σ + 2.3 mm, image
  error 14.8 → 25.0 % as σ goes 0 → 5 mm.
- U-turn spacing only reconciles with our 5–32 mm at rotation efficiency
  0.95–1.0; the twin cannot produce gaps *below* 14 mm, so something else is also
  happening (said plainly).

**5. Discussion.** The ranking depends on the metric, and only the whole-target
one matches the task: pitch 18.5 points, belief 6.1, footprint 5.8. On covered
pixels the order reverses and pitch is zero. Alternative explanation addressed: the
flat pitch curve could be metric saturation rather than physics — we say so and
say what test would separate them. Practical trade-off: 7 mm gives 99.8 percent coverage in 86 s, 10 mm gives 92.5
percent in 60 s with the same error, so 7 mm is chosen only because the task needs
the whole sheet. Limitation stated first: no real scan is scored, so the 5.4-point
gap is model to model. Next experiment: knife
edge at three speeds, which separates footprint from delay. Conclusion: an
uncalibrated twin overstates a real scan by about 11 points.

**Page 4.** Evidence note, the file that holds all 300 runs, and eight references.

## Individual report (Assessment 1)

**Aim.** Characterise the reflectance array's spatial footprint (and channel
matching) because it is the one property the twin cannot infer from the
controller.

**Method.** Geometry from an underside photograph (±2 mm); response measured on
white paper and black ink (400 and 2000 counts); footprint fitted from the deblur
stage of our own rebuild (σ ≈ 3 mm, bounded 2–4 mm by halo onset); edge width
taken as 2.56σ.

**Results.** Array spans 54 mm laterally with a 13 mm fore-aft stagger — treat the
five channels as co-located and every feature shears by 13 mm, more than the
7 mm pitch. At σ = 3 mm the 10–90 % edge is ≈ 7.7 mm, about twice the row pitch
and twenty times the 0.28 mm encoder resolution: that is why a printed triangle
comes back as a mound. Tonally, 400–2000 counts is near-linear, so the 50 %
threshold is 1200 counts = grey 128; ink lighter than about grey 100 vanishes.
Stripes in our rebuilds with a shared range are direct evidence of per-channel
offset and gain differences the model assumes away.

**Twin implication and check.** Model each channel as a Gaussian spot at its
measured offset with its own levels; otherwise the twin rewards a pitch the
hardware cannot use. Proposed check: knife edge at three speeds — constant width
confirms σ, growing width means delay dominates.
