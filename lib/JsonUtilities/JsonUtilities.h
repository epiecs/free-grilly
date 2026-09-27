#pragma once

class Preferences;

struct jsonResult
{
    bool success;
    String message;
    bool unauthorized;      // Rejected because the current admin password was needed (api answers 401)
};

class JsonUtilities{

    public:
        void load_json_status(char *buffer);

        void load_json_settings(char *buffer);
        // admin_password can only be changed with admin_authorized (the current admin password was
        // given, or none is set), and never from mqtt
        jsonResult save_json_settings(char* jsondata, bool admin_authorized, bool from_mqtt = false);

        void load_json_probes(char *buffer);
        jsonResult save_json_probes(char* jsondata);

        void load_opengrill_grill(char *buffer);
        jsonResult save_opengrill_grill(char* jsondata);

        void load_opengrill_probes(char *buffer);
        jsonResult save_opengrill_probes(char* jsondata);

        void load_json_wifiscan(char *buffer);
};
