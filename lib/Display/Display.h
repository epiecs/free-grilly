#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// ***********************************
// * Definitions
// ***********************************

U8G2_ST7565_64128N_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 

class disp {
public:
    disp();
    /**
    *	@brief Initializes the display
    *	@return true if screen was updated
    */
    bool init(void);
    /**
    *	@brief Updates the display
    *	@return true if screen was updated
    */
    bool display_update(void);
};

extern disp display;