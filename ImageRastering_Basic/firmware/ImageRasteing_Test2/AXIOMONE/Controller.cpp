#include "HardwareSerial.h"
#include "Controller.h"

const unsigned long Controller_c::UPDATE_INTERVAL_MS;
const unsigned long Controller_c::TELEMETRY_INTERVAL_MS;
const unsigned long Controller_c::RECOVERY_TIMEOUT_MS;
const uint16_t Controller_c::LINE_THRESHOLD;
constexpr float Controller_c::BASE_PWM;
constexpr float Controller_c::LINE_FOLLOW_GAIN;
constexpr float Controller_c::RECOVERY_LEFT_PWM;
constexpr float Controller_c::RECOVERY_RIGHT_PWM;
constexpr float Controller_c::PIVOT_PWM; // alongside your other definitions

Controller_c::Controller_c()
    : update_timer(UPDATE_INTERVAL_MS),
      telemetry_timer(TELEMETRY_INTERVAL_MS)
{
  reset();
}

void Controller_c::reset()
{
  signal = 0;
  mode = WAITING;
  recovery_start_ms = 0;
  last_error = 0;
  last_side = 1;

  // Rastering
  raster_state = R_INIT;
  raster_row = 0;
  last_surface_ts = 0;
}

uint8_t Controller_c::getSignal() const
{
  return signal;
}

void Controller_c::setSignal(uint8_t requested_signal)
{
  if (requested_signal == 0)
  {
    reset();
    return;
  }

  if (signal == 0)
  {
    signal = 1;
    mode = FOLLOWING_LINE;
    recovery_start_ms = 0;
  }
}

void Controller_c::update(Robot_c &robot, RobotWifiAP_c &server)
{

  // Get the current count of milliseconds since the StampC3 was powered on.
  unsigned long now = robot.getMillis();

  // Check if our TaskTimer_c for the general controller update indicates
  // that it is the correct time to operate.  Note, we give the TaskTimer_c
  // the current count in milliseconds.
  // We do this because we don't want to query the robot faster than at 10ms
  // intervals.
  if (!update_timer.isReady(now))
  {

    // TaskTimer_c says it is not ready, so we simply end this call to
    // the controller here by calling return.
    return;
  }

  // The TaskTimer_c was ready, so we now reset the TaskTimer_c so it
  // begins counting again.  We now run the remaining controller code.
  update_timer.resetTimer(now);

  // Ask the robot for latest information on surface reflectance sensors.
  robot.getSurfaceSensors();

  // Ask the robot for the latest encoder information, and then use this
  // to update the odometry on board the StampC3 (see OdometryModel.h).
  if (robot.getEncoders())
  {
    odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
  }

  // Pose estimation from the Pololu 3Pi robot itself.
  robot.getPose();

  // While waiting, a button press sets signal to 1 and starts the physical
  // robot's line follower, even without Processing connected. The transmitted
  // signal also starts an armed Digital Twin live comparison. This is a start
  // control, not a start/stop toggle; further presses are ignored while signal is 1.
  if (signal == 0)
  {
    if (robot.isButtonPressed())
    {
      setSignal(1);
    }
  }

  // The user has started the demonstration, so we run the line following
  // controller code.
  if (signal == 1)
  {
    // runLineFollower(robot, now);
    // runPathFollower
    // runPathFollower(robot, now);
    testRaster(robot, server, now);
  }

  // Use another TaskTimer_c to limit how often we transmit telemetry data
  // on WiFi and Serial.
  if (telemetry_timer.isReady(now))
  {
    telemetry_timer.resetTimer(now);
    // publishTelemetry(robot, server, now);
  }
}

bool Controller_c::lineDetected(const Robot_c &robot) const
{
  // Note: only using the central sensor to detect a line.
  if (robot.surface.reading[2] >= LINE_THRESHOLD)
  {
    return true;
  }

  return false;
}

bool Controller_c::pathDetected(const Robot_c &robot) const
{
  // Note: only using the central sensor to detect a line.
  bool ok =
      (robot.surface.reading[1] >= LINE_THRESHOLD && robot.surface.reading[2] >= LINE_THRESHOLD) ||
      robot.surface.reading[2] >= LINE_THRESHOLD ||
      (robot.surface.reading[3] >= LINE_THRESHOLD && robot.surface.reading[2] >= LINE_THRESHOLD);

  if (robot.surface.reading[2] >= LINE_THRESHOLD)
  {
    return true;
  }

  return false;
}

