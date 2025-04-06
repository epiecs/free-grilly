#include <Arduino.h>
#include <SPI.h>
#include <math.h>

#include "Probe.h"
#include "Gpio.h"


Probe::Probe(int number, int reference_kohm, int reference_celcius, int reference_beta) {
    Probe::number = number;
    Probe::reference_kohm = reference_kohm;
    Probe::reference_celcius = reference_celcius;
    Probe::reference_beta = reference_beta;
    
    // Default type
    Probe::set_type("GRILLEYE_IRIS");
}


uint16_t Probe::read_adc_value() {
    Probe::select_probe(Probe::number);
    
    vTaskDelay(ADC_READ_DELAY_MS);
    
    digitalWrite(gpio::hspi_probes_cs, HIGH);
    delayMicroseconds(1);
    digitalWrite(gpio::hspi_probes_cs, LOW);
    delayMicroseconds(1);

    return SPI.transfer16(0x0000);
}

void Probe::select_probe(int probe_number) {

    switch (probe_number) {
        case 1:
            digitalWrite(gpio::mux_selector_a, LOW);
            digitalWrite(gpio::mux_selector_b, LOW);
            digitalWrite(gpio::mux_selector_c, HIGH);
            break;
        case 2:
            digitalWrite(gpio::mux_selector_a, LOW);
            digitalWrite(gpio::mux_selector_b, HIGH);
            digitalWrite(gpio::mux_selector_c, HIGH);
            break;
        case 3:
            digitalWrite(gpio::mux_selector_a, HIGH);
            digitalWrite(gpio::mux_selector_b, HIGH);
            digitalWrite(gpio::mux_selector_c, HIGH);
            break;
        case 4:
            digitalWrite(gpio::mux_selector_a, HIGH);
            digitalWrite(gpio::mux_selector_b, LOW);
            digitalWrite(gpio::mux_selector_c, HIGH);
            break;
        case 5:
            digitalWrite(gpio::mux_selector_a, LOW);
            digitalWrite(gpio::mux_selector_b, HIGH);
            digitalWrite(gpio::mux_selector_c, LOW);
            break;
        case 6:
            digitalWrite(gpio::mux_selector_a, HIGH);
            digitalWrite(gpio::mux_selector_b, HIGH);
            digitalWrite(gpio::mux_selector_c, LOW);
            break;
        case 7:
            digitalWrite(gpio::mux_selector_a, HIGH);
            digitalWrite(gpio::mux_selector_b, LOW);
            digitalWrite(gpio::mux_selector_c, LOW);
            break;
        case 8:
            digitalWrite(gpio::mux_selector_a, LOW);
            digitalWrite(gpio::mux_selector_b, LOW);
            digitalWrite(gpio::mux_selector_c, LOW);
            break;
    }
}

float Probe::read_adc_voltage() {
    
    uint16_t adc_value = Probe::read_adc_value();

    if (adc_value > ADC_PROBE_DISCONNECTED_VALUE) {
        return nanf(""); // If disconnected return NaN
    }

    // Calculate the voltage using linear scaling
    float voltage = (static_cast<float>(adc_value) / 65534.0) * ADC_REFERENCE_VOLTAGE;
    return voltage;
}

float Probe::calculate_temperature() {
    uint16_t adc_value = Probe::read_adc_value();

    if (adc_value > ADC_PROBE_DISCONNECTED_VALUE) {
        Probe::celcius    = 0;
        Probe::fahrenheit = 32;
        Probe::connected  = false;
        
        return nanf(""); // If disconnected return NaN
    }

    // Calculate the voltage using linear scaling
    float voltage = (static_cast<float>(adc_value) / 65534.0) * ADC_REFERENCE_VOLTAGE;

    float ref_kelvin = 1 / (reference_celcius + 273.15);
    float ref_beta = 1 / static_cast<float>(reference_beta);
    float ref_volt = (voltage * ADC_REFERENCE_KOHM) / (reference_kohm * (ADC_BASE_VOLTAGE - voltage));
    float log_volt = log(ref_volt);

    float temperature = (1 / (ref_kelvin + ref_beta * log_volt)) - 273.15;

    // Store temp in public property
    Probe::celcius    = temperature;
    Probe::fahrenheit = (temperature * 1.8) + 32;
    Probe::connected  = true;

    return temperature;
}

void Probe::set_type(String probe_type){
    
    if(probe_type == "GRILLEYE_IRIS"){
        Probe::reference_beta    = 4250;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 100;
        Probe::type              = "GRILLEYE_IRIS";
        return;
    }

    if(probe_type == "IKEA_FANTAST"){
        Probe::reference_beta    = 4250;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 230;
        Probe::type              = "IKEA_FANTAST";
        return;
    }

    // default value if not known is custom
    Probe::type              = "CUSTOM";
    return;
}