#include <ArduinoJson.h>
#include <Preferences.h>
#include <WiFi.h>
#include <chrono>

#include "Config.h"
#include "Grill.h"
#include "GrillConfig.h"
#include "JsonUtilities.h"
#include "Probe.h"
#include "SharedLock.h"

// Every function uses its own JsonDocument. The api, mqtt and opengrill tasks call these at the same
// time, a shared document got cleared and filled by one task while another was serializing it.

namespace {

// Reads fields from a json object into variables. A key that is missing or null keeps the current
// value, so partial updates (mqtt, opengrill) only change what they contain. Values are only written
// when apply is true, so the same field list can be checked first and applied after.
// Numbers may arrive as json numbers or as strings, the web pages send input values as strings. An
// empty string counts as 0.
class FieldReader {
    public:
        FieldReader(JsonObjectConst object, bool apply) : object(object), apply(apply) {}

        String error;

        bool present(const char* key){
            return !object[key].isNull();
        }

        void text(const char* key, String& target){
            if(!present(key) || !error.isEmpty()){ return; }
            if(!object[key].is<const char*>()){ error = String(key) + " should be a string"; return; }
            if(apply){ target = object[key].as<String>(); }
        }

        void boolean(const char* key, bool& target){
            if(!present(key) || !error.isEmpty()){ return; }
            if(!object[key].is<bool>()){ error = String(key) + " should be true or false"; return; }
            if(apply){ target = object[key].as<bool>(); }
        }

        void number(const char* key, float& target){
            float value;
            if(!read_number(key, value)){ return; }
            if(apply){ target = value; }
        }

        void number(const char* key, float& target, float min, float max){
            float value;
            if(!read_number(key, value)){ return; }
            if(value < min || value > max){
                error = String(key) + " should be between " + String(min) + " and " + String(max);
                return;
            }
            if(apply){ target = value; }
        }

        void number(const char* key, int& target, long min, long max){
            float value;
            if(!read_number(key, value)){ return; }
            if(value < min || value > max){
                error = String(key) + " should be between " + String(min) + " and " + String(max);
                return;
            }
            if(apply){ target = static_cast<int>(value); }
        }

    private:
        JsonObjectConst object;
        bool apply;

        bool read_number(const char* key, float& value){
            if(!present(key) || !error.isEmpty()){ return false; }

            JsonVariantConst field = object[key];
            if(field.is<float>()){
                value = field.as<float>();
                return true;
            }
            if(field.is<const char*>()){
                const char* raw = field.as<const char*>();
                if(raw[0] == '\0'){ value = 0; return true; }
                char* end;
                value = strtof(raw, &end);
                if(*end == '\0'){ return true; }
            }

            error = String(key) + " should be a number";
            return false;
        }
};

Probe* probe_by_id(int probe_id){
    switch (probe_id){
        case 1: return &grill::probe_1;
        case 2: return &grill::probe_2;
        case 3: return &grill::probe_3;
        case 4: return &grill::probe_4;
        case 5: return &grill::probe_5;
        case 6: return &grill::probe_6;
        case 7: return &grill::probe_7;
        case 8: return &grill::probe_8;
        default: return nullptr;
    }
}

// Seconds since the probe was connected, 0 when it isn't. connected_time is set from the same clock
// in Probe::calculate_temperature.
long connected_seconds(const Probe& probe){
    if(!probe.connected){ return 0; }
    long now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return now - probe.connected_time;
}

// Checks one probe entry and, when apply is true, stores it on the probe. Missing keys keep the
// current value. Opengrill sends null for "no temperature", so null_is_zero turns an explicit null
// temperature into 0 instead of keeping the current value.
String update_probe(Probe& probe, JsonObjectConst item, bool apply, bool null_is_zero){
    FieldReader fields(item, true);     // Reads into the local copies below, the probe is only set at the end

    String name     = probe.name;
    float  target   = probe.target_temperature;
    float  minimum  = probe.minimum_temperature;
    String type     = probe.type;
    int    kohm     = probe.reference_kohm;
    int    celcius  = probe.reference_celcius;
    int    beta     = probe.reference_beta;
    float  offset   = probe.offset_celcius;

    // A missing key is unbound, an explicit null is bound but null
    bool target_null   = null_is_zero && !item["target_temperature"].isUnbound()  && item["target_temperature"].isNull();
    bool minimum_null  = null_is_zero && !item["minimum_temperature"].isUnbound() && item["minimum_temperature"].isNull();
    bool target_given  = fields.present("target_temperature")  || target_null;
    bool minimum_given = fields.present("minimum_temperature") || minimum_null;
    if(target_null)  { target  = 0; }
    if(minimum_null) { minimum = 0; }

    fields.text("name", name);
    fields.number("target_temperature", target);
    fields.number("minimum_temperature", minimum);
    fields.text("probe_type", type);
    fields.number("reference_kohm", kohm, 0, 100000);
    fields.number("reference_celcius", celcius, -100, 500);
    fields.number("reference_beta", beta, 0, 100000);
    fields.number("offset_celcius", offset, -10.0, 10.0);
    if(!fields.error.isEmpty()){ return fields.error; }

    bool type_given = fields.present("probe_type") || fields.present("reference_kohm")
                   || fields.present("reference_celcius") || fields.present("reference_beta");

    bool known_type = type == "grilleye_iris" || type == "ikea_fantast" || type == "maverick_et733" || type == "weber_igrill";
    if(type_given && !known_type && (kohm <= 0 || beta <= 0)){
        return "reference_kohm and reference_beta should be > 0 for a custom probe";
    }

    if(apply){
        if(target_given || minimum_given){ probe.set_temperature(target, minimum); }
        if(type_given){ probe.set_type(type, kohm, celcius, beta); }
        if(fields.present("name")){ probe.set_name(name); }
        if(fields.present("offset_celcius")){ probe.set_offset(offset); }
    }
    return "";
}

}

