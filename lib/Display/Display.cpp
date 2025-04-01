#include <Arduino.h>
#include <U8g2lib.h>

#include "Display.h"
#include "Grill.h"

bool disp::init(void){
    u8g2.begin();
    u8g2.setContrast(20);
}

bool disp::display_update(void) {
    u8g2.clearBuffer(); //todo move to proper function
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(7, 8, "Free-Grilly");
    u8g2.setFont(u8g2_font_t0_22_tr);
    u8g2.setCursor(37, 39);
    u8g2.print(grill::probe_1.calculate_temperature());
    u8g2.sendBuffer();
	return true; 
}