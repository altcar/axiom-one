#pragma once
#include <cstdint>
#include <cstring>
inline unsigned long test_ms = 0;
inline unsigned long millis() { return test_ms; }
inline unsigned long micros() { return test_ms * 1000; }
constexpr float TWO_PI = 6.2831853071795864769f;
