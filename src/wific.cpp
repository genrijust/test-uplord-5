#include "wific.h"
#include <WiFi.h>
#include <Arduino.h>
#include "globals.h"


void startCaptivePortal() {
    dnsServer.start(53, "*", WiFi.softAPIP());   // Redirect all DNS queries
    Serial.println("CAPTIVE: DNS portal started");
}

void handleCaptivePortal() {
    dnsServer.processNextRequest();
}

void apStart() {
    WiFi.mode(WIFI_AP_STA);
    char apSsid[33];
    xSemaphoreTake(cfgMtx, portMAX_DELAY);
    strlcpy(apSsid, g_cfg.apSsid, sizeof(apSsid));
    xSemaphoreGive(cfgMtx);
    if (strlen(apSsid) == 0) {
        Serial.println("apssid == 0"); 
        WiFi.softAP("RGB_Clock");
    }else{
    WiFi.softAP(apSsid);}
    xEventGroupClearBits(wifiEG, EG_WIFI_CONNECTING | EG_WIFI_CONNECTED);
    xEventGroupSetBits(wifiEG, EG_WIFI_AP);
    Serial.printf("WIFI: AP mode started SSID=%s\n", apSsid);

    startCaptivePortal();
}

void apStop() {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    xEventGroupClearBits(wifiEG, EG_WIFI_AP);
    Serial.println("WIFI: AP mode stopped");
}

void staStart() {
    char ssid[33], pass[65];
    xSemaphoreTake(cfgMtx, portMAX_DELAY);
    strlcpy(ssid, g_cfg.staSsid, sizeof(ssid));
    strlcpy(pass, g_cfg.staPass, sizeof(pass));
    xSemaphoreGive(cfgMtx);
    Serial.printf("WIFI: connecting to '%s'\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    xEventGroupSetBits(wifiEG, EG_WIFI_CONNECTING);
}

void wifiPoll() {
    static WiFiState state = WiFiState::WS_IDLE;
    uint8_t cmd;

    if (xQueueReceive(wifiCmdQ, &cmd, 0) == pdTRUE) {
        if (cmd == 0) {
            apStart();
            state = WiFiState::WS_AP_MODE;
        }
    }

    switch (state) {
    case WiFiState::WS_IDLE:
        staStart();
        
        state = WiFiState::WS_CONNECTING;
        break;
    case WiFiState::WS_CONNECTING:
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("WIFI: STA connected  IP=%s\n", WiFi.localIP().toString().c_str());
            xEventGroupClearBits(wifiEG, EG_WIFI_CONNECTING);
            xEventGroupSetBits(wifiEG, EG_WIFI_CONNECTED);
            state = WiFiState::WS_STA_OK;
        }
        break;
    case WiFiState::WS_STA_OK:
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WIFI: lost connection, retrying...");
            xEventGroupClearBits(wifiEG, EG_WIFI_CONNECTED);
            state = WiFiState::WS_IDLE;
        }
        break;
    case WiFiState::WS_AP_MODE:
        break;
    }
}
