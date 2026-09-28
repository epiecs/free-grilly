#pragma once

#include <Preferences.h>

class Mqtt;
class Preferences;

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

    // Diagnostics
    // Records why the device is about to go off/restart on purpose, so grill::last_off_reason can
    // show it after the next boot. reason must be <= 15 chars (NVS key "off_reason"). Do not call
    // this for the "woken but button not held long enough" boot path, that would overwrite the
    // reason that is actually interesting to see.
    void save_off_reason(const char* reason);

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