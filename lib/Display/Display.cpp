#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>
#include <utility>
#include <WiFi.h>

#include "Display.h"
#include "Grill.h"
#include "Probe.h"
#include "Config.h"
#include "Power.h"


U8G2_ST7565_64128N_F_4W_SW_SPI screen(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 
bool is_critical_battery_flash          = false;
int notification_offset                 = 0;
int current_screen_page                 = 0;
String current_active_name              = "";
float current_active_temp               = 0;
int current_target_temp                 = 0;
int current_minimum_temp                = 0;
unsigned long millis_backlight_timeout  = 0;
unsigned long millis_screen_timeout     = 0;

static const unsigned char battery_icon[]       U8X8_PROGMEM = {0xfe,0x03,0x01,0x04,0x01,0x0c,0x01,0x0c,0x01,0x04,0xfe,0x03};
static const unsigned char battery_charging[]   U8X8_PROGMEM = {0x3d,0xe5,0x25,0x26,0xe4,0x3c};
static const unsigned char wifi_disconnected[]  U8X8_PROGMEM = {0x05,0x02,0x05};
static const unsigned char probe_thermometer[]  U8X8_PROGMEM = {0x04,0x0a,0x0a,0x1b,0x0a,0x0a,0x0a,0x11,0x11,0x11,0x0e};
static const unsigned char volume_5[]           U8X8_PROGMEM = {0x48,0x8c,0xaf,0xaf,0x8c,0x48};
static const unsigned char volume_4[]           U8X8_PROGMEM = {0x08,0x8c,0xaf,0xaf,0x8c,0x08};
static const unsigned char volume_3[]           U8X8_PROGMEM = {0x08,0x0c,0xaf,0xaf,0x0c,0x08};
static const unsigned char volume_2[]           U8X8_PROGMEM = {0x08,0x0c,0x2f,0x2f,0x0c,0x08};
static const unsigned char volume_1[]           U8X8_PROGMEM = {0x08,0x0c,0x2f,0x2f,0x0c,0x08};
static const unsigned char volume_mute[]        U8X8_PROGMEM = {0x08,0xac,0x4f,0xaf,0x0c,0x08};
static const unsigned char degrees[]            U8X8_PROGMEM = {0x02,0x05,0x02};
static const unsigned char temp_low[]           U8X8_PROGMEM = {0x80,0x00,0xa0,0x02,0xc8,0x09,0x8c,0x18,0x90,0x04,0xa2,0x22,0xc4,0x11,0xff,0x7f,0xc4,0x11,0xa2,0x22,0x90,0x04,0x8c,0x18,0xc8,0x09,0xa0,0x02,0x80,0x00};
static const unsigned char temp_high[]          U8X8_PROGMEM = {0x44,0x04,0x88,0x08,0x88,0x08,0x88,0x08,0x44,0x04,0x22,0x02,0x11,0x01,0x11,0x01,0x11,0x01,0x22,0x02};



disp::disp() {}

bool disp::init(void){
    screen.begin();
    screen.setContrast(22);
    screen.setFontMode(1);
    screen.setBitmapMode(1);
    screen_background_pwr(ENABLE);
    return true;
}

bool disp::switch_page(void){
    if (current_screen_page >= 1) {current_screen_page = 0;}
    screen_background_pwr(ENABLE);
    screen_pwr(ENABLE);
    return true;
}

bool disp::show_settings_page(void){
    current_screen_page = 10;
    screen_background_pwr(ENABLE);
    screen_pwr(ENABLE);
    return true;
}

bool disp::screen_background_pwr(status_type type){
    switch (type) {
	case ENABLE:
        power.setScreenBrightness(config::backlight_brightness);
        millis_backlight_timeout = millis();
        break;
    case DISABLE:
        power.setScreenBrightness(0);
        break;
    }
    return true;
}

bool disp::screen_pwr(status_type type){
    switch (type) {
	case ENABLE:
        screen.setPowerSave(ENABLE);
        millis_screen_timeout = millis();
        break;
    case DISABLE:
        screen.setPowerSave(DISABLE);
        break;
    }
    return true;
}


bool disp::display_update(void) {
    if (config::backlight_timeout_minutes > 0 and millis_backlight_timeout + (config::backlight_timeout_minutes * 60000) < millis()) {
        screen_background_pwr(DISABLE);
    }
    if (config::screen_timeout_minutes > 0 and millis_screen_timeout + (config::screen_timeout_minutes * 60000) < millis()) {
        screen_pwr(DISABLE);
        return true; 
    }

    screen.clearBuffer(); 
    screen.drawLine(2, 7, 125, 7);
     
    // ***********************************
    // * Battery elements
    // ***********************************
    if (grill::battery_percentage < 10){
        if (is_critical_battery_flash){
            is_critical_battery_flash = false;
            screen.drawXBMP(114, 0, 12, 6, battery_icon);
        }
        else { is_critical_battery_flash = true; }       
    }
    else {
        screen.drawXBMP(114, 0, 12, 6, battery_icon);
        if (grill::battery_percentage >= 80) { screen.drawLine(122, 2, 122, 3); }
        if (grill::battery_percentage >= 60) { screen.drawLine(120, 2, 120, 3); }
        if (grill::battery_percentage >= 40) { screen.drawLine(118, 2, 118, 3); }
        if (grill::battery_percentage >= 10) { screen.drawLine(116, 2, 116, 3); }
    }

    if(grill::battery_charging) {
        screen.drawXBMP(105, 0, 8,  6, battery_charging);
        notification_offset = 9;
    }
    else {notification_offset = 0;}
    
    // ***********************************
    // * Wifi Elements
    // ***********************************
    if(grill::wifi_connected) {
        if (WiFi.RSSI() > -90) {screen.drawLine(101 - notification_offset, 4, 101 - notification_offset, 5);}
        if (WiFi.RSSI() > -80) {screen.drawLine(103 - notification_offset, 3, 103 - notification_offset, 5);}
        if (WiFi.RSSI() > -70) {screen.drawLine(105 - notification_offset, 2, 105 - notification_offset, 5);}
        if (WiFi.RSSI() > -60) {screen.drawLine(107 - notification_offset, 1, 107 - notification_offset, 5);}
        if (WiFi.RSSI() > -50) {screen.drawLine(109 - notification_offset, 0, 109 - notification_offset, 5);}
    }
    else {
        screen.drawLine(100 - notification_offset, 2, 102 - notification_offset, 0); // cross
        screen.drawLine(100 - notification_offset, 0, 102 - notification_offset, 2); // cross
        screen.drawLine(101 - notification_offset, 4, 101 - notification_offset, 5);
        screen.drawLine(103 - notification_offset, 3, 103 - notification_offset, 5);
        screen.drawLine(105 - notification_offset, 2, 105 - notification_offset, 5);
        screen.drawLine(107 - notification_offset, 1, 107 - notification_offset, 5);
        screen.drawLine(109 - notification_offset, 0, 109 - notification_offset, 5);
    }

    // ***********************************
    // * Volume Elements
    // ***********************************
    if (config::beep_volume == 5)       { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_5);}
    else if (config::beep_volume == 4)  { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_4);}
    else if (config::beep_volume == 3)  { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_3);}
    else if (config::beep_volume == 2)  { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_2);}
    else if (config::beep_volume == 1)  { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_1);}
    else if (config::beep_volume == 0)  { screen.drawXBMP(88 - notification_offset, 0, 8, 6, volume_mute);}


    switch (current_screen_page) {
    case 0:
        draw_screen_temp();
        break;
    case 1:
        draw_screen_temp();
        break;
    case 10:
        draw_screen_info();
        break;
    default:
        draw_screen_temp();
        break;
    }    

    screen.sendBuffer();
	return true; 
}

