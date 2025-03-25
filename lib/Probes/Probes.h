#include <Arduino.h>

// ----------------------------------------- //
//                Definitions                //
// ----------------------------------------- //

#define ADC_SPD 					  16000000		// HSPI clock speed
#define ADC_SCLK 							14		// HSPI clock pin
#define ADC_CS 								15		// HSPI chip Select pin
#define ADC_MISO 							12		// HSPI data pin

#define PRB_SelA 							33 		// GPIO probe selector pin A
#define PRB_SelB 							25 		// GPIO probe selector pin B
#define PRB_SelC 							26 		// GPIO probe selector pin B

// Fixed on board
#define ADC_REF_VOLTAGE 				  3.01 		// ADC reference voltage - measured at REF pin
#define ADC_BASE_VOLTAGE 				  2.50 		// ADC base voltage for temp calculation
#define ADC_KOHM_REF						10		// kOhm reference resistor for ADC
#define PROBE_DISCONNECTED_VOLTAGE 		  2.51 		// ADC measured voltage when probe disconnected - after rounding

// Probe values from datasheet
#define NTC_KOHM_REF					   100		// kOhm at reference
#define NTC_REF_C							25      // Reference temperature in Celcius
#define BETA							  4250      // Beta coefficient`


class prb
{
public:
	prb();
	/**
		Initializes the SPI settings for the ADC ic
		@return true if SPI was initiated successful.
	*/
	bool init(void);

	float calculate_temperature(int adc_value, float probe_offset = 0);

	// private:
	//  ----------------------------------------- //
	//              SPI READ ROUTINES             //
	//  ----------------------------------------- //
	/**
		Reads and returns the raw probe SPI value
		@return Raw uint16_t value
	*/
	uint16_t spiReadProbe(void);
	/**
		Set active probe to read
		@return true if sucessful
	*/
	bool selectProbe(int8_t probeNumber);

	double adc_value_to_voltage(int adcValue);
};

extern prb probes;