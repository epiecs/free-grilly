#include <math.h>
#include <unity.h>
#include "History.h"

using namespace history;

static ProbeHistory h;
static uint32_t t;

void setUp(){ h.clear(); t = 1000; }
void tearDown(){}

// Adds `count` connected samples FINE_INTERVAL_S apart, rising `per_sample` degrees each
static void feed(int count, float start, float per_sample){
    for(int i = 0; i < count; i++){
        h.add(t, true, start + per_sample * i);
        t += FINE_INTERVAL_S;
    }
}

static void feed_unplugged(int count){
    for(int i = 0; i < count; i++){
        h.add(t, false, NAN);
        t += FINE_INTERVAL_S;
    }
}

void test_to_tenths(){
    TEST_ASSERT_EQUAL_INT16(714, to_tenths(71.44f));
    TEST_ASSERT_EQUAL_INT16(-50, to_tenths(-5.04f));
    TEST_ASSERT_EQUAL_INT16(NO_VALUE, to_tenths(NAN));
    TEST_ASSERT_EQUAL_INT16(32767, to_tenths(5000.0f));
}

void test_fine_keeps_the_last_180_samples(){
    feed(200, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL_INT(FINE_SIZE, h.fine_count());
    TEST_ASSERT_EQUAL_INT16(200, h.fine_at(0));       // sample 20 = 20.0 degrees
    TEST_ASSERT_EQUAL_INT16(1990, h.fine_at(179));    // sample 199
    TEST_ASSERT_EQUAL_UINT32(t - FINE_INTERVAL_S, h.fine_newest_s());
}

void test_coarse_slot_averages_six_samples(){
    feed(5, 10.0f, 1.0f);
    TEST_ASSERT_EQUAL_INT(0, h.coarse_count());
    h.add(t, true, 15.0f);
    TEST_ASSERT_EQUAL_INT(1, h.coarse_count());
    TEST_ASSERT_EQUAL_INT16(125, h.coarse_at(0));     // average of 10..15 degrees
    TEST_ASSERT_EQUAL_UINT32(COARSE_START_INTERVAL_S, h.coarse_interval_s());
    TEST_ASSERT_EQUAL_UINT32(t, h.coarse_newest_s());
}

void test_coarse_merges_pairs_and_doubles_the_interval_when_full(){
    for(int slot = 0; slot < COARSE_SIZE; slot++){ feed(6, slot, 0.0f); }
    TEST_ASSERT_EQUAL_INT(COARSE_SIZE / 2, h.coarse_count());
    TEST_ASSERT_EQUAL_UINT32(2 * COARSE_START_INTERVAL_S, h.coarse_interval_s());
    TEST_ASSERT_EQUAL_INT16(5, h.coarse_at(0));       // (0 + 10) / 2 tenths
    TEST_ASSERT_EQUAL_INT16(1185, h.coarse_at(59));   // (1180 + 1190) / 2

    feed(11, 50.0f, 0.0f);                            // a 120 s slot needs 12 samples
    TEST_ASSERT_EQUAL_INT(60, h.coarse_count());
    h.add(t, true, 50.0f);
    TEST_ASSERT_EQUAL_INT(61, h.coarse_count());
    TEST_ASSERT_EQUAL_INT16(500, h.coarse_at(60));
}

void test_short_unplug_leaves_a_gap(){
    feed(6, 20.0f, 0.0f);
    feed_unplugged(6);
    feed(6, 22.0f, 0.0f);
    TEST_ASSERT_EQUAL_INT(18, h.fine_count());
    TEST_ASSERT_EQUAL_INT16(NO_VALUE, h.fine_at(6));
    TEST_ASSERT_EQUAL_INT(3, h.coarse_count());
    TEST_ASSERT_EQUAL_INT16(200, h.coarse_at(0));
    TEST_ASSERT_EQUAL_INT16(NO_VALUE, h.coarse_at(1));
    TEST_ASSERT_EQUAL_INT16(220, h.coarse_at(2));
}

void test_clears_after_ten_minutes_unplugged(){
    feed(10, 20.0f, 0.0f);
    feed_unplugged(61);                               // unplugged for 600 s: kept
    TEST_ASSERT_TRUE(h.fine_count() > 0);
    feed_unplugged(1);                                // 610 s: cleared
    TEST_ASSERT_EQUAL_INT(0, h.fine_count());
    TEST_ASSERT_EQUAL_INT(0, h.coarse_count());
    feed_unplugged(10);                               // stays empty
    TEST_ASSERT_EQUAL_INT(0, h.fine_count());
}

void test_unplugged_probe_without_history_stays_empty(){
    feed_unplugged(10);
    TEST_ASSERT_EQUAL_INT(0, h.fine_count());
    TEST_ASSERT_EQUAL_INT(0, h.coarse_count());
}

void test_clear(){
    feed(20, 20.0f, 0.1f);
    h.clear();
    TEST_ASSERT_EQUAL_INT(0, h.fine_count());
    TEST_ASSERT_EQUAL_INT(0, h.coarse_count());
    TEST_ASSERT_EQUAL_UINT32(COARSE_START_INTERVAL_S, h.coarse_interval_s());
    feed(5, 30.0f, 0.0f);                             // the running coarse slot was reset too
    TEST_ASSERT_EQUAL_INT(0, h.coarse_count());
}

void test_eta_rising_one_degree_per_minute(){
    feed(60, 20.0f, 1.0f / 6.0f);                     // ends at about 29.8
    int32_t eta = h.eta_seconds(50.0f);               // about 20.2 degrees to go at 1/min
    TEST_ASSERT_INT_WITHIN(30, 1212, eta);
}

void test_eta_needs_three_minutes_of_samples(){
    feed(17, 20.0f, 1.0f / 6.0f);
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(50.0f));
    feed(1, 20.0f + 17.0f / 6.0f, 0.0f);
    TEST_ASSERT_NOT_EQUAL(ETA_UNKNOWN, h.eta_seconds(50.0f));
}