void JsonUtilities::load_json_status(char *buffer){
    JsonDocument jsondoc;
    jsondoc.clear();
    SharedLock lock;    // Copies config and probe name Strings into the document

    jsondoc["name"]               = config::grill_name;
    jsondoc["unique_id"]          = config::grill_uuid;
    jsondoc["firmware_version"]   = config::grill_firmware_version;
    jsondoc["hostname"]           = String(grill::hostname) + ".local";
    jsondoc["battery_percentage"] = grill::battery_percentage;
    jsondoc["battery_charging"]   = grill::battery_charging;
    jsondoc["battery_millivolts"] = grill::battery_millivolts;
    jsondoc["last_reset_reason"]  = grill::last_reset_reason;
    jsondoc["last_off_reason"]    = grill::last_off_reason;
    jsondoc["wifi_connected"]     = grill::wifi_connected;
    jsondoc["wifi_ssid"]          = config::wifi_ssid;
    jsondoc["wifi_ip"]            = grill::wifi_ip;
    jsondoc["wifi_signal"]        = WiFi.RSSI();
    jsondoc["local_ap_ssid"]      = config::local_ap_ssid;
    jsondoc["local_ap_ip"]        = config::local_ap_ip;
    jsondoc["temperature_unit"]   = config::temperature_unit;
    jsondoc["alarm_sounding"]     = grill::alarm_sounding;

    JsonArray probeData = jsondoc["probes"].to<JsonArray>();

    JsonObject probeData_0 = probeData.add<JsonObject>();
    probeData_0["probe_id"] = 1;
    probeData_0["name"] = grill::probe_1.name;
    probeData_0["temperature"] = grill::probe_1.temperature;
    probeData_0["minimum_temperature"] = grill::probe_1.minimum_temperature;
    probeData_0["target_temperature"] = grill::probe_1.target_temperature;
    probeData_0["connected"] = grill::probe_1.connected;
    probeData_0["connected_seconds"] = connected_seconds(grill::probe_1);
    probeData_0["alarm"] = (grill::alarm_probes & (1 << 0)) != 0;

    JsonObject probeData_1 = probeData.add<JsonObject>();
    probeData_1["probe_id"] = 2;
    probeData_1["name"] = grill::probe_2.name;
    probeData_1["temperature"] = grill::probe_2.temperature;
    probeData_1["minimum_temperature"] = grill::probe_2.minimum_temperature;
    probeData_1["target_temperature"] = grill::probe_2.target_temperature;
    probeData_1["connected"] = grill::probe_2.connected;
    probeData_1["connected_seconds"] = connected_seconds(grill::probe_2);
    probeData_1["alarm"] = (grill::alarm_probes & (1 << 1)) != 0;

    JsonObject probeData_2 = probeData.add<JsonObject>();
    probeData_2["probe_id"] = 3;
    probeData_2["name"] = grill::probe_3.name;
    probeData_2["temperature"] = grill::probe_3.temperature;
    probeData_2["minimum_temperature"] = grill::probe_3.minimum_temperature;
    probeData_2["target_temperature"] = grill::probe_3.target_temperature;
    probeData_2["connected"] = grill::probe_3.connected;
    probeData_2["connected_seconds"] = connected_seconds(grill::probe_3);
    probeData_2["alarm"] = (grill::alarm_probes & (1 << 2)) != 0;

    JsonObject probeData_3 = probeData.add<JsonObject>();
    probeData_3["probe_id"] = 4;
    probeData_3["name"] = grill::probe_4.name;
    probeData_3["temperature"] = grill::probe_4.temperature;
    probeData_3["minimum_temperature"] = grill::probe_4.minimum_temperature;
    probeData_3["target_temperature"] = grill::probe_4.target_temperature;
    probeData_3["connected"] = grill::probe_4.connected;
    probeData_3["connected_seconds"] = connected_seconds(grill::probe_4);
    probeData_3["alarm"] = (grill::alarm_probes & (1 << 3)) != 0;

    JsonObject probeData_4 = probeData.add<JsonObject>();
    probeData_4["probe_id"] = 5;
    probeData_4["name"] = grill::probe_5.name;
    probeData_4["temperature"] = grill::probe_5.temperature;
    probeData_4["minimum_temperature"] = grill::probe_5.minimum_temperature;
    probeData_4["target_temperature"] = grill::probe_5.target_temperature;
    probeData_4["connected"] = grill::probe_5.connected;
    probeData_4["connected_seconds"] = connected_seconds(grill::probe_5);
    probeData_4["alarm"] = (grill::alarm_probes & (1 << 4)) != 0;

    JsonObject probeData_5 = probeData.add<JsonObject>();
    probeData_5["probe_id"] = 6;
    probeData_5["name"] = grill::probe_6.name;
    probeData_5["temperature"] = grill::probe_6.temperature;
    probeData_5["minimum_temperature"] = grill::probe_6.minimum_temperature;
    probeData_5["target_temperature"] = grill::probe_6.target_temperature;
    probeData_5["connected"] = grill::probe_6.connected;
    probeData_5["connected_seconds"] = connected_seconds(grill::probe_6);
    probeData_5["alarm"] = (grill::alarm_probes & (1 << 5)) != 0;

    JsonObject probeData_6 = probeData.add<JsonObject>();
    probeData_6["probe_id"] = 7;
    probeData_6["name"] = grill::probe_7.name;
    probeData_6["temperature"] = grill::probe_7.temperature;
    probeData_6["minimum_temperature"] = grill::probe_7.minimum_temperature;
    probeData_6["target_temperature"] = grill::probe_7.target_temperature;
    probeData_6["connected"] = grill::probe_7.connected;
    probeData_6["connected_seconds"] = connected_seconds(grill::probe_7);
    probeData_6["alarm"] = (grill::alarm_probes & (1 << 6)) != 0;

    JsonObject probeData_7 = probeData.add<JsonObject>();
    probeData_7["probe_id"] = 8;
    probeData_7["name"] = grill::probe_8.name;
    probeData_7["temperature"] = grill::probe_8.temperature;
    probeData_7["minimum_temperature"] = grill::probe_8.minimum_temperature;
    probeData_7["target_temperature"] = grill::probe_8.target_temperature;
    probeData_7["connected"] = grill::probe_8.connected;
    probeData_7["connected_seconds"] = connected_seconds(grill::probe_8);
    probeData_7["alarm"] = (grill::alarm_probes & (1 << 7)) != 0;

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer, config::json_buffer_size);
}

