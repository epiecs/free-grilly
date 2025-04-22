#pragma once
#include <Arduino.h>

#ifndef PROBE_H
	#define PROBE_H
#endif

class Probe
{
private:
	const float ADC_REFERENCE_VOLTAGE          =  3.01; // ADC reference voltage - measured at REF pin
	const float ADC_BASE_VOLTAGE               =  2.50; // ADC base voltage for temp calculation
	const float ADC_REFERENCE_KOHM             =    10; // kOhm reference resistor for ADC
	const int   ADC_PROBE_DISCONNECTED_VALUE   = 54500; // ADC measured value when probe disconnected
	
	const int 	ADC_READ_DELAY_MS			   =     1; // Delay needed for accurate readings. If we switch to fast we get overlaps.

	const int   TEMP_HYSTERISIS_OFFSET		   =     2; // The amount of degrees a probe should be seperated from a temp to be eligible again to beep

	bool has_beeped           				   = false; // Has the probe beeped for the target temperature
	bool has_beeped_before    				   = false; // Has the probe beeped for the before temperature
	bool has_beeped_outside   				   = false; // Has the probe beeped for being outside of the temperature range

	/**
	 * @brief Selects the wanted probe via the MUX pinouts
	 * 
	 * @param probeNumber The probe number
	 */
	void select_probe(int probeNumber);

public:

	int number;											// The probe number
	int reference_kohm;									// Reference kOhm from the datasheet
	int reference_celcius;								// Reference temperature in C from the datasheet
	int reference_beta;									// Beta value from the datasheet

	String type = "grilleye_iris";						// The probe type

	float temperature;								    // The last known temperature for the chosen temperature_unit
														// in the grill settings
	float celcius;										// The last known temperature in Celcius
	float fahrenheit;							        // The last known temperature in Fahrenheit
														// Gets stored on calculate_temperature

	float minimum_temperature = 0;						// The minimum, used for temperature ranges
	float target_temperature  = 0;						// The target temperature
	bool connected            = false;					// If the probe is connected
	
	bool alarm                = false;					// Is the probe currently in an alarm state

	/**
	 * @brief Construct a new Probe object
	 * 
	 * @param number 				The probe number
	 * @param reference_kohm 		The kOhm value from the datasheet at the reference temperature
	 * @param reference_celcius 	The reference temperature from the datasheet in Celcius
	 * @param reference_beta 		The beta coefficient value from the datasheet
	 */
	Probe(int number = 1, int reference_kohm = 100, int reference_celcius = 25, int reference_beta = 4250);

	/**
	 * @brief Read the raw ADC value
	 * 
	 * @return * uint16_t Unsigned 16 bit value containing the raw voltage ratio
	 */
	uint16_t read_adc_value();

	/**
	 * @brief Read the current measured voltage at the ADC in relation to the reference voltage
	 * 
	 * @return float The measured voltage
	 */
	float read_adc_voltage();

	/**
	 * @brief Checks the status of the temperature and checks if beeps/alarms/... are needed
	 */
	void check_temperature_status();
	
	/**
	 * @brief Read the current voltage and use the probe settings to calculate the temperature
	 * 
	 * @return float The temperature in Celcius
	 */
	float calculate_temperature();

	/**
	 * @brief Set the type of probe used. 
	 * 
	 * @param probe_type the type of probe used
	 * @param reference_kohm optional, only needed for custom type
	 * @param reference_celcius optional, only needed for custom type
	 * @param reference_beta optional, only needed for custom type
	 */
	void set_type(String probe_type, int reference_kohm = 100, int reference_celcius = 25, int reference_beta = 4250);

	/**
	 * @brief Sets the temperature values of the probe. Also checks the probe alarms. If the minimum temperature is not set to 0 then we are in range mode.
	 * 
	 * @param target_temperature 	the wanted temperature
	 * @param minimum_temperature 	the minimum temperature. used when setting a temperature range, in which case the target temperature is the max temperature.
	 */
	void set_temperature(float target_temperature, float minimum_temperature = 0.0);
};