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

    int buzzer                 = 32;
    
    int battery_i2c_sda        = 21;
    int battery_i2c_scl        = 22;

    int hspi_probes_sclk 	   = 14;
    int hspi_probes_cs 	       = 15;
    int hspi_probes_miso 	   = 12;

    int mux_selector_a         = 33;
    int mux_selector_b         = 25;
    int mux_selector_c         = 26;
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
    String grill_firmware_version       = "25.04.06";

    // ***********************************
    // * Timezone / NTP
    // ***********************************

    String timezone                     = "Europe/Brussels";
    String ntp_server_1                 = "ptbtime1.ptb.de";
    String ntp_server_2                 = "time-a-wwv.nist.gov";
    String ntp_server_3                 = "ntp.nict.jp";

    // ***********************************
    // * Wifi
    // * default declared in grillconfig.cpp
    // ***********************************

    String wifi_ssid                    = "";
    String wifi_password                = "";

    String wifi_ip                      = "";
    String wifi_subnet                  = "";
    String wifi_gateway                 = "";
    String wifi_dns                     = "";

    // Local AP
    String local_ap_ssid_prefix         = "FreeGrilly";
    String local_ap_ssid                = "";
    String local_ap_password            = "";

    String local_ap_ip                  = "";
    String local_ap_subnet              = "";
    String local_ap_gateway             = "";

    // ***********************************
    // * Probes
    // ***********************************
    int hspi_probes_clockspeed 	   =  16000000;
}

namespace grill{
    
    // ***********************************
    // * Wifi
    // ***********************************

    bool wifi_connected                 = false;
    int wifi_signal                     = -99;

    // ***********************************
    // * Battery
    // ***********************************

    int battery_percentage              = 0;
    bool battery_charging               = false;

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

namespace web{
    // ***********************************
    // * Api / Webserver
    // ***********************************
    
    WebServer webserver(80);
}

namespace task{
    TaskHandle_t batteryTask;
    TaskHandle_t powerbuttonTask;
    TaskHandle_t probesTask;
    TaskHandle_t screenTask;
    TaskHandle_t webserverTask;
    TaskHandle_t stackmonitorTask;
    
    int batteryStackSize      = 10000;
    int powerbuttonStackSize  = 10000;
    int probesStackSize       = 10000;
    int screenStackSize       = 10000;
    int webserverStackSize    = 10000;
    int stackmonitorStackSize = 10000;
}