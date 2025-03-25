#include <WebServer.h>
#include <string>

#include "Probe.h"

// Grill settings
extern std::string grill_name;
extern std::string grill_uuid;
extern std::string grill_firmware_version;

// Battery
extern int battery_percent;
extern bool battery_charging;

// Wifi
extern bool wifi_connected;

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