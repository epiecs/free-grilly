#include "GrillConfig.h"

#include <esp_random.h>
#include <Preferences.h>
#include <UUID.h>

#include <nvs_flash.h>

#include "Config.h"
#include "Util.h"

UUID uuid_generator;

String wifi_ip_default          = "0.0.0.0";
String wifi_subnet_default      = "0.0.0.0";
String wifi_gateway_default     = "0.0.0.0";
String wifi_dns_default         = "0.0.0.0";

String local_ap_ip_default      = "192.168.200.10";
String local_ap_subnet_default  = "255.255.255.0";
String local_ap_gateway_default = "192.168.200.10";

void GrillConfig::load_settings(){

    //Check if nvram is clear or if initialized == false and initialize
    if(config::settings_storage.isKey("initialized") == false){
        initialize_settings();
    }else if(config::settings_storage.getBool("initialized") == false){
        initialize_settings();
    }
    
    // ***********************************
    // * Read from nvs
    // ***********************************
    // Grill
    config::grill_name        = config::settings_storage.getString("grill_name", "Free-Grilly");
    config::grill_uuid        = config::settings_storage.getString("grill_uuid", "");
    
    // Wifi
    config::wifi_ssid         = config::settings_storage.getString("wifi_ssid", "");
    config::wifi_password     = config::settings_storage.getString("wifi_password", "");
    
    config::wifi_ip           = config::settings_storage.getString("wifi_ip", wifi_ip_default);
    config::wifi_subnet       = config::settings_storage.getString("wifi_subnet", wifi_subnet_default);
    config::wifi_gateway      = config::settings_storage.getString("wifi_gateway", wifi_gateway_default);
    config::wifi_dns          = config::settings_storage.getString("wifi_dns", wifi_dns_default);
    
    // shorter keys since keys should max be 15 chars
    config::local_ap_ssid     = config::settings_storage.getString("l_ap_ssid", "");
    config::local_ap_password = config::settings_storage.getString("l_ap_password", "");
    config::local_ap_ip       = config::settings_storage.getString("l_ap_ip", local_ap_ip_default);
    config::local_ap_subnet   = config::settings_storage.getString("l_ap_subnet", local_ap_subnet_default);
    config::local_ap_gateway  = config::settings_storage.getString("l_ap_gateway", local_ap_gateway_default);


    GrillConfig::print();
}

void GrillConfig::initialize_settings(){
    Serial.println("Initializing settings");
    
    config::settings_storage.putBool("initialized", true);
    
    // ***********************************
    // * Generate initial values
    // ***********************************
    
    Serial.println("Generating random numbers for uuid generation");
    uint32_t seed_1 = esp_random();
    uint32_t seed_2 = esp_random();
    
    uuid_generator.seed(seed_1, seed_2);
    uuid_generator.generate();
    config::grill_uuid = ((char*)uuid_generator.toCharArray());
    Serial.println("Generated grill_uuid: " + config::grill_uuid);
    
    config::local_ap_ssid =  generate_hostname(config::local_ap_ssid_prefix);
    Serial.println("Generated local_ap_ssid: " + config::local_ap_ssid);
   
    // ***********************************
    // * Store initial values
    // ***********************************

    config::settings_storage.putString("grill_name", "Free-Grilly");
    config::settings_storage.putString("grill_uuid", config::grill_uuid);

    config::settings_storage.putString("wifi_ssid", "");
    config::settings_storage.putString("wifi_password", "");
    config::settings_storage.putString("wifi_ip", wifi_ip_default);
    config::settings_storage.putString("wifi_subnet", wifi_subnet_default);
    config::settings_storage.putString("wifi_gateway", wifi_gateway_default);
    config::settings_storage.putString("wifi_dns", wifi_dns_default);
    
    config::settings_storage.putString("l_ap_ssid", config::local_ap_ssid);
    config::settings_storage.putString("l_ap_password", "");
    config::settings_storage.putString("l_ap_ip", local_ap_ip_default);
    config::settings_storage.putString("l_ap_subnet", local_ap_subnet_default);
    config::settings_storage.putString("l_ap_gateway", local_ap_gateway_default);

}

void GrillConfig::factory_reset() {
    Serial.println("Erasing nvram");
    nvs_flash_erase();
    nvs_flash_init();
    Serial.println("Nvram erased");
    
    Serial.println("Rebooting!");
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
    Serial.print("local_ap_ip: ");
    Serial.println(config::local_ap_ip);
    Serial.print("local_ap_subnet: ");
    Serial.println(config::local_ap_subnet);

}

// https://forum.arduino.cc/t/how-to-save-ipaddress-to-esp32-preferences/1034330/5
// IPAddress _ip;
// uint32_t ip_addr = (uint32_t) _ip;
// now you can store it in the preferences as integer.

// The reverse transformation is not more difficult

// IPAddress _ip = ip_addr;