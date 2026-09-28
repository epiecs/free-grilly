#pragma once

#include <Arduino.h>

class Probe;
class Buzzer;

namespace grill {

    // Battery
    extern int battery_percentage;
    extern bool battery_charging;

    // Buzzer
    extern Buzzer buzzer;

    // Wifi
    extern bool wifi_connected;
    // Current ip, config::wifi_ip is the static ip setting. A fixed buffer instead of a String: it is
    // written by the wifi event task and read by others without a lock, a String could be freed mid-read.
    extern char wifi_ip[16];
    extern int wifi_signal;
    extern bool internet_connectivity;

    extern Probe probe_1;
    extern Probe probe_2;
    extern Probe probe_3;
    extern Probe probe_4;
    extern Probe probe_5;
    extern Probe probe_6;
    extern Probe probe_7;
    extern Probe probe_8;

}
