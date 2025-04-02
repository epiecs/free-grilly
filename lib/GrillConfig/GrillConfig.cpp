#include "GrillConfig.h"

#include <esp_random.h>
#include <Preferences.h>
#include <WebServer.h>
#include <UUID.h>

#include <nvs_flash.h>

#include "Network.h"

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

void GrillConfig::save_settings(){
    Serial.println("Saving settings");

    config::settings_storage.putString("grill_name", config::grill_name);

    bool reload_wifi     = check_wifi_reload_needed();
    bool reload_local_ap = check_local_ap_reload_needed();

    Serial.println("check if reload is needed");

    config::settings_storage.putString("wifi_ssid", config::wifi_ssid);
    config::settings_storage.putString("wifi_password", config::wifi_password);
    config::settings_storage.putString("wifi_ip", config::wifi_ip);
    config::settings_storage.putString("wifi_subnet", config::wifi_subnet);
    config::settings_storage.putString("wifi_gateway", config::wifi_gateway);
    config::settings_storage.putString("wifi_dns", config::wifi_dns);
    
    config::settings_storage.putString("l_ap_ssid", config::local_ap_ssid);
    config::settings_storage.putString("l_ap_password", config::local_ap_password);
    config::settings_storage.putString("l_ap_ip", config::local_ap_ip);
    config::settings_storage.putString("l_ap_subnet", config::local_ap_subnet);
    config::settings_storage.putString("l_ap_gateway", config::local_ap_gateway);

    if(reload_wifi){
        connect_to_wifi(config::wifi_ssid, config::wifi_password, config::wifi_ip, config::wifi_subnet, config::wifi_gateway, config::wifi_dns);
    }
    if(reload_local_ap){
        start_local_ap(config::local_ap_ssid, config::local_ap_password,config::local_ap_ip,config::local_ap_subnet,config::local_ap_gateway);
    }

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

    GrillConfig::print();
}

void GrillConfig::factory_reset() {
    // https://github.com/espressif/esp-idf/issues/2777
    config::settings_storage.putBool("initialized", false);
    Serial.println("Erasing nvram");
    nvs_flash_deinit();
    nvs_flash_erase();
    nvs_flash_init();
    Serial.println("Nvram erased");
    
    Serial.println("Rebooting!");
    ESP.restart();
}

void GrillConfig::print(){
    
    Serial.println(" ");
    Serial.println("|++++++++++ NVRAM Settings ++++++++++|");
    Serial.print("-- Initialized: ");
    Serial.println(config::settings_storage.getBool("initialized"));
    Serial.print("-- grill_name: ");
    Serial.println(config::grill_name);
    Serial.print("-- grill_uuid: ");
    Serial.println(config::grill_uuid);
    Serial.print("-- wifi_ssid: ");
    Serial.println(config::wifi_ssid);
    Serial.print("-- wifi_password: ");
    Serial.println("hidden - ********");
    // Serial.println(config::wifi_password);
    Serial.print("-- wifi_ip: ");
    Serial.println(config::wifi_ip);
    Serial.print("-- wifi_subnet: ");
    Serial.println(config::wifi_subnet);
    Serial.print("-- wifi_gateway: ");
    Serial.println(config::wifi_gateway);
    Serial.print("-- wifi_dns: ");
    Serial.println(config::wifi_dns);
    Serial.print("-- local_ap_ssid: ");
    Serial.println(config::local_ap_ssid);
    Serial.print("-- local_ap_password: ");
    Serial.println("hidden - ********");
    // Serial.println(config::local_ap_password);
    Serial.print("-- local_ap_ip: ");
    Serial.println(config::local_ap_ip);
    Serial.print("-- local_ap_subnet: ");
    Serial.println(config::local_ap_subnet);
    Serial.println("|++++++++++ NVRAM Settings ++++++++++|");
    Serial.println(" ");

}

bool GrillConfig::check_wifi_reload_needed(){
    
    bool reload_needed = false;
    
    if(config::wifi_ssid     != config::settings_storage.getString("wifi_ssid"))     {reload_needed = true;}
    if(config::wifi_password != config::settings_storage.getString("wifi_password")) {reload_needed = true;};    
    if(config::wifi_ip       != config::settings_storage.getString("wifi_ip"))       {reload_needed = true;};
    if(config::wifi_subnet   != config::settings_storage.getString("wifi_subnet"))   {reload_needed = true;};
    if(config::wifi_gateway  != config::settings_storage.getString("wifi_gateway"))  {reload_needed = true;};
    if(config::wifi_dns      != config::settings_storage.getString("wifi_dns"))      {reload_needed = true;};

    return reload_needed;
}

bool GrillConfig::check_local_ap_reload_needed(){
    
    bool reload_needed = false;

    if(config::local_ap_ssid     != config::settings_storage.getString("l_ap_ssid"))     {reload_needed = true;}
    if(config::local_ap_password != config::settings_storage.getString("l_ap_password")) {reload_needed = true;};    
    if(config::local_ap_ip       != config::settings_storage.getString("l_ap_ip"))       {reload_needed = true;};
    if(config::local_ap_subnet   != config::settings_storage.getString("l_ap_subnet"))   {reload_needed = true;};
    if(config::local_ap_gateway  != config::settings_storage.getString("l_ap_gateway"))  {reload_needed = true;};

    return reload_needed;
}