#include "Controller.h"

const unsigned long Controller_c::UPDATE_INTERVAL_MS;
const unsigned long Controller_c::SELECT_TIMEOUT_MS;
const unsigned long Controller_c::IDLE_TIMEOUT_MS;

namespace {
  const uint8_t PWM_STRAIGHT = 30;   // edge and scan tests
  const uint8_t PWM_CALIB    = 40;   // straight-line calibration
  const uint8_t PWM_SPIN     = 35;   // rotation calibration
  const unsigned long EDGE_MS     = 4000;
  const unsigned long STRAIGHT_MS = 3000;
  const unsigned long SPIN_MS     = 2000;
}

Controller_c::Controller_c()
  : update_timer(UPDATE_INTERVAL_MS), log_timer(20) {
  reset();
}

void Controller_c::reset() {
  selected = NONE;
  running = NONE;
  signal = 0;
  last_press_ms = 0;
  start_ms = 0;
  log_ms = 0;
  trial = 0;
  sample = 0;
}

uint8_t Controller_c::getSignal() const { return signal; }

void Controller_c::setSignal(uint8_t requested_signal) {
  if (requested_signal == 0) { reset(); return; }
  if (signal == 0) { signal = 1; }
}

const char *Controller_c::name(Test t) {
  switch (t) {
    case OFFSET:   return "offset";
    case NOISE:    return "noise";
    case STEP:     return "step";
    case EDGE:     return "edge";
    case STRAIGHT: return "straight";
    case SPIN:     return "spin";
    default:       return "idle";
  }
}

void Controller_c::header(Test t) {
  Serial.print("#mode=");
  Serial.println(name(t));
  Serial.println("mode,trial,sample,t_ms,x_mm,y_mm,theta_rad,dn1,dn2,dn3,dn4,dn5");
}

void Controller_c::row(Robot_c &robot, unsigned long now) {
  Serial.print(name(running));
  Serial.print(','); Serial.print(trial);
  Serial.print(','); Serial.print(sample);
  Serial.print(','); Serial.print(now - start_ms);
  Serial.print(','); Serial.print(odometry.pose.x, 2);
  Serial.print(','); Serial.print(odometry.pose.y, 2);
  Serial.print(','); Serial.print(odometry.pose.theta, 4);
  for (int i = 0; i < 5; i++) {
    Serial.print(','); Serial.print(robot.surface.reading[i]);
  }
  Serial.println();
}

void Controller_c::start(Test t, unsigned long now) {
  running = t;
  start_ms = now;
  log_ms = now;
  sample = 0;
  trial++;
  header(t);
  Serial.print("#trial="); Serial.println(trial);
}

void Controller_c::stop(Robot_c &robot) {
  robot.setMotorPWM(0, 0);
  Serial.print("#stop="); Serial.println(name(running));
  running = NONE;
  selected = NONE;
  signal = 0;
}

void Controller_c::runStatic(Robot_c &robot, unsigned long now, uint32_t limit) {
  robot.setMotorPWM(0, 0);
  if (sample >= limit) { stop(robot); return; }
  row(robot, now);
  sample++;
}

void Controller_c::runDrive(Robot_c &robot, unsigned long now, float left,
                            float right, unsigned long duration_ms) {
  if (now - start_ms > duration_ms) { stop(robot); return; }
  robot.setMotorPWM(left, right);
  row(robot, now);
  sample++;
}

void Controller_c::update(Robot_c &robot, RobotWifiAP_c &server) {
  unsigned long now = robot.getMillis();
  if (!update_timer.isReady(now)) return;
  update_timer.resetTimer(now);

  robot.getSurfaceSensors();
  if (robot.getEncoders()) {
    odometry.update(robot.getLeftEncoderCount(), robot.getRightEncoderCount());
  }

  // Button: press once to stop, or N times to choose test N.
  if (robot.isButtonPressed()) {
    last_press_ms = now;
    if (running != NONE) {
      stop(robot);
      return;
    }
    selected = (Test)((uint8_t)selected + 1);
    if (selected > SPIN) selected = OFFSET;
    for (int i = 0; i < (int)selected; i++) robot.playTone(600 + 80 * i, 40);
    delay(120);
  }

  if (running == NONE) {
    if (selected != NONE && now - last_press_ms > SELECT_TIMEOUT_MS) {
      odometry.reset(0, 0, 0);
      robot.getEncoders();
      start(selected, now);
    }
    robot.setMotorPWM(0, 0);
    return;
  }

  // Log at 50 Hz for the scan and the drives, at 100 Hz for static tests.
  unsigned long period = (running == EDGE || running == STRAIGHT ||
                          running == SPIN) ? 20 : 10;
  if (now - log_ms < period) return;
  log_ms = now;

  switch (running) {
    case OFFSET:   runStatic(robot, now, 200); break;
    case NOISE:    runStatic(robot, now, 2000); break;
    case STEP:     runStatic(robot, now, 1000000); break;
    case EDGE:     runDrive(robot, now, PWM_STRAIGHT, PWM_STRAIGHT, EDGE_MS); break;
    case STRAIGHT: runDrive(robot, now, PWM_CALIB, PWM_CALIB, STRAIGHT_MS); break;
    case SPIN:     runDrive(robot, now, -PWM_SPIN, PWM_SPIN, SPIN_MS); break;
    default:       break;
  }
}