// Passwords are never sent out, this json is served without authentication and published to the
// mqtt broker. Only whether a password is set is included.
void JsonUtilities::load_json_settings(char* buffer){
    JsonDocument jsondoc;
    SharedLock lock;    // Copies config Strings into the document

    jsondoc.clear();

    jsondoc["name"]                      = config::grill_name;
    jsondoc["uuid"]                      = config::grill_uuid;
    jsondoc["firmware_version"]          = config::grill_firmware_version;

    jsondoc["temperature_unit"]          = config::temperature_unit;
    jsondoc["beep_enabled"]              = config::beep_enabled;
    jsondoc["beep_volume"]               = config::beep_volume;
    jsondoc["beep_degrees_before"]       = config::beep_degrees_before;
    jsondoc["beep_outside_target"]       = config::beep_outside_target;
    jsondoc["beep_on_ready"]             = config::beep_on_ready;
    jsondoc["cucaracha_enabled"]         = config::cucaracha_enabled;

    jsondoc["screen_timeout_minutes"]    = config::screen_timeout_minutes;
    jsondoc["backlight_timeout_minutes"] = config::backlight_timeout_minutes;
    jsondoc["backlight_brightness"]      = config::backlight_brightness;

    jsondoc["opengrill_server"]          = config::opengrill_server;

    jsondoc["mqtt_broker"]               = config::mqtt_broker;
    jsondoc["mqtt_port"]                 = config::mqtt_port;
    jsondoc["mqtt_topic"]                = config::mqtt_topic;
    jsondoc["mqtt_user"]                 = config::mqtt_user;
    jsondoc["mqtt_password_set"]         = config::mqtt_password.length() > 0;

    jsondoc["wifi_ssid"]                 = config::wifi_ssid;
    jsondoc["wifi_ip"]                   = config::wifi_ip;
    jsondoc["wifi_subnet"]               = config::wifi_subnet;
    jsondoc["wifi_gateway"]              = config::wifi_gateway;
    jsondoc["wifi_dns"]                  = config::wifi_dns;
    jsondoc["wifi_password_set"]         = config::wifi_password.length() > 0;

    jsondoc["local_ap_ssid"]             = config::local_ap_ssid;
    jsondoc["local_ap_ip"]               = config::local_ap_ip;
    jsondoc["local_ap_subnet"]           = config::local_ap_subnet;
    jsondoc["local_ap_gateway"]          = config::local_ap_gateway;
    jsondoc["local_ap_password_set"]     = config::local_ap_password.length() > 0;

    jsondoc["admin_password_set"]        = config::admin_password.length() > 0;

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer, config::json_buffer_size);
}

