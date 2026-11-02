#!/usr/bin/env python3
"""Synthetic angled-camera testbed for the AprilTag / YOLO raster localiser.

No real camera or robot is needed.

- Warps the tagged map (A2 frame by default) through a random perspective.
- Pastes the real 3Pi+ top-view photo and a 36 mm AprilTag at a known pose.
- Detects the four map tags plus the robot tag and recovers the robot pose.
- Writes a YOLO detection dataset (one class: robot) and a validation record.
- Also validates the 14 mm on-card tag layout for comparison.

Outputs under tools/vision_sim/:
  dataset/                 YOLO images/labels/data.yaml
  scene_records.jsonl      homographies and ground-truth poses for val scenes
  tag_validation.json      AprilTag pose accuracy (frame and on-card)
  samples/                 annotated sample images
"""

import argparse
import json
import math
import random
from pathlib import Path

import cv2
import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
DICT = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_APRILTAG_36h11)
DETECTOR = cv2.aruco.ArucoDetector(DICT, cv2.aruco.DetectorParameters())
PX_PER_MM = 300.0 / 25.4
RENDER_PPM = 4.0
ROBOT_TAG_ID = 10
MAP_TAG_IDS = (0, 1, 2, 3)


def wrap(angle):
    return math.atan2(math.sin(angle), math.cos(angle))


def load_layout():
    with open(HERE / "tag_layout.json", encoding="utf-8") as handle:
        return json.load(handle)


def marker_rgba(marker_mm, quiet_mm, tag_id, ppm=RENDER_PPM):
    marker_px = int(round(marker_mm * ppm))
    quiet_px = int(round(quiet_mm * ppm))
    marker = cv2.aruco.generateImageMarker(DICT, tag_id, marker_px)
    total = marker_px + 2 * quiet_px
    canvas = np.full((total, total), 255, np.uint8)
    canvas[quiet_px:quiet_px + marker_px,
           quiet_px:quiet_px + marker_px] = marker
    return cv2.cvtColor(canvas, cv2.COLOR_GRAY2BGRA)


