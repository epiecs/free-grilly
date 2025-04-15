#include <Arduino.h>
#include <U8g2lib.h>

#include "Display.h"
#include "Grill.h"
#include "Probe.h"

U8G2_ST7565_64128N_F_4W_SW_SPI screen(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 

static const unsigned char battery_icon[]       U8X8_PROGMEM = {0xfe,0x07,0x01,0x08,0x01,0x38,0x01,0x28,0x01,0x28,0x01,0x38,0x01,0x08,0xfe,0x07};
static const unsigned char battery_charging[]   U8X8_PROGMEM = {0x10,0x08,0x04,0x1e,0x0f,0x04,0x02,0x01};
static const unsigned char wifi_signal1[]       U8X8_PROGMEM = {0x01,0x01};
static const unsigned char wifi_signal2[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01};
static const unsigned char wifi_signal3[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01,0x01,0x01};
static const unsigned char wifi_signal4[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
static const unsigned char wifi_disconnected[]  U8X8_PROGMEM = {0x05,0x02,0x05};

bool is_critical_battery_flash = false;

disp::disp() {}

bool disp::init(void){
    screen.begin();
    screen.setContrast(20);
    screen.setFontMode(1);
    screen.setBitmapMode(1);
    return true;
}

bool disp::display_update(void) {
    screen.clearBuffer(); 
     
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
    
    // ***********************************
    // * Wifi Elements
    // ***********************************
    if(grill::wifi_connected) {
     
    }
    else {
        screen.drawXBMP(89, 1, 3, 3, wifi_disconnected);
        screen.drawXBMP(90, 6, 1, 2, wifi_signal1);
        screen.drawXBMP(92, 4, 1, 4, wifi_signal2);
        screen.drawXBMP(94, 2, 1, 6, wifi_signal3);
        screen.drawXBMP(96, 0, 1, 8, wifi_signal4);
    }

    // ***********************************
    // * Probe elements
    // ***********************************
    // Probe numbers
    screen.setFont(u8g2_font_5x8_tr);
    screen.drawStr(2, 20, "1:");
    screen.drawStr(2, 33, "2:");
    screen.drawStr(2, 46, "3:");
    screen.drawStr(2, 59, "4:");
    screen.drawLine(63, 13, 63, 63);
    screen.drawStr(67, 20, "5:");
    screen.drawStr(67, 33, "6:");
    screen.drawStr(67, 46, "7:");
    screen.drawStr(67, 59, "8:");

    // Probe values
    screen.setFont(u8g2_font_profont12_tr);
    screen.setCursor(15, 21); screen.print(grill::probe_1.celcius);
    screen.setCursor(15, 34); screen.print(grill::probe_2.celcius);
    screen.setCursor(15, 47); screen.print(grill::probe_3.celcius);
    screen.setCursor(15, 60); screen.print(grill::probe_4.celcius);
    screen.setCursor(80, 21); screen.print(grill::probe_5.celcius);
    screen.setCursor(80, 34); screen.print(grill::probe_6.celcius);
    screen.setCursor(80, 47); screen.print(grill::probe_7.celcius);
    screen.setCursor(80, 60); screen.print(grill::probe_8.celcius);

    screen.sendBuffer();
        
    
	return true; 
}

disp display;