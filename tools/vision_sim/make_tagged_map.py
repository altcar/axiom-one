#!/usr/bin/env python3
"""Generate the AprilTag 36h11 map/robot tag layouts.

Outputs
-------
Untitled_tagged.png (project root)
    The A3 test card with four 14 mm markers in the black border corners.
    Convenient, but the white quiet zones interrupt the border.

tools/vision_sim/map_frame_A2.png
    RECOMMENDED: a 560 x 417 mm frame with four 36 mm markers outside a
    full-size A3 test card. Print at 100% on A2 landscape (594 x 420 mm).
    The A3 content is at 1:1 and the tags never touch the raster surface.

tools/vision_sim/robot_tag_10_50mm.png
    A 36 mm marker (50 mm including quiet zone), ID 10, for the robot.

tools/vision_sim/tag_layout.json
    Tag IDs, sizes, and centres in mm and pixels, for the camera calibration
    and synthetic tests.
"""

import json
import os
from pathlib import Path

import cv2
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
DICTIONARY = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_APRILTAG_36h11)
DPI = 300.0
PX_PER_MM = DPI / 25.4


def marker_with_quiet(marker_mm, quiet_mm, tag_id):
    marker_px = int(round(marker_mm * PX_PER_MM))
    quiet_px = int(round(quiet_mm * PX_PER_MM))
    marker = cv2.aruco.generateImageMarker(DICTIONARY, tag_id, marker_px)
    total = marker_px + 2 * quiet_px
    canvas = np.full((total, total), 255, np.uint8)
    canvas[quiet_px:quiet_px + marker_px,
           quiet_px:quiet_px + marker_px] = marker
    return canvas, marker_px, quiet_px, total


def paste_tag(image, tag_rgb, center_x, center_y):
    tag = Image.fromarray(tag_rgb).convert("RGB")
    x0 = int(round(center_x - tag.width / 2.0))
    y0 = int(round(center_y - tag.height / 2.0))
    image.paste(tag, (x0, y0))


def make_on_card_layout():
    src = Image.open(ROOT / "Untitled.png").convert("RGB")
    tagged = src.copy()
    # Black border runs measured from the 300 dpi A3 export:
    #   rows 178..430 and 3078..3330; cols 242..492 and 4465..4716.
    x_left, x_right = (242 + 492) / 2.0, (4465 + 4716) / 2.0
    y_top, y_bottom = (178 + 430) / 2.0, (3078 + 3330) / 2.0
    centres_px = [(x_left, y_top), (x_right, y_top),
                  (x_right, y_bottom), (x_left, y_bottom)]
    marker_mm, quiet_mm = 14.0, 3.5
    tags = []
    for tag_id, (cx, cy) in enumerate(centres_px):
        tag_rgb, marker_px, quiet_px, total = marker_with_quiet(
            marker_mm, quiet_mm, tag_id)
        paste_tag(tagged, tag_rgb, cx, cy)
        tags.append({
            "id": tag_id,
            "centre_px": [round(cx, 1), round(cy, 1)],
            "centre_mm": [round(cx / PX_PER_MM, 2), round(cy / PX_PER_MM, 2)],
            "marker_mm": marker_mm,
            "quiet_mm": quiet_mm,
            "total_mm": round(marker_mm + 2 * quiet_mm, 2),
        })
    tagged.save(ROOT / "Untitled_tagged.png", dpi=(DPI, DPI))
    tagged.resize((1240, int(1240 * tagged.height / tagged.width))).save(
        OUT / "Untitled_tagged_preview.png")
    return {
        "image": "Untitled_tagged.png",
        "sheet_mm": [420.0, 297.0],
        "image_px": [src.width, src.height],
        "px_per_mm": round(src.width / 420.0, 4),
        "tags": tags,
    }


