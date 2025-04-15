#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WebServer.h>
#include <Wire.h>
#include <string>

#include "Api.h"
#include "Buzzer.h"
#include "GrillConfig.h"
#include "Network.h"
#include "Power.h"
#include "Preferences.h"
#include "Website.h"
#include "Web.h"
#include "Display.h"

// ************************************
// * Config.h initializes variables
// ************************************
#include "Settings.h"

// Task functions
void task_battery(void* pvParameters);
void task_powerbutton(void* pvParameters);
void task_probes(void* pvParameters);
void task_screen(void* pvParameters);
void task_webserver(void* pvParameters);
void task_stackmonitor(void* pvParameters);

void setup() {
    
    // ***********************************
    // * Serial
    // ***********************************
    
    Serial.begin(115200); // Initialize serial communication at 115200 bits per second
    // delay(5000);          // Give serial monitor time to catch up
    
    // ***********************************
    // * Load nvram settings and init
    // ***********************************
    config::settings_storage.begin("free-grilly", false);
    config::config_helper.load_settings();
    config::config_helper.load_probes();

    // ***********************************
    // * Startup Buzzer
    // ***********************************
    
    grill::buzzer.beep(2, 100);

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

    start_local_ap();
    delay(1000);

    if(config::wifi_ssid != ""){
        connect_to_wifi();
    }
    
    // ***********************************
    // * Launch tasks
    // ***********************************

    // xTaskCreate(task_webserver, "Webserver", task::webserverStackSize, NULL, 1, &task::webserverTask);
    // xTaskCreate(task_powerbutton, "PowerButton", task::powerbuttonStackSize, NULL, 1, &task::powerbuttonTask);
    // xTaskCreate(task_battery, "Battery", task::batteryStackSize, NULL, 1, &task::batteryTask);
    // xTaskCreate(task_probes, "Probes", task::probesStackSize, NULL, 1, &task::probesTask);
    // xTaskCreate(task_screen, "Screen", task::screenStackSize, NULL, 1, &task::screenTask);
    // xTaskCreate(task_stackmonitor, "StackMonitor", task::stackmonitorStackSize, NULL, 1, &task::stackmonitorTask);

    xTaskCreatePinnedToCore(task_battery, "Battery", task::batteryStackSize, NULL, 1, &task::batteryTask, 1);
    xTaskCreatePinnedToCore(task_screen, "Screen", task::screenStackSize, NULL, 1, &task::screenTask, 1);
    delay(1000); //Needed to give the power rail time to adjust
    xTaskCreatePinnedToCore(task_powerbutton, "PowerButton", task::powerbuttonStackSize, NULL, 1, &task::powerbuttonTask, 1);
    xTaskCreatePinnedToCore(task_probes, "Probes", task::probesStackSize, NULL, 1, &task::probesTask, 1);
    xTaskCreatePinnedToCore(task_webserver, "Webserver", task::webserverStackSize, NULL, 1, &task::webserverTask, 1);
    // xTaskCreatePinnedToCore(task_stackmonitor, "StackMonitor", task::stackmonitorStackSize, NULL, 1, &task::stackmonitorTask, 1);
}

// ***********************************
// * TASKS
// ***********************************

// ***********************************
// * API / WEB
// ***********************************

void task_webserver(void* pvParameters) {
    Serial.println("Launching task :: WEBSERVER / API");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    setup_api_routes();
    setup_web_routes();

    web::webserver.enableCORS();
    web::webserver.onNotFound(not_found);
    web::webserver.begin();

    while (true){
        web::webserver.handleClient();
        delay(1);
    }

}
// ***********************************
// * Power Button
// ***********************************

