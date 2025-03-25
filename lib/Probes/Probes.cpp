#include <Probes.h>

#include <Arduino.h>
#include <SPI.h>

#include <math.h>

SPISettings spiSettings(ADC_SPD, MSBFIRST, SPI_MODE0);

prb::prb() {};

bool prb::init(void)
{
    SPI.begin(ADC_SCLK, ADC_MISO, -1, ADC_CS);
    SPI.beginTransaction(spiSettings);

    pinMode(PRB_SelA, OUTPUT);
    pinMode(PRB_SelB, OUTPUT);
    pinMode(PRB_SelC, OUTPUT);
    digitalWrite(PRB_SelA, LOW);
    digitalWrite(PRB_SelB, LOW);
    digitalWrite(PRB_SelC, LOW);

    return true;
}

uint16_t prb::spiReadProbe(void)
{
    digitalWrite(ADC_CS, HIGH);
    delayMicroseconds(1);
    digitalWrite(ADC_CS, LOW);
    delayMicroseconds(1);
    return SPI.transfer16(0x0000);
}

bool prb::selectProbe(int8_t selectedProbe)
{
    switch (selectedProbe)
    {
    case 1:
        digitalWrite(PRB_SelA, LOW);
        digitalWrite(PRB_SelB, LOW);
        digitalWrite(PRB_SelC, LOW);
        return true;
        break;
    case 2:
        digitalWrite(PRB_SelA, HIGH);
        digitalWrite(PRB_SelB, LOW);
        digitalWrite(PRB_SelC, LOW);
        return true;
        break;
    case 3:
        digitalWrite(PRB_SelA, HIGH);
        digitalWrite(PRB_SelB, HIGH);
        digitalWrite(PRB_SelC, LOW);
        return true;
        break;
    case 4:
        digitalWrite(PRB_SelA, LOW);
        digitalWrite(PRB_SelB, HIGH);
        digitalWrite(PRB_SelC, LOW);
        return true;
        break;
    case 5:
        digitalWrite(PRB_SelA, HIGH);
        digitalWrite(PRB_SelB, LOW);
        digitalWrite(PRB_SelC, HIGH);
        return true;
        break;
    case 6:
        digitalWrite(PRB_SelA, HIGH);
        digitalWrite(PRB_SelB, HIGH);
        digitalWrite(PRB_SelC, HIGH);
        return true;
        break;
    case 7:
        digitalWrite(PRB_SelA, LOW);
        digitalWrite(PRB_SelB, HIGH);
        digitalWrite(PRB_SelC, HIGH);
        return true;
        break;
    case 8:
        digitalWrite(PRB_SelA, LOW);
        digitalWrite(PRB_SelB, LOW);
        digitalWrite(PRB_SelC, HIGH);
        return true;
        break;
    }
    return false;
}

double prb::adc_value_to_voltage(int adcValue)
{
    // Assuming the ADC value ranges from 0 to 65534
    // and the voltage range is 0V to 2.97V

    if (adcValue < 0 || adcValue > 65534)
    {
        return -1.0;
    }

    // Calculate the voltage using linear scaling
    double voltage = (static_cast<double>(adcValue) / 65534.0) * ADC_REF_VOLTAGE;
    return voltage;
}

float prb::calculate_temperature(int adc_value, float probe_offset)
{
    // Assuming the ADC value ranges from 0 to 65534
    // and the voltage range is 0V to 3.01V

    if (adc_value < 0 || adc_value > 65534)
    {
        // Handle out-of-range input, you might want to return an error or a specific value
        return -1.0;
    }

    // Calculate the voltage using linear scaling
    float voltage = (static_cast<float>(adc_value) / 65534) * ADC_REF_VOLTAGE;
    
    float ref_kelvin  = 1 / (NTC_REF_C + 273.15);
    float ref_beta    = 1 / static_cast<float>(BETA);
    float ref_volt    = (voltage * ADC_KOHM_REF) / (NTC_KOHM_REF * (ADC_BASE_VOLTAGE - voltage));
    float log_volt    = log(ref_volt);
    
    float temperature = (1 / (ref_kelvin + ref_beta * log_volt)) - 273.15;

    return temperature;
}

prb probes;