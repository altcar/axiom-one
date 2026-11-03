#pragma once
#include <cassert>
#include <cstdint>
#include <deque>
#include <vector>

struct TwoWire {
  struct Reply { int status; std::vector<uint8_t> bytes; };
  std::deque<Reply> replies;
  std::deque<uint8_t> received;
  uint8_t address = 0, reg = 0;
  void beginTransmission(uint8_t a) { address = a; }
  void write(uint8_t r) { reg = r; }
  int endTransmission(bool = true) {
    assert(!replies.empty());
    int status = replies.front().status;
    if (status) replies.pop_front();
    return status;
  }
  uint8_t requestFrom(uint8_t, uint8_t) {
    assert(!replies.empty());
    received.assign(replies.front().bytes.begin(), replies.front().bytes.end());
    replies.pop_front();
    return received.size();
  }
  int read() {
    if (received.empty()) return -1;
    int value = received.front(); received.pop_front(); return value;
  }
};
inline TwoWire Wire;