jsonResult JsonUtilities::save_json_settings(char* raw_json, bool admin_authorized, bool from_mqtt){
    JsonDocument jsondoc;
    DeserializationError err = deserializeJson(jsondoc, raw_json);

    if(err){ return {false, "Could not deserialize json"}; }

    if(!jsondoc.is<JsonObject>()){ return {false, "Settings should be a json object"}; }
    JsonObjectConst json_data = jsondoc.as<JsonObjectConst>();

    {
        SharedLock lock;    // The store pass writes the shared config Strings
        // The admin password protects firmware updates, so changing or removing it needs the current
        // one. The whole payload is rejected before anything is stored.
        if(!json_data["admin_password"].isNull()){
            if(from_mqtt){ return {false, "admin_password can't be changed over MQTT"}; }
            if(!config::admin_password.isEmpty() && !admin_authorized){
                return {false, "The current admin password is needed to change it", true};
            }
        }

        // Missing keys keep their current value. The list is run twice: first to check every value,
        // then to store them, so a payload with one bad value changes nothing.
        auto read_settings = [](FieldReader& fields){
            fields.text("name",                         config::grill_name);

            fields.text("temperature_unit",             config::temperature_unit);
            fields.boolean("beep_enabled",              config::beep_enabled);
            fields.number("beep_volume",                config::beep_volume, 0, 5);
            fields.number("beep_degrees_before",        config::beep_degrees_before, 0, 100);
            fields.boolean("beep_outside_target",       config::beep_outside_target);
            fields.boolean("beep_on_ready",             config::beep_on_ready);
            fields.boolean("cucaracha_enabled",         config::cucaracha_enabled);

            fields.number("screen_timeout_minutes",     config::screen_timeout_minutes, 0, 10000);
            fields.number("backlight_timeout_minutes",  config::backlight_timeout_minutes, 0, 10000);
            fields.number("backlight_brightness",       config::backlight_brightness, 0, 5);

            fields.text("opengrill_server",             config::opengrill_server);

            fields.text("mqtt_broker",                  config::mqtt_broker);
            fields.number("mqtt_port",                  config::mqtt_port, 1, 65535);
            fields.text("mqtt_topic",                   config::mqtt_topic);
            fields.text("mqtt_user",                    config::mqtt_user);
            fields.text("mqtt_password",                config::mqtt_password);

            fields.text("wifi_ssid",                    config::wifi_ssid);
            fields.text("wifi_password",                config::wifi_password);
            fields.text("wifi_ip",                      config::wifi_ip);
            fields.text("wifi_subnet",                  config::wifi_subnet);
            fields.text("wifi_gateway",                 config::wifi_gateway);
            fields.text("wifi_dns",                     config::wifi_dns);

            fields.text("local_ap_ssid",                config::local_ap_ssid);
            fields.text("local_ap_password",            config::local_ap_password);
            fields.text("local_ap_ip",                  config::local_ap_ip);
            fields.text("local_ap_subnet",              config::local_ap_subnet);
            fields.text("local_ap_gateway",             config::local_ap_gateway);

            fields.text("admin_password",               config::admin_password);
        };

        FieldReader check(json_data, false);
        read_settings(check);
        if(!check.error.isEmpty()){ return {false, check.error}; }

        if(check.present("local_ap_password")){
            size_t length = json_data["local_ap_password"].as<String>().length();
            if(length > 0 && length < 8){
                return {false, "local_ap_password should be empty or at least 8 characters"};
            }
        }

        if(check.present("temperature_unit")){
            String unit = json_data["temperature_unit"].as<String>();
            if(unit != "celcius" && unit != "fahrenheit"){
                return {false, "temperature_unit should be celcius or fahrenheit"};
            }
        }

        FieldReader store(json_data, true);
        read_settings(store);

        // Set default value for empty topics
        if(config::mqtt_topic.length() == 0){
            config::mqtt_topic = "grilly-plus";
        }
    }

    // Not under the lock, it may reconnect the wifi. It takes the lock itself for the NVS writes.
    config::config_helper.save_settings();
    return {true, "Ok"};
}

