// Clima-Core Weather Station
// Portrait mode (240×320), auto-cycles DHT → BMP → Soil every 3 seconds
// Logo + title header on every screen, no buttons

#include <TFT_eSPI.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DHT_PIN   26
#define SOIL_PIN  36
#define DHT_TYPE  DHT22

const char* WIFI_SSID     = "138SL-Residents.";
const char* WIFI_PASSWORD = "resident2020@138sl";
const char* MQTT_BROKER   = "www.yanacreations.com";
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "weather/sensors";
const char* MQTT_CLIENT   = "ESP32_WeatherStation";

TFT_eSPI        tft;
DHT             dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;
WiFiClient      wifiClient;
PubSubClient    mqtt(wifiClient);

enum Screen { DHT_PAGE, BMP_PAGE, SOIL_PAGE };
Screen currentScreen = DHT_PAGE;

float temperature = 25.0f, humidity = 60.0f;
float bmpTemp     = 25.0f, pressure = 1013.0f;
int   soilPct     = 50;
bool  bmpOK       = false;

unsigned long lastRead    = 0;
unsigned long lastCycle   = 0;
unsigned long lastPublish = 0;

#define SENSOR_MS   3000
#define CYCLE_MS    3000
#define PUBLISH_MS  3000

#define C_NAVY   0x2008
#define C_WHITE  TFT_WHITE
#define C_CYAN   0xFD59
#define C_LBLUE  0xC4DF
#define C_YELLOW 0xFFB6
#define C_LIME   0x87E0
#define C_RED    0xF810
#define C_ORANGE 0xFBB6
#define C_GRAY   TFT_DARKGREY
#define C_DGRAY  0x2104
#define C_DIM    0x1808
#define C_GREEN  0x07E0
#define C_DRED   0x8000

// ─── Network ──────────────────────────────────────────────────────────────────

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED - continuing without network");
  }
}

void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.print("Connecting to MQTT broker at ");
  Serial.println(MQTT_BROKER);
  if (mqtt.connect(MQTT_CLIENT)) {
    Serial.println("MQTT connected");
    mqtt.subscribe("weather/commands");
  } else {
    Serial.print("MQTT failed, error code: ");
    Serial.println(mqtt.state());
  }
}

void publishSensorData() {
  if (!mqtt.connected()) return;
  char payload[200];
  snprintf(payload, sizeof(payload),
    "{\"temperature\":%.1f,\"humidity\":%.1f,\"bmp_temperature\":%.1f,\"pressure\":%.1f,\"soil_moisture\":%d}",
    temperature, humidity, bmpTemp, pressure, soilPct);
  bool success = mqtt.publish(MQTT_TOPIC, payload);
  Serial.print("Published: ");  Serial.println(payload);
  Serial.print("Success: ");    Serial.println(success ? "YES" : "NO");
}

// ─── Setup & loop ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();
  bmpOK = bmp.begin(0x76);
  if (!bmpOK) bmpOK = bmp.begin(0x77);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(C_NAVY);
  connectWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  connectMQTT();
  readSensors();
  renderScreen();
}

void loop() {
  unsigned long now = millis();
  if (!mqtt.connected()) {
    if (now - lastPublish > 2000) connectMQTT();
  }
  mqtt.loop();
  if (now - lastRead    >= SENSOR_MS)  { lastRead    = now; readSensors();       }
  if (now - lastPublish >= PUBLISH_MS) { lastPublish = now; publishSensorData(); }
  if (now - lastCycle   >= CYCLE_MS)   {
    lastCycle = now;
    currentScreen = (Screen)((currentScreen + 1) % 3);
    renderScreen();
  }
}

// ─── Sensors ──────────────────────────────────────────────────────────────────

void readSensors() {
  float h = dht.readHumidity(), t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) { humidity = h; temperature = t; }
  if (bmpOK) {
    bmpTemp  = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0f;
  }
  int raw = analogRead(SOIL_PIN);
  int soilRaw[]  = {3200, 1350, 1140, 1075, 1040};
  int soilPcts[] = {0, 25, 50, 75, 100};
  if (raw >= soilRaw[0]) {
    soilPct = 0;
  } else if (raw <= soilRaw[4]) {
    soilPct = 100;
  } else {
    for (int i = 0; i < 4; i++) {
      if (raw <= soilRaw[i] && raw >= soilRaw[i+1]) {
        soilPct = map(raw, soilRaw[i], soilRaw[i+1], soilPcts[i], soilPcts[i+1]);
        break;
      }
    }
  }
  soilPct = constrain(soilPct, 0, 100);
  Serial.print("Temp: ");      Serial.println(temperature);
  Serial.print("Humidity: ");  Serial.println(humidity);
  Serial.print("BMP Temp: ");  Serial.println(bmpTemp);
  Serial.print("Pressure: ");  Serial.println(pressure);
  Serial.print("Soil raw: ");  Serial.println(analogRead(SOIL_PIN));
  Serial.print("Soil %: ");    Serial.println(soilPct);
  Serial.println("---");
}

