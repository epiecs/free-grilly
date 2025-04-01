#include <Arduino.h>
#include <U8g2lib.h>

#include "Display.h"
#include "Grill.h"
#include "Probe.h"

U8G2_ST7565_64128N_F_4W_SW_SPI screen(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 

static const unsigned char battery_icon[] U8X8_PROGMEM = {0xfe,0x07,0x01,0x08,0x01,0x38,0x01,0x28,0x01,0x28,0x01,0x38,0x01,0x08,0xfe,0x07};

disp::disp() {}

bool disp::init(void){
    screen.begin();
    screen.setContrast(20);
    return true;
}

bool disp::display_update(void) {
    screen.clearBuffer(); 
    screen.setFontMode(1);
    screen.setBitmapMode(1);
     
    // ***********************************
    // * Battery elements
    // ***********************************
    screen.drawXBMP(114, 0, 14, 8, battery_icon);

    screen.setFont(u8g2_font_5x7_tr);
    screen.drawStr(7, 8, "Free-Grilly");
    screen.setFont(u8g2_font_t0_22_tr);
    screen.setCursor(37, 39);
    screen.print(grill::probe_1.calculate_temperature());
    screen.sendBuffer();
        
    
	return true; 
}

disp display;