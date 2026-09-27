#include "Buzzer.h"

#include <Arduino.h>

#include "Config.h"
#include "Gpio.h"

Buzzer::Buzzer(){    
    ledcSetup(gpio::pwm_buzzer_channel, gpio::pwm_buzzer_frequency, gpio::pwm_buzzer_resolution);
}

void Buzzer::beep(int beeps_amount, int duration){
    ledcAttachPin(gpio::buzzer, gpio::pwm_buzzer_channel);
    
    for (int beep = 0; beep < beeps_amount; beep++){
        ledcWriteTone(gpio::pwm_buzzer_channel, Buzzer::beep_frequency);
        ledcWrite(gpio::pwm_buzzer_channel, Buzzer::volume);
        delay(duration);  
        ledcWriteTone(gpio::pwm_buzzer_channel, 0);
        ledcWrite(gpio::pwm_buzzer_channel, 0);
        delay(duration);  
    } 

    ledcDetachPin(gpio::buzzer);
}

void Buzzer::play_all_notes(){
    
    ledcAttachPin(gpio::buzzer, gpio::pwm_buzzer_channel);
    
    Serial.println(sizeof Buzzer::notes / sizeof Buzzer::notes[0]);

    for(int note = 0; note < 21; note++){
        Serial.println(Buzzer::notes[note]);

        ledcWriteTone(gpio::pwm_buzzer_channel, Buzzer::notes[note]);
        delay(300);
    }
    
    ledcDetachPin(gpio::buzzer);
}

void Buzzer::play_cucaracha(){

    ledcAttachPin(gpio::buzzer, gpio::pwm_buzzer_channel);

    for(int note = 0; note < 34; note++){
        ledcWriteTone(gpio::pwm_buzzer_channel, Buzzer::cucaracha_notes[note]);
        ledcWrite(gpio::pwm_buzzer_channel, Buzzer::volume);
        delay(Buzzer::cucaracha_durations[note]);  
        ledcWriteTone(gpio::pwm_buzzer_channel, 0);
        ledcWrite(gpio::pwm_buzzer_channel, 0);
        delay(Buzzer::cucaracha_wait[note]);   
    }

    ledcDetachPin(gpio::buzzer);
}

// Only sets the buzzer's own volume. config::beep_volume is the user's setting and stays as it is
// when beeps are disabled, so it is still there when they are enabled again.
void Buzzer::set_volume(int volume){

    if(config::beep_enabled == false){
        volume = 0;
    }

    if(volume < 0 ){
        Buzzer::volume = 0;
        return;
    }

    if(volume > 5 ){
        Buzzer::volume = 250;
        return;
    }

    Buzzer::volume = volume * 50;   
}