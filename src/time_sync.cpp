#include "time_sync.h"

#include <time.h>

void updateTimeState(time_t ep) {
    xSemaphoreTake(timeMtx, portMAX_DELAY);
    g_tm.lastSyncEpoch  = ep;
    g_tm.lastSyncMillis = millis();
    g_tm.valid          = true;
    xSemaphoreGive(timeMtx);
}

time_t localEpochNow() {
    TimeState tm;
    xSemaphoreTake(timeMtx, portMAX_DELAY);
    tm = g_tm;
    xSemaphoreGive(timeMtx);
    if (!tm.valid) return 0;
    uint32_t elapsed = millis() - tm.lastSyncMillis;
    return tm.lastSyncEpoch + (time_t)(elapsed / 1000UL);
}

void epochToHMS(time_t ep, int &h, int &m, int &s) {
    ep += (time_t)g_cfg.tzOffsetMin * 60;
    s = ep % 60; ep /= 60;
    m = ep % 60; ep /= 60;
    h = ep % 24;
}

void epochToYMD(time_t ep, int &y, int &mo, int &d) {
    ep += (time_t)g_cfg.tzOffsetMin * 60;
    struct tm info;
    gmtime_r(&ep, &info);   // ep already shifted by tz offset, so treat as UTC calendar
    y  = info.tm_year + 1900;
    mo = info.tm_mon + 1;
    d  = info.tm_mday;
}

void timeSync() {
    Serial.println("NTP: WiFi up, starting sync...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm info = {};
    int tries = 0;
    while (!getLocalTime(&info, 0) && tries++ < 30) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (getLocalTime(&info, 0)) {
        time_t ep = mktime(&info);
        updateTimeState(ep);
        int h, m, s;
        epochToHMS(ep, h, m, s);
        Serial.printf("NTP: synced  local=%02d:%02d:%02d\n", h, m, s);
    } else {
        Serial.println("NTP: first sync failed");
    }
}
