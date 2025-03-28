#include <string>
#include <WiFi.h>
#include <WebServer.h>

#include "Probe.h"

// Power button
#define BTN_PWR 35
#define PWR_PRB 27

// Screen LED
#define SCRN_LED 4

// I2C - Battery
#define I2C_SDA 21 // battery
#define I2C_SCL 22 // battery

// HSPI - Probes
extern const int HSPI_SPD 	        =  16000000;		// HSPI clock speed
extern const int HSPI_SCLK 	        =        14;		// HSPI clock pin
extern const int HSPI_CS 	        =        15;		// HSPI chip Select pin
extern const int HSPI_MISO 	        =        12;		// HSPI data pin


// ***********************************
// * Timers
// ***********************************

unsigned long millis_core1_current;
unsigned long millis_probe_start;    
unsigned long millis_button_start;        
unsigned long millis_battery_start;     
const unsigned long millis_probe_period = 1000;     // Probe read interval
const unsigned long millis_battery_period = 10000;   // Battery read interval

// ***********************************
// * Grill
// ***********************************

std::string grill_name              = "Free-Grilly";
std::string grill_uuid              = "12abc3de-4567-89f0-a123-456b789012c3";
std::string grill_firmware_version  = "25.03.01";

// ***********************************
// * Battery
// ***********************************

int battery_percentage              = 0;
bool battery_charging               = false;

// ***********************************
// * Timezone / NTP
// ***********************************

std::string timezone                = "Europe/Brussels";
std::string ntp_server_1            = "ptbtime1.ptb.de";
std::string ntp_server_2            = "time-a-wwv.nist.gov";
std::string ntp_server_3            = "ntp.nict.jp";

// ***********************************
// * Wifi
// ***********************************

bool wifi_connected                 = false;
int wifi_signal                     = -99;

std::string wifi_ssid               = "";
std::string wifi_password           = "";

bool wifi_static_ip                 = true;
IPAddress wifi_ip(10, 30, 10, 235);
IPAddress wifi_subnet(255, 255, 255, 0);
IPAddress wifi_gateway(10, 30, 10, 1);
IPAddress wifi_dns(10, 30, 50, 254);
IPAddress wifi_dns2(0, 0, 0, 0);

// Local AP
std::string local_ap_ssid_prefix    = "FreeGrilly";
std::string local_ap_password       = "";

// ***********************************
// * Api / Webserver
// ***********************************

WebServer webserver(80);

// ***********************************
// * Probes
// ***********************************

Probe probe_1 = Probe(1);
Probe probe_2 = Probe(2);
Probe probe_3 = Probe(3);
Probe probe_4 = Probe(4);
Probe probe_5 = Probe(5);
Probe probe_6 = Probe(6);
Probe probe_7 = Probe(7);
Probe probe_8 = Probe(8);