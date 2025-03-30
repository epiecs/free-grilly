#include <string>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

#include "Probe.h"
#include "GrillConfig.h"

namespace gpio{

    int power_button           = 35;
    
    // enable/disable power
    int power_probes           = 27;
    int power_screen_backlight =  4;
    int power_adc_circuit      = 13;

    int battery_i2c_sda        = 21;
    int battery_i2c_scl        = 22;

    int hspi_probes_sclk 	   = 14;
    int hspi_probes_cs 	       = 15;
    int hspi_probes_miso 	   = 12;
}

namespace config{

    // ***********************************
    // * Settings - nvram storage
    // ***********************************

    Preferences settings_storage;
    GrillConfig config_helper = GrillConfig();

    // ***********************************
    // * Grill
    // ***********************************

    String grill_name                   = "";
    String grill_uuid                   = "";
    String grill_firmware_version       = "25.03.01";

    // ***********************************
    // * Timezone / NTP
    // ***********************************

    String timezone                     = "Europe/Brussels";
    String ntp_server_1                 = "ptbtime1.ptb.de";
    String ntp_server_2                 = "time-a-wwv.nist.gov";
    String ntp_server_3                 = "ntp.nict.jp";

    // ***********************************
    // * Wifi
    // ***********************************

    bool wifi_connected                 = false;
    int wifi_signal                     = -99;

    String wifi_ssid                    = "";
    String wifi_password                = "";

    bool wifi_static_ip                 = true;
    IPAddress wifi_ip(10, 30, 10, 235);
    IPAddress wifi_subnet(255, 255, 255, 0);
    IPAddress wifi_gateway(10, 30, 10, 1);
    IPAddress wifi_dns(10, 30, 50, 254);
    IPAddress wifi_dns2(0, 0, 0, 0);

    // Local AP
    String local_ap_ssid_prefix    = "FreeGrilly";
    String local_ap_ssid           = "";
    String local_ap_password       = "";

    // ***********************************
    // * Probes
    // ***********************************
    int hspi_probes_clockspeed 	   =  16000000;
}

namespace grill{
    
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
}

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
// * Battery
// ***********************************

int battery_percentage              = 0;
bool battery_charging               = false;


// ***********************************
// * Api / Webserver
// ***********************************

WebServer webserver(80);