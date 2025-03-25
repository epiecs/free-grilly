#include <string>

// Power button
#define BTN_PWR 35
#define PWR_PRB 27

// Screen LED
#define SCRN_LED 4

// I2C - Battery
#define I2C_SDA 21 // battery
#define I2C_SCL 22 // battery

// HSPI - Probes
extern const int HSPI_SPD 	=		  16000000;		// HSPI clock speed
extern const int HSPI_SCLK 	=				14;		// HSPI clock pin
extern const int HSPI_CS 	=				15;		// HSPI chip Select pin
extern const int HSPI_MISO 	=				12;		// HSPI data pin


// Timezone + NTP server
std::string timezone        = "Europe/Brussels";
std::string ntp_server_1    = "ptbtime1.ptb.de";
std::string ntp_server_2    = "time-a-wwv.nist.gov";
std::string ntp_server_3    = "ntp.nict.jp";

// SSID to connect to
std::string wifi_ssid = "WOZ";
std::string wifi_password = "schotelantenne";

bool wifi_static_ip = true;
IPAddress wifi_ip(10, 30, 10, 235);
IPAddress wifi_subnet(255, 255, 255, 0);
IPAddress wifi_gateway(10, 30, 10, 1);
IPAddress wifi_dns(10, 30, 50, 254);
IPAddress wifi_dns2(0, 0, 0, 0);

// Local AP
std::string local_ap_ssid_prefix = "FreeGrilly";
std::string local_ap_password = "";