#include "led_strip.h"
#include "time_sync.h"

void renderClock(bool animate) {
    time_t ep = localEpochNow();
    if (ep == 0) {
        uint8_t p = (uint8_t)((millis() / 6) & 0xFF);
        strip.fill(strip.Color(p >> 1, p >> 4, 0));
        strip.show();
        return;
    }

    int h, m, s;
    epochToHMS(ep, h, m, s);

    static int prevDigits[6]   = {-1, -1, -1, -1, -1, -1};
    static int animDigits[6]   = {0, 0, 0, 0, 0, 0};
    static int targetDigits[6] = {0, 0, 0, 0, 0, 0};
    static int animDir[6]      = {0, 0, 0, 0, 0, 0};
    static bool animActive[6]  = {false, false, false, false, false, false};
    static bool firstRender    = true;
    static uint32_t lastAnimMs  = 0;
    const uint32_t ANIM_STEP_MS = 100;

    xSemaphoreTake(cfgMtx, portMAX_DELAY);
    bool use24 = g_cfg.use24Hour;
    xSemaphoreGive(cfgMtx);

    int h12 = h;
    if (!use24 && h12 > 12) {
        h12 -= 12;
    } else if (!use24 && h12 == 0) {
        h12 = 12;
    }

    int currDigits[6] = {s % 10, s / 10, m % 10, m / 10, h12 % 10, h12 / 10};

    if (firstRender) {
        for (int i = 0; i < 6; i++) {
            prevDigits[i] = currDigits[i];
            animDigits[i] = currDigits[i];
            targetDigits[i] = currDigits[i];
            animDir[i]    = 0;
            animActive[i] = false;
        }
        firstRender = false;
        lastAnimMs = millis();
    }
   
    bool anyAnimating = false;

     if (animate)
    {
    for (int i = 0; i < 6; i++) {
        if (prevDigits[i] != currDigits[i]) {
            if (!animActive[i]) {
                targetDigits[i] = currDigits[i];
                animDir[i] = (currDigits[i] > prevDigits[i]) ? 1 : -1;
                animActive[i] = true;
                animDigits[i] = prevDigits[i];
            }
        }
        if (animActive[i]) anyAnimating = true;
    }

    if (anyAnimating && millis() - lastAnimMs >= ANIM_STEP_MS) {
        lastAnimMs += ANIM_STEP_MS;
        anyAnimating = false;
        for (int i = 0; i < 6; i++) {
            if (!animActive[i]) continue;

            if (animDir[i] > 0) {
                animDigits[i] = (animDigits[i] + 1) % 10;
            } else {
                animDigits[i] = (animDigits[i] == 0) ? 9 : animDigits[i] - 1;
            }

            if (animDigits[i] == targetDigits[i]) {
                animActive[i] = false;
                prevDigits[i] = targetDigits[i];
            } else {
                anyAnimating = true;
            }
        }
    }
    
        if (!anyAnimating) {
            for (int i = 0; i < 6; i++) {
                prevDigits[i] = currDigits[i];
                animDigits[i] = currDigits[i];
                animActive[i] = false;
            }
        }
    }else{    
        for (int i = 0; i < 6; i++) {
            prevDigits[i]   = currDigits[i];
            animDigits[i]   = currDigits[i];
            targetDigits[i] = currDigits[i];
            animActive[i]   = false;
        }
    }
    uint32_t cSec  = strip.Color(g_cfg.secR,  g_cfg.secG,  g_cfg.secB);
    uint32_t cMin  = strip.Color(g_cfg.minR,  g_cfg.minG,  g_cfg.minB);
    uint32_t cHour = strip.Color(g_cfg.hourR, g_cfg.hourG, g_cfg.hourB);

    if (g_alarmActive) {
        bool blink = ((millis() / 250) & 1) == 0;   // fast blink

        uint32_t red  = strip.Color(255, 0, 0);
        uint32_t blue = strip.Color(0, 0, 255);

        strip.clear();

        // Police effect on seconds LEDs (0-19)
        for (int i = 0; i < 20; i++) {           // Seconds area
            if (i < 10) {
                strip.setPixelColor(i, blink ? red : blue);   // First group (0-9)
            } else {
                strip.setPixelColor(i, blink ? blue : red);   // Second group (10-19)
            }
        }

        // Keep normal colors for minutes and hours
        int h, m, s;
        epochToHMS(ep, h, m, s);

        uint32_t cMin  = strip.Color(g_cfg.minR,  g_cfg.minG,  g_cfg.minB);
        uint32_t cHour = strip.Color(g_cfg.hourR, g_cfg.hourG, g_cfg.hourB);

        strip.setPixelColor(20 + (m % 10), cMin);
        strip.setPixelColor(30 + (m / 10), cMin);
        strip.setPixelColor(40 + (h % 10), cHour);
        strip.setPixelColor(50 + (h / 10), cHour);

        strip.show();
        return;   // Important: skip normal clock rendering
    }

    strip.clear();
    strip.setPixelColor( 0 + animDigits[0], cSec);
    strip.setPixelColor(10 + animDigits[1], cSec);
    strip.setPixelColor(20 + animDigits[2], cMin);
    strip.setPixelColor(30 + animDigits[3], cMin);
    strip.setPixelColor(40 + animDigits[4], cHour);
    strip.setPixelColor(50 + animDigits[5], cHour);
    strip.show();
}