void Controller_c::testRaster(Robot_c &robot, RobotWifiAP_c &server, unsigned long now)
{
  if (raster_state == R_DONE)
    return;

  robot.getMotionStatus();

  // Log first, so every fresh sample is tagged with the state it was taken in.
  logRasterSample(robot, server);

  // Initial kick-off
  if (raster_state == R_INIT)
  {
    if (robot.motion_status.state == MOTION_STATUS_IDLE)
    {
      // Use your calibrated values here
      // robot.setMotionHelperGeometry(15.8f, 15.8f, 85.0f);
      robot.setMotionHelperGeometry(32.0f/2, 32.0f/2, 84.0f);
      raster_row = 0;
      raster_state = R_FORWARD;
      robot.startMoveDistance(RASTER_LENGTH_MM);
    }
    return;
  }

  // Only act when the current motion has finished
  if (robot.motion_status.state != MOTION_STATUS_COMPLETE)
    return;

  // Alternate turn direction every row: +PI/2 (left) on even rows, -PI/2 (right) on odd rows
  float turn = (raster_row % 2 == 0) ? (PI / 2.0f) : (-PI / 2.0f);

  switch (raster_state)
  {
  case R_FORWARD:
    if (raster_row >= RASTER_ROWS - 1)
    { // last row: no more lanes
      robot.setMotorPWM(0, 0);
      setSignal(0);
      raster_state = R_DONE;
      return;
    }
    raster_state = R_TURN_A;
    robot.startRotateAngle(turn);
    break;

  case R_TURN_A:
    raster_state = R_SHIFT;
    robot.startMoveDistance(RASTER_LANE_MM);
    break;

  case R_SHIFT:
    raster_state = R_TURN_B;
    robot.startRotateAngle(turn);
    break;

  case R_TURN_B:
    raster_row++;
    raster_state = R_FORWARD;
    robot.startMoveDistance(RASTER_LENGTH_MM);
    break;
  }
}


void Controller_c::logRasterSample(Robot_c &robot, RobotWifiAP_c &server) {
  if (robot.surface.timestamp_us == last_surface_ts) return;   // cached, already logged
  last_surface_ts = robot.surface.timestamp_us;

  // test_name, row, state, timestamp_us, x, y, theta, r0..r4
  server.printf("raster,%d,%d,%lu,%.2f,%.2f,%.4f,%u,%u,%u,%u,%u\n",
    raster_row, raster_state,
    (unsigned long)robot.surface.timestamp_us,
    odometry.pose.x, odometry.pose.y, odometry.pose.theta,
    robot.surface.reading[0], robot.surface.reading[1], robot.surface.reading[2],
    robot.surface.reading[3], robot.surface.reading[4]);
}

void Controller_c::printTest(Robot_c &robot)
{
  Serial.print(robot.surface.reading[0]);
  Serial.print(",");
  Serial.print(robot.surface.reading[1]);
  Serial.print(",");
  Serial.print(robot.surface.reading[2]);
  Serial.print(",");
  Serial.print(robot.surface.reading[3]);
  Serial.print(",");
  Serial.print(robot.surface.reading[4]);
  Serial.print("\n");
}

void Controller_c::runLineFollower(Robot_c &robot, unsigned long now)
{
  bool line_detected = lineDetected(robot);

  if (print_test_timer.isReady(now))
  {
    printTest(robot);
    print_test_timer.resetTimer();
  }

  if (!line_detected && mode == FOLLOWING_LINE)
  {
    mode = SEARCHING_FOR_LINE;
    recovery_start_ms = now;
  }

  if (mode == SEARCHING_FOR_LINE)
  {
    if (line_detected)
    {
      mode = FOLLOWING_LINE;
    }
    else if (now - recovery_start_ms >= RECOVERY_TIMEOUT_MS)
    {
      mode = STOPPED;
    }
  }

  if (mode == STOPPED)
  {
    robot.setMotorPWM(0, 0);
    return;
  }

  if (mode == SEARCHING_FOR_LINE)
  {
    // robot.setMotorPWM(RECOVERY_LEFT_PWM, RECOVERY_RIGHT_PWM);

    robot.setMotorPWM(RECOVERY_RIGHT_PWM, RECOVERY_LEFT_PWM);
    return;
  }

  float error = (float)robot.surface.reading[1] - (float)robot.surface.reading[3];
  // float error = (float)robot.surface.reading[3] - (float)robot.surface.reading[1];
  float turn = error * LINE_FOLLOW_GAIN;

  robot.setMotorPWM(BASE_PWM - turn, BASE_PWM + turn);
  // robot.setMotorPWM(BASE_PWM + turn, BASE_PWM - turn);
}

