#!/usr/bin/env python3
"""Train and evaluate a small YOLO detector on the synthetic raster scenes.

The dataset was produced by vision_sim.py. This script trains YOLOv8n on CPU,
runs the validation split, and measures:
  - detection mAP / precision / recall;
  - bounding-box centre error in pixels and millimetres;
  - inference latency.

The synthetic set validates the pipeline and geometry. Real-camera accuracy
still requires labelled frames from the actual camera and lighting.
"""

import argparse
import json
import math
import os
import time
from pathlib import Path

import cv2
import numpy as np
import torch
from ultralytics import YOLO

torch.set_num_threads(max(1, os.cpu_count() or 1))

HERE = Path(__file__).resolve().parent
DATASET = HERE / "dataset"
RUNS = HERE / "runs"


def load_records():
    records = {}
    path = HERE / "scene_records.jsonl"
    if not path.exists():
        return records
    with open(path, encoding="utf-8") as handle:
        for line in handle:
            record = json.loads(line)
            if record.get("split") == "val":
                records[record["name"]] = record
    return records


def inverse_center(record, u, v):
    homography = np.array(record["homography"], dtype=np.float64)
    inverse = np.linalg.inv(homography)
    point = cv2.perspectiveTransform(
        np.float32([[[u, v]]]), inverse)[0, 0]
    return float(point[0]), float(point[1])


def read_gt_center(name):
    path = DATASET / "labels" / "val" / f"{name}.txt"
    if not path.exists():
        return None
    fields = path.read_text().split()
    if len(fields) < 5:
        return None
    cx, cy = float(fields[1]), float(fields[2])
    return cx, cy


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--epochs", type=int, default=3)
    parser.add_argument("--imgsz", type=int, default=192)
    parser.add_argument("--batch", type=int, default=8)
    parser.add_argument("--conf", type=float, default=0.25)
    parser.add_argument("--fraction", type=float, default=0.3,
                        help="fraction of the dataset used for this quick run")
    parser.add_argument("--freeze", type=int, default=10,
                        help="freeze the first N backbone layers")
    parser.add_argument("--project", default=str(RUNS),
                        help="run output directory (use a non-OneDrive path)")
    parser.add_argument("--save", action="store_true",
                        help="save checkpoints (large; use a local non-synced drive)")
    args = parser.parse_args()

    data_yaml = DATASET / "data.yaml"
    if not data_yaml.exists():
        raise SystemExit("dataset/data.yaml missing; run vision_sim.py first")
    run_dir = Path(args.project)
    run_dir.mkdir(parents=True, exist_ok=True)
    model = YOLO("yolov8n.pt")
    train = model.train(
        data=str(data_yaml), epochs=args.epochs, imgsz=args.imgsz,
        batch=args.batch, device="cpu", workers=0,
        project=str(run_dir), name="robot_sim", exist_ok=True,
        verbose=False, plots=False, seed=7,
        fraction=args.fraction, freeze=args.freeze, save=args.save,
    )
    metrics = model.val(
        data=str(data_yaml), split="val", imgsz=args.imgsz,
        device="cpu", workers=0, verbose=False, plots=False,
    )
    summary = {
        "epochs": args.epochs,
        "run_dir": str(run_dir),
        "imgsz": args.imgsz,
        "map50": float(metrics.box.map50),
        "map50_95": float(metrics.box.map),
        "precision": float(metrics.box.mp),
        "recall": float(metrics.box.mr),
    }

    records = load_records()
    width = height = None
    centre_errors_px = []
    centre_errors_mm = []
    detections = 0
    latencies = []
    montage = []
    for name, record in sorted(records.items()):
        image_path = DATASET / "images" / "val" / f"{name}.jpg"
        image = cv2.imread(str(image_path))
        if image is None:
            continue
        height, width = image.shape[:2]
        start = time.perf_counter()
        result = model.predict(image, imgsz=args.imgsz, conf=args.conf,
                               device="cpu", verbose=False)[0]
        latencies.append((time.perf_counter() - start) * 1000.0)
        gt = read_gt_center(name)
        if result.boxes is None or len(result.boxes) == 0 or gt is None:
            if len(montage) < 6:
                montage.append((image.copy(), None, gt))
            continue
        boxes = result.boxes.xyxy.cpu().numpy()
        confidences = result.boxes.conf.cpu().numpy()
        index = int(np.argmax(confidences))
        x0, y0, x1, y1 = boxes[index]
        predicted = ((x0 + x1) / 2.0, (y0 + y1) / 2.0)
        gt_px = (gt[0] * width, gt[1] * height)
        error_px = math.hypot(predicted[0] - gt_px[0],
                              predicted[1] - gt_px[1])
        centre_errors_px.append(error_px)
        map_pred = inverse_center(record, *predicted)
        map_gt = (record["gt"]["x"], record["gt"]["y"])
        centre_errors_mm.append(math.hypot(map_pred[0] - map_gt[0],
                                           map_pred[1] - map_gt[1]))
        detections += 1
        if len(montage) < 6:
            montage.append((image.copy(), (x0, y0, x1, y1), gt_px))

    if centre_errors_px:
        px = np.array(centre_errors_px)
        mm = np.array(centre_errors_mm)
        summary.update({
            "detections": detections,
            "val_scenes": len(records),
            "detection_rate": round(detections / max(1, len(records)), 3),
            "centre_error_px_mean": round(float(px.mean()), 2),
            "centre_error_px_median": round(float(np.median(px)), 2),
            "centre_error_px_p95": round(float(np.percentile(px, 95)), 2),
            "centre_error_mm_mean": round(float(mm.mean()), 2),
            "centre_error_mm_median": round(float(np.median(mm)), 2),
            "centre_error_mm_p95": round(float(np.percentile(mm, 95)), 2),
        })
    if latencies:
        summary["latency_ms_mean"] = round(float(np.mean(latencies)), 1)

    with open(HERE / "yolo_metrics.json", "w", encoding="utf-8") as handle:
        json.dump(summary, handle, indent=2)

    if montage:
        tiles = []
        for image, box, gt in montage:
            tile = image.copy()
            if box is not None:
                cv2.rectangle(tile, (int(box[0]), int(box[1])),
                              (int(box[2]), int(box[3])), (0, 200, 0), 2)
            if gt is not None:
                cv2.drawMarker(tile, (int(gt[0]), int(gt[1])),
                               (0, 0, 255), cv2.MARKER_CROSS, 24, 2)
            tiles.append(cv2.resize(tile, (640, 480)))
        while len(tiles) < 6:
            tiles.append(np.full((480, 640, 3), 255, np.uint8))
        rows = [np.hstack(tiles[i:i + 3]) for i in range(0, 6, 3)]
        cv2.imwrite(str(HERE / "yolo_val_montage.jpg"), np.vstack(rows))

    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