void renderDate() {
    time_t ep = localEpochNow();
    if (ep == 0) return;

    int y, mo, d;
    epochToYMD(ep, y, mo, d);
    int yy = y % 100;   // last two digits of the year

    uint32_t cDay   = strip.Color(g_cfg.dayR,   g_cfg.dayG,   g_cfg.dayB);
    uint32_t cMonth = strip.Color(g_cfg.monthR, g_cfg.monthG, g_cfg.monthB);
    uint32_t cYear  = strip.Color(g_cfg.yearR,  g_cfg.yearG,  g_cfg.yearB);

    strip.clear();
    strip.setPixelColor( 0 + (d  % 10), cDay);     // 0-9   day, 1st digit
    strip.setPixelColor(10 + (d  / 10), cDay);     // 10-19 day, 10th digit
    strip.setPixelColor(20 + (mo % 10), cMonth);   // 20-29 month, 1st digit
    strip.setPixelColor(30 + (mo / 10), cMonth);   // 30-39 month, 10th digit
    strip.setPixelColor(40 + (yy % 10), cYear);    // 40-49 year, 1st digit
    strip.setPixelColor(50 + (yy / 10), cYear);    // 50-59 year, 10th digit
    strip.show();
}

void renderPattern(uint8_t mode, uint8_t R, uint8_t G, uint8_t B) {
    static uint16_t hue      = 0;
    static int      pos      = 0;
    static int      pulseVal = 0;
    static int      pulseDelta = 3;

    switch (mode) {
    case 1:
        strip.fill(strip.Color(R, G, B));
        break;
    case 2:
        for (int i = 0; i < NUM_LEDS; i++)
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue + (uint16_t)(i * 65536L / NUM_LEDS))));
        hue += 200;
        break;
    case 3:
        pos = (pos + 1) % NUM_LEDS;
        strip.clear();
        for (int i = 0; i < 5; i++)
            strip.setPixelColor((pos + i) % NUM_LEDS, strip.Color(R, G, B));
        break;
    case 4:
        pulseVal += pulseDelta;
        if (pulseVal >= 255 || pulseVal <= 0) pulseDelta = -pulseDelta;
        pulseVal = constrain(pulseVal, 0, 255);
        strip.fill(strip.Color((uint16_t)R * pulseVal / 255,
                               (uint16_t)G * pulseVal / 255,
                               (uint16_t)B * pulseVal / 255));
        break;
    
    }
    strip.show();
}

void ledStripTask(void*) {
    strip.begin();
    strip.setBrightness(50);
    strip.show();

    uint32_t lastStep = 0;
    int step = 0;
    const uint32_t STEP_MS = 18;
    bool done = false;
    while (!done) {
        if (millis() - lastStep >= STEP_MS) {
            lastStep = millis();
            if (step < NUM_LEDS) {
                strip.setPixelColor(step, strip.Color(0, 80, 0));
                strip.show();
            } else if (step < NUM_LEDS * 2) {
                strip.setPixelColor(step - NUM_LEDS, 0);
                strip.show();
            } else {
                done = true;
            }
            step++;
        }
        vTaskDelay(1);
    }

    uint32_t lastFrameMs = 0;
    LedCmd cmd;
    for (;;) {
        while (xQueueReceive(ledQ, &cmd, 0) == pdTRUE) {
            xSemaphoreTake(cfgMtx, portMAX_DELAY);
            switch (cmd.type) {
            case LED_CMD_MODE:       g_cfg.ledMode     = cmd.v1; break;
            case LED_CMD_COLOR:      g_cfg.ledR = cmd.v1; g_cfg.ledG = cmd.v2; g_cfg.ledB = cmd.v3; break;
            case LED_CMD_BRIGHTNESS: g_cfg.brightness  = cmd.v1; break;
            case LED_CMD_SPEED:      g_cfg.speed       = cmd.v1; break;
            }
            xSemaphoreGive(cfgMtx);
        }

        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        uint8_t mode  = g_cfg.ledMode;
        uint8_t bri   = g_cfg.brightness;
        uint8_t spd   = g_cfg.speed;
        uint8_t R     = g_cfg.ledR;
        uint8_t G     = g_cfg.ledG;
        uint8_t B     = g_cfg.ledB;
    const uint32_t  DATE_EVERY_MS = (uint32_t)g_cfg.dateIntervalSec * 1000;
        xSemaphoreGive(cfgMtx);


        bool isClockMode = (mode == LED_MODE_CLOCK1 || mode == LED_MODE_CLOCK2);
        uint32_t frameMs = isClockMode ? CLOCK_FRAME_MS : map(spd, 1, 10, PATTERN_MAX_MS, PATTERN_MIN_MS);

       // uint32_t frameMs = (mode == 0) ? CLOCK_FRAME_MS: map(spd, 1, 10, PATTERN_MAX_MS, PATTERN_MIN_MS);
        if (millis() - lastFrameMs >= frameMs) {
            lastFrameMs = millis();
            strip.setBrightness(bri);
            if (mode == 0) {
                static uint32_t lastSwitchMs  = millis();
                static bool     showingDate   = false;
               
                const uint32_t  DATE_FOR_MS   = 3000;   // ...for 3s

                uint32_t now = millis();
                if (!showingDate && now - lastSwitchMs >= DATE_EVERY_MS) {
                    showingDate  = true;
                    lastSwitchMs = now;
                } else if (showingDate && now - lastSwitchMs >= DATE_FOR_MS) {
                    showingDate  = false;
                    lastSwitchMs = now;
                }

                if (showingDate)
                    renderDate();
                else
                    renderClock(mode == LED_MODE_CLOCK2);
            } else {
                renderPattern(mode, R, G, B);
            }
        }
        vTaskDelay(1);
    }
}
