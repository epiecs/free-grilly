#include <ArduinoJson.h>
#include <string>
#include <WiFi.h>

#include "Probe.h"
#include "GrillConfig.h"

#include "Api.h"
#include "Config.h"
#include "Grill.h"
#include "Web.h"

JsonDocument jsondoc;
char buffer[600];

void setup_api_routes()
{
    web::webserver.on("/api/grill", HTTP_GET, get_api_grill);

    web::webserver.on("/api/probes", HTTP_GET, get_api_probes);
    web::webserver.on("/api/probes", HTTP_POST, post_api_probes);
    
    web::webserver.on("/api/settings", HTTP_GET, get_api_settings);
    web::webserver.on("/api/settings", HTTP_POST, post_api_settings);
}

void get_api_grill()
{
    jsondoc.clear();

    jsondoc["name"]               = config::grill_name;
    jsondoc["unique_id"]          = config::grill_uuid;
    jsondoc["firmware_version"]   = config::grill_firmware_version;
    jsondoc["battery_percentage"] = grill::battery_percentage;
    jsondoc["battery_charging"]   = grill::battery_charging;
    jsondoc["wifi_connected"]     = grill::wifi_connected;
    jsondoc["wifi_ssid"]          = config::wifi_ssid;
    jsondoc["wifi_ip"]            = config::wifi_ip;
    jsondoc["wifi_signal"]        = WiFi.RSSI();
    
    JsonArray probeData = jsondoc["probes"].to<JsonArray>();
    
    JsonObject probeData_0 = probeData.add<JsonObject>();
    probeData_0["probe_id"] = 1;
    probeData_0["temperature"] = grill::probe_1.calculate_temperature();
    
    JsonObject probeData_1 = probeData.add<JsonObject>();
    probeData_1["probe_id"] = 2;
    probeData_1["temperature"] = grill::probe_2.calculate_temperature();
    
    JsonObject probeData_2 = probeData.add<JsonObject>();
    probeData_2["probe_id"] = 3;
    probeData_2["temperature"] = grill::probe_3.calculate_temperature();
    
    JsonObject probeData_3 = probeData.add<JsonObject>();
    probeData_3["probe_id"] = 4;
    probeData_3["temperature"] = grill::probe_4.calculate_temperature();
    
    JsonObject probeData_4 = probeData.add<JsonObject>();
    probeData_4["probe_id"] = 5;
    probeData_4["temperature"] = grill::probe_5.calculate_temperature();
    
    JsonObject probeData_5 = probeData.add<JsonObject>();
    probeData_5["probe_id"] = 6;
    probeData_5["temperature"] = grill::probe_6.calculate_temperature();
    
    JsonObject probeData_6 = probeData.add<JsonObject>();
    probeData_6["probe_id"] = 7;
    probeData_6["temperature"] = grill::probe_7.calculate_temperature();
    
    JsonObject probeData_7 = probeData.add<JsonObject>();
    probeData_7["probe_id"] = 8;
    probeData_7["temperature"] = grill::probe_8.calculate_temperature();

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer);
    web::webserver.send(200, "application/json", buffer);
}

void get_api_probes() 
{

}

void post_api_probes() 
{

}

void get_api_settings() 
{
    jsondoc.clear();

    jsondoc["name"]              = config::grill_name;
    jsondoc["uuid"]              = config::grill_uuid;
    jsondoc["firmware_version"]  = config::grill_firmware_version;
    
    jsondoc["wifi_ssid"]         = config::wifi_ssid;
    jsondoc["wifi_ip"]           = config::wifi_ip;
    jsondoc["wifi_subnet"]       = config::wifi_subnet;
    jsondoc["wifi_gateway"]      = config::wifi_gateway;
    jsondoc["wifi_dns"]          = config::wifi_dns;
    
    jsondoc["local_ap_ssid"]     = config::local_ap_ssid;
    jsondoc["local_ap_ip"]       = config::local_ap_ip;
    jsondoc["local_ap_subnet"]   = config::local_ap_subnet;
    jsondoc["local_ap_gateway"]  = config::local_ap_gateway;
    
    // TODO implement
    // jsondoc["temperature_unit"]         = "celcius";  //TODO have fahrenheit here + also in the main api endpoint
    // jsondoc["alarm_enabled"]            = true;
    // jsondoc["alarm_volume"]             = 1;
    // jsondoc["alarm_degrees_before"]     = 5;
    // jsondoc["alarm_on_wifi_disconnect"] = true;
    // jsondoc["auto_shutdown_time"]       = 180;

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer);
    web::webserver.send(200, "application/json", buffer);
}

void post_api_settings() 
{
    if(web::webserver.hasArg("plain") == false) { web::webserver.send(400, "application/json", "{\"error\": \"empty body\"}"); return;}

    std::string body = web::webserver.arg("plain").c_str();
    DeserializationError err = deserializeJson(jsondoc, body);
    if(err) { web::webserver.send(400, "application/json", "{\"error\": \"malformatted body\"}"); return;}

    JsonObject post_data = jsondoc.as<JsonObject>();
    
    // TODO validate schema!!
    config::grill_name             = post_data["name"].as<String>();
    
    config::wifi_ssid              = post_data["wifi_ssid"].as<String>();
    config::wifi_password          = post_data["wifi_password"].as<String>();
    config::wifi_ip                = post_data["wifi_ip"].as<String>();
    config::wifi_subnet            = post_data["wifi_subnet"].as<String>();
    config::wifi_gateway           = post_data["wifi_gateway"].as<String>();
    config::wifi_dns               = post_data["wifi_dns"].as<String>();
    
    config::local_ap_ssid          = post_data["local_ap_ssid"].as<String>();
    config::local_ap_password      = post_data["local_ap_password"].as<String>();
    config::local_ap_ip            = post_data["local_ap_ip"].as<String>();
    config::local_ap_subnet        = post_data["local_ap_subnet"].as<String>();
    config::local_ap_gateway       = post_data["local_ap_gateway"].as<String>();
    
    config::config_helper.save_settings();

    // If all is ok just send the current settings
    get_api_settings();
}