bool disp::draw_screen_temp(void){
    // ***********************************
    // * Temp unit Elements
    // ***********************************
    screen.setFont(u8g2_font_4x6_tr);
    if (config::temperature_unit == "celcius"){screen.drawXBMP(2, 1, 3, 3, degrees); screen.drawUTF8(6, 6, "C");}
    else                                      {screen.drawXBMP(2, 1, 3, 3, degrees); screen.drawUTF8(6, 6, "F");}

    std::pair<int, std::vector<int>> connectedProbeInfo = get_connected_probes();
    switch (connectedProbeInfo.first) {
    case 0:
        screen.setFont(u8g2_font_profont15_tr);
        screen.drawLine(31, 20, 96, 20);
        screen.drawStr(36, 47, "Detected");
        screen.drawStr(33, 33, "No probes");
        screen.drawLine(31, 49, 96, 49);
        break;
    case 1:
        // variables
        current_active_name = get_name(connectedProbeInfo.second[0]);
        current_active_temp = get_temp(connectedProbeInfo.second[0]);
        current_minimum_temp = get_minimum_temp(connectedProbeInfo.second[0]);
        current_target_temp = get_target_temp(connectedProbeInfo.second[0]);

        // probe name
        screen.setFont(u8g2_font_profont12_tr);
        screen.drawStr(3, 20, "P :");
        screen.setCursor(9, 20); screen.print(connectedProbeInfo.second[0]);
        screen.setCursor(22, 20); screen.print(current_active_name);
        
        // probe temp 
        screen.setFont(u8g2_font_profont29_tr); 
        screen.setCursor(3, 42); screen.printf("%.1f", current_active_temp);      

        // status text
        screen.setFont(u8g2_font_profont10_tr); 
        screen.drawStr(3, 53, "00:00");
        screen.drawStr(3, 62, "PLACEHOLDER STATUS 2");
       
        // progress bar
        if (current_minimum_temp > 0 && current_target_temp > current_minimum_temp) 
            draw_progress_range(current_minimum_temp, current_target_temp, 121);
        if (current_target_temp > 0 && current_minimum_temp == 0) 
            draw_progress_target(current_minimum_temp, current_target_temp, 121);
        break;
        
    case 2:
        // divider 
        screen.drawLine(0, 36, 127, 36);

        for (int i = 0; i <= 1; i++){
            // variables
            int y_offset = 0;
            current_active_name = get_name(connectedProbeInfo.second[i]);
            current_active_temp = get_temp(connectedProbeInfo.second[i]);
            current_minimum_temp = get_minimum_temp(connectedProbeInfo.second[i]);
            current_target_temp = get_target_temp(connectedProbeInfo.second[i]);

            if(i == 0) { y_offset = 16;}
            if(i == 1) { y_offset = 45;}

            // probe name
            screen.setFont(u8g2_font_profont10_tr); 
            screen.drawStr(2, y_offset, "P :");
            screen.setCursor(7, y_offset); screen.print(connectedProbeInfo.second[i]);
            screen.setCursor(18, y_offset); screen.print(current_active_name);

            // status text
            screen.drawStr(102, y_offset, "00:00");

            // probe temp 
            screen.setFont(u8g2_font_profont22_tr); 
            screen.setCursor(2, y_offset+18); screen.printf("%.1f", current_active_temp); 

            // progress status
            screen.setFont(u8g2_font_profont11_tr);
            if (current_minimum_temp <= 0 and current_target_temp > 0 and current_active_temp > current_target_temp){
                screen.drawStr(125-screen.getStrWidth("READY"), y_offset+17, "READY");
                screen.setDrawColor(2);
                screen.drawBox(95, y_offset+9, 31, 9);
                screen.setDrawColor(1);
            }
            else {
                if(current_target_temp > 0 and current_active_temp > current_target_temp) {
                    screen.drawStr(125-screen.getStrWidth("HIGH"), y_offset+17, "HIGH");
                    screen.setDrawColor(2);
                    screen.drawBox(101, y_offset+9, 25, 9);
                    screen.setDrawColor(1);
                    }
                else if (current_active_temp < current_minimum_temp){
                    screen.drawStr(125-screen.getStrWidth("LOW"), y_offset+17, "LOW");
                    screen.setDrawColor(2);
                    screen.drawBox(107, y_offset+9, 19, 9);
                    screen.setDrawColor(1);
                    }
                }
        }
        break;
    /*case 3:
        screen.drawLine(42, 9, 42, 63);
        screen.drawLine(84, 9, 84, 63);
        for (int i = 0; i <= 2; i++){
            screen.setFont(u8g2_font_profont10_tr);
            if(i == 0) { screen.setCursor(28, 17);}
            if(i == 1) { screen.setCursor(61, 17);}
            if(i == 2) { screen.setCursor(104, 17);}
            screen.print(connectedProbeInfo.second[i]);
            screen.setFont(u8g2_font_profont12_tr);  
            if(i == 0) { screen.setCursor(3, 33);}
            if(i == 1) { screen.setCursor(46, 33);}
            if(i == 2) { screen.setCursor(89, 33);}
            draw_temp(connectedProbeInfo.second[i]);
        }
        break;
    case 4:
        screen.drawLine(63, 9, 63, 63);
        screen.drawLine(0, 36, 126, 36);
        for (int i = 0; i <= 3; i++){
            screen.setFont(u8g2_font_profont10_tr);
            if(i == 0) { screen.setCursor(28, 17);}
            if(i == 1) { screen.setCursor(92, 17);}
            if(i == 2) { screen.setCursor(28, 46);}
            if(i == 3) { screen.setCursor(92, 46);}
            screen.print(connectedProbeInfo.second[i]);
            screen.setFont(u8g2_font_profont15_tr);  
            if(i == 0) { screen.setCursor(12, 34);}
            if(i == 1) { screen.setCursor(76, 34);}
            if(i == 2) { screen.setCursor(12, 63);}
            if(i == 3) { screen.setCursor(76, 63);}
            draw_temp(connectedProbeInfo.second[i]);
        }
        break;
    */
    default:
        if(connectedProbeInfo.first <= 4) {
            // divider 
            screen.drawLine(64, 8, 64, 63);
            screen.drawLine(0, 36, 127, 36);

            for (int i = 0; i <= connectedProbeInfo.first-1; i++){
                int y_offset = 0;
                int x_offset = 0;
                current_active_name = get_name(connectedProbeInfo.second[i]);
                current_active_temp = get_temp(connectedProbeInfo.second[i]);
                current_minimum_temp = get_minimum_temp(connectedProbeInfo.second[i]);
                current_target_temp = get_target_temp(connectedProbeInfo.second[i]);

                if(i == 0) { x_offset = 2; y_offset = 16;}
                if(i == 1) { x_offset = 67; y_offset = 16;}
                if(i == 2) { x_offset = 2; y_offset = 45;}
                if(i == 3) { x_offset = 67; y_offset = 45;}
                
                
                
                // progress status
                screen.setFont(u8g2_font_profont10_tr); 
                screen.drawStr(x_offset, y_offset, "P :");
                screen.setCursor(x_offset + 5, y_offset); screen.print(connectedProbeInfo.second[i]);

                if (current_minimum_temp <= 0 and current_target_temp > 0 and current_active_temp > current_target_temp){
                    screen.drawStr(x_offset+16, y_offset, "READY");
                    screen.setDrawColor(2);
                    screen.drawBox(x_offset+15, y_offset-7, 26, 8);
                    screen.setDrawColor(1);
                }
                else {
                    if(current_target_temp > 0 and current_active_temp > current_target_temp) {
                        screen.drawStr(x_offset+16, y_offset, "HIGH");
                        screen.setDrawColor(2);
                        screen.drawBox(x_offset+15, y_offset-7, 21, 8);
                        screen.setDrawColor(1);
                    }
                    else if (current_active_temp < current_minimum_temp){
                        screen.drawStr(x_offset+16, y_offset, "LOW");
                        screen.setDrawColor(2);
                        screen.drawBox(x_offset+15, y_offset-7, 16, 8);
                        screen.setDrawColor(1);
                    }
                    else {
                        // probe name
                        screen.setCursor(x_offset + 16, y_offset); screen.print(current_active_name);  
                    }
                }
                
                
                // probe temp 
                screen.setFont(u8g2_font_profont22_tr); 
                screen.setCursor(x_offset, y_offset+18); screen.printf("%.1f", current_active_temp); 
            }        
        }
        else {
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
        break;
        }

    }
    return true;
}