void task_powerbutton(void* pvParameters) {
    Serial.println("Launching task :: POWER BUTTON");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    // Reference timers
    unsigned long millis_pressed        = 0;
    unsigned long millis_current        = 0;
    unsigned long millis_button_start   = 0;

    // Time in ms that defines each button press breakpoint
    int short_press_time   = 1000;
    int medium_press_time  = 5000;
    int long_press_time    = 10000;

    bool button_pressed    = false;
    pinMode(gpio::power_button, INPUT);
    
    while(true){
        if(digitalRead(gpio::power_button) == LOW && not button_pressed) {
            button_pressed = true;
            millis_button_start = millis();
        }
        else if (digitalRead(gpio::power_button) == HIGH && button_pressed)
        {
            button_pressed = false;
            millis_pressed = millis() - millis_button_start;

            Serial.print("Button pressed for: ");
            Serial.println(millis_pressed);

            if(millis_pressed < short_press_time) {
                Serial.println("Button pressed for less than 1 second");
                grill::buzzer.beep(1, 100);
                // TODO switch between wifi/probes screen
            }
            else if (millis_pressed < medium_press_time) {
                Serial.println("Button pressed 1-5 seconds");
                grill::buzzer.beep(2, 100);
                // TODO show about + help -> pressing times
            }
            else if (millis_pressed < long_press_time) {
                Serial.println("Button pressed 5-10 seconds");
                grill::buzzer.beep(3, 100);
                power.shutdown();
            }
            else if (millis_pressed > long_press_time) {
                Serial.println("Button pressed for more than 10 seconds");
                grill::buzzer.beep(3, 500);
                config::config_helper.factory_reset();
            }            
        }

        delay(100);
    }
}

// ***********************************
// * Probes
// ***********************************

void task_probes(void* pvParameters) {
    Serial.println("Launching task :: PROBES");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    pinMode(gpio::mux_selector_a, OUTPUT);
    pinMode(gpio::mux_selector_b, OUTPUT);
    pinMode(gpio::mux_selector_c, OUTPUT);

    for (;;) {
        grill::probe_1.calculate_temperature();
        grill::probe_2.calculate_temperature();
        grill::probe_3.calculate_temperature();
        grill::probe_4.calculate_temperature();
        grill::probe_5.calculate_temperature();
        grill::probe_6.calculate_temperature();
        grill::probe_7.calculate_temperature();
        grill::probe_8.calculate_temperature();

        delay(500);
    }
}

// ***********************************
// * Battery
// ***********************************

void task_battery(void* pvParameters) {
    Serial.println("Launching task :: BATTERY");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    battery.init();
    power.startup();

    for (;;) {
        battery.read_battery();

        delay(1000);
    }
}

// ***********************************
// * Screen
// ***********************************

void task_screen(void* pvParameters) {
    Serial.println("Launching task :: SCREEN");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    pinMode(gpio::power_screen_backlight, OUTPUT);
    digitalWrite(gpio::power_screen_backlight, HIGH);
    
    display.init();

    for (;;) {
        display.display_update();
        // Serial.println("screen update");
    
        delay(1000);
    }
}

// ***********************************
// * Stack monitor
// ***********************************

void task_stackmonitor(void* pvParameters) {
    Serial.println("Launching task :: STACK MONITOR");
    delay(5);   //Give FreeRtos a chance to properly schedule the task
    
    float stack_free = 0;
    float stack_used = 0;
    
    for (;;) {
         // The high water mark is the maximum value of stack that is still free
        // https://www.freertos.org/Why-FreeRTOS/FAQs/Memory-usage-boot-times-context#how-big-should-the-stack-be

        Serial.println("|++++++++++++++ STACK +++++++++++++++|");
        
        stack_free = (float)uxTaskGetStackHighWaterMark(task::webserverTask);
        stack_used = task::webserverStackSize - stack_free;
        Serial.print("WEBSERVER stack used: ");
        Serial.print(stack_used);
        Serial.print("/");
        Serial.println(task::webserverStackSize);

        stack_free = (float)uxTaskGetStackHighWaterMark(task::probesTask);
        stack_used = task::probesStackSize - stack_free;
        Serial.print("PROBES stack used: ");
        Serial.print(stack_used);
        Serial.print("/");
        Serial.println(task::probesStackSize);

        stack_free = (float)uxTaskGetStackHighWaterMark(task::screenTask);
        stack_used = task::screenStackSize - stack_free;
        Serial.print("SCREEN stack used: ");
        Serial.print(stack_used);
        Serial.print("/");
        Serial.println(task::screenStackSize);

        stack_free = (float)uxTaskGetStackHighWaterMark(task::powerbuttonTask);
        stack_used = task::powerbuttonStackSize - stack_free;
        Serial.print("POWERBUTTON stack used: ");
        Serial.print(stack_used);
        Serial.print("/");
        Serial.println(task::powerbuttonStackSize);

        stack_free = (float)uxTaskGetStackHighWaterMark(task::batteryTask);
        stack_used = task::batteryStackSize - stack_free;
        Serial.print("BATTERY stack used: ");
        Serial.print(stack_used);
        Serial.print("/");
        Serial.println(task::batteryStackSize);
        

        delay(5000);
    }
}



void loop() {
    // To make sure that the loop idle does not block freeRTOS
    delay(10000);
}