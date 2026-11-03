#pragma once
#include "Wire.h"
struct LSM6 {
  enum { device_DS33, sa0_auto, WHO_AM_I = 0x0f, CTRL1_XL = 0x10,
         CTRL2_G = 0x11, CTRL3_C = 0x12, STATUS_REG = 0x1e, OUTX_L_G = 0x22 };
  struct { int16_t x = 0, y = 0, z = 0; } a, g;
  void setBus(TwoWire *) {}
  bool init(int, int) { return true; }
  void enableDefault() {}
  void writeReg(int, int) {}
  uint8_t readReg(int) { return 0; }
  void read() {}
};
