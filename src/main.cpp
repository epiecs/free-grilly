#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WebServer.h>
#include <Wire.h>
#include <string>

#include "Api.h"
#include "GrillConfig.h"
#include "Network.h"
#include "Power.h"
#include "Preferences.h"
#include "Util.h"
#include "Website.h"
#include "Web.h"

// ************************************
// * Config.h initializes variables
// ************************************
#include "Settings.h"

//* Core task handlers
TaskHandle_t taskCore0;
TaskHandle_t taskCore1;

void core_0_code(void* pvParameters);
void core_1_code(void* pvParameters);

void setup() {
    // ***********************************
    // * Load nvram settings and init
    // ***********************************
    config::settings_storage.begin("free-grilly", false);
    config::config_helper.initialize_settings();

    // ***********************************
    // * Serial
    // ***********************************
    
    Serial.begin(115200); // Initialize serial communication at 115200 bits per second
    delay(5000);          // Give serial monitor time to catch up

    // ***********************************
    // * SPI for probes
    // ***********************************

    SPISettings spiSettings(config::hspi_probes_clockspeed, MSBFIRST, SPI_MODE0);
    SPI.begin(gpio::hspi_probes_sclk, gpio::hspi_probes_miso, -1, gpio::hspi_probes_cs);
    SPI.beginTransaction(spiSettings);
    
    pinMode(gpio::hspi_probes_cs, OUTPUT); // Prep CS line for data reading

    // ***********************************
    // * NTP
    // ***********************************

    // configTzTime(get_timezone_code(timezone).c_str(), 
    //     ntp_server_1.c_str(),
    //     ntp_server_2.c_str(),
    //     ntp_server_3.c_str()
    // );

    // ***********************************
    // * WIFI
    // ***********************************

    // Event handlers
    WiFi.onEvent(event_wifi_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(event_wifi_ip_acquired, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(event_wifi_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.disconnect();      // Remove stale settings
    WiFi.mode(WIFI_AP_STA); // AP + STATION
    WiFi.setSleep(false);   // Disable wifi powersaving for a more
                            // stable connection and lower latency

    config::local_ap_ssid =  generate_hostname(config::local_ap_ssid_prefix);
    start_local_ap(config::local_ap_ssid, config::local_ap_password);
    delay(1000);

    connect_to_wifi(config::wifi_ssid, config::wifi_password);

    // ***********************************
    // * Api + Web
    // ***********************************

    setup_api_routes();
    setup_web_routes();

    web::webserver.enableCORS();
    web::webserver.onNotFound(not_found);
    web::webserver.begin();

    // ***********************************
    // * Power
    // ***********************************

    battery.init();
    power.startup();

    xTaskCreatePinnedToCore(core_0_code, "Core0", 10000, NULL, 1, &taskCore0, 0);
    xTaskCreatePinnedToCore(core_1_code, "Core1", 10000, NULL, 1, &taskCore1, 1);
}

void core_0_code(void* pvParameters) {
    Serial.print("Launching tasks for Core: ");
    Serial.println(xPortGetCoreID());

    //* WIFI processes always run on core 0 so we do the same with the 
    //* webserver/api

    //* Loop
    for (;;) {
        web::webserver.handleClient();
    }
}

bool is_button_pressed = false;

void core_1_code(void* pvParameters) {

    Serial.print("Launching tasks for Core: ");
    Serial.println(xPortGetCoreID());

    // ***********************************
    // * Probes
    // ***********************************

    digitalWrite(gpio::hspi_probes_cs, HIGH);

    // power button
    pinMode(gpio::power_button, INPUT);

    pinMode(4, OUTPUT); // SCREEN LED!! VT5
    digitalWrite(4, LOW);

    pinMode(32, OUTPUT); // BUZZZZZER!! VT5

    /*SPI.begin(ADS_SCLK, ADS_MISO, -1, ADS_CS);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));*/

    //* Loop
    for (;;) {
        //* Probe code
        millis_core1_current = millis();  
        if (millis_core1_current - millis_probe_start >= millis_probe_period) {
            Serial.print("1: ");
            Serial.print(grill::probe_1.calculate_temperature());
            Serial.print(" -- 2: ");
            Serial.print(grill::probe_2.calculate_temperature());
            Serial.print(" -- 3: ");
            Serial.print(grill::probe_3.calculate_temperature());
            Serial.print(" -- 4: ");
            Serial.print(grill::probe_4.calculate_temperature());
            Serial.print(" -- 5: ");
            Serial.print(grill::probe_5.calculate_temperature());
            Serial.print(" -- 6: ");
            Serial.print(grill::probe_6.calculate_temperature());
            Serial.print(" -- 7: ");
            Serial.print(grill::probe_7.calculate_temperature());
            Serial.print(" -- 8: ");
            Serial.print(grill::probe_8.calculate_temperature());
    
            Serial.println(" ");
            millis_probe_start = millis_core1_current; 
        } 

        //* Button code 
        if(digitalRead(gpio::power_button) == LOW && not is_button_pressed) {
            is_button_pressed = true;
            millis_button_start = millis_core1_current;
        }
        else if (digitalRead(gpio::power_button) == HIGH && is_button_pressed)
        {
            is_button_pressed = false;

            if(millis_core1_current - millis_button_start < 1000) {
                Serial.println("Button pressed for less than 1 second");
            }
            else if (millis_core1_current - millis_button_start < 10000) {
                Serial.println("Button pressed for less than 10 seconds - Shutting down");
                power.shutdown();
            }
            else if (millis_core1_current - millis_button_start > 10000) {
                Serial.println("Button pressed for more than 10 seconds");
            }
            
        }
        
        //* Battery code 
        if (millis_core1_current - millis_battery_start >= millis_battery_period) {
            battery.read_battery();
            Serial.print("SOC: ");
            Serial.print(grill::battery_percentage);
            Serial.print(" -- is charging?: ");
            Serial.print(grill::battery_charging);
            Serial.println(" ");
            millis_battery_start = millis_core1_current; 
        } 
    }
}

void loop() { }