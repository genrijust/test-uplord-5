#include "status_led.h"

void statusLedTask(void*) {
    pinMode(PIN_STATUS_LED, OUTPUT);
    bool ledOn = false;

    for (;;) {
        EventBits_t bits = xEventGroupGetBits(wifiEG);
        uint32_t period = 1000;
        bool solid = false;

        if (bits & EG_WIFI_CONNECTED) {
            solid = true;
        } else if (bits & EG_WIFI_CONNECTING) {
            period = 500;
        } else if (bits & EG_WIFI_AP) {
            period = 200;
        }

        if (solid) {
            digitalWrite(PIN_STATUS_LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ledOn = !ledOn;
        digitalWrite(PIN_STATUS_LED, ledOn ? HIGH : LOW);
        vTaskDelay(pdMS_TO_TICKS(period));
    }
}
