#include <Website.h>

#include <WebServer.h>
#include <ArduinoJson.h>

#include "Web.h"

#include "HtmlIndex.h"
#include "HtmlProbes.h"
#include "HtmlSettings.h"
#include "HtmlAbout.h"

extern WebServer webserver;

void setup_web_routes() {
    web::webserver.on("/probes", get_probes);
    web::webserver.on("/settings", get_settings);
    web::webserver.on("/about", get_about);
    
    web::webserver.on("/", get_index);
}

void get_index() {
    web::webserver.send(200, "text/html", HTML_INDEX);
}
void get_probes() {

    Serial.println("Handling probes");
    web::webserver.send(200, "text/html", HTML_PROBES);
    Serial.println("Delivered probes");
}
void get_settings() {
    web::webserver.send(200, "text/html", HTML_SETTINGS);
}
void get_about() {
    web::webserver.send(200, "text/html", HTML_ABOUT);
}


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