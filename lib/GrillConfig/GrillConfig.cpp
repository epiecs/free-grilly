#include "GrillConfig.h"

#include <Preferences.h>
#include <UUID.h>

#include <nvs_flash.h>

#include "Config.h"
#include "Util.h"

UUID uuid_generator;

void GrillConfig::initialize_settings(){

    // TODO verder uitwerken

    // ***********************************
    // * Read from nvs
    // ***********************************
    // Grill
    config::grill_name        = config::settings_storage.getString("grill_name", "Free-Grilly");
    config::grill_uuid        = config::settings_storage.getString("grill_uuid", "");
    
    // Wifi
    config::wifi_ssid         = config::settings_storage.getString("wifi_ssid", "");
    config::wifi_password     = config::settings_storage.getString("wifi_password", "");
    
    config::wifi_ip           = config::settings_storage.getString("wifi_ip", "0.0.0.0");
    config::wifi_subnet       = config::settings_storage.getString("wifi_subnet", "0.0.0.0");
    config::wifi_gateway      = config::settings_storage.getString("wifi_gateway", "0.0.0.0");
    config::wifi_dns          = config::settings_storage.getString("wifi_dns", "0.0.0.0");
    
    config::local_ap_ssid     = config::settings_storage.getString("local_ap_ssid", "");
    config::local_ap_password = config::settings_storage.getString("local_ap_password", "");
    config::local_ap_ip       = config::settings_storage.getString("local_ap_ip", "192.168.200.10");
    config::local_ap_subnet   = config::settings_storage.getString("local_ap_subnet", "255.255.255.0");
    config::local_ap_gateway  = config::settings_storage.getString("local_ap_gateway", "192.168.200.10");


    // ***********************************
    // * Generate new values if none exist
    // ***********************************
    
    if(config::local_ap_ssid == "")
    {
        config::local_ap_ssid =  generate_hostname(config::local_ap_ssid_prefix);
        config::settings_storage.putString("local_ap_ssid", config::local_ap_ssid);
    }
    
    if(config::grill_uuid == "")
    {
        config::grill_uuid = ((char*)uuid_generator.toCharArray());
        config::settings_storage.putString("grill_uuid", config::grill_uuid);
    }

    GrillConfig::print();
}

void GrillConfig::factory_reset() {
    nvs_flash_erase();
    nvs_flash_init();

    ESP.restart();
}

void GrillConfig::print(){
 
    Serial.print("grill_name: ");
    Serial.println(config::grill_name);
    Serial.print("grill_uuid: ");
    Serial.println(config::grill_uuid);
    Serial.print("wifi_ssid: ");
    Serial.println(config::wifi_ssid);
    Serial.print("wifi_password: ");
    Serial.println(config::wifi_password);
    Serial.print("local_ap_ssid: ");
    Serial.println(config::local_ap_ssid);

}

// https://forum.arduino.cc/t/how-to-save-ipaddress-to-esp32-preferences/1034330/5
// IPAddress _ip;
// uint32_t ip_addr = (uint32_t) _ip;
// now you can store it in the preferences as integer.

// The reverse transformation is not more difficult

// IPAddress _ip = ip_addr;