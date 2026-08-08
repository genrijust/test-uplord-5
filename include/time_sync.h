#ifndef RGB_CLOCK_TIME_SYNC_H
#define RGB_CLOCK_TIME_SYNC_H

#include "globals.h"

time_t localEpochNow();
void epochToHMS(time_t ep, int &h, int &m, int &s);
void epochToYMD(time_t ep, int &y, int &mo, int &d);
void timeSync();

#endif // RGB_CLOCK_TIME_SYNC_H
