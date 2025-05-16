#pragma once
#include <Arduino.h>
#include <vector>
#include <utility>


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
    *	@brief Subroutine to create the temp screen
    *	@return true 
    */
   std::pair<int, std::vector<int>> get_connected_probes(void);
   /**
   *	@brief Checks how many probes are connected
   *	@return number of probes connected 
   *    @return vector of connected probes
   */
   bool draw_screen_temp(void);
    /**
    *	@brief Subroutine to create the wifi screen
    *	@return true 
    */
   bool draw_screen_info(void);
};

extern disp display;