#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Robot.h"
#include "RobotWifiAP.h"
#include "TaskTimer.h"
#include "OdometryModel.h"
#include "ScanLog.h"

/** Robot keeps facing +x and drives rows alternately forwards and in reverse,
 *  steering gently onto each new row line. No turns on the spot (default). */
#define PATTERN_BACK_AND_FORTH 0
/** Robot turns 90 degrees, shifts, and turns 90 degrees again at each row end. */
#define PATTERN_UTURN 1

/**
 * @brief Raster ("lawn-mower") scan settings. All can be changed from the
 * dashboard between runs without re-uploading code.
 *
 * Back-and-forth pattern (default): the robot never turns. Row 0 is driven
 * forwards along +x, row 1 in reverse along -x, and so on. Each row continues
 * margin_mm past the image at both ends; the robot steers onto the next row
 * line (y = row x pitch) during that run-in, so in-place turns, and the wheel
 * slip they cause, are avoided entirely.
 *
 * U-turn pattern:
 *
 * The robot starts with its wheel axle at the origin facing +x. It drives
 * row 0 along +x, turns 90 degrees, moves sideways by row_pitch_mm, turns 90
 * degrees again and drives row 1 back along -x, and so on:
 *
 *        row 2  -------------------->..
 *                                      :  (overrun, turn, shift, turn)
 *        row 1  ..<--------------------
 *               :
 *        row 0  o------------------->..   o = start (axle centre)
 *
 * The sensors sit sensor_lead_mm ahead of the axle, so on the way back they
 * would be 2 x sensor_lead_mm behind where they were on the way out. To scan
 * the same strip in both directions, every row (except the last) continues
 * for an extra 2 x sensor_lead_mm ("overrun", shown as ..) before turning.
 *
 * side = +1 shifts towards +y (the robot's LEFT at the start),
 * side = -1 shifts towards -y (the robot's RIGHT at the start).
 */
struct RasterConfig_t {
  uint8_t pattern;            // PATTERN_BACK_AND_FORTH (default) or PATTERN_UTURN
  float margin_mm;            // back-and-forth: run-in past each row end for the lane change
  uint8_t rows;               // number of scan rows
  float row_length_mm;        // length of each scanned row
  float row_pitch_mm;         // sideways step between rows
  float scan_pwm;             // forward PWM while scanning a row or shifting
  float turn_pwm;             // largest PWM used while turning on the spot
  float turn_min_pwm;         // smallest turning PWM (set just above your motor deadband)
  int8_t side;                // +1 = shift left, -1 = shift right
  float sensor_lead_mm;       // how far the sensor bar is ahead of the wheel axle
  float wheel_radius_left_mm; // odometry geometry (from your Phase 1 calibration)
  float wheel_radius_right_mm;
  float wheel_separation_mm;
};

/**
 * @brief Image-rasterisation controller for the 3Pi+ / StampC3.
 *
 * Keeps the template's public shape (update(), setSignal(), odometry) so it
 * can be ported to the Processing Digital Twin the same way as the supplied
 * line follower. While a scan runs it stores samples in scan_log, which the
 * web dashboard (WebDashboard.h) streams to a browser.
 */
class Controller_c {

  public:
    /** @brief Controller modes. The numbers are written into the log/CSV. */
    enum Mode : uint8_t {
      WAITING      = 0,  // idle, waiting for button or dashboard Start
      SETTLING     = 1,  // short stop between moves (lets the robot settle)
      SCANNING_ROW = 2,  // driving a row: these samples make the image
      TURNING_OUT  = 3,  // 90 degree turn at the end of a row
      SHIFTING     = 4,  // sideways move to the next row
      TURNING_IN   = 5,  // 90 degree turn onto the next row
      FINISHED     = 6,  // all rows done
      STOPPED      = 7,  // stopped by user or by a safety timeout
      OVERRUN      = 8,  // U-turn: extra travel past the row end (see RasterConfig_t)
      MARGIN       = 9   // back-and-forth: run-in/run-out outside the image area
    };

    /** @brief Local encoder-to-pose model. Reset at the start of every scan. */
    OdometryModel_c odometry;

    /** @brief Samples from the current/last scan (read by the dashboard). */
    ScanLog_c scan_log;

    /** @brief Current scan settings (edit defaults in Controller.cpp). */
    RasterConfig_t config;

