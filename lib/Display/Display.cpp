#include <Arduino.h>
#include <U8g2lib.h>

#include "Display.h"
#include "Grill.h"
#include "Probe.h"

U8G2_ST7565_64128N_F_4W_SW_SPI screen(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 

static const unsigned char battery_icon[]     U8X8_PROGMEM = {0xfe,0x07,0x01,0x08,0x01,0x38,0x01,0x28,0x01,0x28,0x01,0x38,0x01,0x08,0xfe,0x07};
static const unsigned char battery_charging[] U8X8_PROGMEM = {0x10,0x08,0x04,0x1e,0x0f,0x04,0x02,0x01};

bool is_critical_battery_flash = false;

disp::disp() {}

bool disp::init(void){
    screen.begin();
    screen.setContrast(15);
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

    if     (grill::battery_percentage >= 90)                                    { screen.drawBox(115, 1, 10, 6); }
    else if(grill::battery_percentage >= 80 and grill::battery_percentage < 90) { screen.drawBox(115, 1, 9 , 6); }
    else if(grill::battery_percentage >= 70 and grill::battery_percentage < 80) { screen.drawBox(115, 1, 8 , 6); }
    else if(grill::battery_percentage >= 60 and grill::battery_percentage < 70) { screen.drawBox(115, 1, 7 , 6); }
    else if(grill::battery_percentage >= 50 and grill::battery_percentage < 60) { screen.drawBox(115, 1, 6 , 6); }
    else if(grill::battery_percentage >= 40 and grill::battery_percentage < 50) { screen.drawBox(115, 1, 5 , 6); }
    else if(grill::battery_percentage >= 30 and grill::battery_percentage < 40) { screen.drawBox(115, 1, 4 , 6); }
    else if(grill::battery_percentage >= 20 and grill::battery_percentage < 30) { screen.drawBox(115, 1, 3 , 6); }
    else if(grill::battery_percentage >= 10 and grill::battery_percentage < 20) { screen.drawBox(115, 1, 2 , 6); }
    else if(grill::battery_percentage > 10){
        if(is_critical_battery_flash){
            screen.drawLine(115, 1, 115, 6);
            is_critical_battery_flash = false;
        }
        else {
            is_critical_battery_flash = true;
        }       
    }

    if(grill::battery_charging) {
        screen.drawXBMP(108, 0, 5,  8, battery_charging);
    }

    screen.setFont(u8g2_font_5x7_tr);
    screen.drawStr(7, 8, "Free-Grilly");
    screen.setFont(u8g2_font_t0_22_tr);
    screen.setCursor(37, 39);
    screen.print(grill::battery_percentage);
    screen.sendBuffer();
        
    
	return true; 
}

disp display;