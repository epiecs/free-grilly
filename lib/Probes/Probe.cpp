#include <Arduino.h>
#include <SPI.h>
#include <math.h>

#include "Probe.h"

extern int HSPI_CS;

Probe::Probe(int number, int reference_kohm, int reference_celcius, int reference_beta) {
    Probe::number = number;
    Probe::reference_kohm = reference_kohm;
    Probe::reference_celcius = reference_celcius;
    Probe::reference_beta = reference_beta;

    pinMode(MUX_SELECTOR_A, OUTPUT);
    pinMode(MUX_SELECTOR_B, OUTPUT);
    pinMode(MUX_SELECTOR_C, OUTPUT);
}


uint16_t Probe::read_adc_value() {
    Probe::select_probe(Probe::number);
    delay(ADC_READ_DELAY_MS);

    digitalWrite(HSPI_CS, HIGH);
    delayMicroseconds(1);
    digitalWrite(HSPI_CS, LOW);
    delayMicroseconds(1);

    return SPI.transfer16(0x0000);
}

void Probe::select_probe(int probe_number) {

    switch (probe_number) {
        case 1:
            digitalWrite(MUX_SELECTOR_A, LOW);
            digitalWrite(MUX_SELECTOR_B, LOW);
            digitalWrite(MUX_SELECTOR_C, HIGH);
            break;
        case 2:
            digitalWrite(MUX_SELECTOR_A, LOW);
            digitalWrite(MUX_SELECTOR_B, HIGH);
            digitalWrite(MUX_SELECTOR_C, HIGH);
            break;
        case 3:
            digitalWrite(MUX_SELECTOR_A, HIGH);
            digitalWrite(MUX_SELECTOR_B, HIGH);
            digitalWrite(MUX_SELECTOR_C, HIGH);
            break;
        case 4:
            digitalWrite(MUX_SELECTOR_A, HIGH);
            digitalWrite(MUX_SELECTOR_B, LOW);
            digitalWrite(MUX_SELECTOR_C, HIGH);
            break;
        case 5:
            digitalWrite(MUX_SELECTOR_A, LOW);
            digitalWrite(MUX_SELECTOR_B, HIGH);
            digitalWrite(MUX_SELECTOR_C, LOW);
            break;
        case 6:
            digitalWrite(MUX_SELECTOR_A, HIGH);
            digitalWrite(MUX_SELECTOR_B, HIGH);
            digitalWrite(MUX_SELECTOR_C, LOW);
            break;
        case 7:
            digitalWrite(MUX_SELECTOR_A, HIGH);
            digitalWrite(MUX_SELECTOR_B, LOW);
            digitalWrite(MUX_SELECTOR_C, LOW);
            break;
        case 8:
            digitalWrite(MUX_SELECTOR_A, LOW);
            digitalWrite(MUX_SELECTOR_B, LOW);
            digitalWrite(MUX_SELECTOR_C, LOW);
            break;
    }
}

float Probe::read_adc_voltage() {
    
    uint16_t adc_value = Probe::read_adc_value();

    if (adc_value > ADC_PROBE_DISCONNECTED_VALUE) {
        return nanf(""); // If disconnected return NaN
    }

    // Calculate the voltage using linear scaling
    double voltage = (static_cast<double>(adc_value) / 65534.0) * ADC_REFERENCE_VOLTAGE;
    return voltage;
}

float Probe::calculate_temperature() {
    uint16_t adc_value = Probe::read_adc_value();

    if (adc_value > ADC_PROBE_DISCONNECTED_VALUE) {
        return nanf(""); // If disconnected return NaN
    }

    // Calculate the voltage using linear scaling
    float voltage = (static_cast<float>(adc_value) / 65534.0) * ADC_REFERENCE_VOLTAGE;

    float ref_kelvin = 1 / (reference_celcius + 273.15);
    float ref_beta = 1 / static_cast<float>(reference_beta);
    float ref_volt = (voltage * ADC_REFERENCE_KOHM) / (reference_kohm * (ADC_BASE_VOLTAGE - voltage));
    float log_volt = log(ref_volt);

    float temperature = (1 / (ref_kelvin + ref_beta * log_volt)) - 273.15;

    return temperature;
}