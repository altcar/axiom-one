#include "RobotIMU.h"

namespace {
// Choose a 104 Hz sensor output rate close to the controller's nominal 100 Hz
// update rate. The raster controller uses the checked Z-gyro reader below;
// the original raw multi-sensor interface is retained for course exercises.
constexpr uint8_t LSM6_ACCELEROMETER_104_HZ_2_G = 0x40;
constexpr uint8_t LSM6_GYROSCOPE_104_HZ_245_DPS = 0x40;
constexpr uint8_t LSM6_BLOCK_UPDATE_AND_AUTO_INCREMENT = 0x44;
}

RobotIMU_c::RobotIMU_c()
  : lsm6_available(false),
    magnetometer_available(false),
    sample_sequence(0) {
  memset(&sample, 0, sizeof(sample));
}

bool RobotIMU_c::initialise() {
  lsm6.setBus(&Wire);
  lsm6_available = lsm6.init(LSM6::device_DS33, LSM6::sa0_auto);
  if (lsm6_available) {
    // Establish the library's known baseline, then replace its 1.66 kHz output
    // rates with settings suited to polling near the controller's 10 ms interval.
    // This does not add IMU acquisition to the controller.
    lsm6.enableDefault();
    lsm6.writeReg(
      LSM6::CTRL1_XL,
      LSM6_ACCELEROMETER_104_HZ_2_G
    );
    lsm6.writeReg(
      LSM6::CTRL2_G,
      LSM6_GYROSCOPE_104_HZ_245_DPS
    );
    lsm6.writeReg(
      LSM6::CTRL3_C,
      LSM6_BLOCK_UPDATE_AND_AUTO_INCREMENT
    );
  }

  // The Pololu LIS3MDL library uses the default Wire bus, which is also the
  // bus used by Robot_c and the LSM6 above.
  magnetometer_available = magnetometer.init(
    LIS3MDL::device_LIS3MDL,
    LIS3MDL::sa1_auto
  );
  if (magnetometer_available) {
    magnetometer.enableDefault();
  }

  sample_sequence = 0;
  memset(&sample, 0, sizeof(sample));
  return lsm6_available;
}

bool RobotIMU_c::isAvailable() const {
  return lsm6_available;
}

bool RobotIMU_c::readGyroRegisters(uint8_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(gyro_address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(gyro_address, count) != count) return false;
  for (uint8_t i = 0; i < count; i++) {
    const int value = Wire.read();
    if (value < 0) return false;
    data[i] = uint8_t(value);
  }
  return true;
}

bool RobotIMU_c::initialiseGyroscope() {
  gyro_address = 0;
  if (!initialise()) return false;
  // The two LSM6DS33 addresses supported by the course interface.
  const uint8_t addresses[] = {0x6b, 0x6a};
  for (uint8_t address : addresses) {
    gyro_address = address;
    uint8_t identity, settings[2];
    if (readGyroRegisters(LSM6::WHO_AM_I, &identity, 1) && identity == 0x69 &&
        readGyroRegisters(LSM6::CTRL2_G, settings, 2) &&
        settings[0] == LSM6_GYROSCOPE_104_HZ_245_DPS &&
        (settings[1] & LSM6_BLOCK_UPDATE_AND_AUTO_INCREMENT) == LSM6_BLOCK_UPDATE_AND_AUTO_INCREMENT) {
      return true;
    }
  }
  gyro_address = 0;
  return false;
}

bool RobotIMU_c::readGyroscope(int16_t &z, uint32_t &timestamp_us, bool &fresh) {
  fresh = false;
  uint8_t status, data[6];
  if (!gyro_address || !readGyroRegisters(LSM6::STATUS_REG, &status, 1)) return false;
  if (!(status & 0x02)) return true;
  if (!readGyroRegisters(LSM6::OUTX_L_G, data, 6)) return false;
  z = int16_t(uint16_t(data[4]) | (uint16_t(data[5]) << 8));
  timestamp_us = micros();
  fresh = true;
  return true;
}

bool RobotIMU_c::isMagnetometerAvailable() const {
  return magnetometer_available;
}

bool RobotIMU_c::isSampleReady() {
  if (!lsm6_available) {
    return false;
  }

  const uint8_t ACCELEROMETER_READY = 0x01;
  const uint8_t GYROSCOPE_READY = 0x02;
  uint8_t status = lsm6.readReg(LSM6::STATUS_REG);
  return (status & (ACCELEROMETER_READY | GYROSCOPE_READY)) ==
         (ACCELEROMETER_READY | GYROSCOPE_READY);
}

bool RobotIMU_c::read() {
  if (!lsm6_available) {
    return false;
  }

  lsm6.read();
  sample.acceleration[X_AXIS] = lsm6.a.x;
  sample.acceleration[Y_AXIS] = lsm6.a.y;
  sample.acceleration[Z_AXIS] = lsm6.a.z;
  sample.gyroscope[X_AXIS] = lsm6.g.x;
  sample.gyroscope[Y_AXIS] = lsm6.g.y;
  sample.gyroscope[Z_AXIS] = lsm6.g.z;

  if (magnetometer_available) {
    magnetometer.read();
    sample.magnetic_field[X_AXIS] = magnetometer.m.x;
    sample.magnetic_field[Y_AXIS] = magnetometer.m.y;
    sample.magnetic_field[Z_AXIS] = magnetometer.m.z;
  }

  sample.timestamp_us = micros();
  sample.sequence = ++sample_sequence;
  return true;
}

const RobotIMUSample_t &RobotIMU_c::getSample() const {
  return sample;
}

float RobotIMU_c::accelerationToG(int16_t raw_value) {
  return raw_value * 0.000061f;
}

float RobotIMU_c::gyroscopeToDegreesPerSecond(int16_t raw_value) {
  return raw_value * 0.00875f;
}

float RobotIMU_c::magneticFieldToGauss(int16_t raw_value) {
  return raw_value / 6842.0f;
}
