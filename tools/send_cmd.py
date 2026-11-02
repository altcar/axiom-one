"""Send one command line to the 3Pi+ robot over WiFi and print what comes back.

The robot listens on TCP 192.168.4.1:80 and accepts a single client, so close
the live GUI before using this (its socket would block the accept).

Usage:
    python tools/send_cmd.py RECT
    python tools/send_cmd.py STOP
    python tools/send_cmd.py DUMP          # then watch the samples scroll by
    python tools/send_cmd.py START --listen 25

Commands implemented in the firmware: START, STOP, RECT, DUMP, CLEAR,
HIL,1 / HIL,0, PWM,l,r.
"""
import argparse
import socket
import subprocess
import sys
import time

HOST = "192.168.4.1"
PORT = 80


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("command", nargs="?", default="watch",
                    help="e.g. RECT, STOP, START (default: watch, sends nothing)")
    ap.add_argument("--listen", type=float, default=3.0,
                    help="seconds to keep printing telemetry (default 3)")
    args = ap.parse_args()

    # A board reset leaves a stale ARP entry; Windows then times out.
    try:
        subprocess.run(["arp", "-d", HOST], capture_output=True, timeout=5)
    except Exception:
        pass

    try:
        s = socket.create_connection((HOST, PORT), timeout=5)
    except OSError as e:
        print("cannot reach %s:%d (%s)" % (HOST, PORT, e))
        print("connect the PC to the myAP network and try again")
        return 1
    s.settimeout(0.5)
    # "watch" just listens: the robot streams telemetry as soon as a client
    # connects, so no command is needed to see the pose and the EVENT lines.
    if args.command.lower() != "watch":
        s.sendall((args.command + "\n").encode())

    t0 = time.time()
    buf = b""
    while time.time() - t0 < args.listen:
        try:
            chunk = s.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            break
        buf += chunk
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            text = line.decode("ascii", "replace").strip()
            if text:
                print(text)
    s.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
