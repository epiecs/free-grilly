
#include <functional>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Config.h"
#include "Grill.h"
#include "JsonUtilities.h"
#include "Mqtt.h"

// Set this to config::json_buffer_size, cant do this dynamically
char mqtt_json_buffer[3000];

void Mqtt::setup(String mqtt_broker, int mqtt_port){

    Mqtt::client_name            = "free-grilly-" + config::grill_uuid;
    String topic_prefix          = config::mqtt_topic + "/" + config::grill_uuid;

    Mqtt::pub_topic_grill        = topic_prefix + "/grill" ;
    Mqtt::pub_topic_settings     = topic_prefix + "/settings";
    Mqtt::pub_topic_probes       = topic_prefix + "/probes";

    Mqtt::sub_topic_settings     = topic_prefix + "/config/settings";
    Mqtt::sub_topic_probes       = topic_prefix + "/config/probes";

    Mqtt::setServer(mqtt_broker.c_str(), mqtt_port);
    Mqtt::setBufferSize(config::mqtt_buffer_size);

    // Needed because otherwise we'd have to use static members
    // https://blog.mbedded.ninja/programming/languages/c-plus-plus/callbacks/#stdfunction-with-stdbind
    Mqtt::setCallback(std::bind(&Mqtt::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Mqtt::reconnect();
}

void Mqtt::publish_grill(){
    config::json_handler.load_json_status(mqtt_json_buffer);
    Mqtt::publish(Mqtt::pub_topic_grill.c_str(), mqtt_json_buffer);
}

void Mqtt::publish_probes(){
    config::json_handler.load_json_probes(mqtt_json_buffer);
    Mqtt::publish(Mqtt::pub_topic_probes.c_str(), mqtt_json_buffer, true);
}

void Mqtt::publish_settings(){
    config::json_handler.load_json_settings(mqtt_json_buffer);
    Mqtt::publish(Mqtt::pub_topic_settings.c_str(), mqtt_json_buffer, true);
}

void Mqtt::receive_callback(char* topic, byte* payload, unsigned int length){
    
    Serial.printf("MQTT Message arrived on [%s] ", topic);

    for (unsigned int i = 0; i < length; i++){
        // For debugging
        // Serial.print((char)payload[i]);
        mqtt_json_buffer[i] = (char)payload[i]; 
    }
    Serial.println();
  
    if (String(topic) == Mqtt::sub_topic_probes){
        jsonResult result = config::json_handler.save_json_probes(mqtt_json_buffer);

        //Wipe the retained message, unsub and sub again to not trigger an echo loop
        Mqtt::unsubscribe(Mqtt::sub_topic_probes.c_str());
        Mqtt::publish(Mqtt::sub_topic_probes.c_str(), nullptr, 0, true);
        Mqtt::subscribe(Mqtt::sub_topic_probes.c_str());
    }
 
    if (String(topic) == Mqtt::sub_topic_settings){
        jsonResult result = config::json_handler.save_json_settings(mqtt_json_buffer);
        
        //Wipe the retained message, unsub and sub again to not trigger an echo loop
        Mqtt::unsubscribe(Mqtt::sub_topic_settings.c_str());
        Mqtt::publish(Mqtt::sub_topic_settings.c_str(), nullptr, 0, true);
        Mqtt::subscribe(Mqtt::sub_topic_settings.c_str());
    }
}

bool Mqtt::reconnect(){
    while (!Mqtt::connected()) {
        Serial.println("Trying to reconnect to MQTT server");
        
        if(!grill::wifi_connected){
            delay(5000);
            continue;
        }

        Mqtt::setServer(config::mqtt_broker.c_str(), config::mqtt_port);

        if(config::mqtt_user != "" && config::mqtt_password != ""){
            Serial.println("Trying to connect to MQTT using user/pass");
            if(!Mqtt::connect(Mqtt::client_name.c_str(), config::mqtt_user.c_str(), config::mqtt_password.c_str())){
                Serial.print("MQTT Connection failed, rc= ");
                Serial.print(Mqtt::state());
                Serial.println("Waiting 5 seconds to retry");
                delay(5000);
                continue;
            }
        } else {
            Serial.println("Trying to connect to MQTT without authentication");
            if(!Mqtt::connect(Mqtt::client_name.c_str())){
                Serial.print("MQTT Connection failed, rc= ");
                Serial.print(Mqtt::state());
                Serial.println("Waiting 5 seconds to retry");
                delay(5000);
                continue;
            }
        }

        Serial.print("MQTT Connected to server with client ");
        Serial.println(Mqtt::client_name);
        
        Mqtt::subscribe(Mqtt::sub_topic_settings.c_str());
        Mqtt::subscribe(Mqtt::sub_topic_probes.c_str());

        Mqtt::publish_grill();
        Mqtt::publish_probes();
        Mqtt::publish_settings();
    }
    
    return true;
}