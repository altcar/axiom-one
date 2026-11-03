// Deterministic software plant, not measured robot behaviour.
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct Robot_c {
  struct { uint32_t timestamp_us = 0; uint16_t reading[5] = {500,500,500,500,500}; } surface;
  int l = 0, r = 0, lc = 0, rc = 0;
  bool ack = true, surface_ok = true, encoder_ok = true, pressed = false, advance_stamp = true;
  uint32_t clock = 0, surface_delay = 0, encoder_delay = 0, motor_delay = 0;
  unsigned surface_calls = 0, encoder_calls = 0;
  uint32_t getMillis() { return clock; }
  bool getSurfaceSensors() {
    ++surface_calls; clock += surface_delay;
    if (advance_stamp) ++surface.timestamp_us;
    return surface_ok;
  }
  bool getEncoders() { ++encoder_calls; clock += encoder_delay; return encoder_ok; }
  bool isButtonPressed() { return pressed; }
  bool setMotorPWM(float a, float b) {
    l = a; r = b; clock += motor_delay; motor_delay = 0; return ack;
  }
  int getLeftEncoderCount() { return lc; }
  int getRightEncoderCount() { return rc; }
  int getLeftMotorPWM() { return l; }
  int getRightMotorPWM() { return r; }
};

struct RobotWifiAP_c {
  bool connected = true, send_ok = true, healthy = true;
  std::string input;
  std::vector<std::string> lines;
  bool clientConnected() { return connected; }
  bool telemetryOK() { return healthy; }
  size_t pendingBytes() { return 0; }
  unsigned long maxSendMs() { return 0; }
  size_t available() { return input.size(); }
  int read() { int c = input.front(); input.erase(0, 1); return c; }
  bool printf(const char *format, ...) {
    char text[256]; va_list args; va_start(args, format);
    int n = vsnprintf(text, sizeof text, format, args); va_end(args);
    assert(n > 0 && n < 256); // The real transport rejects oversized records.
    lines.emplace_back(text); return send_ok;
  }
};

struct RobotIMU_c {
  bool ok = true, fresh = true;
  int16_t raw = 100;
  uint32_t clock_us = 0;
  bool initialiseGyroscope() { return ok; }
  bool readGyroscope(int16_t &z, uint32_t &time_us, bool &is_fresh) {
    is_fresh = ok && fresh;
    clock_us += 10000;
    if (is_fresh) { z = raw; time_us = clock_us; }
    return ok;
  }
};

// Generated from the current production header and source by run_checks.py.
// Only the hardware interfaces above are substituted.
#define private public
#include "controller_under_test.inc"
#undef private

RobotWifiAP_c server;

OdometryModel_c::Pose &controlPose(Controller_c &c) {
  return ScanConfig::USE_HEADING_FUSION ? c.fused_odometry.pose : c.odometry.pose;
}

// Supply actual raw gyro counts for the synthetic wheel motion. Calibration
// sees the fixed 100-count bias; driving sees that bias plus angular rate.
void setGyroRate(Controller_c &c, double radians_per_second) {
  c.imu.raw = lround(100 + radians_per_second * 57.2957795 / .00875);
}

void tick(Controller_c &c, Robot_c &r, unsigned long t, bool press = false) {
  server.input = "H";
  r.surface.timestamp_us += 10000;
  r.clock = t; c.raster_cycle_ms = t;
  c.imu_read_ok = c.imu_sample_seen = true; c.imu_sample_ms = t;
  c.imu_fresh = false;
  c.runRaster(r, server, true, true, press, t);
}

void start(Controller_c &c, Robot_c &r) {
  server = {}; c = {}; r = {};
  c.initialiseIMU();
  tick(c, r, 600); tick(c, r, 610, true);
  // Motion tests use an already calibrated gyro; real update/calibration below.
  c.imu_calibrating = c.gyro_heading.calibrated = true; tick(c, r, 1610);
  assert(c.raster_phase == c.RASTER_ROW && c.raster_row == 1);
}

bool logged(const char *text) {
  for (const auto &line : server.lines)
    if (line.find(text) != std::string::npos) return true;
  return false;
}

// Regression plant based on the order of magnitude seen in the failing log:
// +/-30 PWM gives about 112 deg/s, and stopped wheels coast for about 30 ms.
// It is synthetic, without slip or unequal wheels; it does not certify hardware.
struct CoastingPlant {
  double left_speed = 0, right_speed = 0, left_mm = 0, right_mm = 0;
  static constexpr double dt = .01, tau = .03;
  static constexpr double mm_per_second_per_pwm = 112 * .0174532925 * 84 / 60;

