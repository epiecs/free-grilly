#pragma once
#include <WebServer.h>
#include <Preferences.h>

#include "Probe.h"

// Settings storage
extern Preferences settings_storage;

// Grill settings
extern String grill_name;
extern String grill_uuid;
extern String grill_firmware_version;

// Battery
extern int battery_percentage;
extern bool battery_charging;

// Wifi
extern bool wifi_connected;
extern int wifi_signal;
extern IPAddress wifi_ip;
extern String wifi_ssid;
extern String wifi_password;

// Webserver for api
extern WebServer webserver;

// Probes
extern Probe probe_1;
extern Probe probe_2;
extern Probe probe_3;
extern Probe probe_4;
extern Probe probe_5;
extern Probe probe_6;
extern Probe probe_7;
extern Probe probe_8;