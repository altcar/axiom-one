// Synthetic samples exercise the estimator, not physical gyro accuracy.
#include <cassert>
#include <cstdio>
#include "GyroHeading.h"

void calibrate(GyroHeading &g, uint32_t base = 0) {
  g.beginCalibration();
  for (unsigned i = 0; i <= 200; ++i) g.add(100, base + i * 10000, true);
  assert(g.calibrated && !g.error && std::fabs(g.bias_dps - .875f) < 1e-5);
}

int main() {
  GyroHeading g; calibrate(g);
  for (unsigned i = 201; i <= 1200; ++i) g.add(100, i * 10000, true);
  assert(g.angle_deg == 0); // Constant bias removed, including a long still trace.
  g.add(10100, 12010000, true); // 87.5 dps after bias removal, half step at onset.
  for (unsigned i = 1202; i <= 1300; ++i) g.add(10100, i * 10000, true);
  assert(std::fabs(g.angle_deg - 87.0625f) < .002);
  const float before = g.angle_deg;
  g.add(10100, 13000000, true);
  assert(g.angle_deg == before); // Duplicate timestamp does not add angle.
  g.zeroAngle(); assert(g.angle_deg == 0 && g.calibrated);

  calibrate(g, UINT32_MAX - 1000000); // micros() wraps during calibration.
  uint32_t t = uint32_t(UINT32_MAX - 1000000) + 2000000U;
  g.add(-9900, t += 7000, true);
  g.add(-9900, t += 13000, true);
  assert(std::fabs(g.angle_deg - (-87.5f * .0165f)) < .0001f);
  const float valid_angle = g.angle_deg;
  g.add(-9900, t += 50001, true);
  assert(g.error && g.angle_deg == valid_angle); // Do not bridge missing time.

  g.beginCalibration(); g.add(0, 0, false);
  assert(g.error && !g.calibrated);
  g.beginCalibration();
  for (unsigned i = 0; i <= 200; ++i) g.add(i % 2 ? 200 : -200, i * 10000, true);
  assert(g.error && !g.calibrated); // Excess variation is not a valid zero bias.
  g.beginCalibration();
  for (unsigned i = 0; i <= 200; ++i) g.add(1000, i * 10000, true);
  assert(g.error && !g.calibrated); // Large constant rotation/bias also rejected.
  calibrate(g); g.add(32767, 2010000, true); assert(g.error);
  std::puts("PASS: gyro bias, stillness checks, signed yaw, real dt, wrap and invalid gaps (synthetic).");
}
