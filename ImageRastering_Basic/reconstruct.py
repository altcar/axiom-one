#!/usr/bin/env python3
"""Plot five-sensor raster measurements using the positions recorded by the robot."""
import argparse
import csv
import json
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from record_wifi import HEADER

# Nominal component centres relative to the axle midpoint: forward and left (mm).
# Pololu control-board drawing, pages 3 and 5; robot drawing, page 2 (see README).
# 44.4 mm is FRONT edge to axle: forward = 44.4 - (33.7 - sensor_from_notch).
# These are drawing-derived positions, not independently measured optical calibration.
SENSOR_FORWARD_MM = np.array([27.9, 39.9, 41.4, 39.9, 27.9])
SENSOR_LEFT_MM = np.array([30.0, 10.0, 0.0, -10.0, -30.0])
COLUMNS = HEADER.split(',')


def load_log(path):
    rows, config, stop = [], {}, None
    for number, line in enumerate(path.read_text().splitlines(), 1):
        if line.startswith(('# scan_config,', '# heading_config,')):
            config.update(item.split('=', 1) for item in line.split(',')[1:])
        elif line.startswith('# stop,'):
            stop = dict(item.split('=', 1) for item in line.split(',')[1:]).get('reason')
        elif line and not line.startswith('#') and line != HEADER:
            fields = line.split(',')
            if len(fields) != len(COLUMNS):
                raise ValueError(f'Line {number}: expected 19 columns. Raw log was not changed.')
            values = [float(value) for value in fields]
            if not np.all(np.isfinite(values)):
                raise ValueError(f'Line {number}: non-finite value.')
            rows.append(dict(zip(COLUMNS, values)))
    if not rows:
        raise ValueError('No sensor records in the log.')
    return rows, config, stop


def sensor_points(rows):
    """Rotate each sensor offset by heading; do not straighten or align tracks."""
    points, stamps = [], set()
    for row in rows:
        if row['phase'] != 2 or any(row[k] != 1 for k in ('surface_ok', 'surface_fresh', 'encoder_ok')):
            continue
        stamp = row['surface_time_us']
        if stamp in stamps:
            raise ValueError('A scanning timestamp is repeated despite surface_fresh=1.')
        stamps.add(stamp)
        theta = row['theta_rad']
        c, s = np.cos(theta), np.sin(theta)
        for i, (forward, left) in enumerate(zip(SENSOR_FORWARD_MM, SENSOR_LEFT_MM)):
            points.append(dict(
                elapsed_ms=row['elapsed_ms'], surface_time_us=stamp, row=int(row['row']),
                sensor=i + 1, reading=row[f'DN{i + 1}'],
                x_mm=row['x_mm'] + forward * c - left * s - SENSOR_FORWARD_MM[2],
                y_mm=row['y_mm'] + forward * s + left * c,
            ))
    if not points:
        raise ValueError('No fresh, successful measurements during straight scanning.')
    return points


def display_scales(points):
    """Per-channel contrast stretch for display only; never overwrite readings."""
    scales = {}
    for sensor in range(1, 6):
        channel = [p for p in points if p['sensor'] == sensor]
        low = min(p['reading'] for p in channel)
        high = max(p['reading'] for p in channel)
        scales[sensor] = dict(low=low, high=high, usable=high - low >= 100)
        for p in channel:
            p['display_darkness'] = (p['reading'] - low) / (high - low) if high - low >= 100 else np.nan
    return scales


def measured_grid(points, x_edges, y_edges):
    """Median of measurements in each cell. Unmeasured cells remain NaN."""
    grid = np.full((len(y_edges) - 1, len(x_edges) - 1), np.nan)
    counts = np.zeros(grid.shape, dtype=int)
    bins = {}
    for p in points:
        ix = np.searchsorted(x_edges, p['x_mm'], side='right') - 1
        iy = np.searchsorted(y_edges, p['y_mm'], side='right') - 1
        if 0 <= ix < grid.shape[1] and 0 <= iy < grid.shape[0] and np.isfinite(p['display_darkness']):
            bins.setdefault((iy, ix), []).append(p['display_darkness'])
    for cell, values in bins.items():
        grid[cell] = np.median(values)
        counts[cell] = len(values)
    return grid, counts


