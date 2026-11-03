#include <cassert>
#include <cstdio>
#include "RobotIMU.h"

int main() {
  RobotIMU_c imu;
  Wire.replies = {{0, {0x69}}, {0, {0x40, 0x44}}};
  assert(imu.initialiseGyroscope() && Wire.address == 0x6b);
  int16_t z = 123; uint32_t time = 456; bool fresh = true;
  Wire.replies = {{0, {0}}};
  assert(imu.readGyroscope(z, time, fresh) && !fresh && z == 123 && time == 456);
  Wire.replies = {{2, {}}};
  assert(!imu.readGyroscope(z, time, fresh) && !fresh && z == 123);
  Wire.replies = {{0, {}}};
  assert(!imu.readGyroscope(z, time, fresh) && !fresh && z == 123);
  Wire.replies = {{0, {2}}, {0, {0, 0, 0, 0, 0x9c}}};
  assert(!imu.readGyroscope(z, time, fresh) && !fresh && z == 123 && time == 456);
  Wire.replies = {{0, {2}}, {0, {0, 0, 0, 0, 0x9c, 0xff}}};
  test_ms = 900;
  assert(imu.readGyroscope(z, time, fresh) && fresh && z == -100 && time == 900000);
  Wire.replies = {{2, {}}, {0, {0x69}}, {0, {0x40, 0x44}}};
  assert(imu.initialiseGyroscope() && Wire.address == 0x6a);
  Wire.replies = {{0, {0x69}}, {0, {0x48, 0x44}}, {2, {}}};
  assert(!imu.initialiseGyroscope()); // Wrong range would invalidate the scale.
  assert(!imu.readGyroscope(z, time, fresh));
  std::puts("PASS: checked IMU configuration, data readiness and failed/short transfers (stubbed bus).");
}