  void advance(Controller_c &c, Robot_c &r) {
    const double decay = std::exp(-dt / tau);
    auto wheel = [&](int pwm, double &speed, double &distance) {
      const double target = pwm * mm_per_second_per_pwm;
      distance += target * dt + (speed - target) * tau * (1 - decay);
      speed = target + (speed - target) * decay;
    };
    wheel(r.l, left_speed, left_mm); wheel(r.r, right_speed, right_mm);
    const double counts_per_mm = 358.3 / (TWO_PI * ScanConfig::WHEEL_RADIUS_MM);
    r.lc = lround(left_mm * counts_per_mm); r.rc = lround(right_mm * counts_per_mm);
    setGyroRate(c, (right_speed - left_speed) / ScanConfig::WHEELBASE_MM);
  }

  double rateDps() const {
    return (right_speed - left_speed) / ScanConfig::WHEELBASE_MM * 57.2957795;
  }
};

void testCoastingTurn(int direction) {
  Controller_c c; Robot_c r; CoastingPlant plant; server = {}; c.initialiseIMU();
  unsigned long t = 600;
  for (; t <= 3610; t += 10) {
    r.clock = t; r.pressed = t == 610; server.input = "H"; c.update(r, server);
  }
  assert(c.gyro_heading.calibrated && c.raster_phase == c.RASTER_ROW);
  // Inject only the requested maneuver; all following odometry, gyro fusion,
  // settling and motor decisions go through the real update() path.
  c.raster_phase = c.RASTER_TURN_OUT;
  c.raster_phase_ms = c.raster_alignment_ms = t;
  c.raster_target_heading = direction * 1.57079633f;
  const unsigned long began = t;
  unsigned approaches = 1;
  bool accepted = false;
  for (; t <= began + 11000; t += 10) {
    plant.advance(c, r);
    r.clock = t; r.pressed = false; server.input = "H";
    const auto before = c.raster_phase;
    c.update(r, server);
    if (before == c.RASTER_SETTLE && c.raster_phase == c.RASTER_TURN_OUT) ++approaches;
    if (c.raster_phase == c.RASTER_SHIFT) {
      accepted = true;
      assert(std::fabs(plant.rateDps()) <= 5);
      assert(std::fabs(c.raster_target_heading - controlPose(c).theta) <= 2 * .0174532925);
      const double actual_heading = (plant.right_mm - plant.left_mm) / ScanConfig::WHEELBASE_MM;
      assert(std::fabs(c.raster_target_heading - actual_heading) <= 2 * .0174532925);
      assert(r.l == 0 && r.r == 0);
      break;
    }
    if (c.raster_phase == c.RASTER_DONE) break;
  }
  fprintf(stderr, "Coasting %s turn (%s): %lu ms, %u approach(es), accepted=%d\n",
          direction > 0 ? "+90" : "-90", ScanConfig::USE_HEADING_FUSION ? "fusion" : "encoder",
          t - began, approaches, int(accepted));
  assert(accepted && t - began < 2500 && approaches <= 3);
}

// This faster plant also challenges the unchanged short row shifts. Preserve
// their position guard rather than silently accepting an overshot next row.
void testCoastingRasterStopsOnShiftOvershoot() {
  Controller_c c; Robot_c r; CoastingPlant plant; server = {}; c.initialiseIMU();
  unsigned rows = 0, turns = 0, approaches = 0;
  unsigned long turn_started = 0, longest_turn = 0;
  for (unsigned long t = 600; t < ScanConfig::TOTAL_TIMEOUT_MS; t += 10) {
    plant.advance(c, r);
    r.clock = t; r.pressed = t == 610; server.input = "H";
    const auto before = c.raster_phase;
    c.update(r, server);
    const bool entered_turn = before != c.raster_phase &&
      (c.raster_phase == c.RASTER_TURN_OUT || c.raster_phase == c.RASTER_TURN_IN);
    if (entered_turn) {
      if (!turn_started) { turn_started = t; approaches = 0; }
      ++approaches;
    }
    if (turn_started && (c.raster_phase == c.RASTER_SHIFT || c.raster_phase == c.RASTER_ROW)) {
      const unsigned long duration = t - turn_started;
      if (duration > longest_turn) longest_turn = duration;
      assert(duration < 2500 && approaches <= 3);
      assert(std::fabs(plant.rateDps()) <= 5);
      assert(std::fabs(plant.left_speed) < 1 && std::fabs(plant.right_speed) < 1);
      assert(r.l == 0 && r.r == 0);
      ++turns; turn_started = 0;
    }
    if (c.raster_phase == c.RASTER_ROW && c.raster_row > rows) rows = c.raster_row;
    if (c.raster_phase == c.RASTER_DONE) break;
  }
  fprintf(stderr, "Coasting raster (%s): %u rows, %u turns, longest turn %lu ms\n",
          ScanConfig::USE_HEADING_FUSION ? "fusion" : "encoder", rows, turns, longest_turn);
  if (!logged("# stop,reason=DONE,"))
    for (const auto &line : server.lines) if (line.find("# stop,") == 0) fputs(line.c_str(), stderr);
  assert(rows >= 2 && turns >= 3 && longest_turn < 2500);
  assert(logged("# stop,reason=row_spacing_limit,"));
  assert(r.l == 0 && r.r == 0);
}

