#include <ArduinoJson.h>
#include <string>
#include <WiFi.h>
#include <Update.h>

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

    web::webserver.on("/api/update", HTTP_POST, post_api_update, upload_api_update);
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

// Firmware updates, replaces ElegantOTA. upload_api_update runs for every chunk while the file comes
// in, post_api_update once the upload is done. The new firmware goes to the other app slot, so a
// failed or rejected update leaves the running firmware untouched.
namespace {
    bool update_rejected = false;
    int update_status = 400;
    String update_error = "";

    void reject_update(int status, const String& error){
        if(!update_rejected){
            update_rejected = true;
            update_status = status;
            update_error = error;
            Serial.printf("Firmware update rejected: %s\n", error.c_str());
        }
        if(Update.isRunning()){ Update.abort(); }
    }
}

void upload_api_update(){
    HTTPUpload& upload = web::webserver.upload();

    if(upload.status == UPLOAD_FILE_START){
        update_rejected = false;
        update_status = 400;
        update_error = "";

        // A custom header forces a CORS preflight, so a web page on another site can't post a firmware
        if(web::webserver.header("X-Grilly-Update") != "1"){
            reject_update(403, "Missing X-Grilly-Update header");
            return;
        }
        if(!config::admin_password.isEmpty() && !web::webserver.authenticate("admin", config::admin_password.c_str())){
            reject_update(401, "Wrong admin password");
            return;
        }
        Serial.printf("Firmware update: %s\n", upload.filename.c_str());
        return;     // Update.begin waits for the first chunk, so the image can be checked first
    }

    if(update_rejected){ return; }

    if(upload.status == UPLOAD_FILE_WRITE){
        if(!Update.isRunning()){
            // An app image starts with 0xE9. The -full.bin for usb flashing starts with 0xFF padding.
            if(upload.currentSize == 0 || upload.buf[0] != 0xE9){
                reject_update(400, "This is not an OTA firmware file. Use the -ota.bin file.");
                return;
            }
            if(!Update.begin(UPDATE_SIZE_UNKNOWN)){
                reject_update(400, Update.errorString());
                return;
            }
        }
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
            reject_update(400, Update.errorString());
        }
        return;
    }

    if(upload.status == UPLOAD_FILE_END){
        if(!Update.isRunning()){
            reject_update(400, "The file is empty");
            return;
        }
        if(!Update.end(true)){
            reject_update(400, Update.errorString());
        }
        return;
    }

    if(upload.status == UPLOAD_FILE_ABORTED){
        reject_update(400, "The upload was interrupted");
    }
}

void post_api_update(){
    bool installed = !update_rejected && Update.isFinished();
    int status = update_status;
    String error = update_error.isEmpty() ? String("No firmware file received") : update_error;

    // Ready for the next attempt
    update_rejected = false;
    update_status = 400;
    update_error = "";

    if(!installed){
        web::webserver.send(status, "application/json", "{\"error\": \"" + error + "\"}");
        return;
    }

    web::webserver.send(200, "application/json", "{\"success\": true}");
    Serial.println("Firmware update installed, restarting");
    delay(1000);    // Let the response reach the browser
    ESP.restart();
}