// ─── Screen router ────────────────────────────────────────────────────────────

void renderScreen() {
  switch (currentScreen) {
    case DHT_PAGE:  drawDHTPage();  break;
    case BMP_PAGE:  drawBMPPage();  break;
    case SOIL_PAGE: drawSoilPage(); break;
  }
}

uint16_t soilColor(int pct) {
  if (pct > 60) return C_LIME;
  if (pct > 30) return C_YELLOW;
  return C_RED;
}

// ─── Logo header ──────────────────────────────────────────────────────────────

void drawLogoHeader() {

  // Step 1: Draw cloud first
  tft.fillRoundRect(12, 26, 38, 17, 6, C_WHITE);
  tft.fillCircle(20, 27, 7, C_WHITE);
  tft.fillCircle(30, 23, 8, C_WHITE);
  tft.fillCircle(38, 28, 6, C_WHITE);

  // Step 2: Draw sun peeking above top-right
  tft.fillCircle(50, 13, 9, C_YELLOW);
  for (int r = 0; r < 8; r++) {
    float a = r * 45.0f * (PI / 180.0f);
    tft.drawLine(
      50 + (int)(12 * cosf(a)), 13 - (int)(12 * sinf(a)),
      50 + (int)(16 * cosf(a)), 13 - (int)(16 * sinf(a)),
      C_YELLOW
    );
  }

  // Step 3: Redraw cloud over sun's lower half
  tft.fillRoundRect(12, 26, 38, 17, 6, C_WHITE);
  tft.fillCircle(20, 27, 7, C_WHITE);
  tft.fillCircle(30, 23, 8, C_WHITE);
  tft.fillCircle(38, 28, 6, C_WHITE);

  // Rain drops
  for (int d = 0; d < 4; d++) {
    int rx = 15 + d * 9;
    tft.drawLine(rx,     36, rx - 3, 45, C_CYAN);
    tft.drawLine(rx + 1, 36, rx - 2, 45, C_CYAN);
  }

  // Title
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(66, 16);
  tft.print("Clima-Core");

  // Wi-Fi / MQTT dots
  uint16_t wifiCol = (WiFi.status() == WL_CONNECTED) ? C_GREEN : C_DRED;
  tft.fillCircle(220, 8, 4, wifiCol);
  uint16_t mqttCol = mqtt.connected() ? C_GREEN : C_DRED;
  tft.fillCircle(232, 8, 4, mqttCol);
  tft.setTextColor(C_DGRAY); tft.setTextSize(1);
  tft.setCursor(209, 17); tft.print("W M");

  // Separator
  tft.drawLine(0, 49, 240, 49, C_DIM);
}

// ─── Graphic primitives ───────────────────────────────────────────────────────

void drawThermometer(int cx, int cy, float tempC, uint16_t col, int tubeH) {
  int top = cy - tubeH;
  tft.fillRoundRect(cx - 6, top, 12, tubeH, 6, C_DIM);
  tft.drawRoundRect(cx - 6, top, 12, tubeH, 6, col);
  int fillH = constrain((int)map((long)tempC, -10, 50, 0, tubeH - 8), 0, tubeH - 8);
  if (fillH > 0)
    tft.fillRoundRect(cx - 3, top + 4 + (tubeH - 8 - fillH), 6, fillH, 2, col);
  int ticks = (tubeH - 6) / 14;
  for (int t = 0; t < ticks; t++)
    tft.drawLine(cx + 6, top + 4 + t * 14, cx + 10, top + 4 + t * 14, col);
  tft.fillCircle(cx, cy + 10, 10, C_DIM);
  tft.drawCircle(cx, cy + 10, 10, col);
  tft.fillCircle(cx, cy + 10,  7, col);
}

void drawDrop(int cx, int cy, uint16_t col) {
  for (int i = 0; i <= 28; i++) {
    int w = (int)((1.0f - (float)i / 28.0f) * 16.0f);
    if (w > 0) tft.drawLine(cx - w, cy - 14 - i, cx + w, cy - 14 - i, col);
  }
  tft.fillCircle(cx, cy - 8, 16, col);
  tft.fillCircle(cx - 4, cy - 3, 3, 0xC71F);
  tft.fillCircle(cx - 4, cy - 3, 2, 0xEEFF);
}

