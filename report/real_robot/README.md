# Real robot tests

Uploadable firmware and host scripts that measure the actual robot. Nothing here
simulates a robot and no number is filled in for you: every value is printed by
the robot and read back from the CSV.

The digital twin work stays in `../group` and `../pipeline`. This folder is only
for hardware.

## What is in here

```
firmware/ImageRaster/   the working scan firmware, copied unmodified from
                        github.com/saintbridgethefirst/IMAGERASTER.
                        Includes ScanLog, WebDashboard and dashboard_html.h,
                        so the WiFi dashboard works as it does in that repo.
firmware/SensorTests/   characterisation tests, template files plus a new Controller
host/capture_serial.py  USB serial to CSV
host/analyse.py         offset, noise, step, edge, straight and spin analysis
host/score_scan.py      scores a dashboard or twin CSV against the sheet
```

Two sketches, two jobs. `ImageRaster` is the robot as you built it: it scans,
logs to a ring buffer, and serves the dashboard on port 8080 so a browser can
download the CSV and rebuild the image. `SensorTests` is for the individual
report: offset, noise, step, edge, straight and spin, printed over USB.

I did not write or modify the dashboard. `ImageRaster/` is a byte-for-byte copy
of your firmware, and the dashboard in it is the one you wrote. Rebuild
`dashboard_html.h` from `dashboard/dashboard.html` with
`python3 dashboard/make_header.py` if you edit the page.

## Upload

Both sketches use the same board settings.

1. Arduino IDE with the M5Stack board package, `LSM6` and `LIS3MDL` libraries.
2. Open the sketch. `firmware/ImageRaster/ImageRaster.ino` or
   `firmware/SensorTests/SensorTests.ino`. Each folder and its `.ino` already
   share a name, which Arduino requires.
3. Tools, Board, M5StampC3. Tools, USB-CDC On Boot, Disabled. Upload speed 115200.
4. Connect USB-C and upload.

For the dashboard, set your own WiFi name and password in `ImageRaster.ino`,
upload, join that network, and open `http://192.168.4.1:8080`.

## Get a scan off the robot, two ways

Dashboard, the way you already work:

```sh
# join the robot's WiFi, then
curl -s "http://192.168.4.1:8080/data" > data/scan_raw.csv
python3 host/score_scan.py data/scan_raw.csv --sheet surface.png
```

USB, for the SensorTests sketch:

```sh
python3 host/capture_serial.py --port /dev/ttyACM0 --out data/offset.csv
```

## Choose a test

Press the button on the StampC3 N times, then wait 1.5 seconds. The robot beeps N
times and the test starts. Press once to stop. The beep count is the test number.

| N | test | what it does |
|---|---|---|
| 1 | offset | stationary, 200 snapshots on one surface |
| 2 | noise | stationary, 2000 snapshots |
| 3 | step | stationary stream, change surface between trials |
| 4 | edge | drives straight at PWM 30 for 4 s |
| 5 | straight | drives straight at PWM 40 for 3 s |
| 6 | spin | rotates on the spot at PWM 35 for 2 s |

Every row is `mode,trial,sample,t_ms,x_mm,y_mm,theta_rad,dn1..dn5`. Readings are
discharge times in microseconds at 10 ms, logged at 50 Hz while moving.

## Capture

```sh
pip install pyserial
python3 host/capture_serial.py --port /dev/ttyACM0 --out data/offset.csv
```

On Windows the port is `COM5` or similar. Ctrl-C ends the capture.

## Analyse

```sh
python3 host/analyse.py offset  data/offset.csv    # five baselines, IQR, spread
python3 host/analyse.py noise   data/noise.csv     # same, plus sd
python3 host/analyse.py step    data/step.csv      # one trial per surface
python3 host/analyse.py edge    data/edge.csv      # 10-90 % width per channel
python3 host/analyse.py straight data/straight.csv # odometry distance
python3 host/analyse.py spin    data/spin.csv      # odometry heading
```

## The experiments these support

**Offset.** Place the robot on white paper, run test 1 three times with a lift and
replace between trials. The per-channel medians are the baselines. If they differ
by more than the IQR, the channels need individual levels in the twin.

**Noise.** Test 2, same position. The sd per channel is the noise the twin sets
to zero.

**Saturation and gain.** Print a strip of grey patches. Run test 3 once per patch,
stopping between patches. Trial number identifies the patch, so keep a note of the
order. Plot median against patch: a flat top is saturation, the slope is gain.

**Footprint.** Print a solid black band on white paper. Run test 4 three times at
different distances from the band so the robot crosses it at different speeds.
`analyse.py edge` gives the 10-90 % width per channel. Width constant across
speeds means the spot; width growing with speed means the sensor delay.

**Wheel geometry.** Test 5: measure the real distance with a rule. The odometry
distance is printed. New radius is 16 mm times real over odometry. Test 6 does the
same for separation from the heading change.

## Known limits

The spin test rotates against the floor, so wheel slip is included in the result.
That is what the real robot does, so it belongs in the measurement, but it means
the number is an effective separation and not a geometry measurement.

Test 4 and 5 drive open loop at fixed PWM. If the robot veers, the run still
gives a valid edge width but the distance will be short.
