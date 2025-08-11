#include "GrillConfig.h"

#include <esp_random.h>
#include <Preferences.h>
#include <WebServer.h>
#include <UUID.h>

#include <nvs_flash.h>

#include "Buzzer.h"
#include "Network.h"

#include "Config.h"
#include "Grill.h"
#include "Mqtt.h"
#include "Probe.h"
#include "Util.h"

UUID uuid_generator;

String wifi_ip_default          = "0.0.0.0";
String wifi_subnet_default      = "0.0.0.0";
String wifi_gateway_default     = "0.0.0.0";
String wifi_dns_default         = "0.0.0.0";

String local_ap_ip_default      = "192.168.200.10";
String local_ap_subnet_default  = "255.255.255.0";
String local_ap_gateway_default = "192.168.200.10";

// ***********************************
// * Settings
// ***********************************

void GrillConfig::load_settings(){

    //Check if nvram is clear or if initialized == false and initialize
    if(config::settings_storage.isKey("initialized") == false){
        initialize_settings();
        initialize_probes();
    }else if(config::settings_storage.getBool("initialized") == false){
        initialize_settings();
        initialize_probes();
    }

    // ***********************************
    // * Read from nvs
    // ***********************************
    // Grill
    config::grill_name                = config::settings_storage.getString("grill_name", "Free-Grilly");
    config::grill_uuid                = config::settings_storage.getString("grill_uuid", "");
    
    config::temperature_unit          = config::settings_storage.getString("temp_unit");
    config::beep_enabled              = config::settings_storage.getBool("beep_enabled");
    config::beep_on_ready             = config::settings_storage.getBool("beep_on_ready");
    config::beep_outside_target       = config::settings_storage.getBool("beep_out_targ");
    config::beep_volume               = config::settings_storage.getInt("beep_volume");
    config::beep_degrees_before       = config::settings_storage.getInt("beep_before");

    config::screen_timeout_minutes    = config::settings_storage.getInt("screen_to_mins");
    config::backlight_timeout_minutes = config::settings_storage.getInt("backl_to_mins");

    config::mqtt_broker               = config::settings_storage.getString("mqtt_broker");
    config::mqtt_port                 = config::settings_storage.getInt("mqtt_port", 1883);
    config::mqtt_topic                = config::settings_storage.getString("mqtt_topic", "free-grilly");
    config::mqtt_user                 = config::settings_storage.getString("mqtt_user");
    config::mqtt_password             = config::settings_storage.getString("mqtt_password");

    // Wifi
    config::wifi_ssid                 = config::settings_storage.getString("wifi_ssid", "");
    config::wifi_password             = config::settings_storage.getString("wifi_password", "");
    
    config::wifi_ip                   = config::settings_storage.getString("wifi_ip", wifi_ip_default);
    config::wifi_subnet               = config::settings_storage.getString("wifi_subnet", wifi_subnet_default);
    config::wifi_gateway              = config::settings_storage.getString("wifi_gateway", wifi_gateway_default);
    config::wifi_dns                  = config::settings_storage.getString("wifi_dns", wifi_dns_default);
    
    // shorter keys since keys should max be 15 chars
    config::local_ap_ssid             = config::settings_storage.getString("l_ap_ssid", "");
    config::local_ap_password         = config::settings_storage.getString("l_ap_password", "");
    
    config::local_ap_ip               = config::settings_storage.getString("l_ap_ip", local_ap_ip_default);
    config::local_ap_subnet           = config::settings_storage.getString("l_ap_subnet", local_ap_subnet_default);
    config::local_ap_gateway          = config::settings_storage.getString("l_ap_gateway", local_ap_gateway_default);

    grill::buzzer.set_volume(config::beep_volume);

    GrillConfig::print_settings();
}

