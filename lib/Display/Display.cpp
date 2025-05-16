#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>
#include <utility>

#include "Display.h"
#include "Grill.h"
#include "Probe.h"
#include "Config.h"

U8G2_ST7565_64128N_F_4W_SW_SPI screen(U8G2_R2, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); 
bool is_critical_battery_flash = false;
int current_screen_page = 0;

static const unsigned char battery_icon[]       U8X8_PROGMEM = {0xfe,0x07,0x01,0x08,0x01,0x38,0x01,0x28,0x01,0x28,0x01,0x38,0x01,0x08,0xfe,0x07};
static const unsigned char battery_charging[]   U8X8_PROGMEM = {0x10,0x08,0x04,0x1e,0x0f,0x04,0x02,0x01};
static const unsigned char wifi_signal1[]       U8X8_PROGMEM = {0x01,0x01};
static const unsigned char wifi_signal2[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01};
static const unsigned char wifi_signal3[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01,0x01,0x01};
static const unsigned char wifi_signal4[]       U8X8_PROGMEM = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
static const unsigned char wifi_disconnected[]  U8X8_PROGMEM = {0x05,0x02,0x05};
static const unsigned char probe_thermometer[]  U8X8_PROGMEM = {0x04,0x0a,0x0a,0x1b,0x0a,0x0a,0x0a,0x11,0x11,0x11,0x0e};


disp::disp() {}

bool disp::init(void){
    screen.begin();
    screen.setContrast(10);
    screen.setFontMode(1);
    screen.setBitmapMode(1);
    return true;
}

bool disp::switch_page(void){
    current_screen_page++;
    if (current_screen_page >= 1) {current_screen_page = 0;}
    return true;
}

bool disp::show_settings_page(void){
    current_screen_page = 10;
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
        if(grill::wifi_signal < 50) {screen.drawXBMP(96, 0, 1, 8, wifi_signal4);}
        if(grill::wifi_signal < 56) {screen.drawXBMP(94, 2, 1, 6, wifi_signal3);}
        if(grill::wifi_signal < 66) {screen.drawXBMP(92, 4, 1, 4, wifi_signal2);}
        if(grill::wifi_signal < 75) {screen.drawXBMP(90, 6, 1, 2, wifi_signal1);}
    }
    else {
        screen.drawXBMP(89, 1, 3, 3, wifi_disconnected);
        screen.drawXBMP(90, 6, 1, 2, wifi_signal1);
        screen.drawXBMP(92, 4, 1, 4, wifi_signal2);
        screen.drawXBMP(94, 2, 1, 6, wifi_signal3);
        screen.drawXBMP(96, 0, 1, 8, wifi_signal4);
    }

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
        screen.setFont(u8g2_font_profont10_tr);
        screen.setCursor(62, 17); screen.print(connectedProbeInfo.second[0]);
        screen.setFont(u8g2_font_profont29_tr);  
        screen.setCursor(28, 45);
        switch (connectedProbeInfo.second[0]) {
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
        break;
    case 2:
        screen.drawLine(63, 9, 63, 63);
        for (int i = 0; i <= 1; i++){
            screen.setFont(u8g2_font_profont10_tr);
            if(i == 0) { screen.setCursor(26, 17);}
            if(i == 1) { screen.setCursor(92, 17);}
            screen.print(connectedProbeInfo.second[i]);
            screen.setFont(u8g2_font_profont15_tr);  
            if(i == 0) { screen.setCursor(12, 34);}
            if(i == 1) { screen.setCursor(76, 34);}
            switch (connectedProbeInfo.second[i]) {
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
        break;
    case 3:
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
            switch (connectedProbeInfo.second[i]) {
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
            switch (connectedProbeInfo.second[i]) {
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
        break;
    
    default:
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
    return true;
}

bool disp::draw_screen_info(void){
    screen.setFont(u8g2_font_5x8_tr);
    screen.drawStr(2, 8, "Info");
    screen.drawLine(2, 9, 124, 9);

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