bool disp::draw_screen_info(void){
    screen.setFont(u8g2_font_4x6_tr);
    screen.drawStr(2, 6, "Info");

    screen.setFont(u8g2_font_4x6_tr);
    // Grill info
    screen.drawStr(10, 18, "NAME:");
    screen.setCursor(33, 18); screen.print(config::grill_name);
    screen.drawStr(18, 26, "FW: ");
    screen.setCursor(33, 26); screen.print(config::grill_firmware_version);
    // AP info
    screen.drawStr(10, 36, "SSID:");
    screen.setCursor(33, 36); screen.print(config::wifi_ssid);
    screen.drawStr(18, 44, "IP: ");
    screen.setCursor(33, 44); screen.print(config::wifi_ip);
    // local AP info
    screen.drawStr(2, 54, "L-SSID:");
    screen.setCursor(33, 54); screen.print(config::local_ap_ssid);
    screen.drawStr(10, 62, "L-IP: ");
    screen.setCursor(33, 62); screen.print(config::local_ap_ip);

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

bool disp::draw_progress_range(int current_minimum_temp, int current_target_temp, int x_Loc) {
    screen.drawFrame(x_Loc, 10, 7, 53);
    screen.drawLine(x_Loc - 1, 21, x_Loc - 3, 21);
    int target_offset = 0;
    if (current_target_temp > 99) {target_offset = 4;}
    screen.setFont(u8g2_font_4x6_tr); screen.setCursor(x_Loc - 11 - target_offset, 24); screen.print(current_target_temp);
    screen.drawLine(x_Loc - 1, 41, x_Loc - 3, 41);
    target_offset = 0;
    if (current_minimum_temp > 99) {target_offset = 4;}
    screen.setFont(u8g2_font_4x6_tr); screen.setCursor(x_Loc - 11 - target_offset, 44); screen.print(current_minimum_temp);

    int range_offset = 20+(20*((current_active_temp-current_minimum_temp)/(current_target_temp-current_minimum_temp)));
    if (range_offset > 50) {range_offset = 50;} if (range_offset < 1) {range_offset = 0;}
    screen.drawBox(x_Loc+1, 60 - range_offset, 5, 2 + range_offset);

    return true;
}

bool disp::draw_progress_target(int current_minimum_temp, int current_target_temp, int x_Loc) {
        screen.drawFrame(x_Loc, 10, 7, 53);

        screen.drawLine(x_Loc - 1, 21, x_Loc - 3, 21);
        int target_offset = 0;
        if (current_target_temp > 99) {target_offset = 4;}
        screen.setFont(u8g2_font_4x6_tr); screen.setCursor(x_Loc - 11 - target_offset, 24); screen.print(current_target_temp);

        int range_offset = 40*(current_active_temp/current_target_temp);
        if (range_offset > 50) {range_offset = 50;}
        screen.drawBox(x_Loc +1 , 60 - range_offset, 5, 2 + range_offset);
    return true;
}



bool disp::draw_temp(int connectedProbe) {
    if(config::temperature_unit == "celcius"){
        switch (connectedProbe) {
            case 1: screen.print(grill::probe_1.celcius); break;
            case 2: screen.print(grill::probe_2.celcius); break;
            case 3: screen.print(grill::probe_3.celcius); break;
            case 4: screen.print(grill::probe_4.celcius); break;
            case 5: screen.print(grill::probe_5.celcius); break;
            case 6: screen.print(grill::probe_6.celcius); break;
            case 7: screen.print(grill::probe_7.celcius); break;
            case 8: screen.print(grill::probe_8.celcius); break;
        default:
            break;
        }
    }
    else {
        switch (connectedProbe) {
            case 1: screen.print(grill::probe_1.fahrenheit); break;
            case 2: screen.print(grill::probe_2.fahrenheit); break;
            case 3: screen.print(grill::probe_3.fahrenheit); break;
            case 4: screen.print(grill::probe_4.fahrenheit); break;
            case 5: screen.print(grill::probe_5.fahrenheit); break;
            case 6: screen.print(grill::probe_6.fahrenheit); break;
            case 7: screen.print(grill::probe_7.fahrenheit); break;
            case 8: screen.print(grill::probe_8.fahrenheit); break;
        default:
            break;
        }
    }
    return true;
} 

String disp::get_name(int connectedProbe) {
    String name = "";
    switch (connectedProbe) {
        case 1: name = grill::probe_1.name; break;
        case 2: name = grill::probe_2.name; break;
        case 3: name = grill::probe_3.name; break;
        case 4: name = grill::probe_4.name; break;
        case 5: name = grill::probe_5.name; break;
        case 6: name = grill::probe_6.name; break;
        case 7: name = grill::probe_7.name; break;
        case 8: name = grill::probe_8.name; break;
    default:
        break;
    }
    return name;
}

float disp::get_temp(int connectedProbe) {
    float temp = 0;
    if(config::temperature_unit == "celcius"){
        switch (connectedProbe) {
            case 1: temp = grill::probe_1.celcius; break;
            case 2: temp = grill::probe_2.celcius; break;
            case 3: temp = grill::probe_3.celcius; break;
            case 4: temp = grill::probe_4.celcius; break;
            case 5: temp = grill::probe_5.celcius; break;
            case 6: temp = grill::probe_6.celcius; break;
            case 7: temp = grill::probe_7.celcius; break;
            case 8: temp = grill::probe_8.celcius; break;
        default:
            break;
        }
    }
    else {
        switch (connectedProbe) {
            case 1: temp = grill::probe_1.fahrenheit; break;
            case 2: temp = grill::probe_2.fahrenheit; break;
            case 3: temp = grill::probe_3.fahrenheit; break;
            case 4: temp = grill::probe_4.fahrenheit; break;
            case 5: temp = grill::probe_5.fahrenheit; break;
            case 6: temp = grill::probe_6.fahrenheit; break;
            case 7: temp = grill::probe_7.fahrenheit; break;
            case 8: temp = grill::probe_8.fahrenheit; break;
        default:
            break;
        }
    }
    return temp;
} 

int disp::get_target_temp(int connectedProbe) {
    int temp = 0;
        switch (connectedProbe) {
            case 1: temp = grill::probe_1.target_temperature; break;
            case 2: temp = grill::probe_2.target_temperature; break;
            case 3: temp = grill::probe_3.target_temperature; break;
            case 4: temp = grill::probe_4.target_temperature; break;
            case 5: temp = grill::probe_5.target_temperature; break;
            case 6: temp = grill::probe_6.target_temperature; break;
            case 7: temp = grill::probe_7.target_temperature; break;
            case 8: temp = grill::probe_8.target_temperature; break;
        default:
            break;
        }
    return temp;
}

int disp::get_minimum_temp(int connectedProbe) {
    int temp = 0;
        switch (connectedProbe) {
            case 1: temp = grill::probe_1.minimum_temperature; break;
            case 2: temp = grill::probe_2.minimum_temperature; break;
            case 3: temp = grill::probe_3.minimum_temperature; break;
            case 4: temp = grill::probe_4.minimum_temperature; break;
            case 5: temp = grill::probe_5.minimum_temperature; break;
            case 6: temp = grill::probe_6.minimum_temperature; break;
            case 7: temp = grill::probe_7.minimum_temperature; break;
            case 8: temp = grill::probe_8.minimum_temperature; break;
        default:
            break;
        }
    return temp;
}

std::pair<int, std::vector<int>> disp::get_connected_probes(void) {
    std::vector<int> connectedProbes;
    int connectedCount = 0;

    if(grill::probe_1.connected) { connectedProbes.push_back(1); connectedCount++; }
    if(grill::probe_2.connected) { connectedProbes.push_back(2); connectedCount++; }
    if(grill::probe_3.connected) { connectedProbes.push_back(3); connectedCount++; }
    if(grill::probe_4.connected) { connectedProbes.push_back(4); connectedCount++; }
    if(grill::probe_5.connected) { connectedProbes.push_back(5); connectedCount++; }
    if(grill::probe_6.connected) { connectedProbes.push_back(6); connectedCount++; }
    if(grill::probe_7.connected) { connectedProbes.push_back(7); connectedCount++; }
    if(grill::probe_8.connected) { connectedProbes.push_back(8); connectedCount++; }
    return {connectedCount, connectedProbes};
}

disp display;