void GrillConfig::save_settings(){
    Serial.println("Saving settings");

    config::settings_storage.putString("grill_name", config::grill_name); 

    Serial.println("check if reload is needed");
    bool reload_wifi     = check_wifi_reload_needed();
    bool reload_local_ap = check_local_ap_reload_needed();

    config::settings_storage.putString("temp_unit", config::temperature_unit);
    config::settings_storage.putBool("beep_enabled", config::beep_enabled);
    config::settings_storage.putBool("beep_on_ready", config::beep_on_ready);
    config::settings_storage.putBool("beep_out_targ", config::beep_outside_target);
    config::settings_storage.putInt("beep_volume", config::beep_volume);
    config::settings_storage.putInt("beep_before", config::beep_degrees_before);    
    config::settings_storage.putInt("screen_to_mins", config::screen_timeout_minutes);
    config::settings_storage.putInt("backl_to_mins", config::backlight_timeout_minutes);

    config::settings_storage.putString("mqtt_broker", config::mqtt_broker);
    config::settings_storage.putInt("mqtt_port", config::mqtt_port);
    config::settings_storage.putString("mqtt_topic", config::mqtt_topic);
    config::settings_storage.putString("mqtt_user", config::mqtt_user);
    config::settings_storage.putString("mqtt_password", config::mqtt_password);

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

    grill::buzzer.set_volume(config::beep_volume);

    if(reload_wifi){
        connect_to_wifi();
    }
    if(reload_local_ap){
        start_local_ap();
    }

    config::mqtt_client.publish_settings();
    GrillConfig::print_settings();
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
    
    config::settings_storage.putString("temp_unit", config::temperature_unit);
    config::settings_storage.putBool("beep_enabled", config::beep_enabled);
    config::settings_storage.putBool("beep_on_ready", config::beep_on_ready);
    config::settings_storage.putBool("beep_out_targ", config::beep_outside_target);
    config::settings_storage.putInt("beep_volume", config::beep_volume);
    config::settings_storage.putInt("beep_before", config::beep_degrees_before);
    config::settings_storage.putInt("screen_to_mins", config::screen_timeout_minutes);
    config::settings_storage.putInt("backl_to_mins", config::backlight_timeout_minutes);

    config::settings_storage.putString("mqtt_broker", config::mqtt_broker);
    config::settings_storage.putInt("mqtt_port", config::mqtt_port);
    config::settings_storage.putString("mqtt_topic", config::mqtt_topic);
    config::settings_storage.putString("mqtt_user", config::mqtt_user);
    config::settings_storage.putString("mqtt_password", config::mqtt_password);

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

    grill::buzzer.set_volume(config::beep_volume);

    GrillConfig::print_settings();
}

