#pragma once
#include <Arduino.h>
#include <PubSubClient.h>

class Mqtt : public PubSubClient{

private:
    String client_name              = "";

    // topics to publish to
    String pub_topic_grill          = "";
    String pub_topic_settings       = "";
    String pub_topic_probes         = "";
    
    // topics to subscribe to
    String sub_topic_settings       = "";
    String sub_topic_probes         = "";

public:
    
    // Overload the class so that we can use our own callback with class methods
    Mqtt(Client& wifiClient) : PubSubClient(wifiClient) {};
    
    void setup(String mqtt_broker, int mqtt_port = 1883);
    
    void publish_grill();
    void publish_probes();
    void publish_settings();
    
    bool reconnect();

protected:
    void receive_callback(char* topic, byte* payload, unsigned int length);

};