void JsonUtilities::load_json_probes(char* buffer){
    JsonDocument jsondoc;
    jsondoc.clear();
    SharedLock lock;    // Copies probe name and type Strings into the document

    JsonObject doc_0 = jsondoc.add<JsonObject>();
    doc_0["probe_id"] = 1;
    doc_0["temperature"] = grill::probe_1.temperature;
    doc_0["name"] = grill::probe_1.name;
    doc_0["minimum_temperature"] = grill::probe_1.minimum_temperature;
    doc_0["target_temperature"] = grill::probe_1.target_temperature;
    doc_0["connected"] = grill::probe_1.connected;
    doc_0["probe_type"] = grill::probe_1.type;
    doc_0["reference_kohm"] = grill::probe_1.reference_kohm;
    doc_0["reference_celcius"] = grill::probe_1.reference_celcius;
    doc_0["reference_beta"] = grill::probe_1.reference_beta;
    doc_0["offset_celcius"] = grill::probe_1.offset_celcius;

    JsonObject doc_1 = jsondoc.add<JsonObject>();
    doc_1["probe_id"] = 2;
    doc_1["temperature"] = grill::probe_2.temperature;
    doc_1["name"] = grill::probe_2.name;
    doc_1["minimum_temperature"] = grill::probe_2.minimum_temperature;
    doc_1["target_temperature"] = grill::probe_2.target_temperature;
    doc_1["connected"] = grill::probe_2.connected;
    doc_1["probe_type"] = grill::probe_2.type;
    doc_1["reference_kohm"] = grill::probe_2.reference_kohm;
    doc_1["reference_celcius"] = grill::probe_2.reference_celcius;
    doc_1["reference_beta"] = grill::probe_2.reference_beta;
    doc_1["offset_celcius"] = grill::probe_2.offset_celcius;

    JsonObject doc_2 = jsondoc.add<JsonObject>();
    doc_2["probe_id"] = 3;
    doc_2["temperature"] = grill::probe_3.temperature;
    doc_2["name"] = grill::probe_3.name;
    doc_2["minimum_temperature"] = grill::probe_3.minimum_temperature;
    doc_2["target_temperature"] = grill::probe_3.target_temperature;
    doc_2["connected"] = grill::probe_3.connected;
    doc_2["probe_type"] = grill::probe_3.type;
    doc_2["reference_kohm"] = grill::probe_3.reference_kohm;
    doc_2["reference_celcius"] = grill::probe_3.reference_celcius;
    doc_2["reference_beta"] = grill::probe_3.reference_beta;
    doc_2["offset_celcius"] = grill::probe_3.offset_celcius;

    JsonObject doc_3 = jsondoc.add<JsonObject>();
    doc_3["probe_id"] = 4;
    doc_3["temperature"] = grill::probe_4.temperature;
    doc_3["name"] = grill::probe_4.name;
    doc_3["minimum_temperature"] = grill::probe_4.minimum_temperature;
    doc_3["target_temperature"] = grill::probe_4.target_temperature;
    doc_3["connected"] = grill::probe_4.connected;
    doc_3["probe_type"] = grill::probe_4.type;
    doc_3["reference_kohm"] = grill::probe_4.reference_kohm;
    doc_3["reference_celcius"] = grill::probe_4.reference_celcius;
    doc_3["reference_beta"] = grill::probe_4.reference_beta;
    doc_3["offset_celcius"] = grill::probe_4.offset_celcius;

    JsonObject doc_4 = jsondoc.add<JsonObject>();
    doc_4["probe_id"] = 5;
    doc_4["temperature"] = grill::probe_5.temperature;
    doc_4["name"] = grill::probe_5.name;
    doc_4["minimum_temperature"] = grill::probe_5.minimum_temperature;
    doc_4["target_temperature"] = grill::probe_5.target_temperature;
    doc_4["connected"] = grill::probe_5.connected;
    doc_4["probe_type"] = grill::probe_5.type;
    doc_4["reference_kohm"] = grill::probe_5.reference_kohm;
    doc_4["reference_celcius"] = grill::probe_5.reference_celcius;
    doc_4["reference_beta"] = grill::probe_5.reference_beta;
    doc_4["offset_celcius"] = grill::probe_5.offset_celcius;

    JsonObject doc_5 = jsondoc.add<JsonObject>();
    doc_5["probe_id"] = 6;
    doc_5["temperature"] = grill::probe_6.temperature;
    doc_5["name"] = grill::probe_6.name;
    doc_5["minimum_temperature"] = grill::probe_6.minimum_temperature;
    doc_5["target_temperature"] = grill::probe_6.target_temperature;
    doc_5["connected"] = grill::probe_6.connected;
    doc_5["probe_type"] = grill::probe_6.type;
    doc_5["reference_kohm"] = grill::probe_6.reference_kohm;
    doc_5["reference_celcius"] = grill::probe_6.reference_celcius;
    doc_5["reference_beta"] = grill::probe_6.reference_beta;
    doc_5["offset_celcius"] = grill::probe_6.offset_celcius;

    JsonObject doc_6 = jsondoc.add<JsonObject>();
    doc_6["probe_id"] = 7;
    doc_6["temperature"] = grill::probe_7.temperature;
    doc_6["name"] = grill::probe_7.name;
    doc_6["minimum_temperature"] = grill::probe_7.minimum_temperature;
    doc_6["target_temperature"] = grill::probe_7.target_temperature;
    doc_6["connected"] = grill::probe_7.connected;
    doc_6["probe_type"] = grill::probe_7.type;
    doc_6["reference_kohm"] = grill::probe_7.reference_kohm;
    doc_6["reference_celcius"] = grill::probe_7.reference_celcius;
    doc_6["reference_beta"] = grill::probe_7.reference_beta;
    doc_6["offset_celcius"] = grill::probe_7.offset_celcius;

    JsonObject doc_7 = jsondoc.add<JsonObject>();
    doc_7["probe_id"] = 8;
    doc_7["temperature"] = grill::probe_8.temperature;
    doc_7["name"] = grill::probe_8.name;
    doc_7["minimum_temperature"] = grill::probe_8.minimum_temperature;
    doc_7["target_temperature"] = grill::probe_8.target_temperature;
    doc_7["connected"] = grill::probe_8.connected;
    doc_7["probe_type"] = grill::probe_8.type;
    doc_7["reference_kohm"] = grill::probe_8.reference_kohm;
    doc_7["reference_celcius"] = grill::probe_8.reference_celcius;
    doc_7["reference_beta"] = grill::probe_8.reference_beta;
    doc_7["offset_celcius"] = grill::probe_8.offset_celcius;

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer, config::json_buffer_size);
}

