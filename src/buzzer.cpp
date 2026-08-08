#include "buzzer.h"
#include <driver/ledc.h>

static const uint8_t BUZZER_LEDC_CHANNEL = 0;

void buzzerTask(void*) {
    ledcSetup(BUZZER_LEDC_CHANNEL, 2700, 10);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);

    static const uint16_t MELODY_NOTES[] = {262,294,330,349,392,440,494,523};
    static const uint16_t ASCENDING_NOTES[] = {500, 700, 900, 1100, 1300, 1500, 1700, 1900};
    static const uint16_t SIREN_NOTES[] = {900, 1300};
    #define MELODY_LEN 8
    #define ASCEND_LEN 8
    bool     melodyActive = false;
    bool     soundActive  = false;
    uint8_t  soundMode    = 0;
    uint8_t  soundStep    = 0;
    bool     soundOn      = true;
    uint32_t noteMs       = 0;
    uint16_t currentFreq  = 0;
    uint8_t  currentVol   = 0;

    BuzCmd cmd;
    for (;;) {
        TickType_t timeout = (melodyActive || soundActive) ? 0 : portMAX_DELAY;
        if (xQueueReceive(buzQ, &cmd, timeout) == pdTRUE) {
            switch (cmd.type) {
            case BUZ_CMD_TONE:
                melodyActive = false;
                soundActive  = false;
                currentFreq  = cmd.freq;
                currentVol   = cmd.volume;
                if (currentFreq == 0) {
                    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
                } else {
                    ledcWriteTone(BUZZER_LEDC_CHANNEL, currentFreq);
                    ledcWrite(BUZZER_LEDC_CHANNEL, (1023 * currentVol) / 100);
                }
                break;
            case BUZ_CMD_VOLUME:
                currentVol = cmd.volume;
                if (currentFreq > 0) {
                    ledcWriteTone(BUZZER_LEDC_CHANNEL, currentFreq);
                    ledcWrite(BUZZER_LEDC_CHANNEL, (1023 * currentVol) / 100);
                }
                break;
            case BUZ_CMD_STOP:
                melodyActive = false;
                soundActive  = false;
                currentFreq  = 0;
                ledcWrite(BUZZER_LEDC_CHANNEL, 0);
                break;
            case BUZ_CMD_MELODY:
                soundActive  = false;
                melodyActive = true;
                soundStep    = 0;
                noteMs       = millis();
                currentFreq  = MELODY_NOTES[0];
                xSemaphoreTake(cfgMtx, portMAX_DELAY);
                currentVol = g_cfg.buzVolume;
                xSemaphoreGive(cfgMtx);
                ledcWriteTone(BUZZER_LEDC_CHANNEL, currentFreq);
                ledcWrite(BUZZER_LEDC_CHANNEL, (1023 * currentVol) / 100);
                break;
            case BUZ_CMD_SOUND:
                melodyActive = false;
                soundActive  = true;
                soundMode    = cmd.freq;
                soundStep    = 0;
                soundOn      = true;
                noteMs       = millis();
                currentVol   = cmd.volume;
                break;
            }
        }

        uint32_t now = millis();
        if (melodyActive && now - noteMs >= 200) {
            soundStep = (soundStep + 1) % MELODY_LEN;
            noteMs    = now;
            currentFreq = MELODY_NOTES[soundStep];
            ledcWriteTone(BUZZER_LEDC_CHANNEL, currentFreq);
            ledcWrite(BUZZER_LEDC_CHANNEL, (1023 * currentVol) / 100);
        }

        if (soundActive && now - noteMs >= 200) {
            noteMs = now;
            soundStep++;
            switch (soundMode) {
            case 0:
                currentFreq = 0;
                break;
            case 1:
                currentFreq = ASCENDING_NOTES[soundStep % ASCEND_LEN];
                break;
            case 2:
                soundOn = !soundOn;
                currentFreq = soundOn ? 1000 : 0;
                break;
            case 3:
                currentFreq = SIREN_NOTES[soundStep % 2];
                break;
            case 4:
                currentFreq = SIREN_NOTES[soundStep % 2];
                break;
            default:
                currentFreq = 1000;
                break;
            }
            if (currentFreq == 0) {
                ledcWrite(BUZZER_LEDC_CHANNEL, 0);
            } else {
                ledcWriteTone(BUZZER_LEDC_CHANNEL, currentFreq);
                ledcWrite(BUZZER_LEDC_CHANNEL, (1023 * currentVol) / 100);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
