#include "globals.h"

Settings g_cfg;
SemaphoreHandle_t cfgMtx = NULL;

TimeState g_tm;
SemaphoreHandle_t timeMtx = NULL;

float g_vsys = 0.0f;
SemaphoreHandle_t voltMtx = NULL;

QueueHandle_t btnQ = NULL;
QueueHandle_t ledQ = NULL;
QueueHandle_t buzQ = NULL;
QueueHandle_t voltForceQ = NULL;
QueueHandle_t wifiCmdQ = NULL;
EventGroupHandle_t wifiEG = NULL;

bool g_alarmActive = false;

Adafruit_NeoPixel strip(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800);
Preferences prefs;
DNSServer dnsServer;


bool g_webServerActive = true;     // start ON
uint32_t g_lastWebActivityMs = 0;

bool serverRunning = false;