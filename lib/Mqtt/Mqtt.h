#pragma once
#include <Arduino.h>
#include <PubSubClient.h>

class Mqtt : public PubSubClient{

private:

    const String MAIN_TOPIC     = "opengrill";
    
    String client_name          = "";

    String topic_temperatures   = "";
    String topic_settings       = "";
    String topic_probes         = "";
    String topic_status         = "";

public:
    
    // Overload the class so that we can use our own callback with class methods
    Mqtt(Client& wifiClient) : PubSubClient(wifiClient) {};
    
    void setup(String mqtt_broker, int mqtt_port = 1883);
    
    void publish_status();
    void publish_settings();
    
    bool reconnect();

    void callback(char* topic, byte* payload, unsigned int length);

protected:
    void receive_callback(char* topic, byte* payload, unsigned int length);

};