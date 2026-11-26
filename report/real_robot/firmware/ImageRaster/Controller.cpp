#include "Controller.h"

Controller_c::Controller_c()
  : update_timer(UPDATE_INTERVAL_MS),
    telemetry_timer(TELEMETRY_INTERVAL_MS),
    log_timer(LOG_INTERVAL_MS) {

  // ---- Default scan settings: a ~160 x 130 mm patch ------------------------
  // These are starting values. Change them from the dashboard, or here.
  // Pitch 7 mm = 3/4 of the ~9 mm DN2-DN3-DN4 spacing, so the middle sensor
  // tracks of neighbouring rows interleave every ~2.3 mm.
  config.pattern = PATTERN_BACK_AND_FORTH;
  config.margin_mm = 50.0f;
  config.rows = 17;
  config.row_length_mm = 160.0f;
  config.row_pitch_mm = 7.0f;
  config.scan_pwm = 30.0f;
  config.turn_pwm = 30.0f;
  config.turn_min_pwm = 24.0f;
  config.side = 1;
  config.sensor_lead_mm = 40.0f;         // DN3 ahead of the axle (3Pi+ approx.)
  config.wheel_radius_left_mm = 16.0f;   // template nominal value
  config.wheel_radius_right_mm = 16.0f;  // template nominal value
  config.wheel_separation_mm = 82.5f;    // calibrated on our robot (template: 90)

  run_id = 0;
  log_tick = 0;
  start_requested = false;
  stop_requested = false;
  message = "Ready";
  reset();
}

void Controller_c::reset() {
  signal = 0;
  mode = WAITING;
  next_mode = WAITING;
  mode_start_ms = 0;
  settle_ms = 0;
  row = 0;
  target_heading = 0.0f;
  segment_start_x = 0.0f;
  segment_start_y = 0.0f;
  pass_dir = 1;
  row_y = 0.0f;
  turn_boost = 0.0f;
  turn_check_theta = 0.0f;
  turn_check_ms = 0;
}

uint8_t Controller_c::getSignal() const {
  return signal;
}

void Controller_c::setSignal(uint8_t requested_signal) {
  if (requested_signal == 0) {
    reset();
    return;
  }
  if (signal == 0) {
    requestStart();
  }
}

void Controller_c::requestStart() {
  start_requested = true;
}

void Controller_c::requestStop() {
  stop_requested = true;
}

bool Controller_c::isRunning() const {
  return mode != WAITING && mode != FINISHED && mode != STOPPED;
}

Controller_c::Mode Controller_c::getMode() const {
  return mode;
}

const char *Controller_c::getModeName() const {
  switch (mode) {
    case WAITING:      return "WAITING";
    case SETTLING:     return "SETTLING";
    case SCANNING_ROW: return "SCANNING_ROW";
    case TURNING_OUT:  return "TURNING_OUT";
    case SHIFTING:     return "SHIFTING";
    case TURNING_IN:   return "TURNING_IN";
    case FINISHED:     return "FINISHED";
    case STOPPED:      return "STOPPED";
    case OVERRUN:      return "OVERRUN";
    case MARGIN:       return "MARGIN";
  }
  return "UNKNOWN";
}

uint8_t Controller_c::getRow() const {
  return row;
}

uint32_t Controller_c::getRunId() const {
  return run_id;
}

const char *Controller_c::getMessage() const {
  return message;
}

bool Controller_c::setConfig(const RasterConfig_t &c) {
  if (isRunning()) {
    return false;
  }
  // Reject values that would make the robot do something silly.
  if (c.rows < 1 || c.rows > 150) return false;
  if (!(c.row_length_mm >= 20.0f && c.row_length_mm <= 1500.0f)) return false;
  if (!(c.row_pitch_mm >= 2.0f && c.row_pitch_mm <= 300.0f)) return false;
  if (!(c.scan_pwm >= 5.0f && c.scan_pwm <= 120.0f)) return false;
  if (!(c.turn_pwm >= 5.0f && c.turn_pwm <= 120.0f)) return false;
  if (!(c.turn_min_pwm >= 0.0f && c.turn_min_pwm <= c.turn_pwm)) return false;
  if (c.side != 1 && c.side != -1) return false;
  if (c.pattern != PATTERN_BACK_AND_FORTH && c.pattern != PATTERN_UTURN) return false;
  if (!(c.margin_mm >= 0.0f && c.margin_mm <= 200.0f)) return false;
  if (!(c.sensor_lead_mm >= 0.0f && c.sensor_lead_mm <= 80.0f)) return false;
  if (!(c.wheel_radius_left_mm >= 5.0f && c.wheel_radius_left_mm <= 50.0f)) return false;
  if (!(c.wheel_radius_right_mm >= 5.0f && c.wheel_radius_right_mm <= 50.0f)) return false;
  if (!(c.wheel_separation_mm >= 20.0f && c.wheel_separation_mm <= 200.0f)) return false;
  config = c;
  message = "Settings updated";
  return true;
}

