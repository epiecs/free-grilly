#include "Mqtt.h"

#include <functional>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Config.h"

JsonDocument jsondoc2;
char msgbuffer[2000];

void Mqtt::setup(String mqtt_broker, int mqtt_port){

    Mqtt::client_name        = "free-grilly-" + config::grill_uuid;
    String topic_prefix      = config::mqtt_topic + "/" + config::grill_uuid;

    Mqtt::topic_temperatures = topic_prefix + "/temperatures";
    Mqtt::topic_settings     = topic_prefix + "/settings";
    Mqtt::topic_probes       = topic_prefix + "/probes";
    Mqtt::topic_status       = topic_prefix + "/status";

    Mqtt::setServer(mqtt_broker.c_str(), mqtt_port);
    
    // Needed because otherwise we'd have to use static members
    // https://blog.mbedded.ninja/programming/languages/c-plus-plus/callbacks/#stdfunction-with-stdbind
    Mqtt::setCallback(std::bind(&Mqtt::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Mqtt::reconnect();
}

// TODO use send buffer

// TODO
void Mqtt::publish_temperatures(){
    Mqtt::publish(Mqtt::topic_temperatures.c_str(), "temperatures payload");
}

// TODO
void Mqtt::publish_status(){
    Mqtt::publish(Mqtt::topic_status.c_str(), "status payload");
}

// TODO
void Mqtt::publish_settings(){
    Mqtt::publish(Mqtt::topic_settings.c_str(), "settings payload");

}

void Mqtt::receive_callback(char* topic, byte* payload, unsigned int length) {

    String payloadBuffer;

    Serial.printf("Message arrived on [%s] ", topic);

    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        payloadBuffer += (char)payload[i]; 
    }
    Serial.println();
  
    // TODO - Optimize payloadbuffer to be statically allocated for heap optimisation
    if ((char)payload[0] == '1') {
    }

    if (String(topic) == Mqtt::topic_probes){
        Serial.println("Received probe settings");
    }
    if (String(topic) == Mqtt::topic_settings){
        Serial.println("Received updated settings");
    }
  }

bool Mqtt::reconnect(){
    while (!Mqtt::connected()) {
        Serial.println("Trying to reconnect to MQTT server");
        
        if(!Mqtt::connect(Mqtt::client_name.c_str())){
            Serial.print("Connection failed, rc=");
            Serial.print(Mqtt::state());
            Serial.println("Waiting 5 seconds to retry");
            delay(5000);
            continue;
        }

        Serial.print("Connected to MQTT server with client ");
        Serial.println(Mqtt::client_name);
        
        Mqtt::subscribe(Mqtt::topic_settings.c_str());
        Mqtt::subscribe(Mqtt::topic_probes.c_str());
        
        // TODO fetch message with retention or which logic?
        // Mqtt::publish_settings();
        Mqtt::publish_status();
    }

    return true;
}

// TODO
// hoe callback doen enkel indien settings changen? -> mqttClient extern maken
// json omzetting voorzien
// subscription code uitwerken
