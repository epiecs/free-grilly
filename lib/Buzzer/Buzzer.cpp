#include "Buzzer.h"

#include <Arduino.h>

#include "Gpio.h"

Buzzer::Buzzer(){    
    ledcSetup(Buzzer::channel, Buzzer::frequency, Buzzer::resolution);
}

void Buzzer::beep(int beeps_amount, int duration){
    ledcAttachPin(gpio::buzzer, channel);
    ledcWrite(Buzzer::channel, Buzzer::volume);

    for (int beep = 0; beep < beeps_amount; beep++){
        ledcWriteTone(Buzzer::channel, Buzzer::beep_frequency);
        delay(duration);  
        ledcWriteTone(Buzzer::channel, 0);
        delay(duration);  
    } 

    ledcDetachPin(gpio::buzzer);
}

void Buzzer::play_all_notes(){
    
    ledcAttachPin(gpio::buzzer, channel);
    
    Serial.println(sizeof Buzzer::notes / sizeof Buzzer::notes[0]);

    for(int note = 0; note < 21; note++){
        Serial.println(Buzzer::notes[note]);

        ledcWriteTone(Buzzer::channel, Buzzer::notes[note]);
        delay(300);
    }
    
    ledcDetachPin(gpio::buzzer);
}

void Buzzer::set_volume(int volume){

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