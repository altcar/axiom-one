#include "Controller.h"
#include "RasterLine.h"

void Controller_c::update(Robot_c &robot, RobotWifiAP_c &server) {

  // Get the current count of milliseconds since the StampC3 was powered on.
  unsigned long now = robot.getMillis();

  // Check if our TaskTimer_c for the general controller update indicates
  // that it is the correct time to operate.  Note, we give the TaskTimer_c
  // the current count in milliseconds.
  // We do this because we don't want to query the robot faster than at 10ms
  // intervals.
  if (!update_timer.isReady(now)) {

    // TaskTimer_c says it is not ready, so we simply end this call to
    // the controller here by calling return.
    return;
  }

  // The TaskTimer_c was ready, so we now reset the TaskTimer_c so it
  // begins counting again.  We now run the remaining controller code.
  update_timer.resetTimer(now);

  // Raster addition: measure cycle delays before issuing another motor command.
  const uint32_t started = now;
  const bool active = raster_phase != RASTER_WAIT && raster_phase != RASTER_DONE;
  const uint32_t gap = raster_cycle_seen ? uint32_t(started - raster_previous_cycle_ms) : 0;
  raster_previous_cycle_ms = raster_cycle_ms = started;
  raster_cycle_seen = true;
  if (active && gap > raster_max_gap_ms) raster_max_gap_ms = gap;
  // Detect a late loop before new reads or a queued heartbeat can hide it.
  // This is a recovery-time guard, not an independent hardware watchdog.
  if (active && gap > 50UL) {
    stopRasterFault(robot, server, "control_gap", "between_cycles", gap);
    return;
  }

  // Ask the robot for latest information on surface reflectance sensors.
  const bool surface_ok = robot.getSurfaceSensors();
  const uint32_t surface_end = robot.getMillis();
  const uint32_t surface_ms = surface_end - started;
  if (active && surface_ms > raster_max_surface_ms) raster_max_surface_ms = surface_ms;
  if (active && (!surface_ok || surface_ms > 50UL)) {
    stopRasterFault(robot, server, surface_ok ? "control_overrun" : "read_failed",
                    "surface_read", surface_ms);
    return;
  }

  // Ask the robot for the latest encoder information, and then use this
  // to update the odometry on board the StampC3 (see OdometryModel.h).
  const bool encoder_ok = robot.getEncoders();
  const uint32_t encoder_end = robot.getMillis();
  const uint32_t encoder_ms = encoder_end - surface_end;
  if (active && encoder_ms > raster_max_encoder_ms) raster_max_encoder_ms = encoder_ms;
  if (active && (!encoder_ok || uint32_t(encoder_end - started) > 50UL)) {
    stopRasterFault(robot, server, encoder_ok ? "control_overrun" : "read_failed",
                    "encoder_read", encoder_ms);
    return;
  }
  const float previous_encoder_heading = odometry.pose.theta;
  if (encoder_ok) {
    odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
  }

  // Integrate each fresh, checked Z-gyro reading once. Keep the independent
  // encoder and gyro estimates for comparison with the fused control pose.
  imu_fresh = false;
  imu_read_ok = imu_available && imu.readGyroscope(imu_raw_z, imu_time_us, imu_fresh);
  if (imu_read_ok && imu_fresh) {
    imu_sample_seen = true;
    imu_sample_ms = robot.getMillis();
    if (active && raster_phase != RASTER_BASELINE) {
      gyro_heading.add(imu_raw_z, imu_time_us, true);
    }
  }
  if (active && (!imu_read_ok || uint32_t(robot.getMillis() - started) > 50UL)) {
    stopRasterFault(robot, server, imu_read_ok ? "control_overrun" : "imu_read_failed",
                    "imu_read", robot.getMillis() - encoder_end);
    return;
  }

  raster_turn_rate_dps = ScanConfig::USE_HEADING_FUSION ? gyro_heading.rate_dps :
    (gap ? (odometry.pose.theta - previous_encoder_heading) * 57295.7795f / gap : 0.0f);

  if (active && raster_phase != RASTER_BASELINE && encoder_ok &&
      imu_read_ok && gyro_heading.calibrated && !gyro_heading.error &&
      uint32_t(robot.getMillis() - imu_sample_ms) <= 50UL) {
    fused_odometry.update(odometry.pose, gyro_heading.angle_deg, imu_time_us, imu_fresh);
  }

  // This scan uses local odometry; the unused middleware getPose()
  // request added two synchronous bus transactions to every cycle.
  const bool button_pressed = robot.isButtonPressed();
  now = robot.getMillis();
  if (uint32_t(now - started) > 50UL) {
    if (active) stopRasterFault(robot, server, "control_overrun", "acquisition", now - started);
    else robot.setMotorPWM(0, 0); // Do not start from a delayed idle acquisition.
    return;
  }
  // All timeout checks use the clock AFTER acquisition.
  runRaster(robot, server, surface_ok, encoder_ok, button_pressed, now);
}

