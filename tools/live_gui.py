"""Live view of the 3Pi+ raster robot over WiFi.

The robot streams a 20 Hz CSV line on TCP 192.168.4.1:80:

    ms,x,y,th,pwmL,pwmR,encL,encR,dn0,dn1,dn2,dn3,dn4,signal

x,y are millimetres in the odometry frame (origin = the corner where SEEK
zeroed the pose), th is radians. EVENT lines arrive on the same socket when
the firmware logs a milestone -- those are what explain a run.

Usage:
    python tools/live_gui.py [host] [port]

Notes on robustness:
  * The robot can reset mid-run (brownout at a motor reversal). After a reset
    the TCP socket stays half-open at the OS level: there is no FIN, so the
    reader needs its own stall watchdog or the view freezes forever.
  * `ms` restarts at zero after a reset, which starts a NEW trail segment, so
    the plot does not draw a false straight line across the arena.
  * Drawing is decimated to ~1500 points at 10 fps; redrawing tens of
    thousands of points every 50 ms is what made the window stop responding.
  * Every received line is appended to live_YYYYmmdd_HHMMSS.csv next to this
    file, so a run is never lost when the window is closed.
"""
import math
import os
import socket
import subprocess
import sys
import threading
import time
import tkinter as tk

HOST = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 80

STATE_NAMES = {0: "idle", 1: "AUTO SCAN", 2: "HIL"}
TRAIL_MAX = 40000
DRAW_POINTS = 1500
STALL_TIMEOUT = 3.0
HERE = os.path.dirname(os.path.abspath(__file__))


class Link(threading.Thread):
    """Owns the socket: reconnects forever, parses telemetry into `state`."""

    def __init__(self):
        super().__init__(daemon=True)
        self.lock = threading.Lock()
        self.state = None
        self.segments = [[]]     # list of trails; a reset starts a new one
        self.status = "starting"
        self.sock = None
        self.tx = []
        self.raw_events = []
        self.last_rx = 0.0
        self.packets = 0
        stamp = time.strftime("%Y%m%d_%H%M%S")
        self.rec_path = os.path.join(HERE, "live_%s.csv" % stamp)
        self.rec = open(self.rec_path, "w", buffering=1)
        self.rec.write("wall_s,line\n")
        self.rec_rows = 0

    def flush_arp(self):
        try:
            subprocess.run(["arp", "-d", HOST], capture_output=True, timeout=5)
        except Exception:
            pass

    def run(self):
        while True:
            try:
                self.flush_arp()
                s = socket.create_connection((HOST, PORT), timeout=4)
                s.settimeout(0.5)
                s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                with self.lock:
                    self.sock = s
                    self.status = "connected"
                    self.last_rx = time.time()
                self._read_loop(s)
            except Exception as e:
                with self.lock:
                    self.status = "link: %s" % (e.__class__.__name__,)
                    self.sock = None
                time.sleep(0.7)

    def _read_loop(self, s):
        buf = b""
        while True:
            with self.lock:
                msg = "".join(self.tx).encode() if self.tx else None
                self.tx = []
            if msg:
                s.sendall(msg)
            try:
                chunk = s.recv(4096)
            except socket.timeout:
                with self.lock:
                    age = time.time() - self.last_rx
                if age > STALL_TIMEOUT:
                    raise IOError("stalled")
                continue
            if not chunk:
                raise IOError("closed")
            with self.lock:
                self.last_rx = time.time()
            buf += chunk
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                self._line(line.decode("ascii", "replace").strip())

    def _line(self, line):
        if not line:
            return
        with self.lock:
            try:
                self.rec.write("%.3f,%s\n" % (time.time(), line))
                self.rec_rows += 1
            except Exception:
                pass
        p = line.split(",")
        if len(p) < 14:
            with self.lock:
                self.raw_events.append(line)
                self.raw_events = self.raw_events[-40:]
            return
        if p[0] in ("DUMP", "BEGIN", "END", "EVENT"):
            with self.lock:
                self.raw_events.append(line)
                self.raw_events = self.raw_events[-40:]
            return
        try:
            v = [float(x) for x in p[:14]]
        except ValueError:
            return
        st = dict(ms=v[0], x=v[1], y=v[2], th=v[3], pwmL=v[4], pwmR=v[5],
                  dn=v[8:13], sig=int(v[13]), t=time.time())
        with self.lock:
            prev = self.state
            self.state = st
            self.packets += 1
            if prev is not None and st["ms"] < prev["ms"] - 500:
                self.segments.append([])
                self.raw_events.append("--- reset detected, new trail ---")
            seg = self.segments[-1]
            if not seg or abs(st["x"] - seg[-1][0]) > 0.25 or \
                    abs(st["y"] - seg[-1][1]) > 0.25:
                seg.append((st["x"], st["y"]))
                if len(seg) > TRAIL_MAX:
                    del seg[:len(seg) - TRAIL_MAX]

    def send(self, cmd):
        with self.lock:
            self.tx.append(cmd + "\n")
            self.raw_events.append(">> " + cmd)
            self.raw_events = self.raw_events[-40:]

    def clear(self):
        with self.lock:
            self.segments = [[]]
            self.raw_events = []
        self.send("CLEAR")


