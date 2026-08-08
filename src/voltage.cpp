#include "voltage.h"

bool readVoltageOnce() {
    int voltage_mv = 0;
    for (int attempt = 0; attempt < 3; attempt++) {
        voltage_mv = analogReadMilliVolts(PIN_VOLTAGE);
        if (voltage_mv == 0) {
        } else {
            float vpin = voltage_mv / 1000.0f;
            float vsys = vpin * VSYS_RATIO;
            xSemaphoreTake(voltMtx, portMAX_DELAY);
            g_vsys = vsys;
            xSemaphoreGive(voltMtx);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    Serial.println("VOLT: read failed");
    return false;
}

void voltageTask(void*) {
    analogReadResolution(12);
    readVoltageOnce();

    uint8_t dummy;
    for (;;) {
        if (xQueueReceive(voltForceQ, &dummy, pdMS_TO_TICKS(VOLT_INTERVAL_MS)) == pdTRUE) {
            readVoltageOnce();
        } else {
            readVoltageOnce();
        }
    }
}
