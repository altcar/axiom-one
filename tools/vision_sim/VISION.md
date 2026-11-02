# Camera localisation for rastering

**Goal:** replace the drifting IMU/odometry raster pose with an absolute camera
pose, without relying on the five surface sensors for navigation. The surface
sensors remain the raster measurement and the independent validation.

## Recommendation

Use **AprilTag 36h11** (`cv2.aruco.DICT_APRILTAG_36h11`).

- 36h11 has strong error detection and a low false-positive rate, and OpenCV's
  ArUco detector supports it directly.
- Four tags define the map plane; one tag on the robot gives its position and
  heading.
- The camera does **not** need to be top-down. A perspective view is fine once
  the lens is calibrated and the plane is visible; `solvePnP` handles the tilt.
- Keep the robot tag low and near the robot centre, or compensate for its
  height. A tag 30 mm above the sheet viewed at 30° is displaced by about
  17 mm if treated as a z=0 point.

## Tag allocation

### Recommended: A2 frame outside the A3 sheet

`map_frame_A2.png` — print at **A2 landscape (594 × 420 mm), 100%**.

- Frame: 560 × 417 mm, white.
- A3 test card: full size at (70, 60) mm, 1:1.
- Four **36 mm** markers (50 mm including a 7 mm quiet zone), IDs **0–3**,
  centres:

| ID | x (mm) | y (mm) | Position |
|---|---|---|---|
| 0 | 30 | 30 | top-left |
| 1 | 530 | 30 | top-right |
| 2 | 530 | 387 | bottom-right |
| 3 | 30 | 387 | bottom-left |

- The tags are outside the A3 sheet: they do not disturb the printed surface,
  the black border, or the reflectance reconstruction.
- Larger tags are easier to detect at a distance and at an angle.

### Requested on-card version

`Untitled_tagged.png` — print at **A3, 100%**.

- Four **14 mm** markers (21 mm including a 3.5 mm quiet zone), IDs 0–3, in the
  black border corners, centres:

| ID | x (mm) | y (mm) | Position |
|---|---|---|---|
| 0 | 31.07 | 25.74 | top-left |
| 1 | 388.63 | 25.74 | top-right |
| 2 | 388.63 | 271.26 | bottom-right |
| 3 | 31.07 | 271.26 | bottom-left |

- **Caveat:** the white quiet-zone squares interrupt the black border at the
  corners. Use this only if the camera raster replaces border-following. If
  the robot still follows the printed border, use the A2 frame instead.
- The 14 mm tags work in the synthetic clean view, but are more sensitive to
  camera resolution, blur and print scaling than the 36 mm frame tags.

### Robot tag

`robot_tag_10_50mm.png` — 36 mm marker + 7 mm quiet zone, ID **10**.

- Print at 300 dpi, cut on the 50 mm quiet-zone boundary.
- Mount flat, **tag +x (top edge) pointing forward**.
- If the tag is not at the robot's rotation centre, measure the offset
  (dx, dy) in the robot frame and add it after rotating by the tag heading.
- Measure the tag height above the sheet; use it in the 3D pose step below.

The JSON file `tag_layout.json` contains the same geometry in mm and pixels.

## Camera pose pipeline

1. **Intrinsics:** print a checkerboard (9 × 6 inner corners, 25 mm squares),
   run `python tools/vision_sim/calibrate_camera.py`, capture 15–25 views at
   the camera angle, and save `camera_intrinsics.json` (K, dist). A phone
   camera works; a fixed mount is strongly preferred.
2. **Map plane:** detect tags 0–3. Either:
   - build a homography from their known marker corners (z=0 plane), or
   - solve the camera pose with `cv2.solvePnP` using all four tags at z=0.
3. **Robot tag:** detect tag 10 with `solvePnP` at its measured height `h`.
   - The tag centre in the map frame is `(x, y, h)`.
   - The robot's rotation centre on the sheet is `(x, y, 0)` after subtracting
     `h` along the map normal, provided the tag is directly above the centre.
   - Heading is the tag's +x direction in the map plane.
4. **Fusion:** camera pose at 20–30 Hz gives absolute x, y, θ. Encoders give
   high-rate distance between frames. Use the camera as the correction and
   the encoders as the prediction. The IMU can be a short-term rate source
   with its bias removed, or ignored. Do not integrate the IMU for minutes.
5. **Latency:** timestamp frames. At 150 mm/s, 100 ms is 15 mm. Use encoder
   prediction or slow the physical raster to 60–80 mm/s. A 30 fps camera and a
   20 Hz control loop are enough if timestamps are handled.

## Validation results (synthetic, no hardware)

`vision_sim.py` renders the map through a random perspective, pastes the real
3Pi+ photo and the 36 mm robot tag at a known pose, adds blur/noise/brightness
variation, then detects the tags and compares the recovered pose with truth.
1600 × 1200 camera, 40 validation scenes per layout.

