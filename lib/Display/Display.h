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

private:
    /**
    *	@brief Subroutine to update the thermome display
    *   @param xLoc x axis base location
    *   @param yLoc y axis base location
    *	@return true 
    */
    bool draw_thermometer(int xLoc = 0, int yLoc = 0, int probe_temp = 0, int probe_target = 0);
};

extern disp display;