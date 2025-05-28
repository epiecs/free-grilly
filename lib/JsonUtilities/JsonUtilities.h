#pragma once


struct jsonResult
{
    bool success;
    String message;
};

class JsonUtilities{

    public:
        void load_json_status(char *buffer);
        
        void load_json_settings(char *buffer);
        jsonResult save_json_settings(char* jsondata);
        
        void load_json_probes(char *buffer);
        jsonResult save_json_probes(char* jsondata);

        void load_json_wifiscan(char *buffer);
};