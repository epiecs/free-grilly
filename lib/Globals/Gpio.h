namespace gpio{

    extern int power_button;

    extern int power_probes;
    extern int power_screen_backlight;
    extern int power_adc_circuit;
    
    extern int buzzer;

    extern int battery_i2c_sda;
    extern int battery_i2c_scl;

    extern int hspi_probes_sclk;
    extern int hspi_probes_cs;
    extern int hspi_probes_miso;

    extern int mux_selector_a;
    extern int mux_selector_b;
    extern int mux_selector_c;

    extern int pwm_screen_channel;                                
    extern int pwm_screen_frequency;
    extern int pwm_screen_resolution;

    extern int pwm_buzzer_channel;                                
    extern int pwm_buzzer_frequency;
    extern int pwm_buzzer_resolution;
}

