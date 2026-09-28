
#include <functional>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Config.h"
#include "Grill.h"
#include "JsonUtilities.h"
#include "Opengrill.h"
#include "SharedLock.h"

// Set this to config::json_buffer_size, cant do this dynamically
char mqtt_opengrill_buffer[3000];

void Opengrill::setup(String opengrill_server, int mqtt_port){

    String topic_prefix;
    {
        SharedLock lock;    // Same lock as every other config String read from a task
        Opengrill::client_name        = "grilly-plus-opengrill-" + config::grill_uuid;
        topic_prefix                  = config::opengrill_topic + "/" + config::grill_uuid;
    }

    Opengrill::pub_topic_grill        = topic_prefix + "/grill" ;
    Opengrill::pub_topic_probes       = topic_prefix + "/probes";

    Opengrill::sub_topic_grill        = topic_prefix + "/config/grill";
    Opengrill::sub_topic_probes       = topic_prefix + "/config/probes";

    Opengrill::server_host            = opengrill_server;
    Opengrill::setServer(Opengrill::server_host.c_str(), mqtt_port);
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

void Opengrill::request_publish_grill(){
    Opengrill::grill_publish_requested = true;
}

void Opengrill::request_publish_probes(){
    Opengrill::probes_publish_requested = true;
}

void Opengrill::publish_requested(){
    if(Opengrill::grill_publish_requested.exchange(false)) { Opengrill::publish_grill(); }
    if(Opengrill::probes_publish_requested.exchange(false)){ Opengrill::publish_probes(); }
}

void Opengrill::receive_callback(char* topic, byte* payload, unsigned int length){

    Serial.printf("Opengrill Message arrived on [%s] ", topic);
    Serial.println();

    // Copy the topic, the publishes below reuse the client buffer it points into
    String received_topic = String(topic);

    // A zero byte message is our own wipe of a retained message below
    if(length == 0){ return; }

    bool is_grill  = received_topic == Opengrill::sub_topic_grill;
    bool is_probes = received_topic == Opengrill::sub_topic_probes;
    if(!is_grill && !is_probes){ return; }

    jsonResult result = {false, "Message is too large"};
    if(length < sizeof(mqtt_opengrill_buffer)){
        memcpy(mqtt_opengrill_buffer, payload, length);
        mqtt_opengrill_buffer[length] = '\0';

        if(is_grill){
            Serial.println("Received grill config update from Opengrill server");
            result = config::json_handler.save_opengrill_grill(mqtt_opengrill_buffer);
        } else {
            result = config::json_handler.save_opengrill_probes(mqtt_opengrill_buffer);
        }
    }

    if(!result.success){
        Serial.printf("Opengrill message on [%s] rejected: %s\n", received_topic.c_str(), result.message.c_str());
    }

    //Wipe the retained message, unsub and sub again to not trigger an echo loop
    Opengrill::unsubscribe(received_topic.c_str());
    Opengrill::publish(received_topic.c_str(), nullptr, 0, true);
    Opengrill::subscribe(received_topic.c_str());
}

bool Opengrill::connect_once(){
    if(!grill::wifi_connected){ return false; }

    Serial.println("Trying to connect to Opengrill server");

    // Copied under the shared lock, connecting is network I/O
    String opengrill_user, opengrill_password, topic_prefix;
    {
        SharedLock lock;
        opengrill_user     = config::opengrill_user;
        opengrill_password = config::opengrill_password;
        topic_prefix       = config::opengrill_topic + "/" + config::grill_uuid;
    }

    bool connected;
    if(opengrill_user != "" && opengrill_password != ""){
        Serial.println("Trying to connect to Opengrill using user/pass");
        connected = Opengrill::connect(Opengrill::client_name.c_str(), opengrill_user.c_str(), opengrill_password.c_str());
    } else {
        Serial.println("Trying to connect to Opengrill without authentication");
        connected = Opengrill::connect(Opengrill::client_name.c_str());
    }

    if(!connected){
        Serial.print("Opengrill Connection failed, rc= ");
        Serial.println(Opengrill::state());
        return false;
    }

    Serial.print("Opengrill Connected to server with client ");
    Serial.println(Opengrill::client_name);
    Serial.print("Opengrill topic prefix ");
    Serial.println(topic_prefix);

    Opengrill::subscribe(Opengrill::sub_topic_grill.c_str());
    Opengrill::subscribe(Opengrill::sub_topic_probes.c_str());

    Opengrill::publish_grill();
    Opengrill::publish_probes();

    // Everything was just published
    Opengrill::grill_publish_requested  = false;
    Opengrill::probes_publish_requested = false;

    return true;
}
