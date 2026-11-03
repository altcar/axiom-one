#include <Wire.h>

#include "RobotWifiAP.h"
#include "Config.h"

/** @brief WiFi access-point and TCP server required for raster telemetry. */
RobotWifiAP_c server(ScanConfig::WIFI_NAME, ScanConfig::WIFI_PASSWORD,
                     ScanConfig::TCP_PORT);

#include "Robot.h"
#include "Controller.h"


/** @brief Hardware-facing interface to the Pololu robot. */
Robot_c robot;
/** @brief Fixed-spacing raster controller. */
Controller_c controller;


/** @brief Initialises serial, hardware and WiFi for a stationary raster start. */
void setup() {

  Serial.begin(115200);

  robot.initialise();
//  robot.initialise("team name");

  // Application-specific limit; leave the course hardware interface unchanged.
  Wire.setTimeOut(5);
  robot.setMotorPWM(0, 0);
  if (!server.begin()) {
    Serial.println("Wi-Fi startup failed; motors remain stopped.");
  }

  // Start the gyro used with the encoders for scan heading.
  if (!controller.initialiseIMU()) {
    Serial.println("IMU startup failed; scanning cannot start. Reset to retry.");
  }

}


/** @brief Maintains telemetry transport and runs the controller repeatedly. */
void loop() {
  // Accept or maintain the raster recorder connection and drain queued data.
  // An active raster stops if the recorder heartbeat is lost.
  server.update();

  
  // Run an iteration of the robot controller code.
  // Develop controller routines in Controller.cpp; edit parameters in Config.h.
  controller.update(robot, server);

}
