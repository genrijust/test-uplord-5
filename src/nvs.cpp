#include "nvs.h"

void loadSettings() {
    prefs.begin(NVS_NS, true);
    size_t n = prefs.getBytes(NVS_KEY, &g_cfg, sizeof(g_cfg));
    prefs.end();

    if (n != sizeof(g_cfg)) {
        strlcpy(g_cfg.staSsid,  DEF_STA_SSID, sizeof(g_cfg.staSsid));
        strlcpy(g_cfg.staPass,  DEF_STA_PASS,  sizeof(g_cfg.staPass));
        strlcpy(g_cfg.apSsid,   DEF_AP_SSID,   sizeof(g_cfg.apSsid));
        strlcpy(g_cfg.apPass,   DEF_AP_PASS,   sizeof(g_cfg.apPass));
        g_cfg.ledMode     = 0;
        g_cfg.ledR = 255; g_cfg.ledG = 255; g_cfg.ledB = 255;
        g_cfg.brightness  = 120;
        g_cfg.speed       = 5;
        g_cfg.secR  =   0; g_cfg.secG  =   0; g_cfg.secB  = 200;
        g_cfg.minR  =   0; g_cfg.minG  = 200; g_cfg.minB  =   0;
        g_cfg.hourR = 200; g_cfg.hourG =   0; g_cfg.hourB =   0;
        g_cfg.dayR   =   0; g_cfg.dayG   = 180; g_cfg.dayB   = 180;
        g_cfg.monthR = 180; g_cfg.monthG =   0; g_cfg.monthB = 180;
        g_cfg.yearR  = 180; g_cfg.yearG  = 130; g_cfg.yearB  =   0;
        g_cfg.buzFreq     = 0;
        g_cfg.buzVolume   = 70;
        g_cfg.tzOffsetMin = 330;
        g_cfg.alarmEnabled = false;
        g_cfg.alarmHour    = 7;
        g_cfg.alarmMin     = 0;
        g_cfg.alarmSound   = 0;
        g_cfg.use24Hour    = true;
        g_cfg.dateIntervalSec = 20;
        Serial.println("CFG: first boot defaults applied");
    } else {
        Serial.println("CFG: loaded from NVS");
    }
}

void saveSettings() {
    prefs.begin(NVS_NS, false);
    prefs.putBytes(NVS_KEY, &g_cfg, sizeof(g_cfg));
    prefs.end();
}
