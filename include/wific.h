#ifndef RGB_CLOCK_WIFI_H
#define RGB_CLOCK_WIFI_H

#include "globals.h"

void apStart();
void apStop();
void staStart();
void wifiPoll();
void startCaptivePortal();
void handleCaptivePortal();

#endif // RGB_CLOCK_WIFI_H
