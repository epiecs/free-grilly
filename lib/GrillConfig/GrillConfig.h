#pragma once

#include <Preferences.h>


class GrillConfig
{

public:

    Preferences preferences;

    /**
     * @brief Initializes all settings from the nvram.
     *
     */
    
    void initialize_settings();
    /**
     * @brief Loads all preferences from the nvram.
     *
     */
    void load_settings();

    /**
     * @brief Prints out the current stored config
     * 
     */
    void print();

    /**
    * @brief Factory resets the nvram and clears all settings
    *
    */
    void factory_reset();

};