    // ---- Timing -----------------------------------------------------------
    static constexpr unsigned long UPDATE_INTERVAL_MS = 10;     // 100 Hz control loop
    static constexpr unsigned long TELEMETRY_INTERVAL_MS = 50;  // template CSV (Serial + TCP)
    static constexpr unsigned long LOG_INTERVAL_MS = 20;        // 50 Hz scan log
    static constexpr unsigned long START_DELAY_MS = 1000;       // time to take your hand away
    static constexpr unsigned long SETTLE_MS = 300;             // pause between moves
    static constexpr unsigned long RAMP_MS = 300;               // soft start on straights

    // ---- Control gains ----------------------------------------------------
    static constexpr float HEADING_GAIN = 60.0f;          // PWM per rad of heading error
    static constexpr float MAX_HEADING_CORRECTION = 15.0f; // PWM limit for steering
    static constexpr float TURN_GAIN = 40.0f;             // PWM per rad while turning
    static constexpr float TURN_TOLERANCE_RAD = 0.03f;    // about 1.7 degrees
    static constexpr float TURN_BOOST_STEP = 3.0f;        // extra PWM when a turn stalls
    static constexpr float TURN_BOOST_MAX = 25.0f;
    static constexpr unsigned long TURN_STALL_MS = 250;   // no progress for this long = stalled
    static constexpr float LINE_LOOKAHEAD_MM = 35.0f;     // row-line steering: larger = gentler
    static constexpr float MAX_LINE_ANGLE_RAD = 0.45f;    // about 26 degrees during a lane change

    // ---- Safety -----------------------------------------------------------
    static constexpr unsigned long TURN_TIMEOUT_MS = 6000;
    static constexpr unsigned long STRAIGHT_TIMEOUT_MS = 30000;

    Controller_c();

    /** @brief Returns to WAITING. Sends no motor command (same as template). */
    void reset();

    /** @brief 1 while a scan runs, 0 otherwise (telemetry column 14; the Digital Twin syncs on it). */
    uint8_t getSignal() const;

    /** @brief Non-zero starts a scan (if idle); zero returns to waiting. */
    void setSignal(uint8_t requested_signal);

    /** @brief Runs one 10 ms acquisition, control, logging and telemetry cycle. */
    void update(Robot_c &robot, RobotWifiAP_c &server);

    /** @brief Ask for a new scan to start on the next update (dashboard/button). */
    void requestStart();

    /** @brief Ask the robot to stop on the next update (dashboard/button). */
    void requestStop();

    /** @brief Applies new settings. Refused (returns false) while a scan is running. */
    bool setConfig(const RasterConfig_t &new_config);

    /** @brief Empties the scan log. Refused while a scan is running. */
    bool clearLog();

    bool isRunning() const;
    Mode getMode() const;
    const char *getModeName() const;
    uint8_t getRow() const;
    uint32_t getRunId() const;
    const char *getMessage() const;

  private:
    uint8_t signal;
    Mode mode;
    Mode next_mode;               // where SETTLING goes next
    unsigned long mode_start_ms;
    unsigned long settle_ms;
    uint8_t row;
    float target_heading;         // rad
    float segment_start_x;        // mm, pose when the current move began
    float segment_start_y;
    int8_t pass_dir;              // back-and-forth: +1 forwards, -1 reverse
    float row_y;                  // y of the current row line (mm)
    float turn_boost;             // extra turning PWM after a stall
    float turn_check_theta;       // heading at the last stall check
    unsigned long turn_check_ms;
    uint32_t run_id;
    uint32_t log_tick;            // counts log intervals (for half-rate turn logging)
    bool start_requested;
    bool stop_requested;
    const char *message;

    TaskTimer_c update_timer;
    TaskTimer_c telemetry_timer;
    TaskTimer_c log_timer;

    void beginScan(Robot_c &robot, unsigned long now);
    void runRaster(Robot_c &robot, unsigned long now);
    void enterMode(Mode new_mode, unsigned long now);
    void settleThen(Robot_c &robot, unsigned long now, unsigned long duration_ms, Mode then);
    void finish(Robot_c &robot, const char *why);
    void fail(Robot_c &robot, const char *why);
    void driveStraight(Robot_c &robot, float base_pwm, unsigned long now);
    void driveOnRowLine(Robot_c &robot, float line_heading, int8_t dir, unsigned long now);
    void runBackAndForth(Robot_c &robot, unsigned long now);
    void endOfRow(Robot_c &robot, unsigned long now);
    void turnInPlace(Robot_c &robot, float heading_error, unsigned long now);
    float headingError() const;
    float distanceFromSegmentStart() const;
    void logSample(Robot_c &robot, unsigned long now);
    void publishTelemetry(Robot_c &robot, RobotWifiAP_c &server, unsigned long timestamp_ms);

    static float wrapAngle(float angle);
    static float rowHeading(uint8_t row_index);
};

#endif