int main() {
  testCoastingTurn(1);
  testCoastingTurn(-1);
  testCoastingRasterStopsOnShiftOvershoot();

  // An angle inside the tolerance is not enough: coast and a single changed
  // encoder count each restart the stationary window before translation.
  {
    Controller_c c; Robot_c r; start(c, r);
    c.raster_phase = c.RASTER_TURN_OUT;
    c.raster_target_heading = controlPose(c).theta = 1.57079633f;
    c.raster_alignment_ms = 1610;
    c.raster_turn_rate_dps = 40;
    tick(c, r, 1620);
    assert(c.raster_phase == c.RASTER_SETTLE && r.l == 0 && r.r == 0);
    tick(c, r, 1820);
    assert(c.raster_phase == c.RASTER_SETTLE);
    c.raster_turn_rate_dps = 0; ++r.lc; tick(c, r, 1830);
    tick(c, r, 2020); assert(c.raster_phase == c.RASTER_SETTLE);
    tick(c, r, 2030); assert(c.raster_phase == c.RASTER_SHIFT);
  }

  // Moving away from the target must not trigger predictive braking. A very
  // large measured speed must not extend the braking zone beyond five degrees.
  for (int direction : {-1, 1}) {
    Controller_c c; Robot_c r; start(c, r);
    c.raster_phase = c.RASTER_TURN_OUT; c.raster_alignment_ms = 1610;
    c.raster_target_heading = direction * 2 * .0174532925f;
    c.raster_turn_rate_dps = -direction * 100;
    tick(c, r, 1620);
    assert(c.raster_phase == c.RASTER_TURN_OUT && r.l * direction < 0 && r.r * direction > 0);
    c.raster_target_heading = direction * 6 * .0174532925f;
    c.raster_turn_rate_dps = direction * 1000;
    tick(c, r, 1630);
    assert(c.raster_phase == c.RASTER_TURN_OUT && r.l * direction < 0 && r.r * direction > 0);
  }

  // Do not accept a bad heading after exhausting correction attempts, and do
  // not let continuous motion extend the existing overall alignment deadline.
  for (bool still : {true, false}) {
    Controller_c c; Robot_c r; start(c, r);
    c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_SHIFT;
    c.raster_phase_ms = c.raster_still_ms = c.raster_alignment_ms = 1610;
    c.raster_target_heading = .1f; c.raster_turn_retries = 2;
    c.raster_turn_rate_dps = still ? 0 : 40;
    tick(c, r, still ? 1810 : 11620);
    assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && r.r == 0);
    assert(logged(still ? "reason=turn_alignment_limit" : "reason=turn_alignment_timeout"));
  }
  // Regression: the real log stopped 200 ms after the button on a single
  // left-encoder decrement. Keep waiting for stillness, not a latched fault.
  {
    Controller_c c; Robot_c r; server = {}; c.initialiseIMU();
    for (unsigned long t = 600; t <= 810; t += 10) {
      r.clock = t; r.pressed = t == 610; server.input = "H";
      if (t == 810) r.lc = -1;
      c.update(r, server);
    }
    assert(c.raster_phase == c.RASTER_BASELINE && r.l == 0 && r.r == 0);
    assert(!logged("reason=imu_calibration_moved"));
  }
  Controller_c c; Robot_c r; server = {}; c.initialiseIMU();
  unsigned rows_seen = 0, turn_out_seen = 0, turn_in_seen = 0, shift_stops = 0;
  double lc = 0, rc = 0;
  const double counts_per_mm = 358.3 / (TWO_PI * ScanConfig::WHEEL_RADIUS_MM);
  for (unsigned long t = 600; t < ScanConfig::TOTAL_TIMEOUT_MS; t += 10) {
    // Ideal differential-drive plant: 1 mm/s per PWM, without slip or coast.
    // Only sensor inputs change; actual update() computes every pose/control.
    lc += r.l * .01 * counts_per_mm; rc += r.r * .01 * counts_per_mm;
    setGyroRate(c, (r.r - r.l) / ScanConfig::WHEELBASE_MM);
    r.lc = lround(lc); r.rc = lround(rc);
    r.clock = t; r.pressed = t == 610; server.input = "H";
    // An occasional missing gyro read is followed by a timestamp covering
    // both cycles. This must not double count encoder fallback predictions.
    c.imu.fresh = t % 170 != 0;
    auto before = c.raster_phase;
    c.update(r, server);
    if (c.raster_phase == c.RASTER_TURN_OUT && before != c.RASTER_TURN_OUT) ++turn_out_seen;
    if (c.raster_phase == c.RASTER_TURN_IN && before != c.RASTER_TURN_IN) ++turn_in_seen;
    if (c.raster_phase == c.RASTER_SHIFT_SETTLE && before != c.RASTER_SHIFT_SETTLE) ++shift_stops;
    if (c.raster_phase == c.RASTER_ROW && c.raster_row > rows_seen) {
      rows_seen = c.raster_row;
      assert(fabs(controlPose(c).y - (rows_seen - 1) * ScanConfig::ROW_SPACING_MM) <= 2);
      assert(fabs(c.raster_target_heading - (rows_seen % 2 ? 0 : 3.14159265f)) < 1e-6);
    }
    if (c.raster_phase == c.RASTER_DONE) break;
  }
  assert(rows_seen == ScanConfig::ROWS && turn_out_seen >= ScanConfig::ROWS - 1 && turn_in_seen >= ScanConfig::ROWS - 1);
  assert(shift_stops == ScanConfig::ROWS - 1);
  assert(logged(("# stop,reason=DONE,row=" + std::to_string(ScanConfig::ROWS)).c_str()));
  assert(fabs(controlPose(c).y - 60) < 2 && r.l == 0 && r.r == 0);
  assert(logged(ScanConfig::USE_HEADING_FUSION ? "control=fused,pose_source=fused" :
                                              "control=encoders,pose_source=encoders"));

  // Re-check overshoot after settling; do not accept a visually convenient angle.
  start(c, r); c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_SHIFT;
  c.raster_phase_ms = c.raster_alignment_ms = 1610; c.raster_target_heading = 1.57079633f;
  controlPose(c).theta = 1.65f; tick(c, r, 2610);
  assert(c.raster_phase == c.RASTER_TURN_OUT);
  tick(c, r, 2620); assert(r.l > 0 && r.r < 0);

  // A shift keeps driving past 60 ms, slows near the target, and stops only once.
  start(c, r); c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_SHIFT;
  c.raster_phase_ms = c.raster_alignment_ms = 1610;
  c.raster_target_heading = controlPose(c).theta = 1.57079633f;
  const float shift_y = ScanConfig::ROW_SPACING_MM;
  controlPose(c).y = shift_y - 10;
  tick(c, r, 2610); tick(c, r, 2620); assert(r.l == 35 && r.r == 35);
  controlPose(c).y = shift_y - 3; tick(c, r, 2630); assert(r.l == 30 && r.r == 30);
  controlPose(c).y = shift_y - 2; tick(c, r, 2730);
  assert(c.raster_phase == c.RASTER_SHIFT && r.l == 30 && r.r == 30);
  controlPose(c).y = shift_y - .8f; tick(c, r, 2740);
  assert(c.raster_phase == c.RASTER_SHIFT_SETTLE && r.l == 0 && r.r == 0);
  controlPose(c).y = shift_y + .8f; tick(c, r, 2930);
  assert(c.raster_phase == c.RASTER_SHIFT_SETTLE && r.l == 0 && r.r == 0);
  tick(c, r, 2940);
  assert(c.raster_phase == c.RASTER_TURN_IN && r.l == 0 && r.r == 0);

  // Crossing the target between reads must stop, not reverse and hunt for it.
  start(c, r); c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_SHIFT;
  c.raster_phase_ms = c.raster_alignment_ms = 1610;
  c.raster_target_heading = controlPose(c).theta = 1.57079633f;
  tick(c, r, 2610); tick(c, r, 2620);
  controlPose(c).y = shift_y + 2.5f; tick(c, r, 2630);
  assert(c.raster_phase == c.RASTER_SHIFT_SETTLE && r.l == 0 && r.r == 0);
  tick(c, r, 2830);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && r.r == 0 && logged("reason=row_spacing_limit"));

  // A retry that starts beyond the row approaches continuously in reverse.
  start(c, r); c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_SHIFT;
  c.raster_phase_ms = c.raster_alignment_ms = 1610;
  c.raster_target_heading = controlPose(c).theta = 1.57079633f;
  controlPose(c).y = shift_y + 3; tick(c, r, 2610); tick(c, r, 2620);
  assert(r.l == -30 && r.r == -30);
  controlPose(c).y = shift_y + 1.5f; tick(c, r, 2730);
  assert(c.raster_phase == c.RASTER_SHIFT && r.l == -30 && r.r == -30);
  controlPose(c).y = shift_y - 1.5f; tick(c, r, 2740);
  assert(c.raster_phase == c.RASTER_SHIFT_SETTLE && r.l == 0 && r.r == 0);
  tick(c, r, 2940); assert(c.raster_phase == c.RASTER_TURN_IN);

  // A translated turn must retry the fixed next-row target, not move the frame.
  start(c, r); c.raster_phase = c.RASTER_SETTLE; c.raster_next_phase = c.RASTER_ROW;
  c.raster_phase_ms = c.raster_alignment_ms = 1700;
  c.raster_target_heading = controlPose(c).theta = 3.14159265f;
  controlPose(c).y = shift_y + 3; tick(c, r, 2700);
  assert(c.raster_phase == c.RASTER_TURN_OUT && c.raster_row == 1 && c.raster_position_retries == 1);

  // Accept one actual snapshot after a distance gap, without fabricating missed samples.
  start(c, r); tick(c, r, 1620);
  controlPose(c).x = 7; tick(c, r, 1630);
  assert(c.raster_next_sample_mm == 8);
  auto line_count = server.lines.size(); tick(c, r, 1640);
  assert(server.lines.size() == line_count);

  start(c, r); server.input = ""; r.clock = c.raster_cycle_ms = 2700;
  c.runRaster(r, server, true, true, false, 2700);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("reason=wifi_or_heartbeat_lost"));
  start(c, r); server.input = "HS"; r.clock = c.raster_cycle_ms = 1620;
  c.runRaster(r, server, true, true, false, 1620);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("reason=remote_stop"));
  start(c, r); server.input = "H"; r.clock = c.raster_cycle_ms = 1720;
  c.imu_sample_ms = 1720; // Isolate stale surface data from gyro freshness.
  c.runRaster(r, server, true, true, false, 1720);
  assert(c.raster_phase == c.RASTER_DONE && logged("reason=surface_stale"));
  start(c, r); server.send_ok = false; controlPose(c).x = 2; tick(c, r, 1620);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && r.r == 0);

  // Actual update(): late loop/read/write operations latch a stop, not a restart.
  start(c, r); tick(c, r, 1620);
  c.raster_cycle_seen = true; c.raster_previous_cycle_ms = 1620;
  r.clock = 2291; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && r.surface_calls == 0);
  assert(logged("reason=control_gap") && logged("duration_ms=671"));
  r.clock += 10; r.pressed = true; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0);
  start(c, r); r.clock = 1630; r.surface_delay = 671; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && r.encoder_calls == 0 && logged("reason=control_overrun"));
  start(c, r); r.clock = 1630; r.encoder_delay = 60; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && logged("stage=encoder_read"));
  start(c, r); r.clock = 1630; r.surface_ok = false; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("reason=read_failed"));
  start(c, r); r.clock = 1630; r.motor_delay = 671; server.input = "H"; c.update(r, server);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("stage=motor_command"));
  start(c, r); server.healthy = false; tick(c, r, 1630);
  assert(c.raster_phase == c.RASTER_DONE && logged("reason=telemetry_backpressure"));

  // Exercise the actual acquisition path: settle, then calibrate for two
  // seconds. Actual gyro rotation influences steering only in fusion mode.
  c = {}; r = {}; server = {}; c.initialiseIMU();
  auto update = [&](unsigned long t, bool press = false) {
    r.clock = t; r.pressed = press; server.input = "H"; c.update(r, server);
  };
  update(600); update(610, true);
  assert(c.raster_phase == c.RASTER_BASELINE && r.l == 0 && r.r == 0);
  for (unsigned long t = 620; t < 3610; t += 10) {
    update(t);
    assert(c.raster_phase == c.RASTER_BASELINE && r.l == 0 && r.r == 0);
    if (t < 1610) assert(c.gyro_heading.samples == 0);
  }
  update(3610);
  assert(c.raster_phase == c.RASTER_ROW && c.gyro_heading.calibrated);
  assert(fabs(c.gyro_heading.bias_dps - .875f) < .0001f && logged("imu_calibration_done"));
  setGyroRate(c, 1); // Wheel counts remain still, gyro measures positive rotation.
  for (unsigned long t = 3620; t <= 3820; t += 10) update(t);
  assert(c.gyro_heading.angle_deg > 10 && c.odometry.pose.theta == 0);
  assert(c.fused_odometry.pose.theta > .15f);
  if (ScanConfig::USE_HEADING_FUSION) assert(r.l > r.r && r.r > 0);
  else assert(r.l == r.r && r.l > 0);
  // Force a record through actual acquisition and check the selected pose in
  // CSV, while #imu retains distinct encoder, gyro and fused diagnostics.
  r.lc = r.rc = 30; update(3830);
  const auto &selected = controlPose(c);
  char pose_csv[100];
  snprintf(pose_csv, sizeof pose_csv, ",%.3f,%.3f,%.5f,30,30,",
           selected.x, selected.y, selected.theta);
  assert(logged(pose_csv));
  assert(logged("encoder_deg=0.000,fused_deg="));
  c.imu.ok = false; update(3840);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("reason=imu_read_failed"));

  c = {}; r = {}; server = {}; c.initialiseIMU();
  update(600); update(610, true);
  for (unsigned long t = 620; t <= 3810; t += 10) {
    if (t == 810) --r.lc; // The exact one-count event at +200 ms from the log.
    update(t);
    if (t < 1810) assert(c.gyro_heading.samples == 0);
    if (t < 3810) assert(c.raster_phase == c.RASTER_BASELINE && r.l == 0 && r.r == 0);
  }
  assert(c.raster_phase == c.RASTER_ROW && !logged("imu_calibration_moved"));

  // Movement after calibration started discards the whole partial window.
  c = {}; r = {}; server = {}; c.initialiseIMU();
  update(600); update(610, true);
  for (unsigned long t = 620; t <= 4810; t += 10) {
    if (t == 1810) --r.lc;
    update(t);
    if (t >= 1810 && t < 2810) assert(c.gyro_heading.samples == 0);
    if (t < 4810) assert(c.raster_phase == c.RASTER_BASELINE && r.l == 0 && r.r == 0);
  }
  assert(c.raster_phase == c.RASTER_ROW && logged("imu_calibration_retry"));
  assert(c.gyro_heading.samples == 201);

  // Holding the first press cannot calibrate or start motion. The overall
  // ten-second timeout is not extended by constant movement/retries.
  for (bool hold_button : {true, false}) {
    c = {}; r = {}; server = {}; c.initialiseIMU(); update(600); update(610, true);
    for (unsigned long t = 620; t <= 10620; t += 10) {
      if (!hold_button && t % 100 == 0) ++r.lc;
      update(t, hold_button);
      assert(r.l == 0 && r.r == 0);
    }
    assert(c.raster_phase == c.RASTER_DONE && logged("imu_calibration_timeout"));
    assert(!logged("imu_calibration_done"));
  }

  c = {}; r = {}; server = {}; c.initialiseIMU();
  update(600); update(610, true); c.imu.fresh = false;
  for (unsigned long t = 620; t <= 670; t += 10) update(t);
  assert(c.raster_phase == c.RASTER_DONE && r.l == 0 && logged("imu_stale_or_failed"));

  c = {}; r = {}; server = {}; // Unavailable IMU cannot start a diagnostic scan.
  update(600); update(610, true);
  assert(c.raster_phase == c.RASTER_WAIT && r.l == 0 && logged("imu_ok=0"));

  // No heartbeat means no start, even during the first second after boot.
  c = {}; r = {}; server = {}; c.raster_button_armed = true; r.clock = c.raster_cycle_ms = 600;
  c.runRaster(r, server, true, true, true, 600);
  assert(c.raster_phase == c.RASTER_WAIT && r.l == 0);
  printf("PASS: %u-row raster, turns, fixed row targets, distance sampling and failure stops (synthetic).\n", ScanConfig::ROWS);
}
