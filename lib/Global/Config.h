#pragma once
#include <WebServer.h>
#include <Preferences.h>

namespace config{

    // Settings storage
    extern Preferences settings_storage;
    
    // Grill settings
    extern String grill_name;
    extern String grill_uuid;
    extern String grill_firmware_version;

    // Wifi
    extern bool wifi_connected;
    extern int wifi_signal;
    extern IPAddress wifi_ip;
    extern String wifi_ssid;
    extern String wifi_password;

    // Local AP
    extern String local_ap_ssid_prefix;
    extern String local_ap_ssid;
    extern String local_ap_password;

}