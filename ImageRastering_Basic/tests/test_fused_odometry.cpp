// Deterministic estimator inputs, not measured robot performance.
#include <cassert>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include "FusedOdometry.h"

using Pose = OdometryModel_c::Pose;
constexpr float RAD = .0174532925f;
void near(float value, float expected, float tolerance = 1e-5f) {
  if (std::fabs(value - expected) >= tolerance) std::fprintf(stderr, "actual %.9f expected %.9f tolerance %.9f\n", value, expected, tolerance);
  assert(std::fabs(value - expected) < tolerance);
}

int main() {
  FusedOdometry f;
  Pose enc{0, 0, 0};
  f.reset(enc, 0, 0);
  // A residual 1 degree/s bias grows without limit in gyro-only yaw, but
  // settles near tau * bias when the stationary encoder reference is valid.
  for (unsigned i = 1; i <= 1000; ++i) f.update(enc, i * .01f, i * 10000U, true);
  near(f.pose.theta / RAD, ScanConfig::HEADING_CORRECTION_TAU_S, .002f);
  assert(f.pose.theta / RAD < 10.0f / 5);

  // Fresh data after an intermediate encoder prediction must not count the
  // intermediate turn twice. Duplicate timestamps behave like a missing read.
  f.reset(enc = {0, 0, 0}, 0, 10000);
  enc.theta = .1f; f.update(enc, 0, 10000, false); near(f.pose.theta, .1f);
  enc.theta = .15f; f.update(enc, 999, 10000, true); near(f.pose.theta, .15f);
  enc.theta = .2f; f.update(enc, .2f / RAD, 30000, true); near(f.pose.theta, .2f);
  enc.theta = .3f; f.update(enc, .2f / RAD, 30000, false); near(f.pose.theta, .3f);
  enc.theta = .4f; f.update(enc, .4f / RAD, 50000, true); near(f.pose.theta, .4f);

  // The uint32 microsecond clock can wrap without a discontinuity.
  f.reset(enc = {0, 0, 0}, 0, UINT32_MAX - 4999U);
  enc.theta = .02f; f.update(enc, .02f / RAD, 5000, true); near(f.pose.theta, .02f);

  // Keep continuous signed headings across +/-pi and several revolutions.
  for (int direction : {-1, 1}) {
    f.reset(enc = {0, 0, direction * 3.1f}, 0, 0);
    for (unsigned i = 1; i <= 1500; ++i) {
      enc.theta = direction * (3.1f + i * .01f);
      f.update(enc, direction * (i * .01f) / RAD, i * 10000U, true);
      near(f.pose.theta, enc.theta, .0001f);
    }
  }

  // Encoder Euler displacement is signed wheel travel, not path length.
  // Forward and reverse follow the fused midpoint rather than encoder axes.
  f.reset(enc = {0, 0, 0}, 0, 0);
  enc = {10, 0, .2f};
  f.update(enc, .2f / RAD, 10000, true);
  near(f.pose.x, 10 * std::cos(.1f)); near(f.pose.y, 10 * std::sin(.1f));
  const Pose before = f.pose;
  enc.x -= 4 * std::cos(.2f); enc.y -= 4 * std::sin(.2f); enc.theta = .4f;
  f.update(enc, .4f / RAD, 20000, true);
  near(f.pose.x, before.x - 4 * std::cos(.3f));
  near(f.pose.y, before.y - 4 * std::sin(.3f));

  // Reset establishes a new origin and gyro baseline without replaying old yaw.
  f.reset(enc = {7, -4, 1.2f}, 150, 123000);
  f.update(enc, 150, 133000, true);
  near(f.pose.x, 7); near(f.pose.y, -4); near(f.pose.theta, 1.2f);
  std::puts("PASS: bounded bias, freshness, timestamp wrap, unwrapped yaw and signed midpoint travel (synthetic).");
}
