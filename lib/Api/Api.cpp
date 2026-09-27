#include <ArduinoJson.h>
#include <string>
#include <WiFi.h>

#include "Probe.h"
#include "Buzzer.h"
#include "GrillConfig.h"

#include "Api.h"
#include "Config.h"
#include "Grill.h"
#include "JsonUtilities.h"
#include "Web.h"

// Set this to config::json_buffer_size, cant do this dynamically
char api_json_buffer[3000];

void setup_api_routes()
{
    web::webserver.on("/api/grill", HTTP_GET, get_api_grill);

    web::webserver.on("/api/probes", HTTP_GET, get_api_probes);
    web::webserver.on("/api/probes", HTTP_POST, post_api_probes);
    web::webserver.on("/api/probes", HTTP_OPTIONS, cors_api_probes);
    
    web::webserver.on("/api/settings", HTTP_GET, get_api_settings);
    web::webserver.on("/api/settings", HTTP_POST, post_api_settings);
    web::webserver.on("/api/settings", HTTP_OPTIONS, cors_api_settings);
    
    web::webserver.on("/api/wifiscan", HTTP_GET, get_api_wifiscan);
}

// Read-only endpoints may be read from other origins. Write endpoints get no CORS headers, and
// only accept a json content type. A json post from another origin needs a CORS preflight, which
// fails without those headers, so a web page on another site can't change settings.
void allow_cross_origin_read(){
    web::webserver.sendHeader("Access-Control-Allow-Origin", "*");
}

bool is_json_request(){
    if(web::webserver.header("Content-Type").startsWith("application/json")){ return true; }

    web::webserver.send(415, "application/json", "{\"error\": \"Content-Type should be application/json\"}");
    return false;
}

void get_api_grill()
{
    config::json_handler.load_json_status(api_json_buffer);
    allow_cross_origin_read();
    web::webserver.send(200, "application/json", api_json_buffer);
}

void get_api_probes(){
    config::json_handler.load_json_probes(api_json_buffer);
    allow_cross_origin_read();
    web::webserver.send(200, "application/json", api_json_buffer);
}

void post_api_probes()
{
    if(!is_json_request()) { return; }
    if(web::webserver.hasArg("plain") == false) { web::webserver.send(400, "application/json", "{\"error\": \"empty body\"}"); return;}

    web::webserver.arg("plain").toCharArray(api_json_buffer, config::json_buffer_size);
    jsonResult result = config::json_handler.save_json_probes(api_json_buffer);
    
    if(!result.success){
        web::webserver.send(400, "application/json", "{\"error\": \"" + result.message + "\"}");
        return;
    }
    
    get_api_probes(); //Return current data if ok
}

// Preflight for a cross-origin write. Answered without CORS headers, so the browser blocks it.
void cors_api_probes(){
    web::webserver.send(204);
    return;
}

void get_api_settings(){
    config::json_handler.load_json_settings(api_json_buffer);
    allow_cross_origin_read();
    web::webserver.send(200, "application/json", api_json_buffer);
}

void post_api_settings(){
    if(!is_json_request()) { return; }
    if(web::webserver.hasArg("plain") == false) { web::webserver.send(400, "application/json", "{\"error\": \"empty body\"}"); return;}

    web::webserver.arg("plain").toCharArray(api_json_buffer, config::json_buffer_size);
    jsonResult result = config::json_handler.save_json_settings(api_json_buffer);
    
    if(!result.success){
        web::webserver.send(400, "application/json", "{\"error\": \"" + result.message + "\"}");
        return;
    }

    get_api_settings(); //Return current data if ok
}

void cors_api_settings(){
    web::webserver.send(204);
    return;
}

void get_api_wifiscan(){
    config::json_handler.load_json_wifiscan(api_json_buffer);
    allow_cross_origin_read();
    web::webserver.send(200, "application/json", api_json_buffer);
    return;
}