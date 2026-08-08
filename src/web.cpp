#include "web.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ESPmDNS.h>

static const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RGB Clock</title>
<style>
:root{--bg:#0f0f1a;--card:#181828;--border:#252540;--acc:#6c7fff;--txt:#d0d0e8;--ok:#50e890;--warn:#ffc844;--red:#ff5555}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt);padding:10px;max-width:780px;margin:0 auto}
h3{color:var(--acc);font-size:.95rem;margin-bottom:10px;padding-bottom:6px;border-bottom:1px solid var(--border)}
.card{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:14px;margin-bottom:12px}
.row{display:flex;gap:10px;flex-wrap:wrap}
.col{flex:1;min-width:140px}
label{display:block;font-size:.72rem;color:#888;margin-bottom:3px}
input,select{width:100%;background:#0a0a16;border:1px solid var(--border);color:var(--txt);padding:7px 10px;border-radius:6px;font-size:.88rem}
input[type=range]{padding:4px 0}
button{background:var(--acc);color:#000;border:none;padding:8px 14px;border-radius:6px;cursor:pointer;font-weight:700;font-size:.82rem;margin-top:6px;margin-right:4px}
button:hover{opacity:.8}
button.red{background:var(--red);color:#fff}
button.grn{background:var(--ok);color:#000}
.big{font-size:2.4rem;font-weight:700;letter-spacing:4px;text-align:center;padding:6px 0}
.volt{color:var(--ok)}
.status{font-size:.75rem;color:var(--warn);text-align:center;padding:2px 0}
.tag{display:inline-block;font-size:.7rem;background:#252540;padding:2px 8px;border-radius:20px;margin-left:6px}
progress{width:100%;height:6px;margin-top:6px;border-radius:3px}
</style>
</head>
<body>

<div class="card">
  <div class="big" id="clk">--:--:--</div>
  <div class="big volt" id="vlt">-.- V</div>
  <div class="status" id="wst">…</div>
  <button onclick="api('readvolt')">⏺ Read Voltage Now</button>
</div>

<div class="card">
  <h3>⏰ Alarm</h3>
  <div class="row">
    <div class="col"><label>Enabled</label><input type="checkbox" id="alarmEn" onchange="api('alarmEn', this.checked ? 1 : 0)"></div>
    <div class="col"><label>Time</label><input type="time" id="alarmTime" value="07:00" onchange="setAlarmTime(this.value)"></div>
    <div class="col"><label>Sound</label><select id="alarmSound" onchange="api('alarmSound', this.value)">
        <option value="0">Silent</option>
        <option value="1">Beep</option>
        <option value="2">Ascending</option>
        <option value="3">Beep Beep</option>
        <option value="4">Siren</option>
      </select></div>
  </div>
  <button onclick="api('alarmTest',1)">🔔 Test Alarm</button>
</div>

<div class="card">
  <h3>💡 LED Strip</h3>
  <div class="row">
    <div class="col">
      <label>Mode</label>
      <select id="mode" onchange="api('mode',this.value)">
        <option value="0">Clock</option><option value="1">Solid</option><option value="2">Rainbow</option>
        <option value="3">Chase</option><option value="4">Pulse</option><option value="5">Clock 2 (Animated)</option>
      </select>
    </div>
    <div class="col">
      <label>Color (patterns)</label>
      <input type="color" id="col" value="#ffffff" onchange="let c=hr(this.value);api('rgb',c[0]+','+c[1]+','+c[2])">
    </div>
    <div class="col">
      <label>Brightness <span id="briL">120</span></label>
      <input type="range" min="0" max="255" value="120" id="bri" oninput="$('briL').textContent=this.value;api('bri',this.value)">
    </div>
    <div class="col">
      <label>Speed <span id="spdL">5</span></label>
      <input type="range" min="1" max="10" value="5" id="spd" oninput="$('spdL').textContent=this.value;api('spd',this.value)">
    </div>
  </div>
</div>

<div class="card">
  <h3>🕐 Clock Display</h3>
  <div class="row">
    <div class="col"><label>Seconds color</label><input type="color" id="csec" value="#0000c8" onchange="let c=hr(this.value);api('csec',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Minutes color</label><input type="color" id="cmin" value="#00c800" onchange="let c=hr(this.value);api('cmin',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Hours color</label><input type="color" id="chour" value="#c80000" onchange="let c=hr(this.value);api('chour',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Timezone (minutes)</label><input type="number" id="tz" min="-720" max="840" value="330" onchange="api('tz',this.value)"></div>
  </div>
  <div class="col">
  <label>Time Format</label>
  <select id="format" onchange="api('format',this.value)">
    <option value="1">24 Hour</option>
    <option value="0">12 Hour</option>
  </select>
  </div>
 </div>

<div class="card">
  <h3>📅 Date Display</h3>
  <div class="row">
    <div class="col"><label>Day color</label><input type="color" id="cday" value="#00b4b4" onchange="let c=hr(this.value);api('cday',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Month color</label><input type="color" id="cmonth" value="#b400b4" onchange="let c=hr(this.value);api('cmonth',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Year color</label><input type="color" id="cyear" value="#b48200" onchange="let c=hr(this.value);api('cyear',c[0]+','+c[1]+','+c[2])"></div>
    <div class="col"><label>Date Interval (minutes)</label><input type="number" id="dateint" min="3" max="3600" value="15" onchange="api('dateint',this.value)"></div>
    </div>
</div>

<div class="card">
  <h3>🔔 Buzzer</h3>
  <div class="row">
    <div class="col"><label>Frequency Hz (0=off)</label><input type="number" id="freq" min="0" max="5000" value="0" onchange="api('freq',this.value)"></div>
    <div class="col"><label>Volume % <span id="volL">70</span></label><input type="range" min="0" max="100" value="70" id="vol" oninput="$('volL').textContent=this.value;api('vol',this.value)"></div>
  </div>
  <button class="red" onclick="api('buzstop')">■ Stop</button>
  <button class="grn" onclick="api('melody')">♪ Play Melody</button>
</div>

<div class="card">
  <h3>📶 WiFi</h3>
  <div class="row">
    <div class="col"><label>STA SSID</label><input id="ssid" placeholder="Router SSID"></div>
    <div class="col"><label>STA Password</label><input type="password" id="pass"></div>
    <div class="col"><label>AP SSID</label><input id="apssid" placeholder="AP hotspot name"></div>
    <div class="col"><label>AP Password</label><input type="password" id="appass"></div>
  </div>
  <button onclick="saveWifi()">💾 Save & Reboot</button>
  <button onclick="scanWifi()">🔍 Scan Networks</button>
  <div id="scanResults" class="row" style="margin-top:8px"></div>
</div>

<div class="card">
  <h3>🔄 OTA Update</h3>
  <input type="file" id="fw" accept=".bin">
  <button onclick="doOta()">Upload Firmware</button>
  <progress id="prog" value="0" max="100" style="display:none"></progress>
  <div id="otaMsg" style="font-size:.8rem;margin-top:4px"></div>
</div>

<script>
function $(id){return document.getElementById(id);}
function api(cmd,val){ fetch('/api?c='+encodeURIComponent(cmd)+'&v='+encodeURIComponent(val===undefined?'':val)).catch(()=>{}); }
function hr(hex){ return[parseInt(hex.slice(1,3),16),parseInt(hex.slice(3,5),16),parseInt(hex.slice(5,7),16)]; }
function saveWifi(){
  var p=new URLSearchParams({ ssid:$('ssid').value, pass:$('pass').value, apssid:$('apssid').value, appass:$('appass').value });
  fetch('/savewifi',{method:'POST',body:p}).catch(()=>{});
}
function scanWifi(){
  var box=$('scanResults');
  box.innerHTML='<div class="status">Scanning…</div>';
  fetch('/wifiscan').then(r=>r.json()).then(function(list){
    box.innerHTML='';
    if(!list.length){ box.innerHTML='<div class="status">No networks found</div>'; return; }
    list.forEach(function(n){
      var b=document.createElement('button');
      b.type='button';
      b.textContent=n.ssid+' ('+n.rssi+' dBm)'+(n.open?'':' 🔒');
      b.onclick=function(){ $('ssid').value=n.ssid; box.innerHTML=''; };
      box.appendChild(b);
    });
  }).catch(function(){ box.innerHTML='<div class="status">Scan failed</div>'; });
}
function doOta(){
  var f=$('fw').files[0]; if(!f){alert('Select .bin file');return;}
  var pr=$('prog'), msg=$('otaMsg'); pr.style.display='block'; msg.textContent='Uploading…';
  var xhr=new XMLHttpRequest();
  xhr.upload.onprogress=function(e){if(e.lengthComputable)pr.value=e.loaded/e.total*100|0;};
  xhr.onload=function(){msg.textContent='Done! Rebooting…';};
  xhr.onerror=function(){msg.textContent='Upload error.';};
  var fd=new FormData(); fd.append('firmware',f);
  xhr.open('POST','/update'); xhr.send(fd);
}
function setAlarmTime(value){
  if (!value || value.length !== 5) return;
  var parts = value.split(':');
  api('alarmH', parts[0]);
  api('alarmM', parts[1]);
}
var firstPoll=true;
function poll(){
  fetch('/status').then(r=>r.json()).then(function(d){
    $('clk').textContent = d.time;
    $('vlt').textContent = d.vsys+' V';
    $('wst').textContent = d.wifi;
    if(firstPoll){
      firstPoll=false;
      $('mode').value    = d.mode;
      $('bri').value     = d.bri; $('briL').textContent=d.bri;
      $('spd').value     = d.spd; $('spdL').textContent=d.spd;
      $('tz').value      = d.tz;
      $('freq').value    = d.freq;
      $('vol').value     = d.vol; $('volL').textContent=d.vol;
      $('alarmEn').checked = d.alarmEn == 1;
      $('alarmTime').value = (d.alarmH < 10 ? '0' + d.alarmH : d.alarmH) + ':' + (d.alarmM < 10 ? '0' + d.alarmM : d.alarmM);
      $('alarmSound').value = d.alarmS;
    }


  }).catch(()=>{});
}
poll();
setInterval(poll,1000);
</script>
</body>
</html>
)HTML";

WebServer server(80);

static void handleRoot() {
    server.send_P(200, "text/html", PAGE);
}

static void handleApi() {

    g_lastWebActivityMs = millis();

    String c = server.arg("c");
    String v = server.arg("v");

    if (c == "mode") {
        uint8_t mode = (uint8_t)v.toInt();
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.ledMode = mode;
        xSemaphoreGive(cfgMtx);
        LedCmd cmd = {LED_CMD_MODE, mode, 0, 0};
        xQueueSend(ledQ, &cmd, 0);
    } else if (c == "bri") {
        uint8_t bri = (uint8_t)v.toInt();
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.brightness = bri;
        xSemaphoreGive(cfgMtx);
        LedCmd cmd = {LED_CMD_BRIGHTNESS, bri, 0, 0};
        xQueueSend(ledQ, &cmd, 0);
    } else if (c == "spd") {
        uint8_t spd = (uint8_t)constrain(v.toInt(), 1, 10);
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.speed = spd;
        xSemaphoreGive(cfgMtx);
        LedCmd cmd = {LED_CMD_SPEED, spd, 0, 0};
        xQueueSend(ledQ, &cmd, 0);
    } else if (c == "csec" || c == "cmin" || c == "chour" || c == "cday" || c == "cmonth" || c == "cyear") {
        int ri = v.indexOf(','), gi = v.lastIndexOf(',');
        uint8_t r = v.substring(0, ri).toInt();
        uint8_t g = v.substring(ri+1, gi).toInt();
        uint8_t b = v.substring(gi+1).toInt();
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        if (c == "csec")  { g_cfg.secR=r;  g_cfg.secG=g;  g_cfg.secB=b; }
        else if (c == "cmin") { g_cfg.minR=r;  g_cfg.minG=g;  g_cfg.minB=b; }
        else if (c == "chour"){ g_cfg.hourR=r; g_cfg.hourG=g; g_cfg.hourB=b; }
        else if (c == "cday")  { g_cfg.dayR=r;   g_cfg.dayG=g;   g_cfg.dayB=b; }
        else if (c == "cmonth"){ g_cfg.monthR=r; g_cfg.monthG=g; g_cfg.monthB=b; }
        else if (c == "cyear") { g_cfg.yearR=r;  g_cfg.yearG=g;  g_cfg.yearB=b; }
        xSemaphoreGive(cfgMtx);
    } else if (c == "tz") {
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.tzOffsetMin = (int16_t)v.toInt();
        xSemaphoreGive(cfgMtx);
    } else if (c == "alarmEn") {
        bool enabled = v.toInt() != 0;
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.alarmEnabled = enabled;
        xSemaphoreGive(cfgMtx);
    } else if (c == "alarmH") {
        uint8_t h = (uint8_t)constrain(v.toInt(), 0, 23);
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.alarmHour = h;
        xSemaphoreGive(cfgMtx);
    } else if (c == "alarmM") {
        uint8_t m = (uint8_t)constrain(v.toInt(), 0, 59);
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.alarmMin = m;
        xSemaphoreGive(cfgMtx);
    } else if (c == "alarmSound") {
        uint8_t s = (uint8_t)constrain(v.toInt(), 0, 4);
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.alarmSound = s;
        xSemaphoreGive(cfgMtx);
    } else if (c == "alarmTest") {
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        uint8_t sound = g_cfg.alarmSound;
        uint8_t volume = g_cfg.buzVolume;
        xSemaphoreGive(cfgMtx);
        BuzCmd buz = {BUZ_CMD_SOUND, sound, volume};
        xQueueSend(buzQ, &buz, 0);
    } else if (c == "rgb") {
        int ri = v.indexOf(','), gi = v.lastIndexOf(',');
        uint8_t r = v.substring(0, ri).toInt();
        uint8_t g = v.substring(ri+1, gi).toInt();
        uint8_t b = v.substring(gi+1).toInt();
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.ledR = r; g_cfg.ledG = g; g_cfg.ledB = b;
        xSemaphoreGive(cfgMtx);
        LedCmd cmd = {LED_CMD_COLOR, r, g, b};
        xQueueSend(ledQ, &cmd, 0);
    } else if (c == "freq") {
        uint16_t freq = (uint16_t)v.toInt();
        uint8_t vol;
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.buzFreq = freq;
        vol = g_cfg.buzVolume;
        xSemaphoreGive(cfgMtx);
        BuzCmd buz = {BUZ_CMD_TONE, freq, vol};
        xQueueSend(buzQ, &buz, 0);
    } else if (c == "vol") {
        uint8_t vol = (uint8_t)v.toInt();
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.buzVolume = vol;
        uint16_t freq = g_cfg.buzFreq;
        xSemaphoreGive(cfgMtx);
        BuzCmd buz = {BUZ_CMD_VOLUME, freq, vol};
        xQueueSend(buzQ, &buz, 0);
    } else if (c == "buzstop") {
        BuzCmd buz = {BUZ_CMD_STOP, 0, 0};
        xQueueSend(buzQ, &buz, 0);
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.buzFreq = 0;
        xSemaphoreGive(cfgMtx);
    } else if (c == "melody") {
        BuzCmd buz = {BUZ_CMD_MELODY, 0, 0};
        xQueueSend(buzQ, &buz, 0);
    } else if (c == "readvolt") {
        uint8_t dummy = 0;
        xQueueSend(voltForceQ, &dummy, 0);
    } else if (c == "format") {
        bool use24 = v.toInt() != 0;
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.use24Hour = use24;
        xSemaphoreGive(cfgMtx);
    } else if (c == "dateint") {
        int sec = v.toInt();
        if (sec < 3) sec = 3;
        if (sec > 3600) sec = 3600;
        xSemaphoreTake(cfgMtx, portMAX_DELAY);
        g_cfg.dateIntervalSec = (uint16_t)sec;
        xSemaphoreGive(cfgMtx);
    }   


    saveSettings();
    server.send(200, "text/plain", "OK");
}

static void handleStatus() {
    g_lastWebActivityMs = millis();

    char timeBuf[10] = "--:--:--";
    time_t ep = localEpochNow();
    if (ep != 0) {
        int h, m, s;
        epochToHMS(ep, h, m, s);
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", h, m, s);
    }

    const char* wifiStr = "Idle";
    EventBits_t bits = xEventGroupGetBits(wifiEG);
    if (bits & EG_WIFI_CONNECTED)      wifiStr = "STA Connected";
    else if (bits & EG_WIFI_CONNECTING) wifiStr = "Connecting…";
    else if (bits & EG_WIFI_AP)        wifiStr = "AP STA Mode";

    float vsys;
    xSemaphoreTake(voltMtx, portMAX_DELAY);
    vsys = g_vsys;
    xSemaphoreGive(voltMtx);

    xSemaphoreTake(cfgMtx, portMAX_DELAY);
    uint8_t mode = g_cfg.ledMode;
    uint8_t bri = g_cfg.brightness;
    uint8_t spd = g_cfg.speed;
    int16_t tz  = g_cfg.tzOffsetMin;
    uint16_t freq = g_cfg.buzFreq;
    uint8_t vol = g_cfg.buzVolume;
    bool alarmEn = g_cfg.alarmEnabled;
    uint8_t alarmH = g_cfg.alarmHour;
    uint8_t alarmM = g_cfg.alarmMin;
    uint8_t alarmS = g_cfg.alarmSound;
    int16_t  dateIn = g_cfg.dateIntervalSec;
    xSemaphoreGive(cfgMtx);

    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    uint32_t maxAlloc = ESP.getMaxAllocHeap();

    char json[420];
    snprintf(json, sizeof(json),
        "{\"time\":\"%s\"," \
        "\"vsys\":\"%.2f\"," \
        "\"wifi\":\"%s\"," \
        "\"mode\":%d," \
        "\"bri\":%d," \
        "\"spd\":%d," \
        "\"tz\":%d," \
        "\"dateint\":%d," \
        "\"freq\":%d," \
        "\"vol\":%d," \
        "\"alarmEn\":%d," \
        "\"alarmH\":%d," \
        "\"alarmM\":%d," \
        "\"alarmS\":%d," \
        "\"freeHeap\":%u," \
        "\"minFreeHeap\":%u," \
        "\"maxAlloc\":%u}",
        timeBuf, vsys, wifiStr,
        mode, bri, spd, tz, dateIn, freq, vol,
        alarmEn ? 1 : 0, alarmH, alarmM, alarmS, freeHeap, minFreeHeap, maxAlloc);
    server.send(200, "application/json", json);
}

static String jsonEscape(const String &in) {
    String out;
    out.reserve(in.length());
    for (size_t i = 0; i < in.length(); i++) {
        char ch = in[i];
        if (ch == '"' || ch == '\\') out += '\\';
        out += ch;
    }
    return out;
}

static void handleWifiScan() {
    g_lastWebActivityMs = millis();
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(true); 
    int n = WiFi.scanNetworks(false, true);   // blocking scan, include hidden
    if (n < 0) {                              // WIFI_SCAN_FAILED / still starting up
        delay(300);
        n = WiFi.scanNetworks(false, true);   // one retry
    }
    

    String json = "[";
    bool first = true;
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue;

        // skip duplicate SSIDs (keep strongest, results come sorted by RSSI already)
        bool dup = false;
        for (int j = 0; j < i; j++) {
            if (WiFi.SSID(j) == ssid) { dup = true; break; }
        }
        if (dup) continue;

        if (!first) json += ",";
        first = false;
        json += "{\"ssid\":\"" + jsonEscape(ssid) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"open\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    json += "]";
    WiFi.scanDelete();

    server.send(200, "application/json", json);
}

static void handleSaveWifi() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "POST only");
        return;
    }
    xSemaphoreTake(cfgMtx, portMAX_DELAY);
    strlcpy(g_cfg.staSsid, server.arg("ssid").c_str(),   sizeof(g_cfg.staSsid));
    strlcpy(g_cfg.staPass, server.arg("pass").c_str(),   sizeof(g_cfg.staPass));
    strlcpy(g_cfg.apSsid,  server.arg("apssid").c_str(), sizeof(g_cfg.apSsid));
    strlcpy(g_cfg.apPass,  server.arg("appass").c_str(), sizeof(g_cfg.apPass));
    saveSettings();
    xSemaphoreGive(cfgMtx);
    server.send(200, "text/plain", "Saved. Rebooting...");
    delay(400);
    ESP.restart();
}

static void handleUpdate() {
}

static void handleUpdateUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("OTA: start  file=%s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("OTA: success  %u bytes\n", upload.totalSize);
            server.send(200, "text/plain", "Update OK. Rebooting...");
            delay(500);
            ESP.restart();
        } else {
            Update.printError(Serial);
            server.send(500, "text/plain", "Update failed");
        }
    }
}

void serverOn() {
    server.on("/",         HTTP_GET,  handleRoot);
    server.on("/api",      HTTP_GET,  handleApi);
    server.on("/status",   HTTP_GET,  handleStatus);
    server.on("/savewifi", HTTP_POST, handleSaveWifi);
    server.on("/wifiscan", HTTP_GET,  handleWifiScan);
    server.on("/update",   HTTP_POST, handleUpdate, handleUpdateUpload);
    
    server.on("/generate_204", HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302); });
    server.on("/fwlink",       HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302); });
    server.on("/connect",      HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302); });
    server.onNotFound([]() {
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });
    
    
    server.begin();
    Serial.println("WEB: server started on :80");
    if (MDNS.begin("rgbclock")) {
        Serial.println("MDNS: rgbclock.local registered");
    }
}

void serverOff() {
    server.stop();
    Serial.println("WEB: server stopped");
}


void webHandleClient() {
    server.handleClient();
}
