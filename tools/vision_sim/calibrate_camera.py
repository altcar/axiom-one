#!/usr/bin/env python3
"""Calibrate the camera used for the AprilTag map.

Print a chessboard (default: 9 x 6 inner corners, 25 mm squares), hold it at
several angles in front of the camera, press SPACE to capture each accepted
view, and ESC when finished. The script saves camera_intrinsics.json next to
this file:

    {"image_size": [w, h], "camera_matrix": [[...]], "dist_coeffs": [...],
     "rms": ...}

Use the same camera resolution and focus for the map run. A fixed mount and
locked focus are strongly preferred.
"""

import argparse
import json
from pathlib import Path

import cv2
import numpy as np

HERE = Path(__file__).resolve().parent
CRITERIA = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--camera", type=int, default=0)
    parser.add_argument("--cols", type=int, default=9,
                        help="inner corners across")
    parser.add_argument("--rows", type=int, default=6,
                        help="inner corners down")
    parser.add_argument("--square-mm", type=float, default=25.0)
    args = parser.parse_args()

    pattern = (args.cols, args.rows)
    object_points = np.zeros((pattern[0] * pattern[1], 3), np.float32)
    object_points[:, :2] = np.mgrid[0:pattern[0], 0:pattern[1]].T.reshape(-1, 2)
    object_points *= args.square_mm

    capture = cv2.VideoCapture(args.camera, cv2.CAP_DSHOW)
    if not capture.isOpened():
        raise SystemExit(f"Could not open camera {args.camera}")
    capture.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

    found_points = []
    image_size = None
    print("SPACE = capture, ESC = finish. Need at least 12 varied views.")
    while True:
        ok, frame = capture.read()
        if not ok:
            break
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        image_size = gray.shape[::-1]
        found, corners = cv2.findChessboardCorners(gray, pattern, None)
        display = frame.copy()
        if found:
            corners = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1),
                                       CRITERIA)
            cv2.drawChessboardCorners(display, pattern, corners, found)
        cv2.putText(display, f"captured {len(found_points)}  "
                             "SPACE capture  ESC finish",
                    (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.8,
                    (0, 255, 0) if found else (0, 0, 255), 2)
        cv2.imshow("camera calibration", display)
        key = cv2.waitKey(1) & 0xFF
        if key == 27:
            break
        if key == 32 and found:
            found_points.append(corners)
            print(f"captured {len(found_points)}")

    capture.release()
    cv2.destroyAllWindows()
    if len(found_points) < 12:
        raise SystemExit("Fewer than 12 accepted views; calibration not saved")

    rms, matrix, distortion, _, _ = cv2.calibrateCamera(
        [object_points] * len(found_points), found_points, image_size,
        None, None)
    result = {
        "image_size": [int(image_size[0]), int(image_size[1])],
        "camera_matrix": matrix.tolist(),
        "dist_coeffs": distortion.ravel().tolist(),
        "rms": float(rms),
        "pattern": {"cols": args.cols, "rows": args.rows,
                    "square_mm": args.square_mm},
    }
    with open(HERE / "camera_intrinsics.json", "w", encoding="utf-8") as handle:
        json.dump(result, handle, indent=2)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
