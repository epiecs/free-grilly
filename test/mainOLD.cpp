#include <Arduino.h>
#include <Wire.h>
#include <power.h>
#include <WiFi.h>

WiFiClient localClient;

const char* ssid = "***";
const char* password = "***";
const uint port = 10000;
const char* ip = "192.168.10.167";

#define BTN_PWR 35 

void setup() {
  Serial.begin(115200);
  Serial.println("Connect Wlan");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  localClient.connect(ip, port);
  pinMode(BTN_PWR, INPUT);
  power.init();

  power.setPowerRail(ENABLE, PWR_IC);
  power.setPowerRail(ENABLE, PWR_PROBES);
}

void loop() {
 delay(1000);
  if (!battery.init()) {
    Serial.println("Error: Unable to communicate with battery ic over I2C.");
  }
  else {
    Serial.println("Connected to battery ic");
  }
  Serial.print(battery.voltage(), 3);
  Serial.print(" V - ");
  Serial.print(battery.current(AVG), 3);
  Serial.print(" A - ");
  Serial.print(battery.power(), 3);
  Serial.print(" W - ");
  Serial.print(battery.soc(UNFILTERED));
  Serial.print(" % - ");
  Serial.print(battery.temp());
  Serial.print(" C - ");
  Serial.print(battery.temp(F));
  Serial.print(" F - ");
  if (battery.chargeFlag()) {
    Serial.print("charging -- ");
  }
  else {
    Serial.print("discharging -- ");
  }

  if(digitalRead(BTN_PWR) == 0){
    power.setPowerRail(ENABLE, PWR_SCREEN_LED);
  }


  localClient.print(battery.voltage(), 3);
  localClient.print(" V - ");
  localClient.print(battery.current(AVG), 3);
  localClient.print(" A - ");
  localClient.print(battery.power(), 3);
  localClient.print(" W - ");
  localClient.print(battery.soc(UNFILTERED));
  localClient.print(" % - ");
  localClient.print(battery.temp());
  localClient.print(" C - ");
  localClient.print(battery.temp(F));
  localClient.print(" F - ");
  if (battery.chargeFlag()) {
    localClient.println("charging -- ");
  }
  else {
    localClient.println("discharging -- ");
  }

  if(digitalRead(BTN_PWR) == 0){
    power.setPowerRail(ENABLE, PWR_SCREEN_LED);
  }
}