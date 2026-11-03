#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Robot.h"
#include "RobotWifiAP.h"
#include "TaskTimer.h"
#include "OdometryModel.h"
#include "Config.h"
#include "RobotIMU.h"
#include "GyroHeading.h"
#include "FusedOdometry.h"

/**
 * @brief Supplies the fixed-spacing raster controller for the StampC3 example.
 *
 * Controller_c owns the controller state, while Robot_c owns the latest hardware
 * snapshot and RobotWifiAP_c owns the required telemetry transport. ScanConfig
 * contains the scan parameters; they are not a calibration of every robot.
 */
class Controller_c {
  
  public:
    /** @brief Editable local encoder-to-pose model; inspect odometry.pose.
     * @note The raster start explicitly resets this estimate for a new trial.
     */
    OdometryModel_c odometry{ScanConfig::WHEEL_RADIUS_MM, ScanConfig::WHEELBASE_MM, 358.3f};

    /** @brief Call after Robot_c starts Wire; enables the checked gyro reader. */
    bool initialiseIMU() { return imu_available = imu.initialiseGyroscope(); }

    /**
     * @brief Runs one time-controlled acquisition, control, and telemetry cycle.
     * @param robot Hardware-facing interface and cached measurement snapshot.
     * @param server Required WiFi/TCP connection for logging and the heartbeat.
     * @note Each accepted update cycle attempts to refresh sensors and encoders.
     * Failed reads retain their previous cached values; local odometry is updated
     * only after a successful encoder read. The method returns early between
     * update intervals. A stopped raster requires a board reset before restarting.
     */
    void update(Robot_c &robot, RobotWifiAP_c &server);

  private:
    /** @brief Raster phases, also written to the CSV phase column (ROW = 2). */
    enum RasterPhase {
      RASTER_WAIT,
      RASTER_BASELINE,
      RASTER_ROW,
      RASTER_SETTLE,
      RASTER_TURN_OUT,
      RASTER_SHIFT,
      RASTER_TURN_IN,
      RASTER_DONE,
      RASTER_SHIFT_SETTLE
    };

    RasterPhase raster_phase = RASTER_WAIT, raster_next_phase = RASTER_ROW;
    TaskTimer_c update_timer{ScanConfig::UPDATE_MS};
    unsigned int raster_row = 0;
    unsigned long raster_start_ms = 0, raster_phase_ms = 0, raster_log_ms = 0;
    unsigned long raster_quiet_ms = 0, raster_fresh_ms = 0;
    unsigned long raster_heartbeat_ms = 0, raster_alignment_ms = 0;
    uint32_t raster_stamp = 0;
    bool raster_has_stamp = false, raster_button_armed = false;
    bool raster_heartbeat_seen = false;
    float raster_anchor_x = 0, raster_anchor_y = 0, raster_target_heading = 0;
    float raster_next_sample_mm = 0;
    unsigned long raster_shift_start_ms = 0;
    float raster_shift_direction = 1.0f;
    unsigned int raster_position_retries = 0;
    int32_t raster_turn_left_count = 0, raster_turn_right_count = 0;
    int32_t raster_stop_left_count = 0, raster_stop_right_count = 0;
    unsigned long raster_still_ms = 0;
    unsigned int raster_turn_retries = 0;
    float raster_turn_rate_dps = 0;
    bool raster_cycle_seen = false;
    uint32_t raster_cycle_ms = 0, raster_previous_cycle_ms = 0, raster_timing_log_ms = 0;
    uint32_t raster_max_gap_ms = 0, raster_max_surface_ms = 0;
    uint32_t raster_max_encoder_ms = 0, raster_max_work_ms = 0;
    RobotIMU_c imu;
    GyroHeading gyro_heading;
    FusedOdometry fused_odometry;
    bool imu_available = false, imu_read_ok = false, imu_sample_seen = false;
    bool imu_fresh = false, imu_calibrating = false;
    int16_t imu_raw_z = 0;
    uint32_t imu_time_us = 0, imu_sample_ms = 0, imu_log_ms = 0;
    uint32_t imu_quiet_ms = 0;
    int32_t imu_calibration_left = 0, imu_calibration_right = 0;

    /** @brief Requests a motor stop and latches a control-timing or read fault. */
    void stopRasterFault(Robot_c &robot, RobotWifiAP_c &server, const char *reason,
                         const char *stage, unsigned long duration_ms);

    /** @brief Applies raster motion, start/stop checks and five-sensor logging. */
    void runRaster(Robot_c &robot, RobotWifiAP_c &server,
                   bool surface_ok, bool encoder_ok,
                   bool button_pressed, unsigned long now);
};

#endif