void test_eta_unknown_when_flat_slow_or_falling(){
    feed(60, 20.0f, 0.0f);
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(50.0f));
    h.clear();
    feed(60, 20.0f, 0.02f / 6.0f);                    // 0.02 degrees per minute
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(50.0f));
    h.clear();
    feed(60, 40.0f, -1.0f / 6.0f);
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(50.0f));
}

void test_eta_unknown_at_or_above_target(){
    feed(60, 40.0f, 1.0f / 6.0f);                     // ends at about 49.8
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(45.0f));
}

void test_eta_unknown_beyond_24_hours(){
    feed(60, 20.0f, 0.1f / 6.0f);                     // 0.1 degrees per minute
    TEST_ASSERT_EQUAL_INT32(ETA_UNKNOWN, h.eta_seconds(190.0f));   // about 1690 minutes
}

void test_eta_only_uses_the_last_ten_minutes(){
    feed(60, 60.0f, -1.0f / 6.0f);                    // falling for 10 minutes
    feed(60, 50.0f, 1.0f / 6.0f);                     // then rising 1/min to about 59.8
    TEST_ASSERT_INT_WITHIN(30, 612, h.eta_seconds(70.0f));
}

void test_eta_ignores_gaps(){
    feed(30, 20.0f, 1.0f / 6.0f);
    feed_unplugged(3);
    feed(30, 20.0f + 33.0f / 6.0f, 1.0f / 6.0f);
    TEST_ASSERT_NOT_EQUAL(ETA_UNKNOWN, h.eta_seconds(50.0f));
}

int main(int argc, char** argv){
    UNITY_BEGIN();
    RUN_TEST(test_to_tenths);
    RUN_TEST(test_fine_keeps_the_last_180_samples);
    RUN_TEST(test_coarse_slot_averages_six_samples);
    RUN_TEST(test_coarse_merges_pairs_and_doubles_the_interval_when_full);
    RUN_TEST(test_short_unplug_leaves_a_gap);
    RUN_TEST(test_clears_after_ten_minutes_unplugged);
    RUN_TEST(test_unplugged_probe_without_history_stays_empty);
    RUN_TEST(test_clear);
    RUN_TEST(test_eta_rising_one_degree_per_minute);
    RUN_TEST(test_eta_needs_three_minutes_of_samples);
    RUN_TEST(test_eta_unknown_when_flat_slow_or_falling);
    RUN_TEST(test_eta_unknown_at_or_above_target);
    RUN_TEST(test_eta_unknown_beyond_24_hours);
    RUN_TEST(test_eta_only_uses_the_last_ten_minutes);
    RUN_TEST(test_eta_ignores_gaps);
    return UNITY_END();
}