def load_robot_sprite(width_mm, ppm=RENDER_PPM):
    image = cv2.imread(str(HERE / "robot_top.png"), cv2.IMREAD_COLOR)
    h, w = image.shape[:2]
    alpha = np.zeros((h, w), np.uint8)
    # Keep the central robot; remove the white photo background at the corners.
    cv2.ellipse(alpha, (w // 2, h // 2), (int(w * 0.49), int(h * 0.49)),
                0, 0, 360, 255, -1)
    sprite = cv2.cvtColor(image, cv2.COLOR_BGR2BGRA)
    sprite[:, :, 3] = alpha
    scale = width_mm * ppm / w
    size = (int(round(w * scale)), int(round(h * scale)))
    return cv2.resize(sprite, size, interpolation=cv2.INTER_AREA)


def paste_rgba(dst, src, center, angle_deg):
    """Alpha-composite src, rotated by angle_deg about its centre."""
    h, w = src.shape[:2]
    diag = int(math.ceil(math.hypot(w, h))) + 4
    matrix = cv2.getRotationMatrix2D((w / 2.0, h / 2.0), angle_deg, 1.0)
    matrix[0, 2] += diag / 2.0 - w / 2.0
    matrix[1, 2] += diag / 2.0 - h / 2.0
    rotated = cv2.warpAffine(src, matrix, (diag, diag),
                             flags=cv2.INTER_LINEAR,
                             borderMode=cv2.BORDER_CONSTANT,
                             borderValue=(0, 0, 0, 0))
    x0 = int(round(center[0] - diag / 2.0))
    y0 = int(round(center[1] - diag / 2.0))
    x1, y1 = x0 + diag, y0 + diag
    dx0, dy0 = max(0, x0), max(0, y0)
    dx1, dy1 = min(dst.shape[1], x1), min(dst.shape[0], y1)
    if dx1 <= dx0 or dy1 <= dy0:
        return
    sx0, sy0 = dx0 - x0, dy0 - y0
    sx1, sy1 = sx0 + (dx1 - dx0), sy0 + (dy1 - dy0)
    patch = rotated[sy0:sy1, sx0:sx1]
    alpha = patch[:, :, 3:4].astype(np.float32) / 255.0
    dst[dy0:dy1, dx0:dx1] = (patch[:, :, :3] * alpha
                             + dst[dy0:dy1, dx0:dx1] * (1.0 - alpha)).astype(np.uint8)


def camera_homography(width_mm, height_mm, rng, image_w=1600, image_h=1200,
                      margin=90, jitter=18):
    src = np.float32([[0, 0], [width_mm, 0],
                      [width_mm, height_mm], [0, height_mm]])
    base = np.float32([
        [margin, margin + 20],
        [image_w - margin, margin],
        [image_w - margin + 30, image_h - margin],
        [margin - 30, image_h - margin + 20],
    ])
    dst = base + np.random.uniform(-jitter, jitter, base.shape).astype(np.float32)
    return cv2.getPerspectiveTransform(src, dst)


def tag_corners_mm(centre_mm, marker_mm):
    cx, cy = centre_mm
    half = marker_mm / 2.0
    return np.float32([
        [cx - half, cy - half], [cx + half, cy - half],
        [cx + half, cy + half], [cx - half, cy + half],
    ])


def detect_pose(image, tags, layout_key):
    """Detect map tags and the robot tag; return pose and diagnostics."""
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    corners, ids, _ = DETECTOR.detectMarkers(gray)
    found = {}
    if ids is not None:
        for tag_id, corner in zip(ids.ravel().tolist(), corners):
            found[tag_id] = corner.reshape(4, 2).astype(np.float32)

    map_points, image_points = [], []
    for tag_id in MAP_TAG_IDS:
        if tag_id not in found:
            continue
        tag = tags[str(tag_id)] if isinstance(tags, dict) else tags[tag_id]
        map_points.append(tag_corners_mm(tag["centre_mm"], tag["marker_mm"]))
        image_points.append(found[tag_id])
    map_count = len(image_points)
    if map_count < 2:
        return {"ok": False, "map_tags": map_count,
                "robot_tag": ROBOT_TAG_ID in found}

    map_points = np.concatenate(map_points, axis=0)
    image_points = np.concatenate(image_points, axis=0)
    homography, mask = cv2.findHomography(map_points, image_points,
                                          cv2.RANSAC, 3.0)
    if homography is None:
        return {"ok": False, "map_tags": map_count,
                "robot_tag": ROBOT_TAG_ID in found}

    projected = cv2.perspectiveTransform(map_points.reshape(-1, 1, 2),
                                         homography).reshape(-1, 2)
    reprojection = float(np.sqrt(np.mean(np.sum(
        (projected - image_points) ** 2, axis=1))))

    result = {"ok": True, "map_tags": map_count,
              "robot_tag": ROBOT_TAG_ID in found,
              "reprojection_px": reprojection}
    if ROBOT_TAG_ID in found:
        inverse = np.linalg.inv(homography)
        robot_corners = cv2.perspectiveTransform(
            found[ROBOT_TAG_ID].reshape(-1, 1, 2), inverse).reshape(-1, 2)
        centre = robot_corners.mean(axis=0)
        heading = math.atan2(robot_corners[1, 1] - robot_corners[0, 1],
                             robot_corners[1, 0] - robot_corners[0, 0])
        result["x"] = float(centre[0])
        result["y"] = float(centre[1])
        result["theta"] = float(wrap(heading))
    return result


class MapScene:
    def __init__(self, layout_key, width_mm, height_mm, image_path, tags):
        self.layout_key = layout_key
        self.width_mm = width_mm
        self.height_mm = height_mm
        raw = cv2.imread(str(image_path), cv2.IMREAD_COLOR)
        self.ppm = RENDER_PPM
        self.image = cv2.resize(raw,
                                (int(round(width_mm * self.ppm)),
                                 int(round(height_mm * self.ppm))),
                                interpolation=cv2.INTER_AREA)
        self.tags = tags
        self.sprite = load_robot_sprite(110.0, self.ppm)
        self.robot_tag = marker_rgba(36.0, 7.0, ROBOT_TAG_ID, self.ppm)

    def scene(self, rng, robot_pose=None, image_w=1600, image_h=1200):
        if robot_pose is None:
            x = rng.uniform(100, self.width_mm - 100)
            y = rng.uniform(90, self.height_mm - 90)
            theta = rng.uniform(-math.pi, math.pi)
        else:
            x, y, theta = robot_pose
        homography = camera_homography(self.width_mm, self.height_mm, rng,
                                       image_w, image_h)
        h_px = homography @ np.array([[1.0 / self.ppm, 0, 0],
                                      [0, 1.0 / self.ppm, 0],
                                      [0, 0, 1]], dtype=np.float64)

        scene = self.image.copy()
        centre_px = (x * self.ppm, y * self.ppm)
        paste_rgba(scene, self.sprite, centre_px, -math.degrees(theta))
        paste_rgba(scene, self.robot_tag, centre_px, -math.degrees(theta))

        mask = np.zeros_like(self.image)
        paste_rgba(mask, self.sprite, centre_px, -math.degrees(theta))

        camera = cv2.warpPerspective(scene, h_px, (image_w, image_h),
                                     flags=cv2.INTER_LINEAR,
                                     borderValue=(255, 255, 255))
        mask_cam = cv2.warpPerspective(mask[:, :, 0], h_px,
                                       (image_w, image_h),
                                       flags=cv2.INTER_NEAREST)
        ys, xs = np.where(mask_cam > 10)
        bbox = None
        if len(xs) > 30:
            bbox = [int(xs.min()), int(ys.min()),
                    int(xs.max()), int(ys.max())]

        # Realistic capture degradation.
        gain = rng.uniform(0.88, 1.12)
        camera = np.clip(camera.astype(np.float32) * gain, 0, 255).astype(np.uint8)
        sigma = rng.uniform(0.0, 1.2)
        if sigma > 0.1:
            camera = cv2.GaussianBlur(camera, (3, 3), sigma)
        noise = rng.gauss(0, 2.5)
        if abs(noise) > 0:
            camera = np.clip(camera.astype(np.float32)
                             + np.random.normal(0, 2.5, camera.shape),
                             0, 255).astype(np.uint8)
        return camera, homography, (x, y, theta), bbox


def yolo_label(bbox, width, height):
    x0, y0, x1, y1 = bbox
    x0, y0 = max(0, x0), max(0, y0)
    x1, y1 = min(width - 1, x1), min(height - 1, y1)
    bw, bh = x1 - x0, y1 - y0
    if bw < 20 or bh < 20:
        return None
    return f"0 {(x0 + x1) / 2.0 / width:.6f} {(y0 + y1) / 2.0 / height:.6f} " \
           f"{bw / width:.6f} {bh / height:.6f}"


def make_frame_scene(layout):
    tag_list = layout["frame"]["tags"]
    tags = {str(tag["id"]): tag for tag in tag_list}
    scene = MapScene("frame", 560.0, 417.0, HERE / "map_frame_A2.png", tags)
    # Keep the 110 mm robot sprite clear of the outer frame tags.
    scene.robot_area = (120.0, 100.0, 430.0, 300.0)
    return scene


def make_oncard_scene(layout):
    tag_list = layout["on_card"]["tags"]
    tags = {str(tag["id"]): tag for tag in tag_list}
    scene = MapScene("on_card", 420.0, 297.0,
                     ROOT / "Untitled_tagged.png", tags)
    scene.robot_area = (90.0, 95.0, 320.0, 200.0)
    return scene


def validate(scene, count, rng, tag):
    rows = []
    for index in range(count):
        x = rng.uniform(scene.robot_area[0], scene.robot_area[2])
        y = rng.uniform(scene.robot_area[1], scene.robot_area[3])
        theta = rng.uniform(-math.pi, math.pi)
        image, homography, gt, bbox = scene.scene(rng, (x, y, theta))
        det = detect_pose(image, scene.tags, scene.layout_key)
        row = {"scene": index, "layout": scene.layout_key,
               "gt": {"x": x, "y": y, "theta": theta},
               "det": det}
        if det.get("ok") and "x" in det:
            row["error_mm"] = math.hypot(det["x"] - x, det["y"] - y)
            row["error_deg"] = math.degrees(wrap(det["theta"] - theta))
        rows.append(row)
        if index < 4:
            preview = image.copy()
            gray = cv2.cvtColor(preview, cv2.COLOR_BGR2GRAY)
            corners, ids, _ = DETECTOR.detectMarkers(gray)
            if ids is not None:
                cv2.aruco.drawDetectedMarkers(preview, corners, ids)
            # GT pose projected through H
            c = cv2.perspectiveTransform(
                np.float32([[gt[0], gt[1]]]).reshape(-1, 1, 2), homography)[0, 0]
            cv2.circle(preview, tuple(np.int32(c)), 8, (0, 0, 255), 2)
            cv2.putText(preview, f"GT {gt[0]:.0f},{gt[1]:.0f}",
                        (int(c[0]) + 10, int(c[1]) - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
            (HERE / "samples").mkdir(exist_ok=True)
            cv2.imwrite(str(HERE / "samples" / f"{tag}_{index}.jpg"), preview)
    return rows


def summarize(rows):
    good = [r for r in rows if r.get("error_mm") is not None]
    map_ok = [r for r in rows if r["det"].get("map_tags", 0) >= 2]
    robot_ok = [r for r in rows if r["det"].get("robot_tag")]
    if not good:
        return {"scenes": len(rows), "pose_success": 0}
    errors = np.array([r["error_mm"] for r in good])
    headings = np.array([abs(r["error_deg"]) for r in good])
    return {
        "scenes": len(rows),
        "map_homography_scenes": len(map_ok),
        "robot_tag_scenes": len(robot_ok),
        "pose_success": len(good),
        "pose_success_rate": round(len(good) / len(rows), 3),
        "pose_error_mm_mean": round(float(errors.mean()), 2),
        "pose_error_mm_median": round(float(np.median(errors)), 2),
        "pose_error_mm_p95": round(float(np.percentile(errors, 95)), 2),
        "heading_error_deg_mean": round(float(headings.mean()), 2),
        "heading_error_deg_median": round(float(np.median(headings)), 2),
        "heading_error_deg_p95": round(float(np.percentile(headings, 95)), 2),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--train", type=int, default=140)
    parser.add_argument("--val", type=int, default=40)
    parser.add_argument("--oncard-val", type=int, default=40)
    parser.add_argument("--seed", type=int, default=7)
    args = parser.parse_args()
    rng = random.Random(args.seed)
    np.random.seed(args.seed)

    layout = load_layout()
    frame = make_frame_scene(layout)
    oncard = make_oncard_scene(layout)

    dataset = HERE / "dataset"
    for split, count in (("train", args.train), ("val", args.val)):
        (dataset / "images" / split).mkdir(parents=True, exist_ok=True)
        (dataset / "labels" / split).mkdir(parents=True, exist_ok=True)

    records = []
    for split, count in (("train", args.train), ("val", args.val)):
        for index in range(count):
            x = rng.uniform(frame.robot_area[0], frame.robot_area[2])
            y = rng.uniform(frame.robot_area[1], frame.robot_area[3])
            theta = rng.uniform(-math.pi, math.pi)
            image, homography, gt, bbox = frame.scene(rng, (x, y, theta))
            if bbox is None:
                continue
            label = yolo_label(bbox, image.shape[1], image.shape[0])
            if label is None:
                continue
            name = f"{split}_{index:04d}"
            cv2.imwrite(str(dataset / "images" / split / f"{name}.jpg"),
                        image, [cv2.IMWRITE_JPEG_QUALITY, 88])
            with open(dataset / "labels" / split / f"{name}.txt", "w") as handle:
                handle.write(label + "\n")
            det = detect_pose(image, frame.tags, "frame")
            record = {"split": split, "name": name,
                      "homography": homography.tolist(),
                      "gt": {"x": x, "y": y, "theta": theta},
                      "bbox": bbox, "det": det}
            if det.get("ok") and "x" in det:
                record["tag_error_mm"] = math.hypot(det["x"] - x, det["y"] - y)
                record["tag_error_deg"] = math.degrees(
                    wrap(det["theta"] - theta))
            records.append(record)

    with open(HERE / "scene_records.jsonl", "w", encoding="utf-8") as handle:
        for record in records:
            handle.write(json.dumps(record) + "\n")

    data = {
        "path": str(dataset.resolve()),
        "train": "images/train",
        "val": "images/val",
        "names": {0: "robot"},
    }
    with open(dataset / "data.yaml", "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2)

    val_rows = [r for r in records if r["split"] == "val"]
    frame_summary = summarize([
        {"det": r["det"], "error_mm": r.get("tag_error_mm"),
         "error_deg": r.get("tag_error_deg")} for r in val_rows
    ])
    oncard_rows = validate(oncard, args.oncard_val, rng, "oncard")
    validate(frame, 4, rng, "frame")
    result = {
        "dictionary": "DICT_APRILTAG_36h11",
        "frame_layout": frame_summary,
        "oncard_layout_14mm": summarize(oncard_rows),
        "dataset": {"train": args.train, "val": args.val},
        "notes": [
            "Synthetic scenes use a perspective homography, not real lens "
            "distortion; calibrate the real camera before measuring.",
            "The on-card 14 mm tags are tested at the same camera resolution; "
            "if their detection rate is poor, use the A2 frame with 36 mm tags.",
        ],
    }
    with open(HERE / "tag_validation.json", "w", encoding="utf-8") as handle:
        json.dump(result, handle, indent=2)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