void Controller_c::stopRasterFault(Robot_c &robot, RobotWifiAP_c &server,
                                  const char *reason, const char *stage,
                                  unsigned long duration_ms) {
  // Latch DONE before any diagnostic output. Recovery requires a reset.
  raster_phase = RASTER_DONE;
  const bool stopped = robot.setMotorPWM(0, 0);
  server.printf("# control_fault,stage=%s,duration_ms=%lu,motor_ack=%u,tx_max_ms=%lu\n",
                stage, duration_ms, unsigned(stopped), server.maxSendMs());
  server.printf("# stop,reason=%s,row=%u\n", reason, raster_row);
}

// Fixed lanes: straight, stop, turn 90 degrees, shift, turn 90 degrees, return.
void Controller_c::runRaster(Robot_c &robot, RobotWifiAP_c &server, bool surface_ok, bool encoder_ok,
                              bool button_pressed, unsigned long now) {
  const auto &pose = ScanConfig::USE_HEADING_FUSION ? fused_odometry.pose : odometry.pose;
  bool remote_stop = false;
  for (unsigned int i=0; i<64 && server.available(); i++) {
    const int command = server.read();
    if (command == 'H') { raster_heartbeat_ms = now; raster_heartbeat_seen = true; }
    if (command == 'S') remote_stop = true;
  }
  const bool link_ok = raster_heartbeat_seen && server.clientConnected() && now-raster_heartbeat_ms < 1000UL;
  const bool imu_ok = imu_read_ok && imu_sample_seen && uint32_t(now - imu_sample_ms) <= 50UL;
  const bool pressed = button_pressed && raster_button_armed;
  if (button_pressed) {
    raster_button_armed = false;
    raster_quiet_ms = now;
  } else if (now - raster_quiet_ms >= 500UL) raster_button_armed = true;
  if (raster_phase == RASTER_WAIT || raster_phase == RASTER_DONE) {
    const bool stopped = robot.setMotorPWM(0, 0);
    if (server.clientConnected() && now - raster_log_ms >= 500UL) {
      const bool hardware_ok = stopped && surface_ok && encoder_ok && imu_ok && server.telemetryOK();
      server.printf("# %s,test=basic_raster_fusion_v2,heartbeat_ok=%u,motor_ack=%u,surface_ok=%u,encoder_ok=%u,button_armed=%u,imu_ok=%u\n",
                    raster_phase == RASTER_DONE ? "finished_reset_required" :
                    link_ok && hardware_ok ? "ready_press_button" : "waiting_for_hardware_or_link",
                    unsigned(link_ok), unsigned(stopped), unsigned(surface_ok),
                    unsigned(encoder_ok), unsigned(raster_button_armed), unsigned(imu_ok));
      raster_log_ms = now;
    }
    if (button_pressed && server.clientConnected()) {
      server.printf("# button_event,start_accepted=%u,motor_ack=%u,surface_ok=%u,encoder_ok=%u,heartbeat_ok=%u,imu_ok=%u\n",
        unsigned(raster_phase == RASTER_WAIT && pressed && stopped && link_ok && surface_ok && encoder_ok && imu_ok && server.telemetryOK() && !remote_stop),
        unsigned(stopped), unsigned(surface_ok), unsigned(encoder_ok), unsigned(link_ok), unsigned(imu_ok));
    }
    if (raster_phase == RASTER_DONE || !pressed || !stopped || !link_ok ||
        !surface_ok || !encoder_ok || !imu_ok || !server.telemetryOK() || remote_stop) return;
    // Keep one local frame throughout all rows and turns.
    odometry.setLeftWheelRadiusMm(ScanConfig::WHEEL_RADIUS_MM);
    odometry.setRightWheelRadiusMm(ScanConfig::WHEEL_RADIUS_MM);
    odometry.wheelSeparationMm = ScanConfig::WHEELBASE_MM;
    odometry.reset();
    odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
    fused_odometry.reset(odometry.pose, 0, imu_time_us);
    raster_phase = RASTER_BASELINE;
    raster_start_ms = raster_phase_ms = raster_fresh_ms = now;
    gyro_heading.beginCalibration();
    imu_calibrating = false;
    imu_quiet_ms = now;
    imu_calibration_left = robot.getLeftEncoderCount();
    imu_calibration_right = robot.getRightEncoderCount();
    if (!server.printf("elapsed_ms,row,phase,surface_time_us,surface_ok,surface_fresh,encoder_ok,x_mm,y_mm,theta_rad,left_count,right_count,DN1,DN2,DN3,DN4,DN5,left_pwm,right_pwm\n")) {
      raster_phase = RASTER_DONE;
      robot.setMotorPWM(0, 0);
      return;
    }
    server.printf("# scan_config,revision=basic_raster_fusion_v2,rows=%u,row_length_mm=%.1f,row_spacing_mm=%.1f,sample_step_mm=%.1f,calibration_test=0\n",
      ScanConfig::ROWS, ScanConfig::ROW_LENGTH_MM, ScanConfig::ROW_SPACING_MM, ScanConfig::SAMPLE_STEP_MM);
    server.printf("# motion_config,row_pwm=%.1f,wheelbase_mm=%.5f,radius_mm=%.5f,counts_per_rev=358.3,shift_mode=continuous,shift_settle_ms=200\n",
      ScanConfig::ROW_PWM, odometry.wheelSeparationMm, odometry.leftWheelRadiusMm);
    server.printf("# heading_config,control=%s,pose_source=%s,correction_tau_s=%.3f\n",
      ScanConfig::USE_HEADING_FUSION ? "fused" : "encoders",
      ScanConfig::USE_HEADING_FUSION ? "fused" : "encoders", ScanConfig::HEADING_CORRECTION_TAU_S);
    server.printf("# turn_config,brake_lead_ms=%.1f,still_ms=%lu,accept_deg=2.0,max_retries=2\n",
      ScanConfig::TURN_BRAKE_LEAD_S * 1000.0f, ScanConfig::TURN_SETTLE_MS);
    server.printf("# imu_wait_still,release_button=1,settle_ms=%lu\n", ScanConfig::GYRO_SETTLE_MS);
  }
  const bool fresh = surface_ok && (!raster_has_stamp || raster_stamp != robot.surface.timestamp_us);
  if (fresh) {
    raster_stamp = robot.surface.timestamp_us;
    raster_has_stamp = true;
    raster_fresh_ms = now;
  }
  const char *reason = nullptr;
  if (pressed && now != raster_start_ms) reason = "button_stop";
  else if (remote_stop) reason = "remote_stop";
  else if (!link_ok) reason = "wifi_or_heartbeat_lost";
  else if (!server.telemetryOK()) reason = "telemetry_backpressure";
  else if (!encoder_ok || !surface_ok) reason = "read_failed";
  else if (!imu_ok) reason = "imu_stale_or_failed";
  else if (gyro_heading.error) reason = gyro_heading.error;
  else if (now - raster_fresh_ms > 100UL) reason = "surface_stale";
  else if (now - raster_start_ms >= ScanConfig::TOTAL_TIMEOUT_MS) reason = "total_timeout";
  else if (hypotf(pose.x, pose.y) > ScanConfig::POSITION_LIMIT_MM) reason = "position_limit";

  const RasterPhase previous_phase = raster_phase;
  float left = 0, right = 0;
  const float heading = pose.theta;
  const float phase_distance = hypotf(pose.x - raster_anchor_x,
                                      pose.y - raster_anchor_y);
  // Targets remain unwrapped: odd rows=0, even rows=pi radians.
  if (!reason && raster_phase == RASTER_BASELINE) {
    if (now - raster_phase_ms > 10000UL) reason = "imu_calibration_timeout";
    else {
      const bool moved = robot.getLeftEncoderCount() != imu_calibration_left ||
                         robot.getRightEncoderCount() != imu_calibration_right;
      // Button handling can move a wheel by one count. Discard the affected
      // window and wait again; never calibrate through known movement.
      if (button_pressed || moved) {
        if (imu_calibrating) server.printf("# imu_calibration_retry,reason=movement,settle_ms=%lu\n", ScanConfig::GYRO_SETTLE_MS);
        gyro_heading.beginCalibration();
        imu_calibrating = false;
        imu_quiet_ms = now;
        imu_calibration_left = robot.getLeftEncoderCount();
        imu_calibration_right = robot.getRightEncoderCount();
      } else if (uint32_t(now - imu_quiet_ms) >= ScanConfig::GYRO_SETTLE_MS) {
        if (!imu_calibrating) {
          imu_calibrating = true;
          server.printf("# imu_calibration_start,keep_still=1,min_samples=%u,min_time_ms=%lu,gyro_sign=%.0f,control=%s\n",
            ScanConfig::GYRO_CALIBRATION_SAMPLES, ScanConfig::GYRO_CALIBRATION_US / 1000UL, ScanConfig::GYRO_Z_SIGN,
            ScanConfig::USE_HEADING_FUSION ? "fused" : "encoders");
        }
        if (imu_fresh) gyro_heading.add(imu_raw_z, imu_time_us, true);
        if (gyro_heading.error) reason = gyro_heading.error;
      }
    }
    if (!reason && gyro_heading.calibrated) {
      odometry.reset();
      odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
      gyro_heading.zeroAngle();
      fused_odometry.reset(odometry.pose, 0, imu_time_us);
      server.printf("# imu_calibration_done,samples=%u,bias_dps=%.5f,std_dps=%.5f\n",
        gyro_heading.samples, gyro_heading.bias_dps, gyro_heading.std_dps);
      raster_row = 1;
      raster_phase = RASTER_ROW;
      raster_phase_ms = now;
      raster_anchor_x = raster_anchor_y = raster_target_heading = 0;
      raster_next_sample_mm = 0;
      server.printf("# scan_row,row=1,target_y_mm=0\n");
    }
  } else if (!reason && raster_phase == RASTER_SETTLE) {
    const bool after_turn = raster_next_phase == RASTER_ROW ||
                            raster_next_phase == RASTER_SHIFT;
    // A zero PWM command does not mean the wheels have stopped. Restart the
    // short quiet window while either wheel or the heading is still moving.
    if (robot.getLeftEncoderCount() != raster_stop_left_count ||
        robot.getRightEncoderCount() != raster_stop_right_count ||
        fabsf(raster_turn_rate_dps) > 5.0f) {
      raster_still_ms = now;
      raster_stop_left_count = robot.getLeftEncoderCount();
      raster_stop_right_count = robot.getRightEncoderCount();
    }
    if (now - raster_alignment_ms > 10000UL) {
      reason = "turn_alignment_timeout";
    } else if (now - raster_still_ms >= ScanConfig::TURN_SETTLE_MS) {
      if (after_turn && fabsf(raster_target_heading - heading) > 0.0349066f) {
        // Stop at 1 degree (or earlier for coasting), accept within 2 degrees
        // once still. This hysteresis avoids chasing tiny errors indefinitely.
        if (raster_turn_retries >= 2) reason = "turn_alignment_limit";
        else {
          raster_turn_retries++;
          raster_phase = raster_next_phase == RASTER_ROW ? RASTER_TURN_IN : RASTER_TURN_OUT;
          raster_phase_ms = now;
        }
      } else if (raster_next_phase == RASTER_ROW &&
                 fabsf(pose.y - raster_row * ScanConfig::ROW_SPACING_MM) > 2.0f) {
        if (raster_position_retries >= 2) reason = "row_spacing_limit";
        else {
          // Keep this row number until the complete repositioning has settled.
          raster_position_retries++;
          raster_phase = RASTER_TURN_OUT;
          raster_target_heading = 1.57079633f;
          raster_alignment_ms = raster_phase_ms = now;
          raster_turn_retries = 0;
          raster_turn_left_count = robot.getLeftEncoderCount();
          raster_turn_right_count = robot.getRightEncoderCount();
        }
      } else {
        if (raster_next_phase == RASTER_ROW) {
          raster_row++;
          server.printf("# scan_row,row=%u,target_y_mm=%.1f\n",raster_row,(raster_row-1)*ScanConfig::ROW_SPACING_MM);
          raster_position_retries = 0;
          raster_next_sample_mm = 0;
        }
        raster_phase = raster_next_phase;
        if (raster_phase == RASTER_SHIFT) {
          raster_shift_start_ms = now;
          raster_shift_direction = raster_row * ScanConfig::ROW_SPACING_MM >= pose.y ? 1.0f : -1.0f;
        }
        if (raster_phase == RASTER_TURN_OUT || raster_phase == RASTER_TURN_IN) {
          raster_turn_left_count = robot.getLeftEncoderCount();
          raster_turn_right_count = robot.getRightEncoderCount();
        }
        raster_phase_ms = now;
        raster_anchor_x = pose.x;
        raster_anchor_y = pose.y;
      }
    }
  } else if (!reason && (raster_phase == RASTER_SHIFT || raster_phase == RASTER_SHIFT_SETTLE)) {
    const float remaining = raster_row * ScanConfig::ROW_SPACING_MM - pose.y;
    const float error = 1.57079633f - heading;
    if (now - raster_shift_start_ms > 10000UL) reason = "shift_alignment_timeout";
    else if (fabsf(error) > 0.35f) reason = "straight_heading_limit";
    else if (raster_phase == RASTER_SHIFT_SETTLE) {
      if (now - raster_phase_ms >= 200UL) {
        // Check the stopped position once; do not restart short correction pulses.
        if (fabsf(remaining) > 2.0f) reason = "row_spacing_limit";
        else {
          raster_phase = RASTER_TURN_IN;
          raster_target_heading = raster_row % 2 == 1 ? 3.14159265f : 0.0f;
          raster_alignment_ms = raster_phase_ms = now;
          raster_turn_retries = 0;
          raster_turn_left_count = robot.getLeftEncoderCount();
          raster_turn_right_count = robot.getRightEncoderCount();
        }
      }
    } else if (raster_shift_direction * remaining <= 1.0f) {
      // Stop on reaching or crossing the target; never reverse during this approach.
      raster_phase = RASTER_SHIFT_SETTLE;
      raster_phase_ms = now;
    } else {
      const float base = raster_shift_direction *
                         (fabsf(remaining) < 6.0f ? ScanConfig::SLOW_PWM : ScanConfig::SHIFT_PWM);
      float correction = -error * 60.0f;
      if (correction > 6) correction = 6;
      if (correction < -6) correction = -6;
      left = roundf(base + correction);
      right = roundf(base - correction);
    }
  } else if (!reason && raster_phase == RASTER_ROW) {
    const float error = raster_target_heading - heading;
    if (fabsf(error) > 0.35f) reason = "straight_heading_limit";
    else if (now - raster_phase_ms > 8000UL) reason = "straight_timeout";
    else if (phase_distance >= ScanConfig::ROW_LENGTH_MM) {
      if (raster_row == ScanConfig::ROWS) reason = "DONE";
      else {
        raster_next_phase = RASTER_TURN_OUT;
        raster_phase = RASTER_SETTLE;
        raster_phase_ms = raster_alignment_ms = now;
        raster_turn_retries = 0;
        raster_target_heading = 1.57079633f;
      }
    } else {
      const float lineError = rasterLineHeadingError(pose.y,
        (raster_row-1)*ScanConfig::ROW_SPACING_MM,raster_target_heading,heading);
      float correction = -lineError * 60.0f;
      if (correction > 6) correction = 6;
      if (correction < -6) correction = -6;
      left = roundf(ScanConfig::ROW_PWM + correction);
      right = roundf(ScanConfig::ROW_PWM - correction);
    }
  } else if (!reason && (raster_phase == RASTER_TURN_OUT || raster_phase == RASTER_TURN_IN)) {
    const float error = raster_target_heading - heading;
    // Estimate the remaining coast from measured speed, only towards the
    // target. Cap the lead at 5 degrees; never snap the measured heading.
    const float toward_rate = (error > 0 ? 1.0f : -1.0f) * raster_turn_rate_dps;
    const float brake_lead = fminf(0.0872665f,
      fmaxf(0.0f, toward_rate) * ScanConfig::TURN_BRAKE_LEAD_S * 0.0174532925f);
    if (now - raster_alignment_ms > 10000UL) reason = "turn_alignment_timeout";
    else if (fabsf(error) <= 0.0174533f || fabsf(error) <= brake_lead) {
      const bool turn_in = raster_phase == RASTER_TURN_IN;
      raster_next_phase = turn_in ? RASTER_ROW : RASTER_SHIFT;
      raster_phase = RASTER_SETTLE;
      raster_phase_ms = now;
    } else {
      // Bounded in-place turning, slowing near the target.
      const float pwm = fabsf(error) < 0.25f ? ScanConfig::SLOW_PWM : ScanConfig::TURN_PWM;
      left = error > 0 ? -pwm : pwm;
      right = -left;
      // Equal and opposite wheel travel keeps the axle midpoint near its pivot.
      const float center_mm = 3.14159265f / odometry.encoderCountsPerRevolution *
        ((int32_t(uint32_t(robot.getLeftEncoderCount()) - uint32_t(raster_turn_left_count))) * odometry.leftWheelRadiusMm +
         (int32_t(uint32_t(robot.getRightEncoderCount()) - uint32_t(raster_turn_right_count))) * odometry.rightWheelRadiusMm);
      float center_correction = -2.0f * center_mm;
      if (center_correction > 6) center_correction = 6;
      if (center_correction < -6) center_correction = -6;
      left = roundf(left + center_correction);
      right = roundf(right + center_correction);
    }
  }
  if (raster_phase == RASTER_SETTLE && previous_phase != RASTER_SETTLE) {
    raster_still_ms = now;
    raster_stop_left_count = robot.getLeftEncoderCount();
    raster_stop_right_count = robot.getRightEncoderCount();
  }
  if (reason) {
    raster_phase = RASTER_DONE;
    left = right = 0;
  }
  if (uint32_t(robot.getMillis() - raster_cycle_ms) > 50UL) {
    stopRasterFault(robot, server, "control_overrun", "before_motor", robot.getMillis() - raster_cycle_ms);
    return;
  }
  if (!robot.setMotorPWM(left, right)) {
    raster_phase = RASTER_DONE;
    reason = "motor_command_failed";
    robot.setMotorPWM(0, 0);
  }
  if (uint32_t(robot.getMillis() - raster_cycle_ms) > 50UL) {
    stopRasterFault(robot, server, "control_overrun", "motor_command", robot.getMillis() - raster_cycle_ms);
    return;
  }
  if (raster_phase != previous_phase && server.clientConnected()) {
    server.printf("# phase_event,elapsed_ms=%lu,row=%u,from=%u,to=%u,x_mm=%.3f,y_mm=%.3f,heading_deg=%.3f\n",
      now-raster_start_ms, raster_row, unsigned(previous_phase), unsigned(raster_phase),
      pose.x, pose.y, pose.theta*57.2957795f);
  }
  // Extra comment records preserve the original 19-column scan CSV. Integrate
  // every fresh gyro read locally; transmit diagnostics at at most 10 Hz.
  if (gyro_heading.calibrated && !gyro_heading.error && imu_ok &&
      server.clientConnected() && (now - imu_log_ms >= 100UL || reason)) {
    imu_log_ms = now;
    server.printf("# imu,elapsed_ms=%lu,gyro_time_us=%lu,raw_z=%d,rate_dps=%.3f,gyro_deg=%.3f,encoder_deg=%.3f,fused_deg=%.3f,sample_age_ms=%lu\n",
      now-raster_start_ms, (unsigned long)imu_time_us, int(imu_raw_z), gyro_heading.rate_dps,
      gyro_heading.angle_deg, odometry.pose.theta*57.2957795f,
      fused_odometry.pose.theta*57.2957795f, (unsigned long)(now-imu_sample_ms));
  }
  // Distance gates accept an actual fresh acquisition; never fabricate samples
  // at skipped grid crossings or reuse a cached snapshot to fill the gap.
  const float along_mm = (pose.x - raster_anchor_x) * cosf(raster_target_heading) +
                         (pose.y - raster_anchor_y) * sinf(raster_target_heading);
  const bool distance_sample = raster_phase == RASTER_ROW && fresh &&
                               along_mm >= raster_next_sample_mm;
  if (server.clientConnected() && (distance_sample || reason ||
      (raster_phase != RASTER_ROW && now - raster_log_ms >= 20UL))) {
    if (distance_sample) raster_next_sample_mm = (floorf(along_mm / ScanConfig::SAMPLE_STEP_MM) + 1.0f) * ScanConfig::SAMPLE_STEP_MM;
    raster_log_ms = now;
    const bool sent = server.printf("%lu,%u,%u,%lu,%u,%u,%u,%.3f,%.3f,%.5f,%ld,%ld,%u,%u,%u,%u,%u,%d,%d\n",
      now-raster_start_ms, raster_row, unsigned(raster_phase), (unsigned long)robot.surface.timestamp_us,
      unsigned(surface_ok), unsigned(fresh), unsigned(encoder_ok),
      pose.x, pose.y, pose.theta,
      (long)robot.getLeftEncoderCount(), (long)robot.getRightEncoderCount(),
      unsigned(robot.surface.reading[0]), unsigned(robot.surface.reading[1]),
      unsigned(robot.surface.reading[2]), unsigned(robot.surface.reading[3]),
      unsigned(robot.surface.reading[4]), int(robot.getLeftMotorPWM()), int(robot.getRightMotorPWM()));
    if (!sent) {
      reason = "telemetry_queue_failed";
      raster_phase = RASTER_DONE;
      robot.setMotorPWM(0, 0);
    }
  }
  const uint32_t work_ms = robot.getMillis() - now;
  if (work_ms > raster_max_work_ms) raster_max_work_ms = work_ms;
  if (reason || now - raster_timing_log_ms >= 1000UL) {
    raster_timing_log_ms = now;
    server.printf("# control_timing,gap_max_ms=%lu,surface_max_ms=%lu,encoder_max_ms=%lu,work_max_ms=%lu,tx_max_ms=%lu,tx_pending=%u\n",
      (unsigned long)raster_max_gap_ms, (unsigned long)raster_max_surface_ms,
      (unsigned long)raster_max_encoder_ms, (unsigned long)raster_max_work_ms,
      server.maxSendMs(), unsigned(server.pendingBytes()));
  }
  // Includes phase/timing messages too; check before allowing another cycle.
  if (!server.telemetryOK()) {
    reason = "telemetry_backpressure";
    raster_phase = RASTER_DONE;
    robot.setMotorPWM(0, 0);
  }
  if (uint32_t(robot.getMillis() - raster_cycle_ms) > 50UL) {
    stopRasterFault(robot, server, "control_overrun", "reporting", robot.getMillis() - raster_cycle_ms);
    return;
  }
  if (reason && server.clientConnected()) server.printf("# stop,reason=%s,row=%u\n", reason, raster_row);
}
