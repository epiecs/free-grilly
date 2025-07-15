#pragma once
#include <Arduino.h>
#include <vector>
#include <utility>

#include "Config.h"

// ***********************************
// * Definitions
// ***********************************

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
    /**
    *	@brief Switches between the various pages
    *	@return true if screen page was switched
    */
   bool switch_page(void);
    /**
    *	@brief Show settings page
    *	@return true if screen page was switched
    */
   bool show_settings_page(void);



private:
    /**
    *	@brief Subroutine to update the thermometer display
    *   @param xLoc x axis base location
    *   @param yLoc y axis base location
    *	@return true 
    */
    bool draw_thermometer(int xLoc = 0, int yLoc = 0, int probe_temp = 0, int probe_target = 0);
    /**
    *	@brief Subroutine to update the temperature 
    *   @param connectedProbe probe number
    *	@return true 
    */
    bool draw_temp(int connectedProbe);
    /**
    *	@brief Subroutine to get the temperature 
    *   @param connectedProbe probe number
    *	@return true 
    */
    int get_target_temp(int connectedProbe);
        /**
    *	@brief Subroutine to get the temperature 
    *   @param connectedProbe probe number
    *	@return true 
    */
    int get_minimum_temp(int connectedProbe);
    /**
    *	@brief Subroutine to get the minimum temperature 
    *   @param connectedProbe probe number
    *	@return true 
    */
    float get_temp(int connectedProbe);
    /**
    *	@brief Checks how many probes are connected
    *	@return number of probes connected 
    *   @return vector of connected probes
    */
    std::pair<int, std::vector<int>> get_connected_probes(void);
    /**
    *	@brief Subroutine to draw the temp page
    *	@return true 
    */
    bool draw_screen_temp(void);
    /**
    *	@brief Subroutine to create the wifi screen
    *	@return true 
    */
    bool draw_screen_info(void);
    /**
    *	@brief enables / disables the screen background LED
    *	@return true 
    */
    bool screen_background_pwr(status_type type = ENABLE);
    /**
    *	@brief enables / disables the screen
    *	@return true 
    */
    bool screen_pwr(status_type type = ENABLE);
};

extern disp display;