void drawGauge(int cx, int cy, float hpa, uint16_t col) {
  int R = 42;
  for (int y = 0; y <= R; y++) {
    int hw = (int)sqrtf((float)(R * R - y * y));
    tft.drawLine(cx - hw, cy - y, cx + hw, cy - y, C_DIM);
  }
  for (float a = 0.0f; a <= 180.0f; a += 1.5f) {
    float rad = a * (PI / 180.0f);
    for (int t = 0; t < 2; t++)
      tft.drawPixel(cx + (int)((R - t) * cosf(rad)), cy - (int)((R - t) * sinf(rad)), col);
  }
  tft.drawLine(cx - R, cy, cx + R, cy, col);
  for (int i = 0; i <= 4; i++) {
    float a = i * 45.0f * (PI / 180.0f);
    tft.drawLine(cx + (int)((R -  3) * cosf(a)), cy - (int)((R -  3) * sinf(a)),
                 cx + (int)((R - 10) * cosf(a)), cy - (int)((R - 10) * sinf(a)), C_GRAY);
  }
  float norm    = constrain((hpa - 950.0f) / 100.0f, 0.0f, 1.0f);
  float needleA = (1.0f - norm) * PI;
  int nx = cx + (int)((R - 10) * cosf(needleA));
  int ny = cy - (int)((R - 10) * sinf(needleA));
  tft.drawLine(cx,     cy, nx,     ny, C_WHITE);
  tft.drawLine(cx + 1, cy, nx + 1, ny, C_WHITE);
  tft.fillCircle(cx, cy, 5, col);
  tft.fillCircle(cx, cy, 3, C_NAVY);
  tft.setTextColor(C_DGRAY); tft.setTextSize(1);
  tft.setCursor(cx - R - 8, cy + 4); tft.print("L");
  tft.setCursor(cx + R + 2,  cy + 4); tft.print("H");
}

// ─── Page draw functions ───────────────────────────────────────────────────────