jsonResult JsonUtilities::save_json_probes(char* raw_json){
    JsonDocument jsondoc;

    DeserializationError err = deserializeJson(jsondoc, raw_json);
    if(err){ return {false, "Could not deserialize json"}; }
    if(!jsondoc.is<JsonArray>()){ return {false, "Probes should be a json array"}; }

    {
        SharedLock lock;    // update_probe reads and writes the probe name and type
        // Check every probe first so a payload with one bad entry changes nothing, then store them
        for (int pass = 0; pass < 2; pass++){
            bool apply = pass == 1;

            for (JsonObjectConst item : jsondoc.as<JsonArrayConst>()) {
                Probe* probe = probe_by_id(item["probe_id"].as<int>());
                if(probe == nullptr){ return {false, "probe_id should be between 1 and 8"}; }

                String error = update_probe(*probe, item, apply, false);
                if(!error.isEmpty()){ return {false, "Probe " + item["probe_id"].as<String>() + ": " + error}; }
            }
        }
    }

    config::config_helper.save_probes();

    return {true, "Ok"};
}

void JsonUtilities::load_opengrill_grill(char *buffer){
    JsonDocument jsondoc;
    jsondoc.clear();
    SharedLock lock;    // Copies config Strings into the document

    jsondoc["name"]                 = config::grill_name;
    jsondoc["battery_percentage"]   = grill::battery_percentage;
    jsondoc["temperature_unit"]     = config::temperature_unit;
    jsondoc["max_supported_probes"] = 8;

    JsonObject temperatures = jsondoc["temperatures"].to<JsonObject>();
    temperatures["1"] = grill::probe_1.temperature;
    temperatures["2"] = grill::probe_2.temperature;
    temperatures["3"] = grill::probe_3.temperature;
    temperatures["4"] = grill::probe_4.temperature;
    temperatures["5"] = grill::probe_5.temperature;
    temperatures["6"] = grill::probe_6.temperature;
    temperatures["7"] = grill::probe_7.temperature;
    temperatures["8"] = grill::probe_8.temperature;

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer, config::json_buffer_size);
}

