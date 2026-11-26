#ifndef SCAN_LOG_H
#define SCAN_LOG_H

#include <Arduino.h>

/**
 * @brief One logged sample: where the robot thought it was, and what the five
 * surface sensors read at that moment. The dashboard turns these into an image.
 *
 * x, y and theta come from the StampC3's own OdometryModel_c (mm, mm, rad).
 * dn[0..4] are DN1..DN5 exactly as the robot reported them (higher = darker).
 */
struct ScanSample_t {
  uint32_t t_ms;     // StampC3 millis() when the sample was logged
  float x;           // mm
  float y;           // mm
  float theta;       // rad
  uint16_t dn[5];    // DN1..DN5 raw readings
  uint8_t state;     // Controller_c::Mode value at the time (see Controller.h)
  uint8_t row;       // raster row index (0 = first row)
};

/**
 * @brief Ring buffer of the current scan run.
 *
 * Every sample gets a sequence number (0, 1, 2, ...). The buffer keeps the
 * newest capacity() samples. The dashboard downloads samples while the scan
 * runs, so a scan can be much longer than the buffer as long as the
 * dashboard stays open. Samples overwritten before anyone downloaded them
 * are counted by dropped().
 */
class ScanLog_c {
  public:
    ScanLog_c();

    /**
     * @brief Allocates storage, trying smaller sizes if memory is short.
     * @param requested_capacity Samples wanted.
     * @param keep_free_bytes Heap that must remain free afterwards.
     * @return The number of samples that fit (0 if allocation failed).
     */
    uint32_t begin(uint32_t requested_capacity, uint32_t keep_free_bytes);

    /** @brief Frees the storage (capacity becomes 0). */
    void release();

    /** @brief Starts a new run: forgets all samples (storage is kept). */
    void clear();

    /** @brief Appends one sample, overwriting the oldest when the buffer is full. */
    void add(const ScanSample_t &sample);

    /** @brief Total samples logged this run (next sequence number). */
    uint32_t count() const;

    /** @brief Oldest sequence number still stored. */
    uint32_t oldest() const;

    uint32_t capacity() const;

    /** @brief Returns the sample with sequence number seq (oldest() <= seq < count()). */
    const ScanSample_t &get(uint32_t seq) const;

    /** @brief Records that a client has downloaded every sample before seq_end. */
    void markSent(uint32_t seq_end);

    /** @brief Samples overwritten before any client downloaded them. */
    uint32_t dropped() const;

  private:
    ScanSample_t *samples;
    uint32_t cap;
    uint32_t total;
    uint32_t sent;
    uint32_t lost;
};

#endif
