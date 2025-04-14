#include <ArduinoJson.h>
#include <string>
#include <WiFi.h>

#include "Probe.h"
#include "Buzzer.h"
#include "GrillConfig.h"

#include "Api.h"
#include "Config.h"
#include "Grill.h"
#include "Web.h"

JsonDocument jsondoc;
char buffer[1500];

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
    jsondoc["temperature_unit"]   = config::temperature_unit;
    
    JsonArray probeData = jsondoc["probes"].to<JsonArray>();
    
    JsonObject probeData_0 = probeData.add<JsonObject>();
    probeData_0["probe_id"] = 1;
    probeData_0["temperature"] = grill::probe_1.celcius;
    probeData_0["target_temperature"] = grill::probe_1.target_temperature;
    probeData_0["connected"] = grill::probe_1.connected;
    
    JsonObject probeData_1 = probeData.add<JsonObject>();
    probeData_1["probe_id"] = 2;
    probeData_1["temperature"] = grill::probe_2.celcius;
    probeData_1["target_temperature"] = grill::probe_2.target_temperature;
    probeData_1["connected"] = grill::probe_2.connected;
    
    JsonObject probeData_2 = probeData.add<JsonObject>();
    probeData_2["probe_id"] = 3;
    probeData_2["temperature"] = grill::probe_3.celcius;
    probeData_2["target_temperature"] = grill::probe_3.target_temperature;
    probeData_2["connected"] = grill::probe_3.connected;
    
    JsonObject probeData_3 = probeData.add<JsonObject>();
    probeData_3["probe_id"] = 4;
    probeData_3["temperature"] = grill::probe_4.celcius;
    probeData_3["target_temperature"] = grill::probe_4.target_temperature;
    probeData_3["connected"] = grill::probe_4.connected;
    
    JsonObject probeData_4 = probeData.add<JsonObject>();
    probeData_4["probe_id"] = 5;
    probeData_4["temperature"] = grill::probe_5.celcius;
    probeData_4["target_temperature"] = grill::probe_5.target_temperature;
    probeData_4["connected"] = grill::probe_5.connected;
    
    JsonObject probeData_5 = probeData.add<JsonObject>();
    probeData_5["probe_id"] = 6;
    probeData_5["temperature"] = grill::probe_6.celcius;
    probeData_5["target_temperature"] = grill::probe_6.target_temperature;
    probeData_5["connected"] = grill::probe_6.connected;
    
    JsonObject probeData_6 = probeData.add<JsonObject>();
    probeData_6["probe_id"] = 7;
    probeData_6["temperature"] = grill::probe_7.celcius;
    probeData_6["target_temperature"] = grill::probe_7.target_temperature;
    probeData_6["connected"] = grill::probe_7.connected;
    
    JsonObject probeData_7 = probeData.add<JsonObject>();
    probeData_7["probe_id"] = 8;
    probeData_7["temperature"] = grill::probe_8.celcius;
    probeData_7["target_temperature"] = grill::probe_8.target_temperature;
    probeData_7["connected"] = grill::probe_8.connected;

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer);
    web::webserver.send(200, "application/json", buffer);
}