jsonResult JsonUtilities::save_opengrill_grill(char* raw_json){
    JsonDocument jsondoc;
    DeserializationError err = deserializeJson(jsondoc, raw_json);

    if(err){ return {false, "Could not deserialize json"}; }
    if(!jsondoc.is<JsonObject>()){ return {false, "Grill should be a json object"}; }

    {
        SharedLock lock;    // The store pass writes config::grill_name
        FieldReader check(jsondoc.as<JsonObjectConst>(), false);
        check.text("name", config::grill_name);
        if(!check.error.isEmpty()){ return {false, check.error}; }
        if(!check.present("name")){ return {true, "Ok"}; }

        FieldReader store(jsondoc.as<JsonObjectConst>(), true);
        store.text("name", config::grill_name);
    }

    // Not under the lock, see save_json_settings
    config::config_helper.save_settings();
    return {true, "Ok"};
}

void JsonUtilities::load_opengrill_probes(char* buffer){
    JsonDocument jsondoc;
    jsondoc.clear();
    SharedLock lock;    // Copies probe name Strings into the document

    JsonObject p1 = jsondoc["1"].to<JsonObject>();
    p1["name"] = grill::probe_1.name;
    p1["target_temperature"] = grill::probe_1.target_temperature;

    if(grill::probe_1.minimum_temperature > 0.00f){
        p1["minimum_temperature"] = grill::probe_1.minimum_temperature;
    } else {
        p1["minimum_temperature"] = nullptr;
    }

    JsonObject p2 = jsondoc["2"].to<JsonObject>();
    p2["name"] = grill::probe_2.name;
    p2["target_temperature"] = grill::probe_2.target_temperature;

    if(grill::probe_2.minimum_temperature > 0.00f){
        p2["minimum_temperature"] = grill::probe_2.minimum_temperature;
    } else {
        p2["minimum_temperature"] = nullptr;
    }

    JsonObject p3 = jsondoc["3"].to<JsonObject>();
    p3["name"] = grill::probe_3.name;
    p3["target_temperature"] = grill::probe_3.target_temperature;

    if(grill::probe_3.minimum_temperature > 0.00f){
        p3["minimum_temperature"] = grill::probe_3.minimum_temperature;
    } else {
        p3["minimum_temperature"] = nullptr;
    }

    JsonObject p4 = jsondoc["4"].to<JsonObject>();
    p4["name"] = grill::probe_4.name;
    p4["target_temperature"] = grill::probe_4.target_temperature;

    if(grill::probe_4.minimum_temperature > 0.00f){
        p4["minimum_temperature"] = grill::probe_4.minimum_temperature;
    } else {
        p4["minimum_temperature"] = nullptr;
    }

    JsonObject p5 = jsondoc["5"].to<JsonObject>();
    p5["name"] = grill::probe_5.name;
    p5["target_temperature"] = grill::probe_5.target_temperature;

    if(grill::probe_5.minimum_temperature > 0.00f){
        p5["minimum_temperature"] = grill::probe_5.minimum_temperature;
    } else {
        p5["minimum_temperature"] = nullptr;
    }

    JsonObject p6 = jsondoc["6"].to<JsonObject>();
    p6["name"] = grill::probe_6.name;
    p6["target_temperature"] = grill::probe_6.target_temperature;

    if(grill::probe_6.minimum_temperature > 0.00f){
        p6["minimum_temperature"] = grill::probe_6.minimum_temperature;
    } else {
        p6["minimum_temperature"] = nullptr;
    }

    JsonObject p7 = jsondoc["7"].to<JsonObject>();
    p7["name"] = grill::probe_7.name;
    p7["target_temperature"] = grill::probe_7.target_temperature;

    if(grill::probe_7.minimum_temperature > 0.00f){
        p7["minimum_temperature"] = grill::probe_7.minimum_temperature;
    } else {
        p7["minimum_temperature"] = nullptr;
    }

    JsonObject p8 = jsondoc["8"].to<JsonObject>();
    p8["name"] = grill::probe_8.name;
    p8["target_temperature"] = grill::probe_8.target_temperature;

    if(grill::probe_8.minimum_temperature > 0.00f){
        p8["minimum_temperature"] = grill::probe_8.minimum_temperature;
    } else {
        p8["minimum_temperature"] = nullptr;
    }

    jsondoc.shrinkToFit();
    serializeJson(jsondoc, buffer, config::json_buffer_size);
}

