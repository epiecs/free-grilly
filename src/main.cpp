#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WebServer.h>
#include <Wire.h>

#include <Api.h>
#include <Network.h>
#include <Probes.h>
#include <Power.h>
#include <Preferences.h>
#include <Util.h>
#include <Website.h>

#include "constants.h"

#define GRILLEYE grilleye

// Core task handlers
TaskHandle_t taskCore0;
TaskHandle_t taskCore1;

// SPIClass ads8865_spi(HSPI);
int16_t read_val;
int8_t dummy;

// For tcp connections
WiFiClient localClient;

// Api/web server
WebServer webserver(80);

void core_0_code(void *pvParameters);
void core_1_code(void *pvParameters);


void setup()
{
    
    Serial.begin(115200); // Initialize serial communication at 115200 bits per second
    delay(5000);          // Give serial monitor time to catch up

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
    // WiFi.onEvent(wifi_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    // WiFi.onEvent(wifi_ip_acquired, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    // WiFi.onEvent(wifi_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    // WiFi.disconnect();      // Remove stale settings
    // WiFi.mode(WIFI_AP_STA); // AP + STATION
    // WiFi.setSleep(false);   // Disable wifi powersaving for a more
    //                         // stable connection and lower latency
    
    // std::string local_ssid =  generate_hostname(local_ap_ssid_prefix);
    // start_local_ap(local_ssid, local_ap_password);
    // delay(1000);
    
    // connect_to_wifi(wifi_ssid, wifi_password);
    
    // ***********************************
    // * Api + Web
    // ***********************************

    // setup_api_routes();
    // setup_web_routes();

    // webserver.enableCORS();
    // webserver.onNotFound(not_found);
    // webserver.begin();

    // https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
    xTaskCreatePinnedToCore(core_0_code, "Core0", 10000, NULL, 1, &taskCore0, 0);
    if (GRILLEYE == true)
    {
        xTaskCreatePinnedToCore(core_1_code, "Core1", 10000, NULL, 1, &taskCore1, 1);
    }
}

void core_0_code(void *pvParameters)
{
    Serial.print("Launching tasks for Core: ");
    Serial.println(xPortGetCoreID());

    //* WIFI processes always run on core 0 so we do the same with the webserver/api

    //* Loop
    for (;;)
    {
        webserver.handleClient();
    }
}

void core_1_code(void *pvParameters)
{

    Serial.print("Launching tasks for Core: ");
    Serial.println(xPortGetCoreID());

    // ***********************************
    // * Probes
    // ***********************************

    pinMode(ADS_CS, OUTPUT);
    digitalWrite(ADS_CS, HIGH);

    // power button
    pinMode(BTN_PWR, INPUT);
    pinMode(PWR_PRB, OUTPUT); // 3.3v line probes!! VT1 VT2

    pinMode(13, OUTPUT); // 3.3v line ICs?
    digitalWrite(13, LOW);

    pinMode(4, OUTPUT); // SCREEN LED!! VT5
    digitalWrite(4, LOW);

    pinMode(32, OUTPUT); // BUZZZZZER!! VT5

    digitalWrite(PWR_PRB, LOW);

    probes.init();
    probes.selectProbe(8);
    /*SPI.begin(ADS_SCLK, ADS_MISO, -1, ADS_CS);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));*/

    //* Loop
    for (;;)
    {
        uint16_t rawValue = probes.spiReadProbe();

        // uint8_t lowByte = rawValue & 0xFF;
        // uint8_t highByte = (rawValue >> 8) & 0xFF;
        // Serial.print("High Byte: ");
        // Serial.print(highByte, BIN);
        // Serial.print(", Low Byte: ");
        // Serial.print(lowByte, BIN);
        std::string printval = "";

        printval = byte_with_leading_0(rawValue, 4);
        Serial.print(" --- ");
        Serial.print(printval.c_str());
        Serial.print(" ");
        Serial.print(rawValue);
        Serial.print(" --- ");
        Serial.print(probes.adc_value_to_voltage(rawValue), 5);
        Serial.print(" V --- ");
        Serial.print(probes.calculate_temperature(rawValue), 1);
        Serial.println(" C");
        delay(400);
    }
}

void loop()
{
    // Does nothing
}