void get_api_probes() 
{
    jsondoc.clear();

    Serial.println(grill::probe_1.target_temperature);

    JsonObject doc_0 = jsondoc.add<JsonObject>();
    doc_0["probe_id"] = 1;
    doc_0["temperature"] = grill::probe_1.celcius;
    doc_0["target_temperature"] = grill::probe_1.target_temperature;
    doc_0["connected"] = grill::probe_1.connected;
    doc_0["probe_type"] = grill::probe_1.type;
    doc_0["reference_kohm"] = grill::probe_1.reference_kohm;
    doc_0["reference_celcius"] = grill::probe_1.reference_celcius;
    doc_0["reference_beta"] = grill::probe_1.reference_beta;

    JsonObject doc_1 = jsondoc.add<JsonObject>();
    doc_1["probe_id"] = 2;
    doc_1["temperature"] = grill::probe_2.celcius;
    doc_1["target_temperature"] = grill::probe_2.target_temperature;
    doc_1["connected"] = grill::probe_2.connected;
    doc_1["probe_type"] = grill::probe_2.type;
    doc_1["reference_kohm"] = grill::probe_2.reference_kohm;
    doc_1["reference_celcius"] = grill::probe_2.reference_celcius;
    doc_1["reference_beta"] = grill::probe_2.reference_beta;

    JsonObject doc_2 = jsondoc.add<JsonObject>();
    doc_2["probe_id"] = 3;
    doc_2["temperature"] = grill::probe_3.celcius;
    doc_2["target_temperature"] = grill::probe_3.target_temperature;
    doc_2["connected"] = grill::probe_3.connected;
    doc_2["probe_type"] = grill::probe_3.type;
    doc_2["reference_kohm"] = grill::probe_3.reference_kohm;
    doc_2["reference_celcius"] = grill::probe_3.reference_celcius;
    doc_2["reference_beta"] = grill::probe_3.reference_beta;

    JsonObject doc_3 = jsondoc.add<JsonObject>();
    doc_3["probe_id"] = 4;
    doc_3["temperature"] = grill::probe_4.celcius;
    doc_3["target_temperature"] = grill::probe_4.target_temperature;
    doc_3["connected"] = grill::probe_4.connected;
    doc_3["probe_type"] = grill::probe_4.type;
    doc_3["reference_kohm"] = grill::probe_4.reference_kohm;
    doc_3["reference_celcius"] = grill::probe_4.reference_celcius;
    doc_3["reference_beta"] = grill::probe_4.reference_beta;

    JsonObject doc_4 = jsondoc.add<JsonObject>();
    doc_4["probe_id"] = 5;
    doc_4["temperature"] = grill::probe_5.celcius;
    doc_4["target_temperature"] = grill::probe_5.target_temperature;
    doc_4["connected"] = grill::probe_5.connected;
    doc_4["probe_type"] = grill::probe_5.type;
    doc_4["reference_kohm"] = grill::probe_5.reference_kohm;
    doc_4["reference_celcius"] = grill::probe_5.reference_celcius;
    doc_4["reference_beta"] = grill::probe_5.reference_beta;

    JsonObject doc_5 = jsondoc.add<JsonObject>();
    doc_5["probe_id"] = 6;
    doc_5["temperature"] = grill::probe_6.celcius;
    doc_5["target_temperature"] = grill::probe_6.target_temperature;
    doc_5["connected"] = grill::probe_6.connected;
    doc_5["probe_type"] = grill::probe_6.type;
    doc_5["reference_kohm"] = grill::probe_6.reference_kohm;
    doc_5["reference_celcius"] = grill::probe_6.reference_celcius;
    doc_5["reference_beta"] = grill::probe_6.reference_beta;

    JsonObject doc_6 = jsondoc.add<JsonObject>();
    doc_6["probe_id"] = 7;
    doc_6["temperature"] = grill::probe_7.celcius;
    doc_6["target_temperature"] = grill::probe_7.target_temperature;
    doc_6["connected"] = grill::probe_7.connected;
    doc_6["probe_type"] = grill::probe_7.type;
    doc_6["reference_kohm"] = grill::probe_7.reference_kohm;
    doc_6["reference_celcius"] = grill::probe_7.reference_celcius;
    doc_6["reference_beta"] = grill::probe_7.reference_beta;

    JsonObject doc_7 = jsondoc.add<JsonObject>();
    doc_7["probe_id"] = 8;
    doc_7["temperature"] = grill::probe_8.celcius;
    doc_7["target_temperature"] = grill::probe_8.target_temperature;
    doc_7["connected"] = grill::probe_8.connected;
    doc_7["probe_type"] = grill::probe_8.type;
    doc_7["reference_kohm"] = grill::probe_8.reference_kohm;
    doc_7["reference_celcius"] = grill::probe_8.reference_celcius;
    doc_7["reference_beta"] = grill::probe_8.reference_beta;

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer);
    web::webserver.send(200, "application/json", buffer);

}