bool Controller_c::clearLog() {
  if (isRunning()) {
    return false;
  }
  scan_log.clear();
  message = "Log cleared";
  return true;
}

void Controller_c::update(Robot_c &robot, RobotWifiAP_c &server) {
  unsigned long now = robot.getMillis();

  // Run at most every UPDATE_INTERVAL_MS (10 ms), same as the template.
  if (!update_timer.isReady(now)) {
    return;
  }
  update_timer.resetTimer(now);

  // 1) Sense: surface sensors, encoders -> local odometry, middleware pose.
  robot.getSurfaceSensors();
  if (robot.getEncoders()) {
    odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
  }
  robot.getPose();

  // 2) StampC3 button: starts a scan when idle, stops it while running.
  if (robot.isButtonPressed()) {
    if (isRunning()) {
      stop_requested = true;
    } else {
      start_requested = true;
    }
  }

  // 3) Requests from the button or the dashboard.
  if (stop_requested) {
    stop_requested = false;
    start_requested = false;
    if (isRunning()) {
      robot.setMotorPWM(0, 0);
      mode = STOPPED;
      signal = 0;  // telemetry: 1 only while scanning (the Digital Twin starts/ends on it)
      message = "Stopped by user";
    }
  }
  if (start_requested) {
    start_requested = false;
    if (!isRunning()) {
      beginScan(robot, now);
    }
  }

  // 4) Act: run the raster state machine.
  runRaster(robot, now);

  // 5) Log for the dashboard while a scan is running.
  // Rows, overruns and shifts are logged at full rate. Turns and run-ins only
  // feed the path drawing, so they are logged at half rate, and pauses not at all.
  if (log_timer.isReady(now)) {
    log_timer.resetTimer(now);
    if (isRunning() && mode != SETTLING) {
      bool turning = (mode == TURNING_OUT || mode == TURNING_IN || mode == MARGIN);
      log_tick++;
      if (!turning || (log_tick % 2) == 0) {
        logSample(robot, now);
      }
    }
  }

  // 6) Template telemetry (unchanged 14-column CSV for Serial / Digital Twin).
  if (telemetry_timer.isReady(now)) {
    telemetry_timer.resetTimer(now);
    publishTelemetry(robot, server, now);
  }
}

void Controller_c::beginScan(Robot_c &robot, unsigned long now) {
  robot.setMotorPWM(0, 0);

  // Use the calibrated geometry, then zero the pose while standing still.
  odometry.setLeftWheelRadiusMm(config.wheel_radius_left_mm);
  odometry.setRightWheelRadiusMm(config.wheel_radius_right_mm);
  odometry.wheelSeparationMm = config.wheel_separation_mm;
  odometry.reset(0.0f, 0.0f, 0.0f);

  scan_log.clear();
  run_id++;
  signal = 1;
  row = 0;
  row_y = 0.0f;
  pass_dir = 1;
  target_heading = rowHeading(0);
  message = "Scanning";
  robot.playTone(1200, 80);

  // Wait a moment so the hand that pressed the button can move away.
  settleThen(robot, now, START_DELAY_MS, SCANNING_ROW);
}

