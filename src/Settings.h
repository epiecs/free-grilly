#include <string>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

#include "Probe.h"
#include "Buzzer.h"
#include "Mqtt.h"
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
    // * Button
    // ***********************************

    int press_seconds_startup       = 2;
    int press_seconds_shutdown      = 2;
    int press_seconds_factory_reset = 10;

    // ***********************************
    // * Grill
    // ***********************************

    String grill_name                   = "";
    String grill_uuid                   = "";
    String grill_firmware_version       = "25.06.01";

    String temperature_unit             = "celcius";
    bool beep_enabled                   = true;
    bool beep_on_ready                  = true;
    bool beep_outside_target            = true;
    int  beep_volume                    = 5;
    int  beep_degrees_before            = 5;

    // ***********************************
    // * Buffers
    // ***********************************

    int json_buffer_size                = 2000;
    int mqtt_buffer_size                = 2000;

    // ***********************************
    // * Mqtt
    // ***********************************
    
    WiFiClient WifiClient;
    Mqtt mqtt_client                    = Mqtt(WifiClient);

    String mqtt_broker                  = "";
    int    mqtt_port                    = 1883;
    String mqtt_topic                   = "free-grilly";
    
    // ***********************************
    // * Json Utilities
    // ***********************************
    JsonUtilities json_handler;

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
    int hspi_probes_clockspeed 	        =  16000000;

    // ***********************************
    // * Buzzer
    // ***********************************

    bool alarm_mute                     = false;
    int alarm_beep_amount               = 20;
    int alarm_beep_duration_ms          = 800;
}

namespace grill{
    
    // ***********************************
    // * Wifi
    // ***********************************

    bool wifi_connected                 = false;
    int wifi_signal                     = -99;
    bool internet_connectivity          = false;

    // ***********************************
    // * Battery
    // ***********************************

    int battery_percentage              = 0;
    bool battery_charging               = false;

    // ***********************************
    // * Buzzer
    // ***********************************

    Buzzer buzzer;    

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
    TaskHandle_t alarmTask;
    TaskHandle_t batteryTask;
    TaskHandle_t mqttTask;
    TaskHandle_t powerbuttonTask;
    TaskHandle_t probesTask;
    TaskHandle_t screenTask;
    TaskHandle_t webserverTask;
    TaskHandle_t stackmonitorTask;
    
    int alarmStackSize        = 1000;
    int batteryStackSize      = 2000;
    int mqttStackSize         = 8000;
    int powerbuttonStackSize  = 10000; //Needed to be able to handle factory reset
    int probesStackSize       = 1000;
    int screenStackSize       = 2000;
    int webserverStackSize    = 4000;
    int stackmonitorStackSize = 5000;
}