#pragma once
#include <cstdint>
struct LIS3MDL {
  enum { device_LIS3MDL, sa1_auto };
  struct { int16_t x = 0, y = 0, z = 0; } m;
  bool init(int, int) { return false; }
  void enableDefault() {}
  void read() {}
};
