#include "Arduino.h"
#include "globals.h"
#include "button.h"
#include "led_strip.h"
#include "buzzer.h"
#include "nvs.h"
#include "voltage.h"
#include "time_sync.h"
#include "wific.h"
#include "web.h"
#include "status_led.h"


void wifiTask(void*) {
    for (;;) {
        wifiPoll();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
//dkslkdmasldk fdytdtrdtrdtrdgfc
void webTask(void*) {
    
    g_webServerActive = true;
    g_lastWebActivityMs = millis();
    for (;;) {
        if (g_webServerActive) {
            if (!serverRunning) {  
                serverOn();
                serverRunning = true;
                g_lastWebActivityMs = millis();
            }
            webHandleClient();
            handleCaptivePortal();

            if (millis() - g_lastWebActivityMs > WEB_INACTIVITY_TIMEOUT_MS) {
                Serial.println("WEB: Inactivity timeout - server OFF");
                serverOff();
                apStop();
                serverRunning = false;
                g_webServerActive = false;
            }
        } else {
            if (serverRunning) {
                serverOff();
                serverRunning = false;
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void ntpTask(void*) {
    xEventGroupWaitBits(wifiEG, EG_WIFI_CONNECTED, pdFALSE, pdTRUE, portMAX_DELAY);
    timeSync();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(NTP_INTERVAL_MS));
        timeSync();
    }
}

void alarmTask(void*) {
    bool alarmActive = false;
    for (;;) {
        time_t ep = localEpochNow();
        if (ep != 0) {
            int h, m, s;
            epochToHMS(ep, h, m, s);

            xSemaphoreTake(cfgMtx, portMAX_DELAY);
            bool enabled = g_cfg.alarmEnabled;
            uint8_t ah = g_cfg.alarmHour;
            uint8_t am = g_cfg.alarmMin;
            uint8_t sound = g_cfg.alarmSound;
            xSemaphoreGive(cfgMtx);

            bool shouldRing = enabled && h == ah && m == am && s < 58;
            if (shouldRing) {
                if (!alarmActive) {
                    BuzCmd buz = {BUZ_CMD_SOUND, sound, g_cfg.buzVolume};
                    xQueueSend(buzQ, &buz, 0);
                    g_alarmActive = true;
                    alarmActive = true;
                }
            } else if (alarmActive) {
                BuzCmd buz = {BUZ_CMD_STOP, 0, 0};
                xQueueSend(buzQ, &buz, 0);
                g_alarmActive = false;
                alarmActive = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(false);
    delay(200);
    Serial.println("\n=== RGB CLOCK v4.1 (FreeRTOS) ===");

    loadSettings();
    saveSettings();


    cfgMtx      = xSemaphoreCreateMutex();
    timeMtx     = xSemaphoreCreateMutex();
    voltMtx     = xSemaphoreCreateMutex();
    btnQ        = xQueueCreate(4, sizeof(BtnEvent_t));
    ledQ        = xQueueCreate(8, sizeof(LedCmd));
    buzQ        = xQueueCreate(8, sizeof(BuzCmd));
    voltForceQ  = xQueueCreate(2, sizeof(uint8_t));
    wifiEG      = xEventGroupCreate();
    wifiCmdQ    = xQueueCreate(4, sizeof(uint8_t));

    g_tm.valid = false;

    xTaskCreate(buttonTask,        "btn",        2048, NULL, 4, NULL);
    xTaskCreate(buttonHandlerTask, "btnHdl",     2048, NULL, 3, NULL);
    xTaskCreate(wifiTask,          "wifi",       4096, NULL, 5, NULL);
    xTaskCreate(statusLedTask,     "statLed",    1024, NULL, 2, NULL);
    xTaskCreate(ledStripTask,      "strip",      4096, NULL, 2, NULL);
    xTaskCreate(buzzerTask,        "buz",        2048, NULL, 2, NULL);
    xTaskCreate(alarmTask,         "alarm",      2048, NULL, 3, NULL);
    xTaskCreate(voltageTask,       "volt",       2048, NULL, 1, NULL);
    xTaskCreate(ntpTask,           "ntp",        4096, NULL, 3, NULL);
    xTaskCreate(webTask,           "web",        8192, NULL, 3, NULL);

    Serial.println("INIT: all tasks created");
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}

