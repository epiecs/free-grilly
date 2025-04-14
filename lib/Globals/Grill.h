#pragma once

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
    extern int wifi_signal;

    extern Probe probe_1;
    extern Probe probe_2;
    extern Probe probe_3;
    extern Probe probe_4;
    extern Probe probe_5;
    extern Probe probe_6;
    extern Probe probe_7;
    extern Probe probe_8;

}
