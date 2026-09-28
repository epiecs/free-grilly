#include <WiFi.h>

void start_local_ap();

// Starts the mDNS responder under grill::hostname and advertises _http._tcp and _grilly-plus._tcp
// on port 80. Safe to call once WiFi.mode() has set up the AP/STA interfaces; the underlying
// esp-idf mdns component registers its own event handler and keeps advertising across STA
// connects/disconnects/reconnects on its own, so this only needs to run once at boot.
void start_mdns();

bool connect_to_wifi();

void print_wifi_connection();

void event_wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_ip_acquired(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);

String get_wifi_error_status(int statuscode);
String get_wifi_connection_status(int statuscode);