void Controller_c::runRaster(Robot_c &robot, unsigned long now) {
  switch (mode) {

    case WAITING:
      return;  // like the template: no motor commands until started

    case FINISHED:
    case STOPPED:
      robot.setMotorPWM(0, 0);  // keep insisting on stopped
      return;

    case SETTLING:
      robot.setMotorPWM(0, 0);
      if (now - mode_start_ms >= settle_ms) {
        enterMode(next_mode, now);
      }
      return;

    case MARGIN:
      runBackAndForth(robot, now);
      return;

    case SCANNING_ROW:
    case OVERRUN: {
      if (config.pattern == PATTERN_BACK_AND_FORTH) {
        runBackAndForth(robot, now);
        return;
      }
      float travelled = distanceFromSegmentStart();
      if (mode == SCANNING_ROW && travelled >= config.row_length_mm) {
        if (row + 1 >= config.rows) {
          finish(robot, "Scan complete");
          return;
        }
        // Keep driving (same segment, no stop) so the sensors on the next,
        // reversed row line up with this one. See RasterConfig_t.
        mode = OVERRUN;
      }
      if (mode == OVERRUN &&
          travelled >= config.row_length_mm + 2.0f * config.sensor_lead_mm) {
        endOfRow(robot, now);
        return;
      }
      if (now - mode_start_ms > STRAIGHT_TIMEOUT_MS) {
        fail(robot, "Row timed out: robot barely moving? Raise scan PWM");
        return;
      }
      // Hold the row line (not just the heading), so turn errors don't
      // shift the whole row sideways.
      driveOnRowLine(robot, rowHeading(row), 1, now);
      return;
    }

    case TURNING_OUT:
    case TURNING_IN: {
      float err = headingError();
      bool aligned = fabsf(err) < TURN_TOLERANCE_RAD;
      bool timed_out = (now - mode_start_ms) > TURN_TIMEOUT_MS;
      if (aligned || timed_out) {
        if (timed_out) {
          message = "Turn timed out: raise turn min PWM (deadband?)";
        }
        if (mode == TURNING_OUT) {
          settleThen(robot, now, SETTLE_MS, SHIFTING);
        } else {
          row++;
          row_y = config.side * row * config.row_pitch_mm;
          settleThen(robot, now, SETTLE_MS, SCANNING_ROW);
        }
        return;
      }
      turnInPlace(robot, err, now);
      return;
    }

    case SHIFTING: {
      // Shift until the robot reaches the next row line, measured from the
      // start of the scan, so sideways creep during turns doesn't add up.
      float next_line_y = config.side * (row + 1) * config.row_pitch_mm;
      bool reached = (config.side > 0) ? (odometry.pose.y >= next_line_y)
                                       : (odometry.pose.y <= next_line_y);
      if (reached) {
        target_heading = rowHeading(row + 1);
        settleThen(robot, now, SETTLE_MS, TURNING_IN);
        return;
      }
      if (now - mode_start_ms > STRAIGHT_TIMEOUT_MS) {
        fail(robot, "Shift timed out: robot barely moving? Raise scan PWM");
        return;
      }
      driveStraight(robot, config.scan_pwm, now);
      return;
    }
  }
}

void Controller_c::endOfRow(Robot_c &robot, unsigned long now) {
  // Face sideways (+y for side=+1, -y for side=-1), then shift to the next row.
  target_heading = config.side * HALF_PI;
  settleThen(robot, now, SETTLE_MS, TURNING_OUT);
}

void Controller_c::runBackAndForth(Robot_c &robot, unsigned long now) {
  // Rows alternate direction without turning: even rows forwards (+x), odd
  // rows in reverse (-x). The axle travels from -margin to length + margin;
  // only the part where it is between 0 and length is the image area.
  const float x = odometry.pose.x;
  const float L = config.row_length_mm;
  const float m = config.margin_mm;
  const bool last = (row + 1 >= config.rows);

  // The last row ends at the image edge; the others continue through the margin.
  const bool done = (pass_dir > 0) ? (x >= (last ? L : L + m))
                                   : (x <= (last ? 0.0f : -m));
  if (done) {
    if (last) {
      finish(robot, "Scan complete");
      return;
    }
    row++;
    pass_dir = -pass_dir;
    row_y = config.side * row * config.row_pitch_mm;
    settleThen(robot, now, SETTLE_MS, MARGIN);
    return;
  }

  if (now - mode_start_ms > STRAIGHT_TIMEOUT_MS) {
    fail(robot, "Row timed out: robot barely moving? Raise scan PWM");
    return;
  }

  // Label the zone (for logging and the dashboard) without restarting the
  // soft-start ramp or the timeout.
  mode = (x >= 0.0f && x <= L) ? SCANNING_ROW : MARGIN;

  driveOnRowLine(robot, 0.0f, pass_dir, now);
}

void Controller_c::driveOnRowLine(Robot_c &robot, float line_heading, int8_t dir, unsigned long now) {
  // Soft start: ramp the PWM up over RAMP_MS to reduce wheel slip.
  float ramp = (float)(now - mode_start_ms) / (float)RAMP_MS;
  if (ramp > 1.0f) ramp = 1.0f;
  float base = config.scan_pwm * ramp * (float)dir;

  // Steer towards the row line y = row_y. For a line along +x (heading 0)
  // driven forwards, being below the line (e > 0) needs a left turn; driving
  // in reverse, or along -x, flips the sign.
  float e = row_y - odometry.pose.y;
  float c = cosf(line_heading) >= 0.0f ? 1.0f : -1.0f;
  float steer = atanf(e / LINE_LOOKAHEAD_MM);
  steer = constrain(steer, -MAX_LINE_ANGLE_RAD, MAX_LINE_ANGLE_RAD);
  target_heading = line_heading + (float)dir * c * steer;

  // Heading hold. Rotation depends only on (right - left), so the same sign
  // works forwards and in reverse. Allow less correction while ramping up.
  float max_corr = 5.0f + (MAX_HEADING_CORRECTION - 5.0f) * ramp;
  float turn = HEADING_GAIN * headingError();
  turn = constrain(turn, -max_corr, max_corr);
  robot.setMotorPWM(base - turn, base + turn);
}

