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

	const int   MUX_SELECTOR_A                 =    33; // MUX pin A
	const int   MUX_SELECTOR_B                 =    25; // MUX pin B
	const int   MUX_SELECTOR_C                 =    26; // MUX pin C

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

	float temperature;									// The last known temperature in Celcius
														// gets stored on calculate_temperature
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
	 * @brief Read the current voltage and use the probe settings to calculate the temperature
	 * 
	 * @return float The temperature in Celcius
	 */
	float calculate_temperature();
};