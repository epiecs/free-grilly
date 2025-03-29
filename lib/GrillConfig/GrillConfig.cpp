#include "GrillConfig.h"

#include <Preferences.h>
#include <nvs_flash.h>

#include "Global.h"


void GrillConfig::initialize_settings(){

    // TODO verder uitwerken
    grill_name    = settings_storage.getString("grill_name", "Free-Grilly");
    grill_uuid    = "12abc3de-4567-89f0-a123-456b789012c3";
    wifi_ssid     = "WOZ";
    wifi_password = "schotelantenne";
}

void GrillConfig::factory_reset() {
    nvs_flash_erase();
    nvs_flash_init();

    ESP.restart();
}

// https://forum.arduino.cc/t/how-to-save-ipaddress-to-esp32-preferences/1034330/5
// IPAddress _ip;
// uint32_t ip_addr = (uint32_t) _ip;
// now you can store it in the preferences as integer.

// The reverse transformation is not more difficult

// IPAddress _ip = ip_addr;