void GrillConfig::print_settings(){
    
    Serial.println(" ");
    Serial.println("|++++++++++ NVRAM Settings ++++++++++|");
    Serial.print("-- Initialized: ");
    Serial.println(config::settings_storage.getBool("initialized"));
    Serial.print("-- grill_name: ");
    Serial.println(config::grill_name);
    Serial.print("-- grill_uuid: ");
    Serial.println(config::grill_uuid);
    
    Serial.print("-- temperature_unit: ");
    Serial.println(config::temperature_unit);
    Serial.print("-- beep_enabled: ");
    Serial.println(config::beep_enabled);
    Serial.print("-- beep_on_ready: ");
    Serial.println(config::beep_on_ready);
    Serial.print("-- beep_outside_target: ");
    Serial.println(config::beep_outside_target);
    Serial.print("-- beep_volume: ");
    Serial.println(config::beep_volume);
    Serial.print("-- beep_degrees_before: ");
    Serial.println(config::beep_degrees_before);
    Serial.print("-- screen_timeout_minutes: ");
    Serial.println(config::screen_timeout_minutes);
    Serial.print("-- backlight_timeout_minutes: ");
    Serial.println(config::backlight_timeout_minutes);

    Serial.print("-- mqtt_broker: ");
    Serial.println(config::mqtt_broker);
    Serial.print("-- mqtt_port: ");
    Serial.println(config::mqtt_port);
    Serial.print("-- mqtt_topic: ");
    Serial.println(config::mqtt_topic);
    Serial.print("-- mqtt_user: ");
    Serial.println(config::mqtt_user);
    Serial.print("-- mqtt_password: ");
    Serial.println(config::mqtt_password);

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

// ***********************************
// * Probes
// ***********************************

void GrillConfig::load_probes(){

    String type = "";
    String name = "";
    int kohm    = 0;
    int beta    = 0;
    int temp    = 0;

    type = config::settings_storage.getString("p1_type");
    name = config::settings_storage.getString("p1_name", "Probe 1");
    kohm = config::settings_storage.getInt("p1_ref_kohm");
    temp = config::settings_storage.getInt("p1_ref_temp");
    beta = config::settings_storage.getInt("p1_ref_beta");
    grill::probe_1.target_temperature = config::settings_storage.getFloat("p1_target_temp");
    grill::probe_1.minimum_temperature = config::settings_storage.getFloat("p1_min_temp");
    grill::probe_1.set_type(type, kohm, temp, beta);
    grill::probe_1.set_name(name);
    
    type = config::settings_storage.getString("p2_type");
    name = config::settings_storage.getString("p2_name", "Probe 2");
    kohm = config::settings_storage.getInt("p2_ref_kohm");
    temp = config::settings_storage.getInt("p2_ref_temp");
    beta = config::settings_storage.getInt("p2_ref_beta");
    grill::probe_2.target_temperature = config::settings_storage.getFloat("p2_target_temp");
    grill::probe_2.minimum_temperature = config::settings_storage.getFloat("p2_min_temp");
    grill::probe_2.set_type(type, kohm, temp, beta);
    grill::probe_2.set_name(name);
    
    type = config::settings_storage.getString("p3_type");
    name = config::settings_storage.getString("p3_name", "Probe 3");
    kohm = config::settings_storage.getInt("p3_ref_kohm");
    temp = config::settings_storage.getInt("p3_ref_temp");
    beta = config::settings_storage.getInt("p3_ref_beta");
    grill::probe_3.target_temperature = config::settings_storage.getFloat("p3_target_temp");
    grill::probe_3.minimum_temperature = config::settings_storage.getFloat("p3_min_temp");
    grill::probe_3.set_type(type, kohm, temp, beta);
    grill::probe_3.set_name(name);
    
    type = config::settings_storage.getString("p4_type");
    name = config::settings_storage.getString("p4_name", "Probe 4");
    kohm = config::settings_storage.getInt("p4_ref_kohm");
    temp = config::settings_storage.getInt("p4_ref_temp");
    beta = config::settings_storage.getInt("p4_ref_beta");
    grill::probe_4.target_temperature = config::settings_storage.getFloat("p4_target_temp");
    grill::probe_4.minimum_temperature = config::settings_storage.getFloat("p4_min_temp");
    grill::probe_4.set_type(type, kohm, temp, beta);
    grill::probe_4.set_name(name);
    
    type = config::settings_storage.getString("p5_type");
    name = config::settings_storage.getString("p5_name", "Probe 5");
    kohm = config::settings_storage.getInt("p5_ref_kohm");
    temp = config::settings_storage.getInt("p5_ref_temp");
    beta = config::settings_storage.getInt("p5_ref_beta");
    grill::probe_5.target_temperature = config::settings_storage.getFloat("p5_target_temp");
    grill::probe_5.minimum_temperature = config::settings_storage.getFloat("p5_min_temp");
    grill::probe_5.set_type(type, kohm, temp, beta);
    grill::probe_5.set_name(name);
    
    type = config::settings_storage.getString("p6_type");
    name = config::settings_storage.getString("p6_name", "Probe 6");
    kohm = config::settings_storage.getInt("p6_ref_kohm");
    temp = config::settings_storage.getInt("p6_ref_temp");
    beta = config::settings_storage.getInt("p6_ref_beta");
    grill::probe_6.target_temperature = config::settings_storage.getFloat("p6_target_temp");
    grill::probe_6.minimum_temperature = config::settings_storage.getFloat("p6_min_temp");
    grill::probe_6.set_type(type, kohm, temp, beta);
    grill::probe_6.set_name(name);
    
    type = config::settings_storage.getString("p7_type");
    name = config::settings_storage.getString("p7_name", "Probe 7");
    kohm = config::settings_storage.getInt("p7_ref_kohm");
    temp = config::settings_storage.getInt("p7_ref_temp");
    beta = config::settings_storage.getInt("p7_ref_beta");
    grill::probe_7.target_temperature = config::settings_storage.getFloat("p7_target_temp");
    grill::probe_7.minimum_temperature = config::settings_storage.getFloat("p7_min_temp");
    grill::probe_7.set_type(type, kohm, temp, beta);
    grill::probe_7.set_name(name);
    
    type = config::settings_storage.getString("p8_type");
    name = config::settings_storage.getString("p8_name", "Probe 8");
    kohm = config::settings_storage.getInt("p8_ref_kohm");
    beta = config::settings_storage.getInt("p8_ref_temp"); 
    temp = config::settings_storage.getInt("p8_ref_beta");
    grill::probe_8.target_temperature = config::settings_storage.getFloat("p8_target_temp");
    grill::probe_8.minimum_temperature = config::settings_storage.getFloat("p8_min_temp");
    grill::probe_8.set_type(type, kohm, temp, beta);
    grill::probe_8.set_name(name);

    GrillConfig::print_probes();
}

void GrillConfig::save_probes(){
    Serial.println("Saving probes");

    // We can take the base values from the object since we already supply this in the constructor
    config::settings_storage.putString("p1_type", grill::probe_1.type);
    config::settings_storage.putString("p1_name", grill::probe_1.name);
    config::settings_storage.putInt("p1_ref_kohm", grill::probe_1.reference_kohm);
    config::settings_storage.putInt("p1_ref_beta", grill::probe_1.reference_beta);
    config::settings_storage.putInt("p1_ref_temp", grill::probe_1.reference_celcius);
    config::settings_storage.putFloat("p1_target_temp", grill::probe_1.target_temperature);
    config::settings_storage.putFloat("p1_min_temp", grill::probe_1.minimum_temperature);
    
    config::settings_storage.putString("p2_type", grill::probe_2.type);
    config::settings_storage.putString("p2_name", grill::probe_2.name);
    config::settings_storage.putInt("p2_ref_kohm", grill::probe_2.reference_kohm);
    config::settings_storage.putInt("p2_ref_beta", grill::probe_2.reference_beta);
    config::settings_storage.putInt("p2_ref_temp", grill::probe_2.reference_celcius);
    config::settings_storage.putFloat("p2_target_temp", grill::probe_2.target_temperature);
    config::settings_storage.putFloat("p2_min_temp", grill::probe_2.minimum_temperature);
    
    config::settings_storage.putString("p3_type", grill::probe_3.type);
    config::settings_storage.putString("p3_name", grill::probe_3.name);
    config::settings_storage.putInt("p3_ref_kohm", grill::probe_3.reference_kohm);
    config::settings_storage.putInt("p3_ref_beta", grill::probe_3.reference_beta);
    config::settings_storage.putInt("p3_ref_temp", grill::probe_3.reference_celcius);
    config::settings_storage.putFloat("p3_target_temp", grill::probe_3.target_temperature);
    config::settings_storage.putFloat("p3_min_temp", grill::probe_3.minimum_temperature);
    
    config::settings_storage.putString("p4_type", grill::probe_4.type);
    config::settings_storage.putString("p4_name", grill::probe_4.name);
    config::settings_storage.putInt("p4_ref_kohm", grill::probe_4.reference_kohm);
    config::settings_storage.putInt("p4_ref_beta", grill::probe_4.reference_beta);
    config::settings_storage.putInt("p4_ref_temp", grill::probe_4.reference_celcius);
    config::settings_storage.putFloat("p4_target_temp", grill::probe_4.target_temperature);
    config::settings_storage.putFloat("p4_min_temp", grill::probe_4.minimum_temperature);
    
    config::settings_storage.putString("p5_type", grill::probe_5.type);
    config::settings_storage.putString("p5_name", grill::probe_5.name);
    config::settings_storage.putInt("p5_ref_kohm", grill::probe_5.reference_kohm);
    config::settings_storage.putInt("p5_ref_beta", grill::probe_5.reference_beta);
    config::settings_storage.putInt("p5_ref_temp", grill::probe_5.reference_celcius);
    config::settings_storage.putFloat("p5_target_temp", grill::probe_5.target_temperature);
    config::settings_storage.putFloat("p5_min_temp", grill::probe_5.minimum_temperature);
    
    config::settings_storage.putString("p6_type", grill::probe_6.type);
    config::settings_storage.putString("p6_name", grill::probe_6.name);
    config::settings_storage.putInt("p6_ref_kohm", grill::probe_6.reference_kohm);
    config::settings_storage.putInt("p6_ref_beta", grill::probe_6.reference_beta);
    config::settings_storage.putInt("p6_ref_temp", grill::probe_6.reference_celcius);
    config::settings_storage.putFloat("p6_target_temp", grill::probe_6.target_temperature);
    config::settings_storage.putFloat("p6_min_temp", grill::probe_6.minimum_temperature);
    
    config::settings_storage.putString("p7_type", grill::probe_7.type);
    config::settings_storage.putString("p7_name", grill::probe_7.name);
    config::settings_storage.putInt("p7_ref_kohm", grill::probe_7.reference_kohm);
    config::settings_storage.putInt("p7_ref_beta", grill::probe_7.reference_beta);
    config::settings_storage.putInt("p7_ref_temp", grill::probe_7.reference_celcius);
    config::settings_storage.putFloat("p7_target_temp", grill::probe_7.target_temperature);
    config::settings_storage.putFloat("p7_min_temp", grill::probe_7.minimum_temperature);
    
    config::settings_storage.putString("p8_type", grill::probe_8.type);
    config::settings_storage.putString("p8_name", grill::probe_8.name);
    config::settings_storage.putInt("p8_ref_kohm", grill::probe_8.reference_kohm);
    config::settings_storage.putInt("p8_ref_beta", grill::probe_8.reference_beta);
    config::settings_storage.putInt("p8_ref_temp", grill::probe_8.reference_celcius);
    config::settings_storage.putFloat("p8_target_temp", grill::probe_8.target_temperature);
    config::settings_storage.putFloat("p8_min_temp", grill::probe_8.minimum_temperature);
    
    config::mqtt_client.publish_probes();
    GrillConfig::print_probes();
}

void GrillConfig::initialize_probes(){
    Serial.println("Initializing probes");

    // We can take the base values from the object since we already supply this in the constructor
    config::settings_storage.putString("p1_type", "grilleye_iris");
    config::settings_storage.putString("p1_name", "Probe 1");
    config::settings_storage.putInt("p1_ref_kohm", grill::probe_1.reference_kohm);
    config::settings_storage.putInt("p1_ref_beta", grill::probe_1.reference_beta);
    config::settings_storage.putInt("p1_ref_temp", grill::probe_1.reference_celcius);
    config::settings_storage.putFloat("p1_min_temp", grill::probe_1.minimum_temperature);
    config::settings_storage.putFloat("p1_target_temp", grill::probe_1.target_temperature);
    
    config::settings_storage.putString("p2_type", "grilleye_iris");
    config::settings_storage.putString("p2_name", "Probe 2");
    config::settings_storage.putInt("p2_ref_kohm", grill::probe_2.reference_kohm);
    config::settings_storage.putInt("p2_ref_beta", grill::probe_2.reference_beta);
    config::settings_storage.putInt("p2_ref_temp", grill::probe_2.reference_celcius);
    config::settings_storage.putFloat("p2_min_temp", grill::probe_2.minimum_temperature);
    config::settings_storage.putFloat("p2_target_temp", grill::probe_2.target_temperature);
    
    config::settings_storage.putString("p3_type", "grilleye_iris");
    config::settings_storage.putString("p3_name", "Probe 3");
    config::settings_storage.putInt("p3_ref_kohm", grill::probe_3.reference_kohm);
    config::settings_storage.putInt("p3_ref_beta", grill::probe_3.reference_beta);
    config::settings_storage.putInt("p3_ref_temp", grill::probe_3.reference_celcius);
    config::settings_storage.putFloat("p3_min_temp", grill::probe_3.minimum_temperature);
    config::settings_storage.putFloat("p3_target_temp", grill::probe_3.target_temperature);
    
    config::settings_storage.putString("p4_type", "grilleye_iris");
    config::settings_storage.putString("p4_name", "Probe 4");
    config::settings_storage.putInt("p4_ref_kohm", grill::probe_4.reference_kohm);
    config::settings_storage.putInt("p4_ref_beta", grill::probe_4.reference_beta);
    config::settings_storage.putInt("p4_ref_temp", grill::probe_4.reference_celcius);
    config::settings_storage.putFloat("p4_min_temp", grill::probe_4.minimum_temperature);
    config::settings_storage.putFloat("p4_target_temp", grill::probe_4.target_temperature);
    
    config::settings_storage.putString("p5_type", "grilleye_iris");
    config::settings_storage.putString("p5_name", "Probe 5");
    config::settings_storage.putInt("p5_ref_kohm", grill::probe_5.reference_kohm);
    config::settings_storage.putInt("p5_ref_beta", grill::probe_5.reference_beta);
    config::settings_storage.putInt("p5_ref_temp", grill::probe_5.reference_celcius);
    config::settings_storage.putFloat("p5_min_temp", grill::probe_5.minimum_temperature);
    config::settings_storage.putFloat("p5_target_temp", grill::probe_5.target_temperature);
    
    config::settings_storage.putString("p6_type", "grilleye_iris");
    config::settings_storage.putString("p6_name", "Probe 6");
    config::settings_storage.putInt("p6_ref_kohm", grill::probe_6.reference_kohm);
    config::settings_storage.putInt("p6_ref_beta", grill::probe_6.reference_beta);
    config::settings_storage.putInt("p6_ref_temp", grill::probe_6.reference_celcius);
    config::settings_storage.putFloat("p6_min_temp", grill::probe_6.minimum_temperature);
    config::settings_storage.putFloat("p6_target_temp", grill::probe_6.target_temperature);
    
    config::settings_storage.putString("p7_type", "grilleye_iris");
    config::settings_storage.putString("p7_name", "Probe 7");
    config::settings_storage.putInt("p7_ref_kohm", grill::probe_7.reference_kohm);
    config::settings_storage.putInt("p7_ref_beta", grill::probe_7.reference_beta);
    config::settings_storage.putInt("p7_ref_temp", grill::probe_7.reference_celcius);
    config::settings_storage.putFloat("p7_min_temp", grill::probe_7.minimum_temperature);
    config::settings_storage.putFloat("p7_target_temp", grill::probe_7.target_temperature);
    
    config::settings_storage.putString("p8_type", "grilleye_iris");
    config::settings_storage.putString("p8_name", "Probe 8");
    config::settings_storage.putInt("p8_ref_kohm", grill::probe_8.reference_kohm);
    config::settings_storage.putInt("p8_ref_beta", grill::probe_8.reference_beta);
    config::settings_storage.putInt("p8_ref_temp", grill::probe_8.reference_celcius);   
    config::settings_storage.putFloat("p8_min_temp", grill::probe_8.minimum_temperature);
    config::settings_storage.putFloat("p8_target_temp", grill::probe_8.target_temperature);
}

void GrillConfig::print_probes(){

    String type  = "";
    String name  = "";
    float target = 0;
    float min    = 0;
    int   kohm   = 0;
    int   beta   = 0;
    int   temp   = 0;

    Serial.println("|++++++++++ PROBE Settings ++++++++++|");

    target = config::settings_storage.getFloat("p1_target_temp");
    name   = config::settings_storage.getString("p1_name", "Probe 1");
    min    = config::settings_storage.getFloat("p1_min_temp");
    type   = config::settings_storage.getString("p1_type");
    kohm   = config::settings_storage.getInt("p1_ref_kohm");
    temp   = config::settings_storage.getInt("p1_ref_temp");
    beta   = config::settings_storage.getInt("p1_ref_beta");
    Serial.printf("Probe 1 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p2_target_temp");
    name   = config::settings_storage.getString("p2_name", "Probe 2");
    min    = config::settings_storage.getFloat("p2_min_temp");
    type   = config::settings_storage.getString("p2_type");
    kohm   = config::settings_storage.getInt("p2_ref_kohm");
    temp   = config::settings_storage.getInt("p2_ref_temp");
    beta   = config::settings_storage.getInt("p2_ref_beta");
    Serial.printf("Probe 2 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p3_target_temp");
    name   = config::settings_storage.getString("p3_name", "Probe 3");
    min    = config::settings_storage.getFloat("p3_min_temp");
    type   = config::settings_storage.getString("p3_type");
    kohm   = config::settings_storage.getInt("p3_ref_kohm");
    temp   = config::settings_storage.getInt("p3_ref_temp");
    beta   = config::settings_storage.getInt("p3_ref_beta");
    Serial.printf("Probe 3 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p4_target_temp");
    name   = config::settings_storage.getString("p4_name", "Probe 4");
    min    = config::settings_storage.getFloat("p4_min_temp");
    type   = config::settings_storage.getString("p4_type");
    kohm   = config::settings_storage.getInt("p4_ref_kohm");
    temp   = config::settings_storage.getInt("p4_ref_temp");
    beta   = config::settings_storage.getInt("p4_ref_beta");
    Serial.printf("Probe 4 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p5_target_temp");
    name   = config::settings_storage.getString("p5_name", "Probe 5");
    min    = config::settings_storage.getFloat("p5_min_temp");
    type   = config::settings_storage.getString("p5_type");
    kohm   = config::settings_storage.getInt("p5_ref_kohm");
    temp   = config::settings_storage.getInt("p5_ref_temp");
    beta   = config::settings_storage.getInt("p5_ref_beta");
    Serial.printf("Probe 5 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p6_target_temp");
    name   = config::settings_storage.getString("p6_name", "Probe 6");
    min    = config::settings_storage.getFloat("p6_min_temp");
    type   = config::settings_storage.getString("p6_type");
    kohm   = config::settings_storage.getInt("p6_ref_kohm");
    temp   = config::settings_storage.getInt("p6_ref_temp");
    beta   = config::settings_storage.getInt("p6_ref_beta");
    Serial.printf("Probe 6 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p7_target_temp");
    name   = config::settings_storage.getString("p7_name", "Probe 7");
    min    = config::settings_storage.getFloat("p7_min_temp");
    type   = config::settings_storage.getString("p7_type");
    kohm   = config::settings_storage.getInt("p7_ref_kohm");
    temp   = config::settings_storage.getInt("p7_ref_temp");
    beta   = config::settings_storage.getInt("p7_ref_beta");
    Serial.printf("Probe 7 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");
    
    target = config::settings_storage.getFloat("p8_target_temp");
    name   = config::settings_storage.getString("p8_name", "Probe 8");
    min    = config::settings_storage.getFloat("p8_min_temp");
    type   = config::settings_storage.getString("p8_type");
    kohm   = config::settings_storage.getInt("p8_ref_kohm");
    temp   = config::settings_storage.getInt("p8_ref_temp");
    beta   = config::settings_storage.getInt("p8_ref_beta");
    Serial.printf("Probe 8 :: %s :: %f %f - %s %d %d %d", name, target, min, type, kohm, temp, beta);
    Serial.println("");

    Serial.println("|++++++++++ PROBE Settings ++++++++++|");
    Serial.println("");
}

// ***********************************
// * Utilities
// ***********************************

void GrillConfig::factory_reset() {
    // https://github.com/espressif/esp-idf/issues/2777
    Serial.println("Erasing nvram");
    nvs_flash_deinit();
    nvs_flash_erase();
    nvs_flash_init();
    
    config::settings_storage.putBool("initialized", false);
    Serial.println("Nvram erased");
    
    Serial.println("Rebooting!");
    ESP.restart();
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