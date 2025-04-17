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
static const unsigned char probe_thermometer[]  U8X8_PROGMEM = {0x04,0x0a,0x0a,0x1b,0x0a,0x0a,0x0a,0x11,0x11,0x11,0x0e};

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
    screen.drawLine(63, 9, 63, 63);
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

    // Thermometer
    if(grill::probe_1.target_temperature > 0) { draw_thermometer(54,11, grill::probe_1.celcius,grill::probe_1.target_temperature); }
    if(grill::probe_2.target_temperature > 0) { draw_thermometer(54,24, grill::probe_2.celcius,grill::probe_2.target_temperature); }
    if(grill::probe_3.target_temperature > 0) { draw_thermometer(54,37, grill::probe_3.celcius,grill::probe_3.target_temperature); }
    if(grill::probe_4.target_temperature > 0) { draw_thermometer(54,50, grill::probe_4.celcius,grill::probe_4.target_temperature); }
    if(grill::probe_5.target_temperature > 0) { draw_thermometer(119,11,grill::probe_5.celcius,grill::probe_5.target_temperature); }
    if(grill::probe_6.target_temperature > 0) { draw_thermometer(119,24,grill::probe_6.celcius,grill::probe_6.target_temperature); }
    if(grill::probe_7.target_temperature > 0) { draw_thermometer(119,37,grill::probe_7.celcius,grill::probe_7.target_temperature); }
    if(grill::probe_8.target_temperature > 0) { draw_thermometer(119,50,grill::probe_8.celcius,grill::probe_8.target_temperature); }


    screen.sendBuffer();
        
	return true; 
}

bool disp::draw_thermometer(int xLoc, int yLoc, int probe_temp, int probe_target) {
    screen.drawXBMP(xLoc, yLoc, 5, 11, probe_thermometer);
    if(probe_temp >= probe_target - 30) { screen.drawLine(xLoc+1, yLoc+9, xLoc+3, yLoc+9); }
    if(probe_temp >= probe_target - 25) { screen.drawLine(xLoc+1, yLoc+8, xLoc+3, yLoc+8); }
    if(probe_temp >= probe_target - 20) { screen.drawLine(xLoc+1, yLoc+7, xLoc+3, yLoc+7); }
    if(probe_temp >= probe_target - 15) { screen.drawLine(xLoc+2, yLoc+6, xLoc+2, yLoc+6); }
    if(probe_temp >= probe_target - 10) { screen.drawLine(xLoc+2, yLoc+5, xLoc+2, yLoc+5); }
    if(probe_temp >= probe_target - 5 ) { screen.drawLine(xLoc+2, yLoc+4, xLoc+2, yLoc+4); }
    if(probe_temp >= probe_target - 0 ) { screen.drawLine(xLoc+2, yLoc+3, xLoc+2, yLoc+3); }
    if(probe_temp >= probe_target + 5 ) { screen.drawLine(xLoc+2, yLoc+2, xLoc+2, yLoc+2); }
    if(probe_temp >= probe_target + 10) { screen.drawLine(xLoc+2, yLoc+1, xLoc+2, yLoc+1); }
    return true;
} 

disp display;