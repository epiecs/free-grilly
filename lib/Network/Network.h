#include <WiFi.h>

void start_local_ap(
    String ssid,
    String password = "",
    String ip = "0.0.0.0",
    String subnet = "0.0.0.0",
    String gateway = "0.0.0.0"
);

bool connect_to_wifi(
    String ssid,
    String password = "",
    String ip = "0.0.0.0",
    String subnet = "0.0.0.0",
    String gateway = "0.0.0.0",
    String dns = "0.0.0.0"
);

void print_wifi_connection();

void event_wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_ip_acquired(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);

String get_wifi_error_status(int statuscode);
String get_wifi_connection_status(int statuscode);