class App:
    def __init__(self, root, link):
        self.root = root
        self.link = link
        root.title("3Pi+ raster - live IMU pose")
        root.configure(bg="#101418")
        root.protocol("WM_DELETE_WINDOW", root.destroy)

        self.w, self.h = 900, 620
        self.canvas = tk.Canvas(root, width=self.w, height=self.h,
                                bg="#0d1117", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        side = tk.Frame(root, bg="#161b22", width=245)
        side.pack(side=tk.RIGHT, fill=tk.Y)
        side.pack_propagate(False)

        self.info = tk.Label(side, text="", justify=tk.LEFT, anchor="nw",
                             font=("Consolas", 10), fg="#e6edf3", bg="#161b22")
        self.info.pack(fill=tk.X, padx=10, pady=(10, 4))

        self.sens = tk.Canvas(side, width=222, height=120, bg="#161b22",
                              highlightthickness=0)
        self.sens.pack(padx=6, pady=2)
        tk.Label(side, text="phototransistors (dark = line)",
                 font=("Consolas", 8), fg="#8b949e", bg="#161b22").pack()

        btns = tk.Frame(side, bg="#161b22")
        btns.pack(pady=8)
        for txt, fn in (("START", lambda: link.send("START")),
                        ("STOP", lambda: link.send("STOP")),
                        ("RECT TEST", lambda: link.send("RECT")),
                        ("DUMP", lambda: link.send("DUMP")),
                        ("CLEAR", link.clear),
                        ("OPEN CSV", self.open_csv)):
            tk.Button(btns, text=txt, width=26, command=fn,
                      bg="#21262d", fg="#e6edf3", activebackground="#30363d",
                      relief=tk.FLAT).pack(pady=2)

        self.follow = tk.BooleanVar(value=True)
        tk.Checkbutton(side, text="follow robot", variable=self.follow,
                       fg="#e6edf3", bg="#161b22", selectcolor="#161b22",
                       activebackground="#161b22").pack(anchor="w", padx=8)

        tk.Label(side, text="events", font=("Consolas", 8), fg="#8b949e",
                 bg="#161b22").pack(anchor="w", padx=8)
        self.log = tk.Label(side, text="", justify=tk.LEFT, anchor="nw",
                            font=("Consolas", 7), fg="#8b949e", bg="#161b22",
                            wraplength=228)
        self.log.pack(fill=tk.BOTH, expand=True, padx=8, pady=(0, 6))

        self.origin = (self.w * 0.5, self.h * 0.8)
        self.scale = 2.0
        self.loaded = ""
        for seq in ("<MouseWheel>", "<Button-4>", "<Button-5>"):
            self.canvas.bind(seq, self.on_wheel)
        self.canvas.bind("<Double-Button-1>", lambda e: self.follow.set(True))
        self.tick()

    # ---- helpers ---------------------------------------------------------
    def view(self):
        ox, oy = self.origin
        return (-ox / self.scale, (oy - self.h) / self.scale,
                (self.w - ox) / self.scale, oy / self.scale)

    def fit(self):
        pts = []
        for seg in self.link.segments:
            if seg:
                pts.append(seg[0])
                pts.append(seg[-1])
        if self.link.state:
            pts.append((self.link.state["x"], self.link.state["y"]))
        if not pts:
            return
        xs = [p[0] for p in pts]
        ys = [p[1] for p in pts]
        spanx = max(60.0, max(xs) - min(xs))
        spany = max(60.0, max(ys) - min(ys))
        self.scale = min((self.w - 120) / spanx, (self.h - 120) / spany, 6.0)
        cx = (max(xs) + min(xs)) / 2.0
        cy = (max(ys) + min(ys)) / 2.0
        self.origin = (self.w * 0.5 - cx * self.scale,
                       self.h * 0.5 + cy * self.scale)

    def w2s(self, x, y):
        ox, oy = self.origin
        return (ox + x * self.scale, oy - y * self.scale)

    def on_wheel(self, event):
        num = getattr(event, "num", None)
        if num == 4:
            f = 1.12
        elif num == 5:
            f = 1.0 / 1.12
        else:
            d = getattr(event, "delta", 0)
            if not d:
                return
            f = 1.12 ** (d / 120.0)
        sx, sy = event.x, event.y
        ox, oy = self.origin
        wx = (sx - ox) / self.scale
        wy = (oy - sy) / self.scale
        new = max(0.25, min(24.0, self.scale * f))
        if abs(new - self.scale) < 1e-9:
            return
        self.follow.set(False)
        self.scale = new
        self.origin = (sx - wx * new, sy + wy * new)
        self.draw()

    def open_csv(self):
        """Replot a finished capture (wall_s,line rows or raw telemetry)."""
        from tkinter import filedialog
        path = filedialog.askopenfilename(
            initialdir=HERE, filetypes=[("CSV", "*.csv"), ("All", "*.*")])
        if not path:
            return
        seg = []
        for line in open(path, "r", errors="replace"):
            p = line.strip().split(",")
            if len(p) >= 15:
                off = 1
            elif len(p) == 14:
                off = 0
            else:
                continue
            try:
                v = [float(t) for t in p[off:off + 14]]
            except ValueError:
                continue
            seg.append((v[1], v[2]))
        if not seg:
            self.log.config(text="no telemetry rows in %s" % path)
            return
        with self.link.lock:
            self.link.segments = [seg]
            self.link.state = None
        self.loaded = os.path.basename(path)
        self.follow.set(True)

    def grid(self):
        c = self.canvas
        step = 50.0
        while self.scale * step < 35:
            step *= 2
        while self.scale * step > 160:
            step /= 2
        x0, y0, x1, y1 = self.view()
        gx = math.floor(x0 / step) * step
        while gx <= x1:
            sx = self.origin[0] + gx * self.scale
            major = abs((gx / step) % 5) < 1e-6
            c.create_line(sx, 0, sx, self.h,
                          fill="#161b22" if not major else "#263040")
            if major:
                c.create_text(sx + 2, self.h - 8, text="%d" % gx,
                              fill="#484f58", font=("Consolas", 8), anchor="w")
            gx += step
        gy = math.floor(y0 / step) * step
        while gy <= y1:
            sy = self.origin[1] - gy * self.scale
            major = abs((gy / step) % 5) < 1e-6
            c.create_line(0, sy, self.w, sy,
                          fill="#161b22" if not major else "#263040")
            if major:
                c.create_text(4, sy - 1, text="%d" % gy, fill="#484f58",
                              font=("Consolas", 8), anchor="sw")
            gy += step

    def draw_sensors(self, dn):
        c = self.sens
        c.delete("all")
        names = ["-45", "-14", "0", "+14", "+45"]
        for i, v in enumerate(dn):
            h = max(2, min(92, v / 3200.0 * 92))
            x = 8 + i * 43
            col = "#f85149" if v < 1400 else "#3fb950"
            c.create_rectangle(x, 96 - h, x + 30, 96, fill=col, outline="")
            c.create_text(x + 15, 108, text=names[i], fill="#8b949e",
                          font=("Consolas", 7))

    def tick(self):
        try:
            self.draw()
        except Exception as e:
            self.log.config(text="draw error: %r" % (e,))
        self.root.after(100, self.tick)

    def draw(self):
        c = self.canvas
        with self.link.lock:
            st = dict(self.link.state) if self.link.state else None
            segs = [list(s) for s in self.link.segments]
            status = self.link.status
            events = list(self.link.raw_events)
            age = time.time() - self.link.last_rx if self.link.last_rx else 0.0

        if self.follow.get():
            self.fit()
        c.delete("all")
        self.grid()

        for seg in segs:
            if len(seg) < 2:
                continue
            step = max(1, len(seg) // DRAW_POINTS)
            pts = []
            for (x, y) in seg[::step]:
                pts.extend(self.w2s(x, y))
            if len(pts) >= 4:
                c.create_line(*pts, fill="#2f81f7", width=2)

        if st:
            x, y = self.w2s(st["x"], st["y"])
            th = st["th"]
            ca, sa = math.cos(th), math.sin(th)

            def P(dx, dy):
                return (x + (ca * dx - sa * dy) * self.scale,
                        y - (sa * dx + ca * dy) * self.scale)

            tri = [P(45, 0), P(-25, 30), P(-25, -30)]
            c.create_polygon(*[q for p in tri for q in p], fill="#f0883e",
                             outline="#ffffff")
            c.create_line(x, y, *P(110, 0), fill="#f0883e", arrow=tk.LAST,
                          width=2)
            # sensor bar: 45 mm radius, 45/14/0/-14/-45 deg
            for (dx, dy) in ((31.8, 31.8), (10.9, 43.6), (0, 45),
                             (-10.9, 43.6), (-31.8, 31.8)):
                c.create_oval(x + (ca * dx - sa * dy) * self.scale - 2,
                              y - (sa * dx + ca * dy) * self.scale - 2,
                              x + (ca * dx - sa * dy) * self.scale + 2,
                              y - (sa * dx + ca * dy) * self.scale + 2,
                              fill="#8b949e", outline="")
            self.draw_sensors(st["dn"])
            self.info.config(
                text=("x   %8.1f mm\n"
                      "y   %8.1f mm\n"
                      "th  %8.2f deg\n"
                      "\n"
                      "pwm  %4.0f %4.0f\n"
                      "ms   %8.0f\n"
                      "state %s\n"
                      "link  %s %.1fs\n"
                      "pts   %d")
                % (st["x"], st["y"], math.degrees(st["th"]), st["pwmL"],
                   st["pwmR"], st["ms"],
                   STATE_NAMES.get(st["sig"], str(st["sig"])), status, age,
                   sum(len(s) for s in segs)))
            if age > STALL_TIMEOUT:
                c.create_text(self.w / 2, 24, text="NO TELEMETRY",
                              fill="#f85149", font=("Consolas", 14))
        else:
            self.info.config(text="waiting for\ntelemetry...\n\nlink  %s" % status)

        c.create_text(8, 8, anchor="nw",
                      text="zoom %.2fx  (wheel; double-click = fit)" % self.scale,
                      fill="#484f58", font=("Consolas", 9))
        if self.loaded:
            c.create_text(8, 24, anchor="nw", text="loaded %s" % self.loaded,
                          fill="#3fb950", font=("Consolas", 8))
        self.log.config(text="\n".join(events[-6:]))


if __name__ == "__main__":
    link = Link()
    link.start()
    root = tk.Tk()
    App(root, link)
    root.mainloop()
