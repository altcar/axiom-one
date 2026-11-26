/*
 * Image rasterisation for the Pololu 3Pi+ with an M5Stack StampC3.
 *
 * The robot drives back and forth over a printed image in rows, logging its
 * odometry pose and the five surface sensors. A web dashboard served by the
 * StampC3 rebuilds the image live.
 *
 *   1. Change the WiFi name below so it is unique in the lab.
 *   2. Upload. Place the robot at the start corner of the image (see Controller.h).
 *   3. Join the robot's WiFi on your laptop or phone and open
 *        http://192.168.4.1:8080
 *   4. Press Start on the dashboard, or press the StampC3 button.
 *      Pressing the button again while it runs stops the robot.
 */
#include <Wire.h>

#include "RobotWifiAP.h"

// WiFi name (make it unique) and password (at least 8 characters).
// Port 80 is the template's TCP telemetry; the dashboard uses port 8080.
RobotWifiAP_c server("RasterBot-1", "change-me-123", 80);

#include "Robot.h"
#include "RobotIMU.h"
#include "Controller.h"
#include "WebDashboard.h"

/** @brief Hardware-facing interface to the Pololu robot. */
Robot_c robot;
/** @brief Direct interface to the 3Pi+ accelerometer, gyro, and magnetometer. */
RobotIMU_c imu;
/** @brief Raster-scan controller (Controller.cpp). */
Controller_c controller;
/** @brief Web page + data server for the dashboard. */
WebDashboard_c dashboard(robot, controller, 8080);

/** Ring-buffer size (each sample is 28 bytes; 5000 = 140 KB). With the dashboard
 *  open, samples are downloaded during the scan, so scans can be longer than this. */
static const uint32_t SCAN_LOG_SAMPLES = 5000;
/** Heap left free for WiFi and the web server after the log is allocated. */
static const uint32_t HEAP_FOR_WIFI_BYTES = 110000;

void setup() {
  Serial.begin(115200);

  // Reserve the scan log first, while memory is still in one big piece.
  // (Allocating it after WiFi starts only found room for ~2500 samples.)
  uint32_t capacity = controller.scan_log.begin(SCAN_LOG_SAMPLES, HEAP_FOR_WIFI_BYTES);
  bool wifi_ok = server.begin();
  if (!wifi_ok) {
    // Give WiFi priority: free the log, start WiFi, then take what's left.
    controller.scan_log.release();
    wifi_ok = server.begin();
    capacity = controller.scan_log.begin(SCAN_LOG_SAMPLES, 40000);
  }

  robot.initialise();
  // robot.initialise("team name");   // use this instead if your unit asks for it

  if (!imu.initialise()) {
    Serial.println("Warning: LSM6DS33 IMU was not detected.");
    robot.setLED(90, 50, 0, 100);  // orange warning
  }

  dashboard.begin();

  delay(500);  // give the USB serial port a moment
  Serial.println();
  Serial.println("=== Image raster scanner ===");
  if (!wifi_ok) {
    Serial.println("WiFi access point FAILED to start (password must be 8+ characters).");
    robot.setLED(255, 0, 0, 60);  // red
  } else {
    Serial.print("Join WiFi and open:  http://");
    Serial.print(server.ip());
    Serial.println(":8080");
  }
  Serial.printf("Scan buffer %lu samples (%.0f s without the dashboard open). Free heap %lu bytes.\n",
                (unsigned long)capacity,
                capacity * Controller_c::LOG_INTERVAL_MS / 1000.0f,
                (unsigned long)ESP.getFreeHeap());
}

void loop() {
  server.update();                    // template TCP telemetry connection
  dashboard.update();                 // web page requests
  controller.update(robot, server);   // sense -> raster control -> log
}
