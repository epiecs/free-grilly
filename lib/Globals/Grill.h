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
    // True while an alarm is actively beeping, written only by task_alarm
    extern volatile bool alarm_sounding;

    // Wifi
    extern bool wifi_connected;
    // Current ip, config::wifi_ip is the static ip setting. A fixed buffer instead of a String: it is
    // written by the wifi event task and read by others without a lock, a String could be freed mid-read.
    extern char wifi_ip[16];
    extern int wifi_signal;
    extern bool internet_connectivity;
    // "grilly-plus-" + first 8 hex chars of config::grill_uuid, computed once at boot (Settings.h)
    // after load_settings and before WiFi starts. Read-only after that, so no lock is needed.
    extern char hostname[32];

    extern Probe probe_1;
    extern Probe probe_2;
    extern Probe probe_3;
    extern Probe probe_4;
    extern Probe probe_5;
    extern Probe probe_6;
    extern Probe probe_7;
    extern Probe probe_8;

}
