import io
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import numpy as np
from record_wifi import HEADER, record, main as record_main
from reconstruct import load_log, sensor_points, display_scales, measured_grid, reconstruct


def snapshot(stamp=10000, x=0, y=0, theta=0, row=1, phase=2, reading=500):
    values = [stamp / 1000, row, phase, stamp, 1, 1, 1, x, y, theta, 0, 0,
              reading, reading, reading, reading, reading, 40, 40]
    return dict(zip(HEADER.split(','), values))


def csv_line(row):
    return ','.join(str(v) for v in row.values()) + '\n'


class FakeSocket:
    def __init__(self, chunks):
        self.chunks = iter(chunks)
        self.commands = []
        self.closed = False

    def settimeout(self, timeout):
        pass

    def setsockopt(self, *args):
        pass

    def close(self):
        self.closed = True

    def sendall(self, data):
        self.commands.append(data)

    def recv(self, size):
        chunk = next(self.chunks, b'')
        if isinstance(chunk, BaseException):
            raise chunk
        return chunk


class DataTests(unittest.TestCase):
    def test_sensor_rotation_and_common_origin(self):
        p = sensor_points([snapshot(), snapshot(20000, x=200, theta=np.pi, row=2)])
        self.assertEqual(len(p), 10)
        self.assertAlmostEqual(p[2]['x_mm'], 0)
        # Drawing-derived coordinates at an axle position of (200, 0), facing back.
        np.testing.assert_allclose([q['x_mm'] for q in p[5:]],
                                   [130.7, 118.7, 117.2, 118.7, 130.7])
        self.assertAlmostEqual(p[0]['y_mm'], 30)
        self.assertAlmostEqual(p[5]['y_mm'], -30)

    def test_dn3_same_surface_point_on_opposite_passes(self):
        # DN3 is 41.4 mm ahead of the axle. Axles 82.8 mm apart and facing
        # each other put DN3 over the same physical point on both passes.
        p = sensor_points([snapshot(), snapshot(20000, x=82.8, theta=np.pi)])
        self.assertAlmostEqual(p[2]['x_mm'], p[7]['x_mm'])
        self.assertAlmostEqual(p[2]['y_mm'], p[7]['y_mm'])

    def test_diagnostics_are_retained_but_not_mapped(self):
        rows = [snapshot(), snapshot(20000, phase=4), snapshot(30000)]
        rows[2]['surface_fresh'] = 0
        before = [dict(r) for r in rows]
        self.assertEqual(len(sensor_points(rows)), 5)
        self.assertEqual(rows, before)

    def test_false_fresh_duplicate_is_rejected(self):
        with self.assertRaises(ValueError):
            sensor_points([snapshot(), snapshot()])

    def test_grid_does_not_fill_gaps_or_move_points(self):
        p = [dict(x_mm=1, y_mm=0.1, display_darkness=0),
             dict(x_mm=3, y_mm=-0.1, display_darkness=1)]
        before = [dict(q) for q in p]
        grid, counts = measured_grid(p, np.array([0, 2, 4, 6]), np.array([-2.5, 2.5]))
        np.testing.assert_equal(grid, [[0, 1, np.nan]])
        np.testing.assert_equal(counts, [[1, 1, 0]])
        self.assertEqual(p, before)

    def test_constant_channels_do_not_become_false_black_or_white(self):
        p = sensor_points([snapshot()])
        scales = display_scales(p)
        self.assertTrue(all(not v['usable'] for v in scales.values()))
        self.assertTrue(all(np.isnan(q['display_darkness']) for q in p))
        self.assertTrue(all(q['reading'] == 500 for q in p))

    def test_reconstruction_keeps_every_raw_row(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            log = root / 'synthetic.txt'
            rows = [snapshot(), snapshot(20000, x=10, reading=1800), snapshot(30000, phase=4)]
            content = '# SYNTHETIC SOFTWARE TEST\n' + HEADER + '\n'
            content += '# imu,elapsed_ms=100,gyro_deg=0.5,encoder_deg=0.6\n'
            content += ''.join(csv_line(r) for r in rows) + '# stop,reason=DONE,row=1\n'
            log.write_text(content)
            report = reconstruct(log, root / 'result', 120, 60, 2, 5)
            self.assertEqual(report['raw_records'], 3)
            self.assertEqual(report['sensor_points'], 10)
            self.assertTrue(report['completed'])
            self.assertEqual(len((root / 'result/records.csv').read_text().splitlines()), 4)
            self.assertEqual(len((root / 'result/points.csv').read_text().splitlines()), 11)
            self.assertEqual(log.read_text(), content)
            self.assertTrue((root / 'result/reconstruction.png').exists())

    def test_heading_metadata_keeps_pose_source_and_numeric_records(self):
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / 'synthetic.txt'
            row = snapshot(x=8, y=-3, theta=.15)
            scan = '# scan_config,rows=13,row_spacing_mm=5\n'
            for source in ('fused', 'encoders'):
                heading = f'# heading_config,control={source},pose_source={source},correction_tau_s=1.000\n'
                for metadata in (scan + heading, heading + scan):
                    content = HEADER + '\n' + metadata + csv_line(row)
                    log.write_text(content)
                    records, config, stop = load_log(log)
                    self.assertEqual(records, [row])
                    self.assertEqual(config['pose_source'], source)
                    self.assertEqual(config['control'], source)
                    self.assertEqual(float(config['row_spacing_mm']), 5)
                    self.assertEqual(float(config['correction_tau_s']), 1)
                    self.assertIsNone(stop)
                    self.assertEqual(log.read_text(), content)

    def test_missing_stop_and_truncated_line_are_not_success(self):
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / 'partial.txt'
            log.write_text(HEADER + '\n' + csv_line(snapshot()))
            self.assertIsNone(load_log(log)[2])
            log.write_text(log.read_text() + '123,4,2,99')
            with self.assertRaises(ValueError):
                load_log(log)

    def test_manual_full_extent_includes_all_sensor_points(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            log = root / 'synthetic.txt'
            rows = [snapshot(x=-20), snapshot(20000, x=180, y=80, reading=1800),
                    snapshot(30000, phase=4)]
            content = HEADER + '\n' + ''.join(csv_line(r) for r in rows) + '# stop,reason=DONE,row=1\n'
            log.write_text(content)
            result = root / 'results/synthetic'
            report = reconstruct(log, result, 120, 60, 2, 5, full_extent=True)
            self.assertTrue(report['completed'] and report['full_extent'])
            self.assertEqual(report['raw_records'], 3)
            self.assertEqual(report['roi_points'], 10)
            self.assertEqual(report['displayed_points'], 10)
            bounds = report['display_bounds_mm']
            self.assertLess(bounds[0], 0)
            self.assertGreater(bounds[1], 120)
            self.assertLess(bounds[2], 0)
            self.assertGreater(bounds[3], 60)
            for name in ('reconstruction.png', 'records.csv', 'points.csv', 'grid.npz'):
                self.assertTrue((result / name).exists())
            self.assertEqual(log.read_text(), content)

    @patch('builtins.print')
    def test_recorder_main_saves_log_and_exits_without_reconstruction(self, unused):
        cases = [(b'# stop,reason=DONE,row=1\n', 0),
                 (b'# stop,reason=CALIBRATION_DONE,row=1\n', 0),
                 (b'# stop,reason=read_failed,row=1\n', 1),
                 (b'', 1), (KeyboardInterrupt(), 130)]
        for end, expected in cases:
            with self.subTest(end=end), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                log = root / 'partial.txt'
                raw = (HEADER + '\n' + csv_line(snapshot())).encode()
                sock = FakeSocket([raw, end])
                with patch('sys.argv', ['record_wifi.py', '--output', str(log)]), \
                     patch('record_wifi.__file__', str(root / 'record_wifi.py')), \
                     patch('record_wifi.socket.create_connection', return_value=sock), \
                     patch('reconstruct.reconstruct') as plot:
                    self.assertEqual(record_main(), expected)
                plot.assert_not_called()
                self.assertTrue(sock.closed)
                self.assertEqual(log.read_bytes(), raw + (end if isinstance(end, bytes) else b''))
                self.assertEqual(sock.commands[-1], b'S')
                self.assertFalse((root / 'results').exists())

    @patch('builtins.print')
    def test_recorder_retains_fragmented_stream(self, unused):
        raw = (HEADER + '\n' + csv_line(snapshot()) +
               '# imu,elapsed_ms=100,gyro_deg=0.5,encoder_deg=0.6\n' +
               '# stop,reason=DONE,row=1\n').encode()
        sock = FakeSocket([raw[:15], raw[15:50], raw[50:]])
        output = io.BytesIO()
        self.assertEqual(record(sock, output), 0)
        self.assertEqual(output.getvalue(), raw)
        self.assertEqual(sock.commands[0], b'H')

    @patch('builtins.print')
    def test_recorder_preserves_incomplete_tail_on_disconnect(self, unused):
        raw = (HEADER + '\n123,4,2,99').encode()
        output = io.BytesIO()
        with self.assertRaises(ConnectionError):
            record(FakeSocket([raw]), output)
        self.assertEqual(output.getvalue(), raw)

    @patch('builtins.print')
    def test_fault_stop_has_nonzero_status(self, unused):
        self.assertEqual(record(FakeSocket([b'# stop,reason=read_failed,row=1\n']), io.BytesIO()), 1)


if __name__ == '__main__':
    unittest.main()
