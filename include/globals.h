#ifndef RGB_CLOCK_GLOBALS_H
#define RGB_CLOCK_GLOBALS_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <DNSServer.h>
#include <WebServer.h>
// ─── Pin definitions ──────────────────────────────────────────────────────
#define PIN_STATUS_LED   4
#define PIN_VOLTAGE      0
#define PIN_WS2812       7
#define PIN_BUZZER       1
#define PIN_BUTTON       10

// ─── Hardware constants ──────────────────────────────────────────────────
#define NUM_LEDS         60
#define VSYS_RATIO       2.0f

// ─── Timing constants ────────────────────────────────────────────────────
#define DEBOUNCE_MS        30
#define DOUBLE_CLICK_MS   350
#define LONG_PRESS_MS     1000
#define WIFI_TIMEOUT_MS  20000
#define NTP_INTERVAL_MS  60000
#define VOLT_INTERVAL_MS 10000
#define CLOCK_FRAME_MS     100   // fixed refresh rate for clock/date display (unchanged from before)
#define PATTERN_MIN_MS      20   // fastest pattern frame time (speed=10)
#define PATTERN_MAX_MS     300   // slowest pattern frame time (speed=1)

// ─── NVS ──────────────────────────────────────────────────────────────────
#define NVS_NS   "rgbclk"
#define NVS_KEY  "cfg4"

// ─── Defaults ────────────────────────────────────────────────────────────
#define DEF_STA_SSID  "YourSSID"
#define DEF_STA_PASS  "YourPass"
#define DEF_AP_SSID   "RGB_Clock"
#define DEF_AP_PASS   "12345678"

// ═══════════════════════════════════════════════════════════════════════════
//  SETTINGS  (saved to NVS)
// ═══════════════════════════════════════════════════════════════════════════
struct Settings {
    char     staSsid[33];
    char     staPass[65];
    char     apSsid[33];
    char     apPass[65];

    uint8_t  ledMode;
    uint8_t  ledR, ledG, ledB;
    uint8_t  brightness;
    uint8_t  speed;

    uint8_t  secR, secG, secB;
    uint8_t  minR, minG, minB;
    uint8_t  hourR, hourG, hourB;

    uint8_t  dayR, dayG, dayB;
    uint8_t  monthR, monthG, monthB;
    uint8_t  yearR, yearG, yearB;

    uint16_t buzFreq;
    uint8_t  buzVolume;
    int16_t  tzOffsetMin;

    bool     alarmEnabled;
    uint8_t  alarmHour;
    uint8_t  alarmMin;
    uint8_t  alarmSound;

    bool     use24Hour;

    uint16_t dateIntervalSec;  

};

// ═══════════════════════════════════════════════════════════════════════════
//  ENUMS & STRUCTS
// ═══════════════════════════════════════════════════════════════════════════
typedef enum : uint8_t {
    BTN_SHORT,
    BTN_DOUBLE,
    BTN_LONG
} BtnEvent_t;

typedef enum : uint8_t {
    LED_CMD_MODE,
    LED_CMD_COLOR,
    LED_CMD_BRIGHTNESS,
    LED_CMD_SPEED
} LedCmdType_t;

struct LedCmd {
    LedCmdType_t type;
    uint8_t  v1, v2, v3;
};

typedef enum : uint8_t {
    BUZ_CMD_TONE,
    BUZ_CMD_VOLUME,
    BUZ_CMD_STOP,
    BUZ_CMD_MELODY,
    BUZ_CMD_SOUND
} BuzCmdType_t;

struct BuzCmd {
    BuzCmdType_t type;
    uint16_t freq;
    uint8_t  volume;
};

enum class WiFiState : uint8_t {
    WS_IDLE, WS_CONNECTING, WS_STA_OK, WS_AP_MODE
};

#define EG_WIFI_CONNECTING  (1 << 0)
#define EG_WIFI_CONNECTED   (1 << 1)
#define EG_WIFI_AP          (1 << 2)

// ═══════════════════════════════════════════════════════════════════════════
//  GLOBALS
// ═══════════════════════════════════════════════════════════════════════════
extern Settings            g_cfg;
extern SemaphoreHandle_t   cfgMtx;

struct TimeState {
    bool    valid;
    time_t  lastSyncEpoch;
    uint32_t lastSyncMillis;
};

extern TimeState           g_tm;
extern SemaphoreHandle_t   timeMtx;

extern float               g_vsys;
extern SemaphoreHandle_t   voltMtx;

extern QueueHandle_t       btnQ;
extern QueueHandle_t       ledQ;
extern QueueHandle_t       buzQ;
extern QueueHandle_t       voltForceQ;
extern QueueHandle_t       wifiCmdQ;
extern EventGroupHandle_t  wifiEG;

extern bool   g_alarmActive;
extern Adafruit_NeoPixel strip;
extern Preferences prefs;
extern DNSServer dnsServer;
extern WebServer server;
extern bool g_webServerActive;
extern uint32_t g_lastWebActivityMs;
extern bool serverRunning;
#define WEB_INACTIVITY_TIMEOUT_MS  300000

#define LED_MODE_CLOCK1      0   // plain clock, digits snap straight to the new value
#define LED_MODE_CLOCK2      5   // clock with the roll/odometer animation on digit change

#endif // RGB_CLOCK_GLOBALS_H
