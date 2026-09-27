
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

    Mqtt::client_name            = "grilly-plus-" + config::grill_uuid;
    String topic_prefix          = config::mqtt_topic + "/" + config::grill_uuid;

    Mqtt::pub_topic_grill        = topic_prefix + "/grill" ;
    Mqtt::pub_topic_settings     = topic_prefix + "/settings";
    Mqtt::pub_topic_probes       = topic_prefix + "/probes";
    Mqtt::pub_topic_error        = topic_prefix + "/error";

    Mqtt::sub_topic_settings     = topic_prefix + "/config/settings";
    Mqtt::sub_topic_probes       = topic_prefix + "/config/probes";

    Mqtt::server_host            = mqtt_broker;
    Mqtt::setServer(Mqtt::server_host.c_str(), mqtt_port);
    Mqtt::setBufferSize(config::mqtt_buffer_size);

    // Needed because otherwise we'd have to use static members
    // https://blog.mbedded.ninja/programming/languages/c-plus-plus/callbacks/#stdfunction-with-stdbind
    Mqtt::setCallback(std::bind(&Mqtt::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

void Mqtt::request_publish_settings(){
    Mqtt::settings_publish_requested = true;
}

void Mqtt::request_publish_probes(){
    Mqtt::probes_publish_requested = true;
}

void Mqtt::publish_requested(){
    if(Mqtt::settings_publish_requested.exchange(false)){ Mqtt::publish_settings(); }
    if(Mqtt::probes_publish_requested.exchange(false))  { Mqtt::publish_probes(); }
}

// Rejected config messages are reported on <prefix>/<uuid>/error, otherwise there is no way to see
// over mqtt why a change was ignored
void Mqtt::publish_error(const String& topic, const String& error){
    Serial.printf("MQTT message on [%s] rejected: %s\n", topic.c_str(), error.c_str());

    JsonDocument jsondoc;
    jsondoc["topic"] = topic;
    jsondoc["error"] = error;

    String message;
    serializeJson(jsondoc, message);
    Mqtt::publish(Mqtt::pub_topic_error.c_str(), message.c_str());
}

void Mqtt::receive_callback(char* topic, byte* payload, unsigned int length){

    Serial.printf("MQTT Message arrived on [%s] ", topic);
    Serial.println();

    // Copy the topic, the publishes below reuse the client buffer it points into
    String received_topic = String(topic);

    // A zero byte message is our own wipe of a retained message below
    if(length == 0){ return; }

    bool is_probes   = received_topic == Mqtt::sub_topic_probes;
    bool is_settings = received_topic == Mqtt::sub_topic_settings;
    if(!is_probes && !is_settings){ return; }

    jsonResult result = {false, "Message is too large"};
    if(length < sizeof(mqtt_json_buffer)){
        memcpy(mqtt_json_buffer, payload, length);
        mqtt_json_buffer[length] = '\0';

        if(is_probes){
            result = config::json_handler.save_json_probes(mqtt_json_buffer);
        } else {
            result = config::json_handler.save_json_settings(mqtt_json_buffer);
        }
    }

    //Wipe the retained message, unsub and sub again to not trigger an echo loop
    Mqtt::unsubscribe(received_topic.c_str());
    Mqtt::publish(received_topic.c_str(), nullptr, 0, true);
    Mqtt::subscribe(received_topic.c_str());

    if(!result.success){
        Mqtt::publish_error(received_topic, result.message);
    }
}

bool Mqtt::connect_once(){
    if(!grill::wifi_connected){ return false; }

    Serial.println("Trying to connect to MQTT server");

    bool connected;
    if(config::mqtt_user != "" && config::mqtt_password != ""){
        Serial.println("Trying to connect to MQTT using user/pass");
        connected = Mqtt::connect(Mqtt::client_name.c_str(), config::mqtt_user.c_str(), config::mqtt_password.c_str());
    } else {
        Serial.println("Trying to connect to MQTT without authentication");
        connected = Mqtt::connect(Mqtt::client_name.c_str());
    }

    if(!connected){
        Serial.print("MQTT Connection failed, rc= ");
        Serial.println(Mqtt::state());
        return false;
    }

    String topic_prefix = config::mqtt_topic + "/" + config::grill_uuid;

    Serial.print("MQTT Connected to server with client ");
    Serial.println(Mqtt::client_name);
    Serial.print("MQTT topic prefix ");
    Serial.println(topic_prefix);

    Mqtt::subscribe(Mqtt::sub_topic_settings.c_str());
    Mqtt::subscribe(Mqtt::sub_topic_probes.c_str());

    Mqtt::publish_grill();
    Mqtt::publish_probes();
    Mqtt::publish_settings();

    // Everything was just published
    Mqtt::settings_publish_requested = false;
    Mqtt::probes_publish_requested   = false;

    return true;
}
