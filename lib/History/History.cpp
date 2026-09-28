#include <math.h>
#include "History.h"

namespace history {

namespace {
    constexpr int   ETA_WINDOW_SAMPLES    = 60;          // 10 minutes of fine samples
    constexpr int   ETA_MIN_SAMPLES       = 18;          // 3 minutes
    constexpr float ETA_MIN_SLOPE_PER_MIN = 0.05f;
    constexpr int32_t ETA_MAX_S           = 24 * 3600;

    int16_t average(int16_t a, int16_t b){
        if(a == NO_VALUE){ return b; }
        if(b == NO_VALUE){ return a; }
        return (int16_t)(((int32_t)a + b) / 2);
    }
}

int16_t to_tenths(float celcius){
    if(!isfinite(celcius)){ return NO_VALUE; }
    long tenths = lroundf(celcius * 10.0f);
    if(tenths >  32767){ return  32767; }
    if(tenths < -32767){ return -32767; }
    return (int16_t)tenths;
}

// Field by field: `*this = ProbeHistory()` would put a 600 byte temporary on the caller's stack
void ProbeHistory::clear(){
    fine_head_ = 0; fine_count_ = 0; fine_newest_s_ = 0;
    coarse_count_ = 0; coarse_interval_s_ = COARSE_START_INTERVAL_S; coarse_newest_s_ = 0;
    slot_sum_ = 0; slot_valid_ = 0; slot_samples_ = 0;
    plugged_in_ = false; unplugged_since_s_ = 0;
}

void ProbeHistory::add(uint32_t now_s, bool connected, float celcius){
    if(connected){
        plugged_in_ = true;
        add_value(now_s, to_tenths(celcius));
        return;
    }

    bool empty = fine_count_ == 0 && coarse_count_ == 0 && slot_samples_ == 0;
    if(empty){ return; }

    if(plugged_in_){
        plugged_in_        = false;
        unplugged_since_s_ = now_s;
    }
    if(now_s - unplugged_since_s_ > RESET_AFTER_UNPLUGGED_S){
        clear();
        return;
    }
    add_value(now_s, NO_VALUE);
}

void ProbeHistory::add_value(uint32_t now_s, int16_t value){
    fine_[fine_head_] = value;
    fine_head_        = (fine_head_ + 1) % FINE_SIZE;
    if(fine_count_ < FINE_SIZE){ fine_count_++; }
    fine_newest_s_    = now_s;

    slot_samples_++;
    if(value != NO_VALUE){
        slot_sum_ += value;
        slot_valid_++;
    }
    if(slot_samples_ * FINE_INTERVAL_S < coarse_interval_s_){ return; }

    coarse_[coarse_count_++] = slot_valid_ ? (int16_t)(slot_sum_ / slot_valid_) : NO_VALUE;
    coarse_newest_s_ = now_s;
    slot_sum_ = 0; slot_valid_ = 0; slot_samples_ = 0;

    // Full: merge neighbouring pairs right away, so every value in the tier has the same interval
    if(coarse_count_ == COARSE_SIZE){
        for(int i = 0; i < COARSE_SIZE / 2; i++){
            coarse_[i] = average(coarse_[2 * i], coarse_[2 * i + 1]);
        }
        coarse_count_       = COARSE_SIZE / 2;
        coarse_interval_s_ *= 2;
    }
}

int16_t ProbeHistory::fine_at(int i) const {
    return fine_[(fine_head_ - fine_count_ + i + FINE_SIZE) % FINE_SIZE];
}

int32_t ProbeHistory::eta_seconds(float target_celcius) const {
    int first = fine_count_ > ETA_WINDOW_SAMPLES ? fine_count_ - ETA_WINDOW_SAMPLES : 0;

    // Least squares line through the valid samples: x in seconds, y in degrees
    double n = 0, sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    int16_t newest = NO_VALUE;
    for(int i = first; i < fine_count_; i++){
        int16_t value = fine_at(i);
        if(value == NO_VALUE){ continue; }
        double x = (double)(i - first) * FINE_INTERVAL_S;
        double y = value / 10.0;
        n++; sum_x += x; sum_y += y; sum_xx += x * x; sum_xy += x * y;
        newest = value;
    }
    if(n < ETA_MIN_SAMPLES){ return ETA_UNKNOWN; }

    double denominator = n * sum_xx - sum_x * sum_x;
    if(denominator <= 0){ return ETA_UNKNOWN; }
    double slope_per_s = (n * sum_xy - sum_x * sum_y) / denominator;
    if(slope_per_s * 60.0 < ETA_MIN_SLOPE_PER_MIN){ return ETA_UNKNOWN; }

    double remaining = target_celcius - newest / 10.0;
    if(remaining <= 0){ return ETA_UNKNOWN; }

    double eta = remaining / slope_per_s;
    if(eta > ETA_MAX_S){ return ETA_UNKNOWN; }
    return (int32_t)(eta + 0.5);
}

}
