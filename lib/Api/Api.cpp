#include <ArduinoJson.h>
#include <string>

#include "Api.h"
#include "Global.h"

JsonDocument jsondoc;
char buffer[500];

void get_api_grill()
{
    jsondoc.clear();

    jsondoc["name"]             = grill_name;
    jsondoc["unique_id"]        = grill_uuid;
    jsondoc["firmware_version"] = grill_firmware_version;
    jsondoc["battery"]          = battery_percent;
    jsondoc["battery_charging"] = battery_charging;
    jsondoc["wifi_connected"]   = wifi_connected;
    jsondoc["wifi_ssid"]        = "WOZ";
    jsondoc["wifi_signal"]      = "-61";
    
    JsonArray probeData = jsondoc["probes"].to<JsonArray>();
    
    JsonObject probeData_0 = probeData.add<JsonObject>();
    probeData_0["probe_id"] = 1;
    probeData_0["temperature"] = probe_1.calculate_temperature();
    
    JsonObject probeData_1 = probeData.add<JsonObject>();
    probeData_1["probe_id"] = 2;
    probeData_1["temperature"] = probe_2.calculate_temperature();
    
    JsonObject probeData_2 = probeData.add<JsonObject>();
    probeData_2["probe_id"] = 3;
    probeData_2["temperature"] = probe_3.calculate_temperature();
    
    JsonObject probeData_3 = probeData.add<JsonObject>();
    probeData_3["probe_id"] = 4;
    probeData_3["temperature"] = probe_4.calculate_temperature();
    
    JsonObject probeData_4 = probeData.add<JsonObject>();
    probeData_4["probe_id"] = 5;
    probeData_4["temperature"] = probe_5.calculate_temperature();
    
    JsonObject probeData_5 = probeData.add<JsonObject>();
    probeData_5["probe_id"] = 6;
    probeData_5["temperature"] = probe_6.calculate_temperature();
    
    JsonObject probeData_6 = probeData.add<JsonObject>();
    probeData_6["probe_id"] = 7;
    probeData_6["temperature"] = probe_7.calculate_temperature();
    
    JsonObject probeData_7 = probeData.add<JsonObject>();
    probeData_7["probe_id"] = 8;
    probeData_7["temperature"] = probe_8.calculate_temperature();

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer);
    webserver.send(200, "application/json", buffer);
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

    jsondoc["name"] = "Opengrill";
    jsondoc["temperature_unit"] = "celcius";
    jsondoc["alarm_enabled"] = true;
    jsondoc["alarm_volume"] = 1;
    jsondoc["alarm_degrees_before"] = 5;
    jsondoc["alarm_on_wifi_disconnect"] = true;
    jsondoc["auto_shutdown_time"] = 180;
    jsondoc["wifi_ssid"] = "WOZ";
    jsondoc["wifi_passphrase"] = "hidden";
    jsondoc["local_ssid"] = "Opengrill";
    jsondoc["local_passphrase"] = "";

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer);
    webserver.send(200, "application/json", buffer);
}

void post_api_settings() 
{
    if(webserver.hasArg("plain") == false) { webserver.send(400, "application/json", "{\"error\": \"empty body\"}"); return;}

    std::string body = webserver.arg("plain").c_str();
    DeserializationError err = deserializeJson(jsondoc, body);
    if(err) { webserver.send(400, "application/json", "{\"error\": \"malformatted body\"}"); return;}
    
    Serial.println(body.c_str());
    
    // https://arduinojson.org/v7/tutorial/deserialization/
    jsondoc.clear();

    jsondoc["name"] = "Opengrill";
    jsondoc["temperature_unit"] = "celcius";
    jsondoc["alarm_enabled"] = true;
    jsondoc["alarm_volume"] = 1;
    jsondoc["alarm_degrees_before"] = 5;
    jsondoc["alarm_on_wifi_disconnect"] = true;
    jsondoc["auto_shutdown_time"] = 180;
    jsondoc["wifi_ssid"] = "WOZ";
    jsondoc["wifi_passphrase"] = "hidden";
    jsondoc["local_ssid"] = "Opengrill";
    jsondoc["local_passphrase"] = "";

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer);
    
    // return the body for now, if all checks pass then the body is the correct data for the client.
    webserver.send(200, "application/json", body.c_str());
}

void setup_api_routes()
{
    webserver.on("/api/grill", HTTP_GET, get_api_grill);

    webserver.on("/api/probes", HTTP_GET, get_api_probes);
    webserver.on("/api/probes", HTTP_POST, post_api_probes);
    
    webserver.on("/api/settings", HTTP_GET, get_api_settings);
    webserver.on("/api/settings", HTTP_POST, post_api_settings);
}
