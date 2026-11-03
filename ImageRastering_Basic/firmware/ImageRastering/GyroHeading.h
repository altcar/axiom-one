#pragma once
#include <cmath>
#include <cstdint>
#include "Config.h"

// Independent gyro yaw, also used in fusion. Not an absolute heading or a
// drift-free estimate. Feed only fresh, checked Z-gyro samples at +/-245 dps.
class GyroHeading {
  public:
    bool calibrated = false;
    const char *error = nullptr;
    unsigned samples = 0;
    float bias_dps = 0, std_dps = 0, angle_deg = 0, rate_dps = 0;

    void beginCalibration() { *this = GyroHeading{}; }

    void add(int16_t raw_z, uint32_t time_us, bool wheels_still) {
      if (error) return;
      const uint32_t dt = time_us - previous_us;
      if (have_previous && dt == 0) return; // Never integrate a timestamp twice.
      if (have_previous && dt > 50000) { error = "imu_sample_gap"; return; }
      const float raw_dps = raw_z * 0.00875f;
      if (std::abs(int(raw_z)) >= 32000) { error = "imu_range_limit"; return; }
      if (!have_previous) first_us = time_us;
      previous_us = time_us;
      have_previous = true;
      if (!calibrated) {
        if (!wheels_still) { error = "imu_calibration_moved"; return; }
        ++samples;
        const float delta = raw_dps - bias_dps;
        bias_dps += delta / samples;
        squared_deviation += delta * (raw_dps - bias_dps);
        if (samples >= ScanConfig::GYRO_CALIBRATION_SAMPLES &&
            uint32_t(time_us - first_us) >= ScanConfig::GYRO_CALIBRATION_US) {
          std_dps = std::sqrt(squared_deviation / (samples - 1));
          if (std::fabs(bias_dps) > ScanConfig::GYRO_MAX_BIAS_DPS ||
              std_dps > ScanConfig::GYRO_MAX_STD_DPS) {
            error = "imu_calibration_unstable";
            return;
          }
          calibrated = true;
          rate_dps = previous_rate_dps = ScanConfig::GYRO_Z_SIGN * (raw_dps - bias_dps);
        }
        return;
      }
      rate_dps = ScanConfig::GYRO_Z_SIGN * (raw_dps - bias_dps);
      angle_deg += 0.5f * (previous_rate_dps + rate_dps) * (dt * 0.000001f);
      previous_rate_dps = rate_dps;
    }

    // Reset alongside encoder pose once, at the beginning of the first row.
    void zeroAngle() { angle_deg = 0; }

  private:
    bool have_previous = false;
    uint32_t previous_us = 0, first_us = 0;
    float squared_deviation = 0, previous_rate_dps = 0;
};
