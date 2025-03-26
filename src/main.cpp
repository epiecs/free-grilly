#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WebServer.h>
#include <Wire.h>

#include "Api.h"
#include "Network.h"

#include "Power.h"
#include "Preferences.h"
#include "Util.h"
#include "Website.h"

#include "config.h"
#include "Global.h"

// Grill
std::string grill_name              = "Free-Grilly";
std::string grill_uuid              = "12abc3de-4567-89f0-a123-456b789012c3";
std::string grill_firmware_version  = "25.03.01";

// Battery
int battery_percent                 = 0;
bool battery_charging               = false;

// Wifi
bool wifi_connected                 = false;

// Api/web server
WebServer webserver(80);

// Probes
Probe probe_1 = Probe(1);
Probe probe_2 = Probe(2);
Probe probe_3 = Probe(3);
Probe probe_4 = Probe(4);
Probe probe_5 = Probe(5);
Probe probe_6 = Probe(6);
Probe probe_7 = Probe(7);
Probe probe_8 = Probe(8);

// timers 
unsigned long millis_core1_current;
unsigned long millis_probe_start;    
unsigned long millis_button_start;            
const unsigned long millis_probe_period = 1000;     // Probe read interval

// Core task handlers
TaskHandle_t taskCore0;
TaskHandle_t taskCore1;

void core_0_code(void* pvParameters);
void core_1_code(void* pvParameters);

void setup() {

    // ***********************************
    // * Serial
    // ***********************************
    
    Serial.begin(115200); // Initialize serial communication at 115200 bits per second
    delay(5000);          // Give serial monitor time to catch up

    // ***********************************
    // * SPI for probes
    // ***********************************

    SPISettings spiSettings(HSPI_SPD, MSBFIRST, SPI_MODE0);
    SPI.begin(HSPI_SCLK, HSPI_MISO, -1, HSPI_CS);
    SPI.beginTransaction(spiSettings);
    
    pinMode(HSPI_CS, OUTPUT); // Prep CS line for data reading

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

    std::string local_ssid =  generate_hostname(local_ap_ssid_prefix);
    start_local_ap(local_ssid, local_ap_password);
    delay(1000);

    connect_to_wifi(wifi_ssid, wifi_password);

    // ***********************************
    // * Api + Web
    // ***********************************

    setup_api_routes();
    setup_web_routes();

    webserver.enableCORS();
    webserver.onNotFound(not_found);
    webserver.begin();

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
        webserver.handleClient();
    }
}

bool is_button_pressed = false;

void core_1_code(void* pvParameters) {

    Serial.print("Launching tasks for Core: ");
    Serial.println(xPortGetCoreID());

    // ***********************************
    // * Probes
    // ***********************************

    digitalWrite(HSPI_CS, HIGH);

    // power button
    pinMode(BTN_PWR, INPUT);
    pinMode(PWR_PRB, OUTPUT); // 3.3v line probes!! VT1 VT2

    pinMode(13, OUTPUT); // 3.3v line ICs?
    digitalWrite(13, LOW);

    pinMode(4, OUTPUT); // SCREEN LED!! VT5
    digitalWrite(4, LOW);

    pinMode(32, OUTPUT); // BUZZZZZER!! VT5

    digitalWrite(PWR_PRB, LOW);

    /*SPI.begin(ADS_SCLK, ADS_MISO, -1, ADS_CS);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));*/

    //* Loop
    for (;;) {
        millis_core1_current = millis();  
        if (millis_core1_current - millis_probe_start >= millis_probe_period) {
            Serial.print("1: ");
            Serial.print(probe_1.calculate_temperature());
            Serial.print(" -- 2: ");
            Serial.print(probe_2.calculate_temperature());
            Serial.print(" -- 3: ");
            Serial.print(probe_3.calculate_temperature());
            Serial.print(" -- 4: ");
            Serial.print(probe_4.calculate_temperature());
            Serial.print(" -- 5: ");
            Serial.print(probe_5.calculate_temperature());
            Serial.print(" -- 6: ");
            Serial.print(probe_6.calculate_temperature());
            Serial.print(" -- 7: ");
            Serial.print(probe_7.calculate_temperature());
            Serial.print(" -- 8: ");
            Serial.print(probe_8.calculate_temperature());
    
            Serial.println(" ");
            millis_probe_start = millis_core1_current; 
        } 

        if(digitalRead(BTN_PWR) == LOW && not is_button_pressed) {
            is_button_pressed = true;
            millis_button_start = millis_core1_current;
        }
        else if (digitalRead(BTN_PWR) == HIGH && is_button_pressed)
        {
            is_button_pressed = false;

            if(millis_core1_current - millis_button_start < 1000) {
                Serial.println("Button pressed for less than 1 second");
            }
            else if (millis_core1_current - millis_button_start < 10000) {
                Serial.println("Button pressed for less than 10 seconds - Shutting down");
            }
            else if (millis_core1_current - millis_button_start > 10000) {
                Serial.println("Button pressed for more than 10 seconds");
            }
            
        }
        
        
    }
}

void loop() { }