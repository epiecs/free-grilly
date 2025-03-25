#include <Website.h>

#include <WebServer.h>
#include <ArduinoJson.h>

#include <Index.h>

extern WebServer webserver;

void setup_web_routes() {
    webserver.on("/", get_index);
    webserver.on("/settings", get_settings);
    webserver.on("/settings", HTTP_POST, post_settings);
}

void get_index() {
    Serial.println("Loaded index page");
    webserver.send(200, "text/html", HTML_INDEX);
}

void get_settings() {}

void post_settings() {}

void not_found() {
    Serial.println("404 - Not Found");

    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webserver.uri();
    message += "\nMethod: ";
    message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webserver.args();
    message += "\n";
    for (uint8_t i = 0; i < webserver.args(); i++) {
        message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
    }
    webserver.send(404, "text/plain", message);
}