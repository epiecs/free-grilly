#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WebServer.h>
#include <Wire.h>
#include <ElegantOTA.h>
#include <string>

#include "Api.h"
#include "Buzzer.h"
#include "GrillConfig.h"
#include "JsonUtilities.h"
#include "Mqtt.h"
#include "Network.h"
#include "Opengrill.h"
#include "Power.h"
#include "Preferences.h"
#include "Website.h"
#include "Web.h"
#include "Display.h"

#include "esp_heap_caps.h"

// ************************************
// * Config.h initializes variables
// ************************************
#include "Settings.h"

// Task functions
void task_alarm(void* pvParameters);
void task_battery(void* pvParameters);
void task_opengrill(void* pvParameters);
void task_mqtt(void* pvParameters);
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
    // * Power button bootup
    // ***********************************

    //* Power button pin is set here so that we can use it to check for boot
    pinMode(gpio::power_button, INPUT);

    unsigned long millis_pressed        = 0;
    unsigned long millis_button_start   = 0;

    int bootup_press_time   = config::press_seconds_startup * 1000;

    if(digitalRead(gpio::power_button) == LOW){
        millis_button_start = millis();
    }

    while(digitalRead(gpio::power_button) == LOW){
        millis_pressed = millis() - millis_button_start;

        // beep if the button is held long enough
        if(millis_pressed > bootup_press_time){
            grill::buzzer.beep(1, 200);
            break;
        }
    }

    if(millis_pressed < bootup_press_time){
        power.shutdown();
    }

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
    // * Launch DEVICE tasks
    // ***********************************

    delay(100); //Needed to give the power rail time to adjust
    xTaskCreatePinnedToCore(task_battery, "Battery", task::batteryStackSize, NULL, 1, &task::batteryTask, 1);
    delay(100); //Needed to give the power rail time to adjust
    xTaskCreatePinnedToCore(task_screen, "Screen", task::screenStackSize, NULL, 1, &task::screenTask, 1);
    delay(1000); //Needed to give the power rail time to adjust
    xTaskCreatePinnedToCore(task_powerbutton, "PowerButton", task::powerbuttonStackSize, NULL, 1, &task::powerbuttonTask, 1);
    delay(100); //Needed to give the power rail time to adjust
    xTaskCreatePinnedToCore(task_probes, "Probes", task::probesStackSize, NULL, 1, &task::probesTask, 1);
    xTaskCreatePinnedToCore(task_alarm, "Alarm", task::alarmStackSize, NULL, 1, &task::alarmTask, 1);

    // ***********************************
    // * WIFI
    // ***********************************

    // Event handlers
    WiFi.onEvent(event_wifi_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(event_wifi_ip_acquired, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(event_wifi_disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.disconnect(true);  // Remove stale settings
    delay(100);             // Delay for stability
    WiFi.mode(WIFI_AP_STA); // AP + STATION
    WiFi.setSleep(false);   // Disable wifi powersaving for a more
                            // stable connection and lower latency

    start_local_ap();
    delay(1000);            //Needed to give the power rail time to adjust

    if(config::wifi_ssid != ""){
        Serial.println("Waiting a bit before STA connect...");
        delay(800);         // Extra breathing room for the power rail on cold boot
        connect_to_wifi();
    }

    // ***********************************
    // * Launch NETWORK tasks
    // ***********************************
    xTaskCreatePinnedToCore(task_webserver, "Webserver", task::webserverStackSize, NULL, 1, &task::webserverTask, 1);
    xTaskCreatePinnedToCore(task_mqtt, "Mqtt", task::mqttStackSize, NULL, 1, &task::mqttTask, 1);
    xTaskCreatePinnedToCore(task_opengrill, "Opengrill", task::opengrillStackSize, NULL, 1, &task::opengrillTask, 1);
    xTaskCreatePinnedToCore(task_stackmonitor, "StackMonitor", task::stackmonitorStackSize, NULL, 1, &task::stackmonitorTask, 1);
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

    ElegantOTA.begin(&web::webserver); // OTA webserver

    while (true){
        web::webserver.handleClient();
        ElegantOTA.loop();
        delay(1);
    }
}

// ***********************************
// * Opengrill
// ***********************************

void task_opengrill(void* pvParameters) {
    Serial.println("Launching task :: Opengrill");
    delay(5);   //Give FreeRtos a chance to properly schedule the task

    String opengrill_server = "";
    int opengrill_port = 1883;

    unsigned long last_publish_time = 0;
    const unsigned long publish_interval_ms = 1000;

    while (true){

        if(opengrill_server != config::opengrill_server || opengrill_port != config::opengrill_port){
            Serial.println("(Re)loaded Opengrill Settings");
            // Settings have changed. We have to update our vars and set a new client
            // This can also be used for launching since we initialize empty vars - also i'm lazy
            opengrill_server = config::opengrill_server;
            opengrill_port = config::opengrill_port;

            config::opengrill_client.setup(opengrill_server, opengrill_port);

            // Only loop/reconnect if we have a broker filled in
            if(opengrill_server != ""){
                Serial.println("Opengrill server set, initializing connection");
                config::opengrill_client.reconnect();
            } else {
                Serial.println("Opengrill server not set, skipping Opengrill connection");
            }
        }

        if(opengrill_server != "" && config::opengrill_client.connected()){
            config::opengrill_client.loop();

            unsigned long now = millis();
            if (now - last_publish_time >= publish_interval_ms) {
                last_publish_time = now;
                config::opengrill_client.publish_grill();
            }
        }

        if(opengrill_server != "" && !config::opengrill_client.connected() && grill::wifi_connected){
            Serial.println("Opengrill client disconnected, trying to reconnect");
            config::opengrill_client.reconnect();
        }

        delay(50);
    }
}

// ***********************************
// * MQTT
// ***********************************

void task_mqtt(void* pvParameters) {
    Serial.println("Launching task :: MQTT");
    delay(5);   //Give FreeRtos a chance to properly schedule the task

    String mqtt_broker = "";
    int mqtt_port = 1883;

    unsigned long last_mqtt_publish_time = 0;
    const unsigned long mqtt_publish_interval_ms = 1000;

    while (true){

        if(mqtt_broker != config::mqtt_broker || mqtt_port != config::mqtt_port){
            Serial.println("(Re)loaded MQTT Settings");
            // Settings have changed. We have to update our vars and set a new client
            // This can also be used for launching since we initialize empty vars - also i'm lazy
            mqtt_broker = config::mqtt_broker;
            mqtt_port = config::mqtt_port;

            config::mqtt_client.setup(mqtt_broker, mqtt_port);

            // Only loop/reconnect if we have a broker filled in
            if(mqtt_broker != ""){
                Serial.println("MQTT broker set, initializing connection");
                config::mqtt_client.reconnect();
            } else {
                Serial.println("MQTT broker not set, skipping MQTT connection");
            }
        }

        if(mqtt_broker != "" && config::mqtt_client.connected()){
            config::mqtt_client.loop();

            unsigned long now = millis();
            if (now - last_mqtt_publish_time >= mqtt_publish_interval_ms) {
                last_mqtt_publish_time = now;
                config::mqtt_client.publish_grill();
            }
        }

        if(mqtt_broker != "" && !config::mqtt_client.connected() && grill::wifi_connected){
            Serial.println("MQTT client disconnected, trying to reconnect");
            config::mqtt_client.reconnect();
        }

        delay(50);
    }
}

// ***********************************
// * Buzzer alarm
// ***********************************

void task_alarm(void* pvParameters) {
    Serial.println("Launching task :: Alarm");
    delay(5);   //Give FreeRtos a chance to properly schedule the task

    int alarm_beep_todo = 0;  // The amount of beeps remaining when sounding the alarm

    while (true){

        int alarm = 0;  // Counter for easy checks to see if there is an alarm

        //* Check for alarms
        if(grill::probe_1.alarm){ alarm++; };
        if(grill::probe_2.alarm){ alarm++; };
        if(grill::probe_3.alarm){ alarm++; };
        if(grill::probe_4.alarm){ alarm++; };
        if(grill::probe_5.alarm){ alarm++; };
        if(grill::probe_6.alarm){ alarm++; };
        if(grill::probe_7.alarm){ alarm++; };
        if(grill::probe_8.alarm){ alarm++; };

        //* Trigger alarms if needed
        if(alarm > 0 && alarm_beep_todo == 0){
            alarm_beep_todo = config::alarm_beep_amount;
        }

        //* Mute alarms if needed
        if(config::alarm_mute == true){
            // When we need to mute we remove all needed alarms and wait
            // for 2 seconds for the probes and other devices to catch up
            delay(2000);

            alarm_beep_todo = 0;
            config::alarm_mute = false;
        }

        if(alarm_beep_todo > 0){
            alarm_beep_todo--;
            if(config::cucaracha_enabled)
                grill::buzzer.play_cucaracha();
            else
                grill::buzzer.beep(1, config::alarm_beep_duration_ms);
        }

        delay(100);
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
    int medium_press_time  = config::press_seconds_shutdown * 1000;
    int long_press_time    = config::press_seconds_factory_reset * 1000;

    bool button_pressed    = false;
    bool buzzed_short      = false;
    bool buzzed_medium     = false;
    bool buzzed_long       = false;

    if(digitalRead(gpio::power_button) == LOW){
        millis_button_start = millis();
    }

    while(true){
        if(digitalRead(gpio::power_button) == LOW && not button_pressed) {
            // Initialize millis counter
            button_pressed = true;
            millis_button_start = millis();
        } else if(digitalRead(gpio::power_button) == LOW){
            millis_pressed = millis() - millis_button_start;

            // beep if the button is held long enough to indicate the action
            if(millis_pressed > short_press_time && buzzed_short == false){
                buzzed_short = true;
                grill::buzzer.beep(2, 100);
            }
            if(millis_pressed > medium_press_time && buzzed_medium == false){
                buzzed_medium = true;
                grill::buzzer.beep(3, 100);
            }
            if(millis_pressed > long_press_time && buzzed_long == false){
                buzzed_long = true;
                grill::buzzer.beep(3, 500);
            }
        }
        else if (digitalRead(gpio::power_button) == HIGH && button_pressed)
        {
            button_pressed    = false;
            buzzed_short      = false;
            buzzed_medium     = false;
            buzzed_long       = false;

            millis_pressed = millis() - millis_button_start;

            // Serial.print("Button pressed for: ");
            // Serial.println(millis_pressed);

            if(millis_pressed < short_press_time) {
                Serial.println("Button pressed for less than 1 second");

                // Interrupts all running beeps
                config::alarm_mute = true;
                grill::buzzer.beep(1, 100);

                display.switch_page();
            }
            else if (millis_pressed < medium_press_time) {
                Serial.println("Button pressed 1-3 seconds");
                display.show_settings_page();
            }
            else if (millis_pressed < long_press_time) {
                Serial.println("Button pressed 3-10 seconds");
                power.shutdown();
            }
            else if (millis_pressed > long_press_time) {
                Serial.println("Button pressed for more than 10 seconds");
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
        // Read probes and also check if beeps/alarms/.. are needed
        grill::probe_1.calculate_temperature(); grill::probe_1.check_temperature_status();
        grill::probe_2.calculate_temperature(); grill::probe_2.check_temperature_status();
        grill::probe_3.calculate_temperature(); grill::probe_3.check_temperature_status();
        grill::probe_4.calculate_temperature(); grill::probe_4.check_temperature_status();
        grill::probe_5.calculate_temperature(); grill::probe_5.check_temperature_status();
        grill::probe_6.calculate_temperature(); grill::probe_6.check_temperature_status();
        grill::probe_7.calculate_temperature(); grill::probe_7.check_temperature_status();
        grill::probe_8.calculate_temperature(); grill::probe_8.check_temperature_status();

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

    //pinMode(gpio::power_screen_backlight, OUTPUT);
    //digitalWrite(gpio::power_screen_backlight, HIGH);

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

void check_heap() {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_8BIT);

    size_t used  = info.total_allocated_bytes;
    size_t free  = info.total_free_bytes;
    size_t total = used + free;

    printf("Heap: used=%u free=%u total=%u min_free=%u\n",
           used, free, total, info.minimum_free_bytes);
}

size_t get_task_stack_used(TaskHandle_t handle, size_t stackSizeBytes) {
    size_t freeBytes = uxTaskGetStackHighWaterMark(handle) * sizeof(StackType_t);
    return stackSizeBytes - freeBytes;
}

void task_stackmonitor(void* pvParameters) {
    Serial.println("Launching task :: STACK MONITOR");
    delay(5);   //Give FreeRtos a chance to properly schedule the task

    size_t used = 0;

    for (;;) {
        // https://www.freertos.org/Why-FreeRTOS/FAQs/Memory-usage-boot-times-context#how-big-should-the-stack-be

        Serial.println("|++++++++++++++ HEAP  +++++++++++++++|");
        check_heap();

        Serial.println("|++++++++++++++ STACK +++++++++++++++|");

        used = get_task_stack_used(task::alarmTask, task::alarmStackSize);
        printf("ALARM stack used: %u/%u\n", used, task::alarmStackSize);

        used = get_task_stack_used(task::batteryTask, task::batteryStackSize);
        printf("BATTERY stack used: %u/%u\n", used, task::batteryStackSize);

        used = get_task_stack_used(task::mqttTask, task::mqttStackSize);
        printf("MQTT stack used: %u/%u\n", used, task::mqttStackSize);

        used = get_task_stack_used(task::powerbuttonTask, task::powerbuttonStackSize);
        printf("POWERBUTTON stack used: %u/%u\n", used, task::powerbuttonStackSize);

        used = get_task_stack_used(task::probesTask, task::probesStackSize);
        printf("PROBES stack used: %u/%u\n", used, task::probesStackSize);

        used = get_task_stack_used(task::screenTask, task::screenStackSize);
        printf("SCREEN stack used: %u/%u\n", used, task::screenStackSize);

        used = get_task_stack_used(task::webserverTask, task::webserverStackSize);
        printf("WEBSERVER stack used: %u/%u\n", used, task::webserverStackSize);

        used = get_task_stack_used(task::stackmonitorTask, task::stackmonitorStackSize);
        printf("STACKMONITOR stack used: %u/%u\n", used, task::stackmonitorStackSize);

        delay(5000);
    }
}



void loop() {
    // To make sure that the loop idle does not block freeRTOS
    delay(10000);
}
