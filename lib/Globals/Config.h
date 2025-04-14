#pragma once

// Forward declarations
class WebServer;
class Preferences;
class GrillConfig;

namespace config{

    // Settings storage
    extern Preferences settings_storage;
    extern GrillConfig config_helper;

    // Grill settings
    extern String grill_name;
    extern String grill_uuid;
    extern String grill_firmware_version;

    extern String temperature_unit;

    // Wifi
    extern String wifi_ip;
    extern String wifi_subnet;
    extern String wifi_gateway;
    extern String wifi_dns;

    extern String wifi_ssid;
    extern String wifi_password;

    // Local AP
    extern String local_ap_ssid_prefix;
    extern String local_ap_ssid;
    extern String local_ap_password;

    extern String local_ap_ip;
    extern String local_ap_subnet;
    extern String local_ap_gateway;

}