jsonResult JsonUtilities::save_opengrill_probes(char* raw_json){
    JsonDocument jsondoc;

    DeserializationError err = deserializeJson(jsondoc, raw_json);
    if(err){ return {false, "Could not deserialize json"}; }
    if(!jsondoc.is<JsonObject>()){ return {false, "Probes should be a json object"}; }

    {
        SharedLock lock;    // update_probe reads and writes the probe name and type
        // Opengrill sends {"<probe_id>": {...}}. Check every probe first, then store them.
        for (int pass = 0; pass < 2; pass++){
            bool apply = pass == 1;

            for (JsonPairConst item : jsondoc.as<JsonObjectConst>()) {
                Probe* probe = probe_by_id(atoi(item.key().c_str()));
                if(probe == nullptr){ return {false, "probe_id should be between 1 and 8"}; }
                if(!item.value().is<JsonObjectConst>()){ return {false, "Probe " + String(item.key().c_str()) + " should be a json object"}; }

                String error = update_probe(*probe, item.value().as<JsonObjectConst>(), apply, true);
                if(!error.isEmpty()){ return {false, "Probe " + String(item.key().c_str()) + ": " + error}; }
            }
        }
    }

    config::config_helper.save_probes();

    return {true, "Ok"};
}

void JsonUtilities::load_json_wifiscan(char* buffer){
    JsonDocument jsondoc;

    Serial.println("Starting WIFI scan");

    int scanned_networks = WiFi.scanNetworks();

    if (scanned_networks == 0) {
        Serial.println("no networks found");
    }

    jsondoc.clear();

    JsonArray networks = jsondoc.to<JsonArray>();

    // At most 20 networks, more don't fit in the api buffer and would give truncated json
    int listed_networks = scanned_networks < 20 ? scanned_networks : 20;
    for (int network_nr = 0; network_nr < listed_networks; ++network_nr) {

        JsonObject scanned_network = networks.add<JsonObject>();

        scanned_network["ssid"]            = WiFi.SSID(network_nr).c_str();
        scanned_network["signal_strength"] = WiFi.RSSI(network_nr);

        switch (WiFi.encryptionType(network_nr)) {
            case WIFI_AUTH_OPEN:            scanned_network["auth_method"] = "open";            break;
            case WIFI_AUTH_WEP:             scanned_network["auth_method"] = "wep";             break;
            case WIFI_AUTH_WPA_PSK:         scanned_network["auth_method"] = "wpa_psk";         break;
            case WIFI_AUTH_WPA2_PSK:        scanned_network["auth_method"] = "wpa2_psk";        break;
            case WIFI_AUTH_WPA_WPA2_PSK:    scanned_network["auth_method"] = "wpa_wpa2_psk";    break;
            case WIFI_AUTH_WPA2_ENTERPRISE: scanned_network["auth_method"] = "wpa2_enterprise"; break;
            case WIFI_AUTH_WPA3_PSK:        scanned_network["auth_method"] = "wpa3_psk";        break;
            case WIFI_AUTH_WPA2_WPA3_PSK:   scanned_network["auth_method"] = "wpa2_wpa3_psk";   break;
            case WIFI_AUTH_WAPI_PSK:        scanned_network["auth_method"] = "wpapi_psk";       break;
            default:                        scanned_network["auth_method"] = "unknown";         break;
        }
    }
    // Free memory
    WiFi.scanDelete();

    jsondoc.shrinkToFit();

    serializeJson(jsondoc, buffer, config::json_buffer_size);
}
