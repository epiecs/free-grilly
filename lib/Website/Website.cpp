#include <Website.h>

#include <WebServer.h>
#include <ArduinoJson.h>

#include "Web.h"

#include "WebApp.h"
#include "HtmlSettings.h"
#include "HtmlAbout.h"

#include "AssetCss.h"
#include "AssetJs.h"

extern WebServer webserver;

void setup_web_routes() {
    web::webserver.on("/probes", [](){ redirect_to("/"); });
    web::webserver.on("/settings", get_settings);
    web::webserver.on("/about", get_about);
    
    web::webserver.on("/custom-boostrap.min.css", get_css);
    web::webserver.on("/bootstrap.min.js", get_js);
    
    web::webserver.on("/", get_index);
}

// The web app is one gzipped page generated from web/ by tools/build_web.py. The ETag changes with
// every page change, so browsers keep their copy until a firmware update brings a new one.
void get_index() {
    if (web::webserver.header("If-None-Match") == WEB_APP_ETAG) {
        web::webserver.sendHeader("ETag", WEB_APP_ETAG);
        web::webserver.send(304);
        return;
    }
    web::webserver.sendHeader("Content-Encoding", "gzip");
    web::webserver.sendHeader("Cache-Control", "no-cache");
    web::webserver.sendHeader("ETag", WEB_APP_ETAG);
    web::webserver.send_P(200, "text/html", (PGM_P)WEB_APP_GZ, WEB_APP_GZ_LEN);
}

// Old page urls redirect into the web app, so bookmarks keep working
void redirect_to(const char* location) {
    web::webserver.sendHeader("Location", location);
    web::webserver.send(302, "text/plain", "");
}

void get_settings() {
    web::webserver.send(200, "text/html", HTML_SETTINGS);
}

void get_about() {
    web::webserver.send(200, "text/html", HTML_ABOUT);
}

void get_css() {
    web::webserver.send_P(200, "text/css", ASSET_CSS);
}

void get_js() {
    web::webserver.send_P(200, "text/javascript", ASSET_JS);
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

    Serial.println(message);

    web::webserver.send(404, "text/plain", message);
}