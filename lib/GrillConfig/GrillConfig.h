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
     * @brief Saves all preferences to the nvram. Re-initializes where needed.
     *
     */
    void save_settings();

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

private:
    /**
     * @brief Checks if the wifi needs to be restarted
     * 
     * @return true Wifi needs a restart
     * @return false Wifi does not need a restart
     */
    bool check_wifi_reload_needed();
    
    /**
     * @brief Checks if the local ap needs to be restarted
     * 
     * @return true local ap needs a restart
     * @return false local ap does not need a restart
     */
    bool check_local_ap_reload_needed();

};