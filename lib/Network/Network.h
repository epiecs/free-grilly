#include <WiFi.h>

void start_local_ap(
    String ssid,
    String password = "",
    IPAddress ip = IPAddress(192, 168, 200, 10),
    IPAddress subnet = IPAddress(255, 255, 255, 0),
    IPAddress gateway = IPAddress(192, 168, 200, 10));

bool connect_to_wifi(
    String ssid,
    String password = "",
    bool static_ip = false,
    IPAddress ip = IPAddress(0, 0, 0, 0),
    IPAddress subnet = IPAddress(0, 0, 0, 0),
    IPAddress gateway = IPAddress(0, 0, 0, 0),
    IPAddress dns1 = IPAddress(0, 0, 0, 0),
    IPAddress dns2 = IPAddress(0, 0, 0, 0));

void print_wifi_connection();

void event_wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_ip_acquired(WiFiEvent_t event, WiFiEventInfo_t info);
void event_wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);

String get_wifi_error_status(int statuscode);
String get_wifi_connection_status(int statuscode);