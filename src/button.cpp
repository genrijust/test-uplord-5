#include "button.h"
#include "nvs.h"
#include "globals.h"
#include "wific.h"

void buttonTask(void*) {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    bool     pressed    = false;
    uint8_t  clickCount = 0;
    uint32_t pressMs    = 0;
    uint32_t releaseMs  = 0;
    bool     longFired  = false;

    for (;;) {
        bool down = !digitalRead(PIN_BUTTON);

        if (down && !pressed) {
            pressed   = true;
            pressMs   = millis();
            longFired = false;
        }

        if (pressed && !longFired && (millis() - pressMs >= LONG_PRESS_MS)) {
            longFired = true;
            BtnEvent_t ev = BTN_LONG;
            xQueueSend(btnQ, &ev, 0);
            clickCount = 0;
            Serial.println("BTN: long");
        }

        if (!down && pressed) {
            pressed = false;
            if (!longFired) {
                clickCount++;
                releaseMs = millis();
            }
        }

        if (clickCount > 0 && !pressed && (millis() - releaseMs > DOUBLE_CLICK_MS)) {
            BtnEvent_t ev;
            if (clickCount >= 2) {
                ev = BTN_DOUBLE;
                Serial.println("BTN: double");
            } else {
                ev = BTN_SHORT;
                Serial.println("BTN: short");
            }
            xQueueSend(btnQ, &ev, 0);
            clickCount = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
    }
}

void buttonHandlerTask(void*) {
    BtnEvent_t ev;
    for (;;) {
        if (xQueueReceive(btnQ, &ev, portMAX_DELAY) != pdTRUE) continue;

        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        if (ev == BTN_SHORT) {
            if (g_alarmActive) {
                // Turn off alarm
                BuzCmd buz = {BUZ_CMD_STOP, 0, 0};
                xQueueSend(buzQ, &buz, 0);
                g_alarmActive = false;
                Serial.println("BTN: alarm stopped by button");
            } else {
            g_cfg.ledMode = (g_cfg.ledMode + 1) % 5;
            Serial.printf("BTN: mode → %d\n", g_cfg.ledMode);
            LedCmd cmd = {LED_CMD_MODE, g_cfg.ledMode, 0, 0};
            xQueueSend(ledQ, &cmd, 0);
            }
        } else if (ev == BTN_DOUBLE) {
            static const uint8_t briSteps[] = {50, 100, 150, 200, 255};
            uint8_t next = 50;
            for (int i = 0; i < 4; i++) {
                if (g_cfg.brightness == briSteps[i]) { next = briSteps[i+1]; break; }
                if (i == 3) next = briSteps[0];
            }
            g_cfg.brightness = next;
            Serial.printf("BTN: brightness → %d\n", g_cfg.brightness);
            LedCmd cmd = {LED_CMD_BRIGHTNESS, g_cfg.brightness, 0, 0};
            xQueueSend(ledQ, &cmd, 0);
        } else if (ev == BTN_LONG) {
            saveSettings();
            uint8_t dummy = 0;
            xQueueSend(voltForceQ, &dummy, 0);
            g_webServerActive = true; 
                Serial.println("BTN: Long press - Web server ON");
                uint8_t apCmd = 0;
                xQueueSend(wifiCmdQ, &apCmd, 0);

            
            BuzCmd buz = {BUZ_CMD_TONE, 1000, 40};
            xQueueSend(buzQ, &buz, 0);
            vTaskDelay(pdMS_TO_TICKS(80));
            buz = {BUZ_CMD_STOP, 0, 0};
            xQueueSend(buzQ, &buz, 0);
            Serial.println("BTN: long – settings saved, voltage updated");
        }
        xSemaphoreGive(cfgMtx);
    }
}
