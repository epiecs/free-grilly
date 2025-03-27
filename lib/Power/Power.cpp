#include <Arduino.h>
#include <Power.h>
#include <Wire.h>
#include "Global.h"

// Initializes class variables
bat::bat() : _deviceAddress(BAT_I2C) {}

bool bat::init(void) {
    uint16_t deviceID = 0;
    Wire.begin();
    deviceID = deviceType();
    if (deviceID == BAT_DEVICE_ID)
		return true; 
	
	return false; 
}

uint16_t bat::deviceType(void) {
	return readControlWord(BAT_CONTROL_DEVICE_TYPE);
}

float bat::voltage(void) {
    float volts = 0;
    volts = (readWord(BAT_VOLTAGE) * 0.001);
	return volts;
}

float bat::current(current_measure type) {
	int16_t currentmA = 0;
    float current = 0;
	switch (type)
	{
	case AVG:
        currentmA = (int16_t) readWord(BAT_CURRENT_AVG);
		break;
	case STBY:
        currentmA = (int16_t) readWord(BAT_CURRENT_STBY);
		break;
	case MAX:
        currentmA = (int16_t) readWord(BAT_CURRENT_MAX);
		break;
	}
	current = (currentmA * 0.001);
	return current;
}

float bat::power(void) {
    float watt = 0;
	watt = ((int16_t) readWord(BAT_POWER) * 0.001);
    return watt;
}

uint16_t bat::soc(soc_measure type) {
	uint16_t soc = 0;
	switch (type)
	{
	case FILTERED:
		soc = readWord(BAT_SOC_FILTERED);
		break;
	case UNFILTERED:
		soc = readWord(BAT_SOC_UNFILTERED);
		break;
	}
	return soc;
}

float bat::temp(temp_unit type) {
	uint16_t temp = 0;
    temp = readWord(BAT_TEMP);
	switch (type)
	{
	case C:
		return ((temp * 0.1) - 273.15); 
		break;
	case F:
        return (1.8 * (temp * 0.1) - 459.67); 
		break;
	}
	return false;
}

bool bat::chargeFlag(void) {
	uint16_t flagState = flags();
	bool is_charging = flagState & BAT_FLAG_CHARGE;
	return !is_charging;
}

bool bat::read_battery(void) {
	battery_percent  = soc(FILTERED);
	battery_charging = chargeFlag();
	return true;
}


// ----------------------------------------------------------------------------------------------- //

uint16_t bat::flags(void) {
	return readWord(BAT_FLAGS);
}

uint16_t bat::readWord(uint16_t subAddress) {
	uint8_t data[2];
	i2cReadBytes(subAddress, data, 2);
	return ((uint16_t) data[1] << 8) | data[0];
}

uint16_t bat::readControlWord(uint16_t function) {
	uint8_t subCommandMSB = (function >> 8);
	uint8_t subCommandLSB = (function & 0x00FF);
	uint8_t command[2] = {subCommandLSB, subCommandMSB};
	uint8_t data[2] = {0, 0};
	
	i2cWriteBytes((uint8_t) 0, command, 2);
	
	if (i2cReadBytes((uint8_t) 0, data, 2))
		return ((uint16_t)data[1] << 8) | data[0];
	
	return false;
}

uint16_t bat::i2cReadBytes(uint8_t subAddress, uint8_t * dest, uint8_t count) {
	int16_t timeout = BAT_TIMEOUT;	
	Wire.beginTransmission(_deviceAddress);
	Wire.write(subAddress);
	Wire.endTransmission(true);
	
	Wire.requestFrom(_deviceAddress, count);
	
	for (int i=0; i<count; i++) {
		dest[i] = Wire.read();
	}
	
	return timeout;
}

uint16_t bat::i2cWriteBytes(uint8_t subAddress, uint8_t * src, uint8_t count) {
	Wire.beginTransmission(_deviceAddress);
	Wire.write(subAddress);
	for (int i=0; i<count; i++) {
		Wire.write(src[i]);
	}	
	Wire.endTransmission(true);
	
	return true;	
}

// ----------------------------------------------------------------------------------------------- //

pwr::pwr() {};

bool pwr::init(void) {
	pinMode(PWR_SCREEN_LED, OUTPUT);
	pinMode(PWR_IC, OUTPUT);
	pinMode(PWR_PROBES, OUTPUT);
	return true; 
}

bool pwr::setPowerRail(pwr_state type, int GPIO) {
	switch (type)
	{
	case ENABLE:
		digitalWrite(GPIO, LOW);
		break;
	case DISABLE:
		digitalWrite(GPIO, HIGH);
		break;
	}
	return false;
}

bool pwr::shutdown(void) {
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0);
	setPowerRail(DISABLE,PWR_PROBES);
	setPowerRail(DISABLE,PWR_IC);
	esp_deep_sleep_start();
	return true;
}

bool pwr::startup(void) {
	init();
	setPowerRail(ENABLE,PWR_IC);
	setPowerRail(ENABLE,PWR_PROBES);
	return true;
}

bat battery;
pwr power;