| Layout | Map tags | Robot tag | Pose success | Pose error mean | Pose p95 | Heading mean |
|---|---|---|---|---|---|---|
| A2 frame, 36 mm | 40/40 | 40/40 | 40/40 | **0.24 mm** | 0.45 mm | **0.26°** |
| On-card, 14 mm | 40/40 | 40/40 | 40/40 | 0.27 mm | 0.45 mm | 0.23° |

These numbers validate the **geometry and detection pipeline**, not real
accuracy: the synthetic camera has no lens distortion, no print scaling error,
no motion blur, and the tag is on the map plane. Expect millimetre-to-centimetre
errors in reality until the intrinsic calibration, tag height and lighting are
handled.

The sample images in `samples/` show the detected tags, the ground-truth robot
centre and the detected centre.

## YOLO simulated environment

A YOLO detection dataset is generated from the same scenes:

- 140 training images, 40 validation images, 1600 × 1200;
- one class, `robot`; labels from the transformed robot-sprite mask;
- the robot sprite is the real 3Pi+ photo from the course hardware page,
  with the 36 mm tag rendered on top;
- random perspective, scale, rotation, blur, brightness and noise.

Training command (GPU strongly recommended):

```text
python tools/vision_sim/train_yolo.py --epochs 100 --imgsz 640 --batch 16 \
    --freeze 0 --fraction 1.0 --save --project D:\yolo_runs
```

Large checkpoint writes stalled in testing, including in a synced folder. Use
a local, non-synced drive and verify that `last.pt` grows. The smoke run uses
`save=False` by default; `--save` enables checkpoints for a real run.

### Quick CPU result (smoke test only)

The pipeline was exercised on 42 training / 40 validation images at 192 px
with the first 10 backbone layers frozen. One epoch completed; the remaining
epochs were stopped because the CPU was contended by other work:

| Metric | Value |
| --- | --- |
| Precision | 0.302 |
| Recall | 0.55 |
| mAP50 | 0.302 |
| mAP50-95 | 0.117 |
| Weights saved | none (checkpoint write stalled) |

This is **not a usable detector**. It shows the dataset/loader/training
pipeline runs, but the model needs a GPU, the full dataset, an unfrozen
backbone and many more epochs. The synthetic evaluation is also easier than
real frames. For the raster, use the AprilTag pose; keep YOLO as a fallback
and reacquisition detector. If YOLO must provide heading, train an
oriented-bounding-box model and label the box corners.

Full details are in `yolo_metrics.json`.

**Important:** YOLO gives a bounding box, not an accurate heading, and its
centre error must still be converted through the map homography. For control,
use the tag for pose and YOLO only as a reacquisition/fallback detector. If
you later want YOLO to supply heading, train an oriented-bounding-box model
(`yolo11n-obb` / `yolov8n-obb`) with the box corners as labels.

## Real-camera procedure

1. Print the recommended A2 frame and the robot tag; check a 100 mm ruler on
   the print (no "fit to page").
2. Mount the camera so the whole frame is visible; it can be angled. Avoid
   glare on the tags.
3. Calibrate intrinsics; save K and distortion.
4. Place the robot at 9–16 known points. Record tag pose vs tape. Target a few
   mm and ~1–2° before attempting the raster.
5. Drive a straight 500 mm and a 90° in-place turn. Log camera pose, encoder
   pose and gyro. Measure drift, overshoot and latency; fit the IMU bias/scale
   from this data if you want an IMU prior.
6. Only then close the raster loop with the camera pose. Build the lane plan
   from the tag-defined rectangle, not from the drifting robot frame.
7. Use the surface sensors to validate the lane positions and the
   reconstruction, not to steer.

## Integration options

- **HIL on the PC (recommended first):** camera node sends `x,y,theta` over
  TCP/UDP to the Processing HIL controller; the controller uses it as the
  external pose and sends PWM to the robot. This is the quickest way to test
  the camera feedback without touching the auto-raster firmware.
- **On the robot:** add a `CAM,x,y,theta` command to the WiFi/serial protocol
  and use it to reset/correct the raster pose at lane starts and corners. The
  camera can run at 20–30 Hz; the robot only needs the correction at those
  instants.
- Either way, keep a timestamp with every pose and reject stale frames.

## Files

| File | Purpose |
|---|---|
| `Untitled_tagged.png` | A3 test card with 14 mm corner tags |
| `map_frame_A2.png` | Recommended A2 frame with 36 mm map tags |
| `robot_tag_10_50mm.png` | Robot tag, ID 10 |
| `tag_layout.json` | Tag IDs, sizes, centres (mm and px) |
| `make_tagged_map.py` | Regenerates the layouts |
| `calibrate_camera.py` | Chessboard intrinsic calibration for the real camera |
| `vision_sim.py` | Synthetic angled-camera + tag validation + YOLO dataset |
| `train_yolo.py` | YOLO training/evaluation |
| `samples/` | Annotated synthetic scenes |
| `dataset/` | YOLO images, labels and `data.yaml` |
| `tag_validation.json` | Synthetic tag metrics |
| `yolo_metrics.json` | YOLO metrics (after training) |
