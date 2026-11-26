#include "ScanLog.h"

ScanLog_c::ScanLog_c() : samples(nullptr), cap(0), total(0), sent(0), lost(0) {
}

uint32_t ScanLog_c::begin(uint32_t requested_capacity, uint32_t keep_free_bytes) {
  release();

  // Take the largest log that fits in one block while still leaving
  // keep_free_bytes of heap for WiFi and the web server. Step down in small
  // amounts so a slightly-too-big request doesn't lose half the log.
  uint32_t want = requested_capacity;
  while (want >= 250) {
    size_t bytes = (size_t)want * sizeof(ScanSample_t);
    if (ESP.getMaxAllocHeap() >= bytes && ESP.getFreeHeap() >= bytes + keep_free_bytes) {
      samples = (ScanSample_t *)malloc(bytes);
      if (samples != nullptr) {
        cap = want;
        break;
      }
    }
    want -= 250;
  }
  clear();
  return cap;
}

void ScanLog_c::release() {
  if (samples != nullptr) {
    free(samples);
    samples = nullptr;
  }
  cap = 0;
  clear();
}

void ScanLog_c::clear() {
  total = 0;
  sent = 0;
  lost = 0;
}

void ScanLog_c::add(const ScanSample_t &sample) {
  if (samples == nullptr || cap == 0) {
    lost++;
    return;
  }
  if (total >= cap) {
    // The slot being reused holds sample (total - cap). Count it as lost if
    // no client has downloaded it yet.
    if (total - cap >= sent) lost++;
  }
  samples[total % cap] = sample;
  total++;
}

uint32_t ScanLog_c::count() const {
  return total;
}

uint32_t ScanLog_c::oldest() const {
  return total > cap ? total - cap : 0;
}

uint32_t ScanLog_c::capacity() const {
  return cap;
}

const ScanSample_t &ScanLog_c::get(uint32_t seq) const {
  return samples[seq % cap];
}

void ScanLog_c::markSent(uint32_t seq_end) {
  if (seq_end > sent) sent = seq_end;
}

uint32_t ScanLog_c::dropped() const {
  return lost;
}
