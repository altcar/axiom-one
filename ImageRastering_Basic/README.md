# Image Rastering

The Pololu 3pi+ 32U4 drives parallel rows across a printed target. At each row end it turns 90°, drives across one row spacing, then turns 90° again to scan in the opposite direction. The five downward sensors record the surface about every 2 mm along each row.

Wheel encoders measure travel. The Z gyroscope measures turns, and encoder heading gradually corrects gyro drift. The controller uses the combined heading to steer and estimate where each reading was taken.

## Files

| File | Use |
| --- | --- |
| `firmware/ImageRastering/ImageRastering.ino` | Start the StampC3, robot interface, IMU and Wi-Fi. |
| `Config.h` | Set scan dimensions, motor PWM, wheel geometry and Wi-Fi in the sketch folder. |
| `Controller.h/.cpp` and `RasterLine.h` | Drive the rows, turns and row changes, check faults and log fresh samples. |
| `GyroHeading.h` and `FusedOdometry.h` | Estimate gyro bias at rest, integrate turns and combine gyro and encoder heading. |
| `build.sh` | Compile the sketch and optionally upload it through USB. |
| `record_wifi.py` | Send the heartbeat and save one scan on the computer. |
| `reconstruct.py` | Map a saved log using the position of each of the five sensors. |

## Changes to the course template

This scanner builds on the [course StampC3 template at commit 2dd5854](https://github.com/paulodowd/RoboticsScienceAndSystems/tree/2dd585408a3e0a2f389c8e56dc42f4b960d3cb71/StampC3_Template). The group modified the sketch entry point and controller for raster motion and five-sensor logging, added checked gyro reads to RobotIMU, and added queued telemetry to RobotWifiAP. Config.h, RasterLine.h, GyroHeading.h and FusedOdometry.h implement scan settings, steering and heading estimation. The course Robot, TaskTimer, OdometryModel and I2C protocol sources are preserved. The recorder, reconstruction script and build script are group additions.

See the [course GPL licence](licenses/GPL-3.0.txt) and the [Freenove LGPL licence for the LED code](licenses/LGPL-3.0-Freenove.txt).

## Set up the scan

The default is **13 rows, 200 mm per row, 5 mm between rows and 2 mm between samples**. The first and last axle paths are 60 mm apart. Sensor positions extend past these paths.

Place **DN3**, the middle sensor, on white paper about **10 mm left of the target's left edge and 10 mm below its bottom edge**. Point the robot to the right. The rows advance up the sheet. Allow room for the robot body at both ends of each row. The starting position of DN3 is the origin of the reconstructed image.

Edit `firmware/ImageRastering/Config.h` to change `ROWS`, `ROW_LENGTH_MM`, `ROW_SPACING_MM` or `SAMPLE_STEP_MM`. The tested scan uses PWM 40 along rows, PWM 35 for turns and shifts, and PWM 30 near the target. Normal turns allow 30 ms for coasting, then require 200 ms of stillness and an estimated heading within 2° of the target. The encoder model uses a 16 mm wheel radius and 84 mm wheel separation. Rebuild and upload after changing these settings.

## Compile and upload

Use Arduino CLI with Espressif's ESP32 core 3.3.12 and the course middleware already installed on the 32U4. Run these commands from the folder containing `build.sh`. Install the two IMU libraries once.

```bash
arduino-cli lib install "LSM6@2.0.1" "LIS3MDL@2.0.0"
```

Connect the StampC3 by USB. To compile only, run `bash build.sh`. To compile and upload, find the USB port, then replace the example port below with yours. The script uploads at 115200 baud.

```bash
arduino-cli board list
bash build.sh /dev/cu.usbserial-XXXX
```

## Record one scan

1. Power the robot from its battery, unplug USB and place it at the start.
2. Connect the computer to **RasterRobot** using the password **raster-demo**.
3. Run `python3 record_wifi.py`. When `ready_press_button` appears, press and release the robot button once. Keep the robot still for about three seconds while it measures gyro bias.
4. Let the robot finish. The default run ends with `# stop,reason=DONE,row=13`. The recorder prints the path to `logs/raster_<timestamp>.txt`. Pressing the button again or using Ctrl+C requests a stop. Reset the StampC3 before the next run.

## Reconstruct the image

Install the Python dependencies once, with internet access.

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
```

Replace the log name below with your saved file and choose a new output directory for each run.

```bash
.venv/bin/python reconstruct.py logs/YOUR_LOG.txt --full-extent --output results/run_01
```

Open `results/run_01/reconstruction.png`. The left panel shows individual sensor readings at their estimated positions. The right panel shows the median darkness of the readings in each 2 × 5 mm cell. Blue cells have no usable measurement. Shades show contrast scaled separately for each sensor. The original log and exported raw readings are available for comparison.