void post_api_probes() 
{
    if(web::webserver.hasArg("plain") == false) { web::webserver.send(400, "application/json", "{\"error\": \"empty body\"}"); return;}

    std::string body = web::webserver.arg("plain").c_str();
    DeserializationError err = deserializeJson(jsondoc, body);
    if(err) { web::webserver.send(400, "application/json", "{\"error\": \"malformatted body\"}"); return;}

    JsonObject post_data = jsondoc.as<JsonObject>();

    for (JsonObject item : jsondoc.as<JsonArray>()) {

        int    probe_id           = item["probe_id"];
        float  target_temperature = item["target_temperature"];
        String probe_type         = item["probe_type"].as<String>();
        int    reference_kohm     = item["reference_kohm"];
        int    reference_celcius  = item["reference_celcius"];
        int    reference_beta     = item["reference_beta"];

        Serial.println(target_temperature);

        switch (probe_id){
            case 1:
                grill::probe_1.target_temperature = target_temperature;
                grill::probe_1.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 2:
                grill::probe_2.target_temperature = target_temperature;    
                grill::probe_2.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 3:
                grill::probe_3.target_temperature = target_temperature;    
                grill::probe_3.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 4:
                grill::probe_4.target_temperature = target_temperature;    
                grill::probe_4.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 5:
                grill::probe_5.target_temperature = target_temperature;    
                grill::probe_5.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 6:
                grill::probe_6.target_temperature = target_temperature;    
                grill::probe_6.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 7:
                grill::probe_7.target_temperature = target_temperature;    
                grill::probe_7.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            case 8:
                grill::probe_8.target_temperature = target_temperature;    
                grill::probe_8.set_type(probe_type, reference_kohm, reference_celcius, reference_beta);
                break;
            default:
                break;
        }
    }

    config::config_helper.save_probes();
    // If all is ok just send the current settings
    get_api_probes();

}

void get_api_settings() 
{
    jsondoc.clear();

    jsondoc["name"]                 = config::grill_name;
    jsondoc["uuid"]                 = config::grill_uuid;
    jsondoc["firmware_version"]     = config::grill_firmware_version;
    
    jsondoc["temperature_unit"]     = config::temperature_unit;
    jsondoc["beep_enabled"]         = config::beep_enabled;
    jsondoc["beep_volume"]          = config::beep_volume;
    jsondoc["beep_degrees_before"]  = config::beep_degrees_before;
    jsondoc["beep_outside_target"]  = config::beep_outside_target;
    jsondoc["beep_on_ready"]        = config::beep_on_ready;
    jsondoc["mqtt_broker"]          = config::mqtt_broker;
    
    jsondoc["wifi_ssid"]            = config::wifi_ssid;
    jsondoc["wifi_ip"]              = config::wifi_ip;
    jsondoc["wifi_subnet"]          = config::wifi_subnet;
    jsondoc["wifi_gateway"]         = config::wifi_gateway;
    jsondoc["wifi_dns"]             = config::wifi_dns;
    
    jsondoc["local_ap_ssid"]        = config::local_ap_ssid;
    jsondoc["local_ap_ip"]          = config::local_ap_ip;
    jsondoc["local_ap_subnet"]      = config::local_ap_subnet;
    jsondoc["local_ap_gateway"]     = config::local_ap_gateway;
    
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

    // Data ingress
    config::grill_name             = post_data["name"].as<String>();
    
    config::temperature_unit       = post_data["temperature_unit"].as<String>();
    config::beep_enabled           = post_data["beep_enabled"];
    config::beep_volume            = post_data["beep_volume"];
    config::beep_degrees_before    = post_data["beep_degrees_before"];
    config::beep_outside_target    = post_data["beep_outside_target"];
    config::beep_on_ready          = post_data["beep_on_ready"];
    config::mqtt_broker            = post_data["mqtt_broker"].as<String>();

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

    if(config::local_ap_password.length() > 0 && config::local_ap_password.length() < 8){
        web::webserver.send(400, "application/json", "{\"error\": \"local_ap_password should be empty or at least 8 characters\"}"); 
        return;
    }
    
    config::config_helper.save_settings();

    // If all is ok just send the current settings
    get_api_settings();
}
