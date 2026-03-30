#pragma once

class Buzzer
{

private:
    int volume          = 5;                                      // Volume 0-5
    
    // DOREMIFASOLASIDO notes low/mid/high in Hz
    int notes[21]       = {262, 294, 330, 370, 392, 440, 494,     // falt(low) 0-6
                           523, 587, 659, 698, 784, 880, 988,     // natural(middle) 7-13
                           1047,1175,1319,1397,1568,1760,1967};   // sharp(high) 14-20

    int cucaracha_notes[34]      = {523, 523, 523, 698, 880,
                                    523, 523, 523, 698, 880,
                                    698, 698, 659, 659, 587, 587, 523,
                                    523, 523, 523, 659, 784,
                                    523, 523, 523, 659, 784,
                                    1047, 1175, 1047, 932, 880, 784, 698};

    int cucaracha_durations[34]  = {150, 150, 150, 300, 600,
                                    150, 150, 150, 300, 600,
                                    150, 150, 150, 150, 150, 150, 600,
                                    150, 150, 150, 300, 600,
                                    150, 150, 150, 300, 600,
                                    150, 150, 150, 150, 150, 150, 600};

    int cucaracha_wait[34]       = {50,  50,  50,  50,  300,
                                    50,  50,  50,  50,  300,
                                    50,  50,  50,  50,  50,  50,  400,
                                    50,  50,  50,  50,  300,
                                    50,  50,  50,  50,  300,
                                    50,  50,  50,  50,  50,  50,  400};

    //int channel         = 2;                                      // The starting PWM channel
    //int frequency       = 100;                                    // The starting frequency
    //int resolution      = 8;                                      // 8 bits resolution from 0-255
                                                                  // used for volume control

    int beep_frequency  = 2500;                                   // Frequency used for beeps

public:

    Buzzer();

    /**
     * @brief Give a few short beeps
     * 
     * @param amount The amount of beeps
     * @param duration How long a beep should last
     */
    void beep(int beeps_amount = 3, int duration = 300);

    /**
     * @brief Set the volume for the buzzer. 
     * 
     * @param volume from 0 to 5
     */
    void set_volume(int volume = 5);

    /**
     * @brief Plays all notes
     * 
     */
    void play_all_notes();

    /**
     * @brief Plays La Cucaracha song (easter egg)
     * 
     */
    void play_cucaracha();
};