def write_csv(path, rows):
    with path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def reconstruct(log, output, width, height, cell_x, cell_y, full_extent=False):
    raw, config, stop = load_log(log)
    points = sensor_points(raw)
    scales = display_scales(points)
    # Horizontal scan tracks lie near multiples of the row spacing. Put them
    # at cell centres, not on bin boundaries. Crop limits do not move any points.
    if full_extent:
        xmin = np.floor(min(p['x_mm'] for p in points) / cell_x) * cell_x
        xmax = (np.floor(max(p['x_mm'] for p in points) / cell_x) + 1) * cell_x
        ymin = (np.floor(min(p['y_mm'] for p in points) / cell_y + .5) - .5) * cell_y
        ymax = (np.floor(max(p['y_mm'] for p in points) / cell_y + .5) + .5) * cell_y
        xe = np.arange(xmin, xmax + cell_x / 2, cell_x)
        ye = np.arange(ymin, ymax + cell_y / 2, cell_y)
        roi = points
    else:
        xmin, xmax, ymin, ymax = 0, width, 0, height
        xe = np.arange(0, width + cell_x, cell_x)
        ye = np.arange(-cell_y / 2, height + cell_y, cell_y)
        roi = [p for p in points if 0 <= p['x_mm'] < width and 0 <= p['y_mm'] <= height]
    grid, counts = measured_grid(roi, xe, ye)
    output.mkdir(parents=True, exist_ok=False)
    write_csv(output / 'records.csv', raw)   # Includes diagnostic/turning rows.
    write_csv(output / 'points.csv', points) # All valid scan points, even outside the ROI.
    np.savez_compressed(output / 'grid.npz', darkness=grid, count=counts,
                        x_edges_mm=xe, y_edges_mm=ye)

    cmap = plt.get_cmap('gray_r').with_extremes(bad='#b5d5e8')
    fig, axes = plt.subplots(1, 2, figsize=(12, 5), constrained_layout=True)
    for sensor in range(1, 6):
        q = [p for p in points if p['sensor'] == sensor]
        colors = dict(c=[p['display_darkness'] for p in q], cmap=cmap, vmin=0, vmax=1)
        if not scales[sensor]['usable']:
            colors = dict(color='orange')
        axes[0].scatter([p['x_mm'] for p in q], [p['y_mm'] for p in q], s=7, linewidths=0, **colors)
    axes[0].set_title('All five sensors: estimated sample positions')
    axes[1].pcolormesh(xe, ye, np.ma.masked_invalid(grid), cmap=cmap, vmin=0, vmax=1)
    axes[1].set_title(f'Measured cells: {cell_x:g} x {cell_y:g} mm')
    for ax in axes:
        ax.set(xlabel='Sensor x (mm)', ylabel='Sensor y (mm)', aspect='equal',
               xlim=(xmin, xmax), ylim=(ymin, ymax), facecolor='#b5d5e8')
    status = 'Completed' if stop == 'DONE' else f'PARTIAL / stopped: {stop or "no stop record"}'
    fig.suptitle(f'{status} | display contrast only, not calibrated reflectance\n'
                 'Blue = no usable measurement; orange = insufficient channel contrast; no interpolation')
    fig.savefig(output / 'reconstruction.png', dpi=180)
    plt.close(fig)
    report = dict(stop_reason=stop, completed=stop == 'DONE', configuration=config,
                  raw_records=len(raw), scan_snapshots=len(points) // 5, sensor_points=len(points),
                  roi_points=len(roi), displayed_points=int(counts.sum()),
                  channel_display_scales=scales, cell_mm=[cell_x, cell_y],
                  full_extent=full_extent, display_bounds_mm=[xmin, xmax, ymin, ymax],
                  notes='Coordinates use nominal sensor offsets and the recorded robot pose. Pose and surface are acquired '
                        'sequentially, not synchronised. Grid size is not measured optical resolution. '
                        'Per-channel min/max is display scaling, not black/white calibration.')
    (output / 'summary.json').write_text(json.dumps(report, indent=2))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path, required=True, help='New output directory')
    parser.add_argument('--width-mm', type=float, default=120)
    parser.add_argument('--height-mm', type=float, default=60)
    parser.add_argument('--cell-x-mm', type=float, default=2)
    parser.add_argument('--cell-y-mm', type=float, default=5)
    parser.add_argument('--full-extent', action='store_true',
                        help='Show all sampled positions instead of the width/height crop')
    args = parser.parse_args()
    dimensions = [args.width_mm, args.height_mm, args.cell_x_mm, args.cell_y_mm]
    if any(not np.isfinite(v) or v <= 0 for v in dimensions):
        parser.error('Dimensions must be finite and positive.')
    print(json.dumps(reconstruct(args.log, args.output, *dimensions, full_extent=args.full_extent), indent=2))


if __name__ == '__main__':
    main()
