#pragma once
#include <cmath>
#include <cstdint>
#include "OdometryModel.h"
#include "Config.h"

// Encoder distance with complementary encoder/gyro yaw. Both inputs are
// relative estimates: this cannot remove wheel slip or provide absolute yaw.
class FusedOdometry {
  public:
    OdometryModel_c::Pose pose{0, 0, 0};

    void reset(const OdometryModel_c::Pose &encoder, float gyro_deg, uint32_t time_us) {
      pose = previous_encoder = encoder;
      gyro_anchor = encoder.theta;
      previous_gyro_deg = gyro_deg;
      previous_gyro_us = time_us;
    }

    void update(const OdometryModel_c::Pose &encoder, float gyro_deg,
                uint32_t time_us, bool fresh) {
      const float old_heading = pose.theta;
      const uint32_t elapsed_us = time_us - previous_gyro_us;
      if (fresh && elapsed_us != 0) {
        const float predicted = gyro_anchor + (gyro_deg - previous_gyro_deg) * 0.0174532925f;
        const float difference = encoder.theta - predicted;
        const float error = std::atan2(std::sin(difference), std::cos(difference));
        const float dt = elapsed_us * 0.000001f;
        const float weight = dt / (ScanConfig::HEADING_CORRECTION_TAU_S + dt);
        pose.theta = predicted + weight * error;
        gyro_anchor = pose.theta;
        previous_gyro_deg = gyro_deg;
        previous_gyro_us = time_us;
      } else {
        // Predict between fresh gyro samples. The next fresh update starts
        // from gyro_anchor, so these encoder increments are not counted twice.
        pose.theta += encoder.theta - previous_encoder.theta;
      }
      // The unchanged teacher model uses Euler integration at its old heading.
      // Project its position increment back to signed wheel travel, then use
      // the fused midpoint heading to locate that same travel on our map.
      const float distance = (encoder.x - previous_encoder.x) * std::cos(previous_encoder.theta) +
                             (encoder.y - previous_encoder.y) * std::sin(previous_encoder.theta);
      const float midpoint = (old_heading + pose.theta) * 0.5f;
      pose.x += distance * std::cos(midpoint);
      pose.y += distance * std::sin(midpoint);
      previous_encoder = encoder;
    }

  private:
    OdometryModel_c::Pose previous_encoder{0, 0, 0};
    float gyro_anchor = 0, previous_gyro_deg = 0;
    uint32_t previous_gyro_us = 0;
};
