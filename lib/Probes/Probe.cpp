#include <Arduino.h>
#include <SPI.h>
#include <math.h>
#include <algorithm>
#include <chrono>

#include "Config.h"
#include "Buzzer.h"
#include "Probe.h"
#include "Gpio.h"
#include "Grill.h"
#include "SharedLock.h"

Probe::Probe(int number, int reference_kohm, int reference_celcius, int reference_beta) {
    Probe::number = number;
    Probe::reference_kohm = reference_kohm;
    Probe::reference_celcius = reference_celcius;
    Probe::reference_beta = reference_beta;
    
    // Default type
    Probe::set_type("grilleye_iris");
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

    // Calculate the voltage using linear scaling
    float voltage = (static_cast<float>(adc_value) / 65534.0) * ADC_REFERENCE_VOLTAGE;

    // An empty jack reads close to ADC_PROBE_DISCONNECTED_VALUE. From ADC_BASE_VOLTAGE (about 54431)
    // up the formula below divides by zero or takes the log of a negative value, so anything at or
    // above that voltage is treated as disconnected as well.
    float temperature = nanf("");
    if (adc_value <= ADC_PROBE_DISCONNECTED_VALUE && voltage < ADC_BASE_VOLTAGE) {
        float ref_kelvin = 1 / (reference_celcius + 273.15);
        float ref_beta = 1 / static_cast<float>(reference_beta);
        float ref_volt = (voltage * ADC_REFERENCE_KOHM) / (reference_kohm * (ADC_BASE_VOLTAGE - voltage));
        float log_volt = log(ref_volt);

        temperature = (1 / (ref_kelvin + ref_beta * log_volt)) - 273.15;

        // Apply the user calibration offset to a valid reading only, before fahrenheit/temperature are derived
        if (isfinite(temperature)) {
            temperature += Probe::offset_celcius;
        }
    }
    bool reading_connected = isfinite(temperature);

    // ADC noise around the threshold made an empty jack flip between connected and disconnected.
    // Only change state after a few readings in a row agree.
    if (reading_connected != Probe::connected) {
        Probe::state_change_readings++;
        if (Probe::state_change_readings >= READINGS_TO_CHANGE_STATE) {
            Probe::state_change_readings = 0;
            Probe::connected = reading_connected;
            if (Probe::connected){Probe::connected_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
        }
    } else {
        Probe::state_change_readings = 0;
    }

    if (!Probe::connected) {
        Probe::celcius     = 0;
        Probe::fahrenheit  = 32;
        Probe::temperature = 0;

        return nanf(""); // If disconnected return NaN
    }

    // Still connected but this reading was invalid, keep the last temperature
    if (!reading_connected) {
        return Probe::celcius;
    }

    // Store temp in public property
    Probe::celcius     = temperature;
    Probe::fahrenheit  = (temperature * 1.8) + 32;

    Probe::temperature = 0;

    bool unit_celcius, unit_fahrenheit;
    {
        SharedLock lock;    // temperature_unit can be changed by the web, mqtt or opengrill task
        unit_celcius    = config::temperature_unit == "celcius";
        unit_fahrenheit = config::temperature_unit == "fahrenheit";
    }

    if(unit_celcius){
        Probe::temperature = Probe::celcius;
    }

    if(unit_fahrenheit){
        Probe::temperature = Probe::fahrenheit;
    }

    return temperature;
}

void Probe::check_temperature_status(){

    Probe::alarm      = false;

    // If the target temperature is 0.0 we do not beep. This prevents Grilly+
    // from beeping every time you boot or when you connect a new probe
    if(Probe::connected && Probe::target_temperature != 0.0){

        //* Target temperature mode, a minimum of 0 or below means there is no range
        if(Probe::minimum_temperature <= 0.0){

            //* Ready temperature beep + alarm. has_beeped is also set when the alarm is switched off,
            //* so switching it on later doesn't sound an alarm for a target that was already reached.
            if(Probe::temperature >= Probe::target_temperature && Probe::has_beeped == false){
                Probe::has_beeped = true;
                Probe::alarm      = config::beep_on_ready;
            }

            //* Reset the alarm and beep if the temperature drops way too low
            if(Probe::temperature < (Probe::target_temperature - Probe::TEMP_HYSTERISIS_OFFSET) && Probe::has_beeped == true){
                Probe::has_beeped = false;
            }

            //* Only run if we need to beep before we reach the temperature
            if(config::beep_degrees_before > 0){

                //* Almost ready temperature beep
                if(Probe::temperature >= (Probe::target_temperature - config::beep_degrees_before) && Probe::has_beeped_before == false){
                    Probe::has_beeped_before = true;

                    // Played by the alarm task: beeping here blocked the probe readings for about
                    // 2.4 seconds and could cut off a beep the alarm task was playing
                    Probe::warn_before = true;
                }

                //* Reset the beep if the almost temperature drops way too low
                if(Probe::temperature < (Probe::target_temperature - config::beep_degrees_before - Probe::TEMP_HYSTERISIS_OFFSET) && Probe::has_beeped_before == true){
                    Probe::has_beeped_before = false;
                }
            }

        } else {
            //* Temperature range mode

            //* Alarm and Beep if outside
            if((Probe::temperature < Probe::minimum_temperature || Probe::temperature > Probe::target_temperature ) && Probe::has_beeped_outside == false){
                Probe::has_beeped_outside = true;
                Probe::alarm              = config::beep_outside_target;
            }

            //* Reset the beep and alarm if inside. The hysteresis is capped at a quarter of the range,
            //* with a full offset on both sides a narrow range (4 degrees or less) could never re-arm.
            float hysteresis = std::min(static_cast<float>(Probe::TEMP_HYSTERISIS_OFFSET), (Probe::target_temperature - Probe::minimum_temperature) / 4);
            if((Probe::temperature > (Probe::minimum_temperature + hysteresis) && Probe::temperature < (Probe::target_temperature - hysteresis) ) && Probe::has_beeped_outside == true){
                Probe::has_beeped_outside = false;
            }
        }
    }
}

void Probe::set_name(String probe_name){
    SharedLock lock;    // name is read by the screen, web, mqtt and opengrill tasks
    Probe::name = probe_name;
}

void Probe::set_offset(float offset_celcius){
    SharedLock lock;    // offset_celcius is read by the probes task on the next reading
    // A NaN offset makes every reading NaN, so the probe looks unplugged. Preferences returns NaN
    // for a missing key, which is how upgraded devices got it; a stored NaN heals on the next save.
    Probe::offset_celcius = isfinite(offset_celcius) ? offset_celcius : 0.0f;
}

void Probe::set_type(String probe_type, int reference_kohm, int reference_celcius, int reference_beta){
    SharedLock lock;    // type is read by the web, mqtt and opengrill tasks. A no-op for the global constructors.

    if(probe_type == "grilleye_iris"){
        Probe::reference_beta    = 4250;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 100;
        Probe::type              = "grilleye_iris";
        return;
    }

    if(probe_type == "ikea_fantast"){
        Probe::reference_beta    = 4250;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 230;
        Probe::type              = "ikea_fantast";
        return;
    }
    
    if(probe_type == "maverick_et733"){
        Probe::reference_beta    = 4250;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 200;
        Probe::type              = "maverick_et733";
        return;
    }

    if(probe_type == "weber_igrill"){
        Probe::reference_beta    = 3830;
        Probe::reference_celcius = 25;
        Probe::reference_kohm    = 100;
        Probe::type              = "weber_igrill";
        return;
    }
    
    Probe::reference_beta    = reference_beta;
    Probe::reference_celcius = reference_celcius;
    Probe::reference_kohm    = reference_kohm;
    Probe::type              = "custom";
    return;
}

void Probe::set_temperature(float target_temperature, float minimum_temperature){

    bool temperatures_changed = target_temperature != Probe::target_temperature || minimum_temperature != Probe::minimum_temperature;

    Probe::target_temperature = target_temperature;
    Probe::minimum_temperature = minimum_temperature;

    // Only re-enable the beeps when the temperatures change. Saving a probe without changing them
    // (renaming it, or a retained mqtt message after a reconnect) would otherwise replay an alarm
    // that already went off.
    if(!temperatures_changed){ return; }

    if(minimum_temperature <= 0.0){
        Probe::has_beeped        = false;
        Probe::has_beeped_before = false;
    } else {
        Probe::has_beeped_outside = false;
    }
}