#include <Arduino.h>
#include <Power.h>
#include <Wire.h>
#include <esp_pm.h>
#include <driver/gpio.h>

#include "Config.h"
#include "Grill.h"

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
	return is_charging;
}

bool bat::read_battery(void) {
	grill::battery_percentage  = soc(FILTERED);
	grill::battery_charging 	= chargeFlag();
	grill::battery_millivolts   = (int) (voltage() * 1000.0f + 0.5f);
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
	// A wake from deep sleep leaves the previous output holds latched until they
	// are explicitly released.  Do this before attaching PWM or changing rails.
	gpio_hold_dis((gpio_num_t)gpio::power_screen_backlight);
	gpio_hold_dis((gpio_num_t)gpio::power_probes);
	gpio_hold_dis((gpio_num_t)gpio::power_adc_circuit);
	ledcSetup(gpio::pwm_screen_channel, gpio::pwm_screen_frequency, gpio::pwm_screen_resolution);
	ledcAttachPin(gpio::power_screen_backlight, gpio::pwm_screen_channel);
	setScreenBrightness(config::backlight_brightness);
	pinMode(gpio::power_adc_circuit, OUTPUT);
	pinMode(gpio::power_probes, OUTPUT);
	return true; 
}

bool pwr::setPowerRail(status_type type, int GPIO) {
	switch (type)
	{
	case ENABLE:
		/*if(GPIO = gpio::power_screen_backlight) { digitalWrite(GPIO, HIGH);  }
		else { }*/
		digitalWrite(GPIO, LOW);
		break;
	case DISABLE:
		/*if(GPIO = gpio::power_screen_backlight) { digitalWrite(GPIO, LOW);  }
		else { }*/
		digitalWrite(GPIO, HIGH); 
		break;
	}
	return true;
}

bool pwr::setScreenBrightness(int brightness) {

	// Convert brightness from 0-5 to 0-255 with a min/max setting
	if (brightness == 0) {
		ledcWrite(gpio::pwm_screen_channel, 0);
		return true;
	}

	brightness = (brightness * 50) + 5;

	if(brightness < 25){
		brightness = 25;
	} else if (brightness > 255){
		brightness = 255;
	}

	ledcWrite(gpio::pwm_screen_channel, brightness);
	return true;
}

bool pwr::shutdown(void) {
	// GPIO35 is the active-low wake button.  Do not enter deep sleep while it is
	// still pressed: EXT0 is level-triggered and would wake the ESP32 immediately.
	// A timeout keeps a defective/stuck button from blocking the shutdown path forever.
	unsigned long wait_started = millis();
	while (digitalRead(gpio::power_button) == LOW && millis() - wait_started < 5000) {
		delay(10);
	}

	// The backlight is PWM driven and is not one of the switched rails.  Force its
	// output low before sleep so the display cannot remain powered.
	setScreenBrightness(0);
	pinMode(gpio::power_screen_backlight, OUTPUT);
	digitalWrite(gpio::power_screen_backlight, LOW);

	// shutdown() can also be reached from setup(), before power.startup() has
	// initialized the rail pins.
	pinMode(gpio::power_probes, OUTPUT);
	pinMode(gpio::power_adc_circuit, OUTPUT);
	setPowerRail(DISABLE,gpio::power_probes);
	setPowerRail(DISABLE,gpio::power_adc_circuit);

	// Preserve the inactive levels while the digital power domain is powered down.
	// Without these holds the external rail-enable inputs can float and re-enable
	// the probe/ADC circuitry during deep sleep.
	gpio_hold_en((gpio_num_t)gpio::power_screen_backlight);
	gpio_hold_en((gpio_num_t)gpio::power_probes);
	gpio_hold_en((gpio_num_t)gpio::power_adc_circuit);

	esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0);
	esp_deep_sleep_start();
	return true;
}

bool pwr::startup(void) {
	init();
	setPowerRail(ENABLE,gpio::power_adc_circuit);
	setPowerRail(ENABLE,gpio::power_probes);
	return true;
}

bool pwr::enable_power_management(void) {
    // Dynamic Frequency Scaling: CPU idles at 80 MHz, boosts to 240 MHz under load.
    // light_sleep_enable = false keeps HSPI (probes) and I2C (battery gauge) always-on.
    esp_pm_config_esp32_t pm_cfg = {};
    pm_cfg.max_freq_mhz      = 240;
    pm_cfg.min_freq_mhz      = 80;
    pm_cfg.light_sleep_enable = false;

    esp_err_t ret = esp_pm_configure(&pm_cfg);
    if (ret != ESP_OK) {
        Serial.println("Power: DFS not available on this build (esp_pm_configure failed)");
        return false;
    }
    Serial.println("Power: DFS enabled (80–240 MHz)");
    return true;
}

bat battery;
pwr power;