void Controller_c::runPathFollower(Robot_c &robot, unsigned long now)
{
  // Lateral offset of each sensor from the robot centreline, in mm (+ = left).
  // Estimated from the bottom-view picture; adjust if you measure them.
  static const float X_MM[5] = {+31.0f, +8.0f, 0.0f, -7.0f, -28.0f};

  float r[5];
  bool on[5];
  float sum = 0;
  for (int i = 0; i < 5; i++)
  {
    r[i] = (float)robot.surface.reading[i];
    on[i] = r[i] >= LINE_THRESHOLD;
    sum += r[i];
  }
  bool any_line = on[0] || on[1] || on[2] || on[3] || on[4];
  bool centre_on = on[1] || on[2] || on[3];

  if (print_test_timer.isReady(now))
  {
    printTest(robot);
    print_test_timer.resetTimer(now);
  }

  // ---- state transitions -------------------------------------------------
  if (!any_line && mode == FOLLOWING_LINE)
  {
    mode = SEARCHING_FOR_LINE;
    recovery_start_ms = now;
  }
  if (mode == SEARCHING_FOR_LINE)
  {
    if (any_line)
    {
      mode = FOLLOWING_LINE;
    }
    else if (now - recovery_start_ms >= RECOVERY_TIMEOUT_MS)
    {
      mode = STOPPED;
    }
  }

  if (mode == STOPPED)
  {
    robot.setMotorPWM(0, 0);
    return;
  }

  // ---- lost: pivot towards where the line was last seen ------------------
  if (mode == SEARCHING_FOR_LINE)
  {
    if (last_side > 0)
      robot.setMotorPWM(-PIVOT_PWM, PIVOT_PWM); // spin left
    else
      robot.setMotorPWM(PIVOT_PWM, -PIVOT_PWM); // spin right
    return;
  }

  // ---- sharp corners: middle trio has run out of line, outer sensor sees it
  if (!centre_on && on[0] && !on[4])
  {
    last_side = +1;
    robot.setMotorPWM(-PIVOT_PWM, PIVOT_PWM); // pivot left
    return;
  }
  if (!centre_on && on[4] && !on[0])
  {
    last_side = -1;
    robot.setMotorPWM(PIVOT_PWM, -PIVOT_PWM); // pivot right
    return;
  }

  // ---- normal following: weighted centroid in mm -------------------------
  float error = 0;
  for (int i = 0; i < 5; i++)
    error += X_MM[i] * r[i];
  error /= sum; // roughly -28 .. +31 mm, + = line on left

  last_error = error;
  if (error > 3.0f)
    last_side = +1;
  else if (error < -3.0f)
    last_side = -1;

  float turn = error * LINE_FOLLOW_GAIN; // PWM per mm
  robot.setMotorPWM(BASE_PWM - turn, BASE_PWM + turn);
}

void Controller_c::publishTelemetry(Robot_c &robot, RobotWifiAP_c &server, unsigned long timestamp_ms)
{
  server.printf(
      "%lu,%.2f,%.2f,%.5f,%d,%d,%ld,%ld,%u,%u,%u,%u,%u,%u\n",
      timestamp_ms,
      odometry.pose.x,
      odometry.pose.y,
      odometry.pose.theta,
      robot.getLeftMotorPWM(),
      robot.getRightMotorPWM(),
      (long)robot.getLeftEncoderCount(),
      (long)robot.getRightEncoderCount(),
      robot.surface.reading[0],
      robot.surface.reading[1],
      robot.surface.reading[2],
      robot.surface.reading[3],
      robot.surface.reading[4],
      signal);
  if (Serial)
  { // If serial is connected
    Serial.printf(
        "%lu,%.2f,%.2f,%.5f,%d,%d,%ld,%ld,%u,%u,%u,%u,%u,%u\n",
        timestamp_ms,
        odometry.pose.x,
        odometry.pose.y,
        odometry.pose.theta,
        robot.getLeftMotorPWM(),
        robot.getRightMotorPWM(),
        (long)robot.getLeftEncoderCount(),
        (long)robot.getRightEncoderCount(),
        robot.surface.reading[0],
        robot.surface.reading[1],
        robot.surface.reading[2],
        robot.surface.reading[3],
        robot.surface.reading[4],
        signal);
  }
}
