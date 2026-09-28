#pragma once
#include <stdint.h>

// Temperature history of one probe, kept in RAM only (lost on restart). Plain C++ without
// Arduino, so test/test_history runs on the host: pio test -e native
//
// Two tiers: the fine tier holds the last 30 minutes at 10 s per value, the coarse tier the whole
// cook. The coarse tier starts at 60 s per value; when its 120 slots are full, neighbouring pairs
// are merged and the interval doubles, so it never runs out and never grows.
namespace history {

constexpr int      FINE_SIZE               = 180;
constexpr uint32_t FINE_INTERVAL_S         = 10;
constexpr int      COARSE_SIZE             = 120;
constexpr uint32_t COARSE_START_INTERVAL_S = 60;
constexpr uint32_t RESET_AFTER_UNPLUGGED_S = 600;        // a probe unplugged this long starts fresh
constexpr int16_t  NO_VALUE                = INT16_MIN;  // a gap: the probe was unplugged
constexpr int32_t  ETA_UNKNOWN             = -1;

// Tenths of a degree, NO_VALUE for NaN or infinity, clamped to the int16 range
int16_t to_tenths(float celcius);

class ProbeHistory {
public:
    // Adds one sample. Call every FINE_INTERVAL_S seconds with the seconds since boot.
    void add(uint32_t now_s, bool connected, float celcius);
    void clear();

    int      fine_count() const        { return fine_count_; }
    int16_t  fine_at(int i) const;     // i = 0 is the oldest
    uint32_t fine_newest_s() const     { return fine_newest_s_; }

    int      coarse_count() const      { return coarse_count_; }
    int16_t  coarse_at(int i) const    { return coarse_[i]; }   // i = 0 is the oldest
    uint32_t coarse_interval_s() const { return coarse_interval_s_; }
    uint32_t coarse_newest_s() const   { return coarse_newest_s_; }

    // Seconds until target_celcius at the rate of the last 10 minutes, or ETA_UNKNOWN when there
    // are less than 3 minutes of readings, the temperature isn't rising by at least 0.05 degrees
    // per minute, the target is already reached, or it would take more than 24 hours.
    int32_t eta_seconds(float target_celcius) const;

private:
    int16_t  fine_[FINE_SIZE];
    int      fine_head_         = 0;     // next write position in the ring
    int      fine_count_        = 0;
    uint32_t fine_newest_s_     = 0;

    int16_t  coarse_[COARSE_SIZE];
    int      coarse_count_      = 0;
    uint32_t coarse_interval_s_ = COARSE_START_INTERVAL_S;
    uint32_t coarse_newest_s_   = 0;

    // The coarse slot being filled
    int32_t  slot_sum_          = 0;     // tenths
    int      slot_valid_        = 0;
    int      slot_samples_      = 0;

    bool     plugged_in_        = false;
    uint32_t unplugged_since_s_ = 0;

    void add_value(uint32_t now_s, int16_t value);
};

}
