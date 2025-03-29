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
    * @brief Factory resets the nvram and clears all settings
    *
    */
    void factory_reset();

};