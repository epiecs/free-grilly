#include <Arduino.h>


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

public:

	int number;											// The probe number
	int reference_kohm;									// Reference kOhm from the datasheet
	int reference_celcius;								// Reference temperature in C from the datasheet
	int reference_beta;									// Beta value from the datasheet

	Probe(int number = 1, int reference_kohm = 100, int reference_celcius = 25, int reference_beta = 4250);

	void select_probe(int probeNumber);
	uint16_t read_adc_value();
	float read_adc_voltage();
	float calculate_temperature();
};