void drawDHTPage() {
  tft.fillScreen(C_NAVY);
  drawLogoHeader();

  // ── Temperature ──
  tft.setTextColor(C_CYAN); tft.setTextSize(1);
  tft.setCursor(120 - (11 * 6) / 2, 54);
  tft.print("TEMPERATURE");

  drawThermometer(120, 118, temperature, C_CYAN, 50);

  char tStr[8];   dtostrf(temperature, 4, 1, tStr);
  char tDisp[12]; snprintf(tDisp, sizeof(tDisp), "%s\xF7""C", tStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(120 - (int)(strlen(tDisp) * 12) / 2, 144);
  tft.print(tDisp);

  tft.drawLine(0, 162, 240, 162, C_DIM);

  // ── Humidity ──
  tft.setTextColor(C_LBLUE); tft.setTextSize(1);
  tft.setCursor(120 - (8 * 6) / 2, 168);
  tft.print("HUMIDITY");

  drawDrop(120, 235, C_LBLUE);

  char hStr[8];   dtostrf(humidity, 4, 1, hStr);
  char hDisp[10]; snprintf(hDisp, sizeof(hDisp), "%s%%", hStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(120 - (int)(strlen(hDisp) * 12) / 2, 250);
  tft.print(hDisp);

  tft.drawRect(20, 274, 200, 10, C_LBLUE);
  int bw = (int)(constrain(humidity, 0.0f, 100.0f) / 100.0f * 198.0f);
  tft.fillRect(21, 275, bw, 8, C_LBLUE);
}

// BMP page — everything pushed down, more breathing room
// Header:        y   0– 49
// "PRESSURE":    y  64
// Gauge:         cy=118, arc top=76, base=118
// Value text:    y 130
// Divider:       y 148
// "TEMPERATURE": y 158
// Thermometer:   cy=222, tube top=172, bulb bottom=232
// Value text:    y 242

void drawBMPPage() {
  tft.fillScreen(C_NAVY);
  drawLogoHeader();

  if (!bmpOK) {
    tft.setTextColor(C_RED); tft.setTextSize(2);
    tft.setCursor(20, 170); tft.print("BMP280 not found");
    return;
  }

  // ── Pressure ──
  tft.setTextColor(C_YELLOW); tft.setTextSize(1);
  tft.setCursor(120 - (8 * 6) / 2, 64);
  tft.print("PRESSURE");

  // cy=118, R=42 → arc top=76, plenty of space below header
  drawGauge(120, 118, pressure, C_YELLOW);

  char pStr[8];   dtostrf(pressure, 5, 0, pStr);
  char pDisp[12]; snprintf(pDisp, sizeof(pDisp), "%s hPa", pStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(1);
  tft.setCursor(120 - (int)(strlen(pDisp) * 6) / 2, 130);
  tft.print(pDisp);

  tft.drawLine(0, 148, 240, 148, C_DIM);

  // ── BMP Temperature ──
  tft.setTextColor(C_ORANGE); tft.setTextSize(1);
  tft.setCursor(120 - (11 * 6) / 2, 158);
  tft.print("TEMPERATURE");

  // cy=222, tubeH=50 → tube top=172, bulb centre=232, bulb bottom=242
  drawThermometer(120, 222, bmpTemp, C_ORANGE, 50);

  char btStr[8];   dtostrf(bmpTemp, 4, 1, btStr);
  char btDisp[12]; snprintf(btDisp, sizeof(btDisp), "%s\xF7""C", btStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(120 - (int)(strlen(btDisp) * 12) / 2, 248);
  tft.print(btDisp);
}

// Soil page — content vertically centred in the 270px below the header
// Total content height: label(8) + gap(8) + percent(40) + gap(8) +
//                       status(16) + gap(20) + bar(26) + gap(8) +
//                       progress(14) + gap(8) + scale(8) = 164px
// Start y = 50 + (270 - 164) / 2 = 50 + 53 = 103 → round to 100

void drawSoilPage() {
  tft.fillScreen(C_NAVY);
  drawLogoHeader();

  uint16_t sc = soilColor(soilPct);

  // ── "MOISTURE" label — centred start y=100 ──
  tft.setTextColor(sc); tft.setTextSize(1);
  tft.setCursor(120 - (8 * 6) / 2, 100);
  tft.print("MOISTURE");
  tft.drawLine(10, 110, 230, 110, C_DIM);

  // ── Large percentage ──
  char sStr[6]; snprintf(sStr, sizeof(sStr), "%d%%", soilPct);
  tft.setTextColor(sc); tft.setTextSize(5);
  tft.setCursor(120 - (int)(strlen(sStr) * 30) / 2, 116);
  tft.print(sStr);                           // ends y≈156

  // ── Status word ──
  const char* status = soilPct > 60 ? "WET"     :
                        soilPct > 30 ? "OPTIMAL" : "DRY";
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(120 - (int)(strlen(status) * 12) / 2, 162);
  tft.print(status);                         // ends y≈178

  // ── Tricolour zone bar ──
  int bx = 10, by = 188, bw = 220, bh = 26;
  tft.fillRect(bx,             by, bw * 30 / 100,        bh, C_RED);
  tft.fillRect(bx + bw*30/100, by, bw * 30 / 100,        bh, C_YELLOW);
  tft.fillRect(bx + bw*60/100, by, bw - (bw * 60 / 100), bh, C_LIME);
  tft.drawRect(bx, by, bw, bh, C_WHITE);
  tft.drawLine(bx + bw*30/100, by, bx + bw*30/100, by + bh, C_WHITE);
  tft.drawLine(bx + bw*60/100, by, bx + bw*60/100, by + bh, C_WHITE);

  // Pointer triangle above bar
  int mx = constrain(bx + (int)(soilPct / 100.0f * bw), bx + 3, bx + bw - 3);
  tft.fillTriangle(mx - 6, by - 10, mx + 6, by - 10, mx, by, C_WHITE);
  tft.drawLine(mx, by - 10, mx, by - 16, C_WHITE);

  // Zone labels inside bar
  tft.setTextColor(C_NAVY); tft.setTextSize(1);
  tft.setCursor(bx + bw * 15 / 100 - 9,  by + 9); tft.print("DRY");
  tft.setCursor(bx + bw * 45 / 100 - 15, by + 9); tft.print("OPTIMAL");
  tft.setCursor(bx + bw * 80 / 100 - 9,  by + 9); tft.print("WET");

  // ── Progress bar ──
  int by2 = by + bh + 12;                    // y=226
  tft.drawRect(bx, by2, bw, 14, C_DGRAY);
  int fill = (int)(soilPct / 100.0f * (bw - 2));
  tft.fillRect(bx + 1, by2 + 1, fill, 12, sc);

  // Scale labels
  tft.setTextColor(C_DGRAY); tft.setTextSize(1);
  tft.setCursor(bx,           by2 + 18); tft.print("0%");
  tft.setCursor(bx + bw - 18, by2 + 18); tft.print("100%");
}