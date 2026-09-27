
#include <functional>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Config.h"
#include "Grill.h"
#include "JsonUtilities.h"
#include "Opengrill.h"

// Set this to config::json_buffer_size, cant do this dynamically
char mqtt_opengrill_buffer[3000];

void Opengrill::setup(String opengrill_server, int mqtt_port){

    Opengrill::client_name            = "grilly-plus-opengrill-" + config::grill_uuid;
    String topic_prefix               = config::opengrill_topic + "/" + config::grill_uuid;

    Opengrill::pub_topic_grill        = topic_prefix + "/grill" ;
    Opengrill::pub_topic_probes       = topic_prefix + "/probes";

    Opengrill::sub_topic_grill        = topic_prefix + "/config/grill";
    Opengrill::sub_topic_probes       = topic_prefix + "/config/probes";

    Opengrill::setServer(opengrill_server.c_str(), mqtt_port);
    Opengrill::setBufferSize(config::opengrill_buffer_size);

    // Needed because otherwise we'd have to use static members
    // https://blog.mbedded.ninja/programming/languages/c-plus-plus/callbacks/#stdfunction-with-stdbind
    Opengrill::setCallback(std::bind(&Opengrill::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}

void Opengrill::publish_grill(){
    config::json_handler.load_opengrill_grill(mqtt_opengrill_buffer);
    Opengrill::publish(Opengrill::pub_topic_grill.c_str(), mqtt_opengrill_buffer);
}

void Opengrill::publish_probes(){
    config::json_handler.load_opengrill_probes(mqtt_opengrill_buffer);
    Opengrill::publish(Opengrill::pub_topic_probes.c_str(), mqtt_opengrill_buffer, true);
}

void Opengrill::receive_callback(char* topic, byte* payload, unsigned int length){

    Serial.printf("Opengrill Message arrived on [%s] ", topic);

    for (unsigned int i = 0; i < length; i++){
        // For debugging
        // Serial.print((char)payload[i]);
        mqtt_opengrill_buffer[i] = (char)payload[i];
    }
    Serial.println();

    if (String(topic) == Opengrill::sub_topic_grill){
        Serial.println("Received grill config update from Opengrill server");
        jsonResult result = config::json_handler.save_opengrill_grill(mqtt_opengrill_buffer);

        //Wipe the retained message, unsub and sub again to not trigger an echo loop
        Opengrill::unsubscribe(Opengrill::sub_topic_grill.c_str());
        Opengrill::publish(Opengrill::sub_topic_grill.c_str(), nullptr, 0, true);
        Opengrill::subscribe(Opengrill::sub_topic_grill.c_str());
    }

    if (String(topic) == Opengrill::sub_topic_probes){
        jsonResult result = config::json_handler.save_opengrill_probes(mqtt_opengrill_buffer);

        //Wipe the retained message, unsub and sub again to not trigger an echo loop
        Opengrill::unsubscribe(Opengrill::sub_topic_probes.c_str());
        Opengrill::publish(Opengrill::sub_topic_probes.c_str(), nullptr, 0, true);
        Opengrill::subscribe(Opengrill::sub_topic_probes.c_str());
    }
}

bool Opengrill::reconnect(){
    while (!Opengrill::connected()) {
        Serial.println("Trying to reconnect to Opengrill server");

        if(!grill::wifi_connected){
            delay(5000);
            continue;
        }

        Opengrill::setServer(config::opengrill_server.c_str(), config::opengrill_port);

        if(config::opengrill_user != "" && config::opengrill_password != ""){
            Serial.println("Trying to connect to Opengrill using user/pass");
            if(!Opengrill::connect(Opengrill::client_name.c_str(), config::opengrill_user.c_str(), config::opengrill_password.c_str())){
                Serial.print("Opengrill Connection failed, rc= ");
                Serial.print(Opengrill::state());
                Serial.println("Waiting 5 seconds to retry");
                delay(5000);
                continue;
            }
        } else {
            Serial.println("Trying to connect to Opengrill without authentication");
            if(!Opengrill::connect(Opengrill::client_name.c_str())){
                Serial.print("Opengrill Connection failed, rc= ");
                Serial.print(Opengrill::state());
                Serial.println("Waiting 5 seconds to retry");
                delay(5000);
                continue;
            }
        }
    }

    if(Opengrill::connected()){
        String topic_prefix = config::opengrill_topic + "/" + config::grill_uuid;

        Serial.print("Opengrill Connected to server with client ");
        Serial.println(Opengrill::client_name);
        Serial.print("Opengrill topic prefix ");
        Serial.println(topic_prefix);

        Opengrill::subscribe(Opengrill::sub_topic_grill.c_str());
        Opengrill::subscribe(Opengrill::sub_topic_probes.c_str());

        Opengrill::publish_grill();
        Opengrill::publish_probes();
    }

    return true;
}
