#include <Website.h>

#include <WebServer.h>
#include <ArduinoJson.h>

#include "Web.h"

#include "Index.h"

extern WebServer webserver;

void setup_web_routes() {
    web::webserver.on("/", get_index);
    web::webserver.on("/settings", get_settings);
    web::webserver.on("/settings", HTTP_POST, post_settings);
}

void get_index() {
    Serial.println("Loaded index page");
    web::webserver.send(200, "text/html", HTML_INDEX);
}

void get_settings() {}

void post_settings() {}

void not_found() {
    Serial.println("404 - Not Found");

    String message = "File Not Found\n\n";
    message += "URI: ";
    message += web::webserver.uri();
    message += "\nMethod: ";
    message += (web::webserver.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += web::webserver.args();
    message += "\n";
    for (uint8_t i = 0; i < web::webserver.args(); i++) {
        message += " " + web::webserver.argName(i) + ": " + web::webserver.arg(i) + "\n";
    }
    web::webserver.send(404, "text/plain", message);
}