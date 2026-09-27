#include <WebServer.h>

void not_found();

void get_index();
void get_touch_icon();
void redirect_to(const char* location);

void setup_web_routes();
