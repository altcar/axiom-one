#pragma once

// Fixed serpentine raster.
namespace ScanConfig {
constexpr unsigned ROWS = 13;
constexpr float ROW_LENGTH_MM = 200.0f;
constexpr float ROW_SPACING_MM = 5.0f;
constexpr float SAMPLE_STEP_MM = 2.0f;

constexpr float ROW_PWM = 40.0f;
constexpr float SHIFT_PWM = 35.0f;
constexpr float SLOW_PWM = 30.0f;
constexpr float TURN_PWM = 35.0f;
constexpr float TURN_BRAKE_LEAD_S = 0.030f; // Initial coast allowance; verify on the robot.
constexpr unsigned long TURN_SETTLE_MS = 200; // Unchanged wheel counts and low turn rate.
constexpr float WHEEL_RADIUS_MM = 16.0f; // Shared effective radius; calibrate first.
constexpr float WHEELBASE_MM = 84.0f; // Provisional effective value, not ground truth.
constexpr unsigned long UPDATE_MS = 10;
// Use the same fused pose for steering and reconstruction; false allows comparison.
constexpr bool USE_HEADING_FUSION = true;
constexpr float HEADING_CORRECTION_TAU_S = 1.0f; // Working value, not physically tuned.
constexpr float GYRO_Z_SIGN = 1.0f; // Verify that a left turn gives a positive angle.
constexpr unsigned long GYRO_SETTLE_MS = 1000;
constexpr unsigned GYRO_CALIBRATION_SAMPLES = 200;
constexpr unsigned long GYRO_CALIBRATION_US = 2000000;
constexpr float GYRO_MAX_BIAS_DPS = 5.0f;
constexpr float GYRO_MAX_STD_DPS = 0.5f;
constexpr unsigned long TOTAL_TIMEOUT_MS = 300000;
constexpr float POSITION_LIMIT_MM = 280.0f;

// Public demonstration credentials. Replace these if your group needs privacy.
constexpr char WIFI_NAME[] = "RasterRobot";
constexpr char WIFI_PASSWORD[] = "raster-demo";
constexpr unsigned short TCP_PORT = 80;

static_assert(HEADING_CORRECTION_TAU_S > 0, "Heading correction time must be positive");
static_assert(TURN_BRAKE_LEAD_S >= 0 && TURN_BRAKE_LEAD_S <= 0.1f,
              "Turn brake lead must be within 0 to 100 ms");
static_assert(TURN_SETTLE_MS >= 100, "Allow at least 100 ms of stillness after turning");
static_assert(ROWS >= 1, "At least one scan row is required");
static_assert(WHEEL_RADIUS_MM > 0 && WHEELBASE_MM > 0, "Geometry must be positive");
static_assert(ROW_LENGTH_MM > 0 && ROW_SPACING_MM > 0 && SAMPLE_STEP_MM > 0,
              "Scan distances must be positive");
}
