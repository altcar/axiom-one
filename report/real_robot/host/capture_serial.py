#!/usr/bin/env python3
"""Capture the SensorTests CSV stream from the StampC3 over USB serial.

    pip install pyserial
    python3 host/capture_serial.py --port /dev/ttyACM0 --out data/offset.csv

Stops when you press Ctrl-C, or after --seconds. Lines starting with # are kept
as comments so the mode and trial markers survive into the file.
"""
import argparse, pathlib, sys, time

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="/dev/ttyACM0, /dev/ttyUSB0 or COM5")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", required=True)
    ap.add_argument("--seconds", type=float, default=120.0)
    a = ap.parse_args()

    import serial  # imported here so --help works without pyserial

    pathlib.Path(a.out).parent.mkdir(parents=True, exist_ok=True)
    t0 = time.time()
    lines = 0
    with serial.Serial(a.port, a.baud, timeout=1) as port, open(a.out, "w") as fh:
        print(f"reading {a.port} at {a.baud}, Ctrl-C to stop")
        try:
            while time.time() - t0 < a.seconds:
                raw = port.readline().decode("utf-8", "replace").strip()
                if not raw:
                    continue
                if raw.startswith("#") or raw.startswith("mode,"):
                    fh.write(raw + "\n")
                    fh.flush()
                    continue
                if raw.count(",") != 10:
                    continue          # boot noise, not a data row
                fh.write(raw + "\n")
                fh.flush()
                lines += 1
                if lines % 200 == 0:
                    print(f"  {lines} rows", flush=True)
        except KeyboardInterrupt:
            pass
    print(f"wrote {lines} rows to {a.out}")

if __name__ == "__main__":
    main()