void Controller_c::enterMode(Mode new_mode, unsigned long now) {
  mode = new_mode;
  mode_start_ms = now;
  segment_start_x = odometry.pose.x;
  segment_start_y = odometry.pose.y;
  if (new_mode == TURNING_OUT || new_mode == TURNING_IN) {
    turn_boost = 0.0f;
    turn_check_theta = odometry.pose.theta;
    turn_check_ms = now;
  }
}

void Controller_c::settleThen(Robot_c &robot, unsigned long now, unsigned long duration_ms, Mode then) {
  robot.setMotorPWM(0, 0);
  settle_ms = duration_ms;
  next_mode = then;
  enterMode(SETTLING, now);
}

void Controller_c::finish(Robot_c &robot, const char *why) {
  robot.setMotorPWM(0, 0);
  mode = FINISHED;
  signal = 0;
  message = why;
  robot.playTone(880, 300);
}

void Controller_c::fail(Robot_c &robot, const char *why) {
  robot.setMotorPWM(0, 0);
  mode = STOPPED;
  signal = 0;
  message = why;
  robot.playTone(300, 500);
}

void Controller_c::driveStraight(Robot_c &robot, float base_pwm, unsigned long now) {
  // Soft start: ramp the forward PWM up over RAMP_MS to reduce wheel slip.
  float ramp = (float)(now - mode_start_ms) / (float)RAMP_MS;
  if (ramp > 1.0f) ramp = 1.0f;
  float forward = base_pwm * ramp;

  // Heading hold: positive error means "turn left", so speed up the right wheel.
  float turn = HEADING_GAIN * headingError();
  turn = constrain(turn, -MAX_HEADING_CORRECTION, MAX_HEADING_CORRECTION);

  robot.setMotorPWM(forward - turn, forward + turn);
}

void Controller_c::turnInPlace(Robot_c &robot, float heading_error, unsigned long now) {
  // If the heading hasn't moved for a while, a wheel is stuck in the motor
  // deadband: add a little more PWM until it moves again.
  if (now - turn_check_ms >= TURN_STALL_MS) {
    if (fabsf(odometry.pose.theta - turn_check_theta) < 0.01f) {
      turn_boost = min(turn_boost + TURN_BOOST_STEP, TURN_BOOST_MAX);
    }
    turn_check_theta = odometry.pose.theta;
    turn_check_ms = now;
  }

  // Proportional turn, but never below turn_min_pwm (+ any stall boost) and
  // never above turn_pwm (+ boost).
  float lo = config.turn_min_pwm + turn_boost;
  float hi = max(config.turn_pwm, lo);
  float magnitude = TURN_GAIN * fabsf(heading_error);
  magnitude = constrain(magnitude, lo, hi);
  float direction = (heading_error > 0.0f) ? 1.0f : -1.0f;  // +1 = turn left
  robot.setMotorPWM(-direction * magnitude, direction * magnitude);
}

float Controller_c::headingError() const {
  return wrapAngle(target_heading - odometry.pose.theta);
}

float Controller_c::distanceFromSegmentStart() const {
  float dx = odometry.pose.x - segment_start_x;
  float dy = odometry.pose.y - segment_start_y;
  return sqrtf(dx * dx + dy * dy);
}

void Controller_c::logSample(Robot_c &robot, unsigned long now) {
  ScanSample_t s;
  s.t_ms = now;
  s.x = odometry.pose.x;
  s.y = odometry.pose.y;
  s.theta = odometry.pose.theta;
  for (int i = 0; i < 5; i++) {
    s.dn[i] = robot.surface.reading[i];
  }
  s.state = (uint8_t)mode;
  s.row = row;

  // The log is a ring buffer that the dashboard empties while the scan runs.
  // If nobody is downloading, the oldest samples are overwritten: warn once.
  uint32_t lost_before = scan_log.dropped();
  scan_log.add(s);
  if (lost_before == 0 && scan_log.dropped() > 0) {
    message = "Scan bigger than robot memory and no dashboard open: oldest samples lost";
  }
}

float Controller_c::wrapAngle(float angle) {
  return atan2f(sinf(angle), cosf(angle));
}

float Controller_c::rowHeading(uint8_t row_index) {
  // Even rows go along +x, odd rows come back along -x.
  return (row_index % 2 == 0) ? 0.0f : PI;
}

void Controller_c::publishTelemetry(Robot_c &robot, RobotWifiAP_c &server, unsigned long timestamp_ms) {
  // Same 14 columns as the template, so existing Digital Twin / Serial tools
  // keep working: timestamp_ms, x, y, theta, left_pwm, right_pwm, left_count,
  // right_count, DN1, DN2, DN3, DN4, DN5, signal
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
    signal
  );
  if (Serial) {
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
      signal
    );
  }
}
