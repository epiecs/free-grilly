#include <Website.h>

#include <WebServer.h>

#include "Web.h"
#include "WebApp.h"

void setup_web_routes() {
    web::webserver.on("/", HTTP_GET, get_index);
    web::webserver.on("/apple-touch-icon.png", HTTP_GET, get_touch_icon);

    // Old page urls, kept so bookmarks keep working
    web::webserver.on("/probes", [](){ redirect_to("/"); });
    web::webserver.on("/settings", [](){ redirect_to("/#settings"); });
    web::webserver.on("/about", [](){ redirect_to("/#about"); });
    web::webserver.on("/update", [](){ redirect_to("/#settings"); });
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

// Home screen icon for iOS, which asks for this path on its own as well. It rarely changes, so it
// may be cached for a day.
void get_touch_icon() {
    web::webserver.sendHeader("Cache-Control", "max-age=86400");
    web::webserver.send_P(200, "image/png", (PGM_P)WEB_TOUCH_ICON, WEB_TOUCH_ICON_LEN);
}

// Old page urls redirect into the web app, so bookmarks keep working
void redirect_to(const char* location) {
    web::webserver.sendHeader("Location", location);
    web::webserver.send(302, "text/plain", "");
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
