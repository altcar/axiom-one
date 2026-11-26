#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Robot.h"
#include "RobotWifiAP.h"
#include "TaskTimer.h"
#include "OdometryModel.h"

/**
 * @brief Sensor and locomotion characterisation tests for the 3Pi+.
 *
 * Replaces the line-following demo with six short tests. Every test prints
 * labelled CSV over Serial at 115200. Press the StampC3 button N times to
 * choose test N, then wait 1.5 s and it starts. Press once to stop.
 *
 *   1 offset   stationary, 200 snapshots on one surface
 *   2 noise    stationary, 2000 snapshots on one surface
 *   3 step     stationary stream, change surface between trials
 *   4 edge     drive straight at PWM 30 for 4 s
 *   5 straight drive straight at PWM 40 for 3 s, for wheel-radius calibration
 *   6 spin     rotate on the spot at PWM 35 for 2 s, for separation calibration
 *
 * CSV: mode,trial,sample,t_ms,x_mm,y_mm,theta_rad,dn1,dn2,dn3,dn4,dn5
 */
class Controller_c {

  public:
    OdometryModel_c odometry;

    static const unsigned long UPDATE_INTERVAL_MS = 10;
    static const unsigned long SELECT_TIMEOUT_MS = 1500;
    static const unsigned long IDLE_TIMEOUT_MS = 3000;

    Controller_c();

    void reset();
    uint8_t getSignal() const;
    void setSignal(uint8_t requested_signal);
    void update(Robot_c &robot, RobotWifiAP_c &server);

  private:
    enum Test : uint8_t {
      NONE = 0, OFFSET = 1, NOISE = 2, STEP = 3,
      EDGE = 4, STRAIGHT = 5, SPIN = 6
    };

    Test selected;
    Test running;
    uint8_t signal;
    unsigned long last_press_ms;
    unsigned long start_ms;
    unsigned long log_ms;
    uint32_t trial;
    uint32_t sample;

    TaskTimer_c update_timer;
    TaskTimer_c log_timer;

    static const char *name(Test t);
    void start(Test t, unsigned long now);
    void stop(Robot_c &robot);
    void header(Test t);
    void row(Robot_c &robot, unsigned long now);
    void runStatic(Robot_c &robot, unsigned long now, uint32_t limit);
    void runDrive(Robot_c &robot, unsigned long now, float left, float right,
                  unsigned long duration_ms);
};

#endif