def make_frame_layout():
    frame_w_mm, frame_h_mm = 560.0, 417.0
    a3_x_mm, a3_y_mm = 70.0, 60.0
    width = int(round(frame_w_mm * PX_PER_MM))
    height = int(round(frame_h_mm * PX_PER_MM))
    frame = Image.new("RGB", (width, height), (255, 255, 255))
    a3 = Image.open(ROOT / "Untitled.png").convert("RGB")
    ax, ay = int(round(a3_x_mm * PX_PER_MM)), int(round(a3_y_mm * PX_PER_MM))
    frame.paste(a3, (ax, ay))
    # Thin guide around the A3 placement (outside the sheet itself).
    draw = np.asarray(frame).copy()
    cv2.rectangle(draw, (ax - 3, ay - 3),
                  (ax + a3.width + 3, ay + a3.height + 3), (180, 180, 180), 2)
    frame = Image.fromarray(draw)

    marker_mm, quiet_mm = 36.0, 7.0
    centres_mm = [(30.0, 30.0), (frame_w_mm - 30.0, 30.0),
                  (frame_w_mm - 30.0, frame_h_mm - 30.0),
                  (30.0, frame_h_mm - 30.0)]
    tags = []
    for tag_id, (cx_mm, cy_mm) in enumerate(centres_mm):
        tag_rgb, marker_px, quiet_px, total = marker_with_quiet(
            marker_mm, quiet_mm, tag_id)
        paste_tag(frame, tag_rgb, cx_mm * PX_PER_MM, cy_mm * PX_PER_MM)
        tags.append({
            "id": tag_id,
            "centre_mm": [round(cx_mm, 2), round(cy_mm, 2)],
            "centre_px": [round(cx_mm * PX_PER_MM, 1),
                          round(cy_mm * PX_PER_MM, 1)],
            "marker_mm": marker_mm,
            "quiet_mm": quiet_mm,
            "total_mm": round(marker_mm + 2 * quiet_mm, 2),
        })
    frame.save(OUT / "map_frame_A2.png", dpi=(DPI, DPI))
    frame.resize((1240, int(1240 * frame.height / frame.width))).save(
        OUT / "map_frame_A2_preview.png")
    return {
        "image": "tools/vision_sim/map_frame_A2.png",
        "frame_mm": [frame_w_mm, frame_h_mm],
        "a3_origin_mm": [a3_x_mm, a3_y_mm],
        "a3_size_mm": [420.0, 297.0],
        "print": "A2 landscape (594 x 420 mm) at 100%",
        "tags": tags,
    }


def make_robot_tag():
    marker_mm, quiet_mm = 36.0, 7.0
    tag_rgb, marker_px, quiet_px, total = marker_with_quiet(
        marker_mm, quiet_mm, 10)
    tag = Image.fromarray(tag_rgb).convert("RGB")
    tag.save(OUT / "robot_tag_10_50mm.png", dpi=(DPI, DPI))
    return {
        "id": 10,
        "marker_mm": marker_mm,
        "quiet_mm": quiet_mm,
        "total_mm": round(marker_mm + 2 * quiet_mm, 2),
        "mount": "flat on the robot top, tag +x axis pointing forward",
    }


def main():
    layout = {
        "dictionary": "DICT_APRILTAG_36h11",
        "on_card": make_on_card_layout(),
        "frame": make_frame_layout(),
        "robot_tag": make_robot_tag(),
        "notes": [
            "Map coordinates use the image convention: +x right, +y down, "
            "origin at the A3 top-left.",
            "The on-card tags are convenient but their white quiet zones "
            "interrupt the black border; use them only if the camera raster "
            "replaces border-following.",
            "The A2 frame is recommended: tags are outside the A3, larger, "
            "and do not disturb the printed surface or its border.",
        ],
    }
    with open(OUT / "tag_layout.json", "w", encoding="utf-8") as handle:
        json.dump(layout, handle, indent=2)
    print(json.dumps(layout, indent=2))


if __name__ == "__main__":
    main()
