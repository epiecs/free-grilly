#pragma once
#include <Arduino.h>


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
   bool draw_screen_temp(void);
    /**
    *	@brief Subroutine to create the wifi screen
    *	@return true 
    */
   bool draw_screen_info(void);
};

extern disp display;