#pragma once

#include <Preferences.h>


class GrillConfig
{

public:

    Preferences preferences;

    // Settings
    void initialize_settings();
    void load_settings();
    void save_settings();
    void print_settings();
    
    // Probes
    void initialize_probes();
    void load_probes();
    void save_probes();
    void print_probes();

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