#!/usr/bin/env python3
"""Save the robot's raw TCP stream. The physical button starts motion."""
import argparse
from datetime import datetime
from pathlib import Path
import math
import socket
import time

HEADER = ('elapsed_ms,row,phase,surface_time_us,surface_ok,surface_fresh,encoder_ok,'
          'x_mm,y_mm,theta_rad,left_count,right_count,DN1,DN2,DN3,DN4,DN5,left_pwm,right_pwm')


def record(sock, output):
    """Write bytes before parsing, so even a truncated final line is retained."""
    buffer = b''
    records = 0
    last_heartbeat = 0.0
    last_received = time.monotonic()
    while True:
        now = time.monotonic()
        if now - last_heartbeat >= 0.2:
            sock.sendall(b'H')
            last_heartbeat = now
        if now - last_received > 3:
            raise TimeoutError('No data for 3 seconds; partial log retained.')
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            raise ConnectionError('Robot disconnected; partial log retained.')
        output.write(chunk)
        output.flush()
        last_received = time.monotonic()
        buffer += chunk
        if len(buffer) > 65536:
            raise ValueError('Invalid stream: no newline within 64 KB.')
        while b'\n' in buffer:
            raw, buffer = buffer.split(b'\n', 1)
            line = raw.decode('utf-8').rstrip('\r')
            if line.startswith('#'):
                print(line, flush=True)
                if line.startswith('# stop,'):
                    reason = dict(item.split('=', 1) for item in line.split(',')[1:]).get('reason')
                    print(f'Saved {records} records. Stop reason: {reason}', flush=True)
                    return 0 if reason in ('DONE', 'CALIBRATION_DONE') else 1
                if '# finished_reset_required' in line:
                    raise ValueError('Reset the robot before starting another recording.')
            elif line == HEADER:
                print('Recording started: all five sensors and the scan pose.', flush=True)
            elif line:
                fields = line.split(',')
                if len(fields) != 19 or not all(math.isfinite(float(v)) for v in fields):
                    raise ValueError('Invalid data record; original bytes retained.')
                records += 1
                if records % 100 == 0:
                    print(f'Recorded {records} snapshots', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='192.168.4.1')
    parser.add_argument('--port', type=int, default=80)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    path = args.output or Path(__file__).resolve().parent / 'logs' / (
        'raster_' + datetime.now().strftime('%Y%m%d_%H%M%S_%f') + '.txt')
    path.parent.mkdir(parents=True, exist_ok=True)
    sock = None
    try:
        # Exclusive creation prevents accidental overwrite of an earlier run.
        with path.open('xb') as output:
            print(f'Connecting to {args.host}:{args.port}; log: {path.resolve()}', flush=True)
            sock = socket.create_connection((args.host, args.port), timeout=5)
            sock.settimeout(0.1)
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            print('Wait for ready_press_button, then press the robot button. Ctrl+C requests STOP.', flush=True)
            return record(sock, output)
    except KeyboardInterrupt:
        print('\nSTOP requested; partial log retained.')
        return 130
    except (OSError, ValueError, UnicodeError) as error:
        print(f'Recording stopped: {error}')
        return 1
    finally:
        if sock is not None:
            try:
                sock.sendall(b'S')
            except OSError:
                pass
            sock.close()


if __name__ == '__main__':
    raise SystemExit(main())
