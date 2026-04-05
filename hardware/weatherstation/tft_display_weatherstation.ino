//include libraries
#include <TFT_eSPI.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <math.h>

//define pins
#define DHT_PIN   26        //GPIO pin connected to DHT22 data wire
#define SOIL_PIN  36        //GPIO pin connected to soil sensor, analog input only
#define BTN1_PIN  25        //button 1 - toggles DHT page
#define BTN2_PIN  27        //button 2 - toggles BMP page
#define BTN3_PIN  33        //button 3 - toggles soil page
#define DHT_TYPE  DHT22     //using DHT22 for higher accuracy over DHT11

//create sensor and display objects
TFT_eSPI        tft;
DHT             dht(DHT_PIN, DHT_TYPE);     //link DHT sensor to its pin and type
Adafruit_BMP280 bmp;                        //BMP280 address is set later in setup

//define the four possible screens
enum Screen { HOME, DHT_PAGE, BMP_PAGE, SOIL_PAGE };
Screen currentScreen = HOME;    //start on home screen when powered on

//default values shown before first real sensor reading
float temperature = 25.0f, humidity = 60.0f;    //from DHT22
float bmpTemp     = 25.0f, pressure = 1013.0f;  //from BMP280, 1013 is standard sea level pressure
int   soilPct     = 50;                          //default soil moisture percentage
bool  bmpOK       = false;                       //tracks if BMP280 was found on startup

//timestamps for tracking when things last happened
unsigned long lastRead = 0, lastDraw = 0;               //sensor and screen timers
unsigned long lastBtn1 = 0, lastBtn2 = 0, lastBtn3 = 0; //individual button timers

//timing intervals in milliseconds
#define SENSOR_MS   2000    //DHT22 requires minimum 2 seconds between readings
#define DRAW_MS      800    //refresh screen roughly once per second
#define DEBOUNCE_MS  300    //ignore button noise for 300ms after each press

//colour definitions in RGB565 format used by the TFT display
#define C_NAVY   0x2008     //dark navy blue, used as background
#define C_WHITE  TFT_WHITE  //pure white for text and borders
#define C_CYAN   0xFD59     //cyan for temperature on DHT page
#define C_LBLUE  0xC4DF     //light blue for humidity elements
#define C_YELLOW 0xFFB6     //yellow for pressure gauge
#define C_LIME   0x87E0     //green for wet soil and plant icon
#define C_RED    0xF810     //red for dry soil warning
#define C_ORANGE 0xFBB6     //orange for BMP temperature
#define C_GRAY   TFT_DARKGREY   //gray for gauge tick marks
#define C_DGRAY  0x2104     //dark gray for subtle UI elements
#define C_DIM    0x1808     //very dim color for backgrounds and dividers
#define C_PINK   0xF81F     //pink for plant pot icon

void setup(){
  Serial.begin(115200);           //open serial monitor for debugging sensor values

  //configure buttons with internal pull-up, so they read HIGH normally and LOW when pressed
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  Wire.begin(21, 22);             //start I2C using GPIO21 as SDA and GPIO22 as SCL
  dht.begin();                    //initialise DHT22 sensor

  //try both possible BMP280 I2C addresses depending on how SDO pin is wired
  bmpOK = bmp.begin(0x76);                //try address 0x76 first (SDO connected to GND)
  if (!bmpOK) bmpOK = bmp.begin(0x77);   //if not found, try 0x77 (SDO connected to VCC)

  tft.init();                     //initialise the TFT display
  tft.setRotation(1);             //rotate to landscape orientation
  tft.fillScreen(C_NAVY);         //clear screen to navy background

  readSensors();                  //take first reading immediately instead of waiting 2 seconds
  drawHome();                     //draw home screen straight away
}

void loop(){
  checkButtons();                 //check buttons every loop so no press is missed
  unsigned long now = millis();   //get current time in milliseconds since power on

  //read sensors every 2 seconds
  if(now-lastRead >= SENSOR_MS) {lastRead = now; readSensors();}

  //redraw current screen every 800ms with latest data
  if(now-lastDraw >= DRAW_MS) {lastDraw = now; renderScreen();}
}

void readSensors(){
  float h = dht.readHumidity(), t = dht.readTemperature();

  //only update stored values if reading succeeded, NaN means the sensor failed
  if (!isnan(h) && !isnan(t)) {humidity = h; temperature = t;}

  if (bmpOK){
    bmpTemp  = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0f;   //convert Pascals to hPa by dividing by 100
  }

  //calibrated soil reading: 3265 = dry/air, 1500 = fully wet, mapped to 0-100%
  //constrain prevents values going outside 0-100 if sensor reads unexpectedly
  soilPct = constrain(map(analogRead(SOIL_PIN), 3265, 1500, 0, 100), 0, 100);

  //print all sensor values to serial monitor for debugging and calibration
  Serial.print("Temp: ");      Serial.println(temperature);
  Serial.print("Humidity: ");  Serial.println(humidity);
  Serial.print("BMP Temp: ");  Serial.println(bmpTemp);
  Serial.print("Pressure: ");  Serial.println(pressure);
  Serial.print("Soil raw: ");  Serial.println(analogRead(SOIL_PIN));   //raw ADC value for calibration
  Serial.print("Soil %: ");    Serial.println(soilPct);
  Serial.println("---");
}

void checkButtons(){
  unsigned long now = millis();

  //button 1 - if pressed and debounce time has passed, toggle between DHT page and home
  if (digitalRead(BTN1_PIN) == LOW && now - lastBtn1 > DEBOUNCE_MS){
    lastBtn1 = now;
    switchTo(currentScreen == DHT_PAGE ? HOME: DHT_PAGE);
  }

  //button 2 - toggles between BMP page and home
  if (digitalRead(BTN2_PIN) == LOW && now - lastBtn2 > DEBOUNCE_MS){
    lastBtn2 = now;
    switchTo(currentScreen == BMP_PAGE ? HOME: BMP_PAGE);
  }

  //button 3 - toggles between soil page and home
  if (digitalRead(BTN3_PIN) == LOW && now - lastBtn3 > DEBOUNCE_MS){
    lastBtn3 = now;
    switchTo(currentScreen == SOIL_PAGE ? HOME: SOIL_PAGE);
  }
}

void switchTo(Screen s){
  currentScreen = s;
  tft.fillScreen(C_NAVY);     //wipe screen to prevent old graphics showing through
  renderScreen();             //draw new screen immediately without waiting for timer
}

void renderScreen() {
  //call the correct draw function based on which screen is active
  switch (currentScreen) {
    case HOME:      drawHome();     break;
    case DHT_PAGE:  drawDHTPage();  break;
    case BMP_PAGE:  drawBMPPage();  break;
    case SOIL_PAGE: drawSoilPage(); break;
  }
}

//returns a colour based on soil moisture level for consistent color coding across the page
uint16_t soilColor(int pct) {
  if (pct > 60) return C_LIME;    //wet - green
  if (pct > 30) return C_YELLOW;  //optimal - yellow
  return C_RED;                   //dry - red
}

//AI was used to help in design
//homepage thermometer icon - decorative only, not linked to live data
void drawThermometerIcon(int cx, int cy, uint16_t col) {
  int top = cy - 52;
  tft.fillRoundRect(cx - 5, top, 10, 46, 5, C_DIM);      //tube background
  tft.drawRoundRect(cx - 5, top, 10, 46, 5, col);         //tube outline
  tft.fillRoundRect(cx - 3, top + 14, 6, 28, 2, col);     //liquid fill inside tube
  for (int t = 0; t < 4; t++)
    tft.drawLine(cx + 5, top + 5 + t * 10, cx + 9, top + 5 + t * 10, col); //tick marks on side
  tft.fillCircle(cx, cy, 10, C_DIM);      //bulb background
  tft.drawCircle(cx, cy, 10, col);        //bulb outline
  tft.fillCircle(cx, cy,  6, col);        //bulb fill
}

//homepage raindrop icon - decorative only
void drawRaindropIcon(int cx, int cy, uint16_t col) {
  //draw the pointed top of the drop using decreasing width lines
  for (int i = 0; i <= 20; i++) {
    int w = (int)((1.0f - (float)i / 20.0f) * 12.0f);
    if (w > 0) tft.drawLine(cx - w, cy - 12 - i, cx + w, cy - 12 - i, col);
  }
  tft.fillCircle(cx, cy-6, 12, col);         //round bottom of the drop
  tft.fillCircle(cx - 4, cy - 6, 3, 0xC71F); //highlight shadow circle
  tft.fillCircle(cx - 4, cy - 6, 1, 0xEEFF); //highlight bright spot for 3D effect
}

//homepage pressure gauge icon - decorative only
void drawPressureIcon(int cx, int cy, uint16_t col) {
  int R = 30;

  //fill the top half circle with white to create gauge background
  for (int y = 0; y <= R; y++) {
    int hw = (int)sqrtf((float)(R * R - y * y));
    tft.drawLine(cx - hw, cy - y, cx + hw, cy - y, C_WHITE);
  }

  //draw the arc border of the gauge by plotting pixels along the semicircle
  for (float a = 0.0f; a <= 180.0f; a += 2.0f) {
    float rad = a * (PI / 180.0f);
    tft.drawPixel(cx + (int)(R * cosf(rad)), cy - (int)(R * sinf(rad)), 0x0000);
    tft.drawPixel(cx + (int)((R - 1) * cosf(rad)), cy - (int)((R - 1) * sinf(rad)), 0x0000);
  }

  tft.drawLine(cx - R, cy, cx + R, cy, 0x0000);  //flat bottom line of the gauge

  //draw red needle pointing at 60 degrees (decorative fixed position)
  tft.drawLine(cx, cy,
               cx + (int)((R-6)*cosf(60.0f*PI/180.0f)),
               cy - (int)((R-6)*sinf(60.0f*PI/180.0f)), 0xF800);

  //draw needle again 1 pixel over to make it 2 pixels thick
  tft.drawLine(cx+1, cy,
               cx+1+(int)((R-6)*cosf(60.0f*PI/180.0f)),
               cy-(int)((R-6)*sinf(60.0f*PI/180.0f)), 0xF800);

  tft.fillCircle(cx, cy, 4, 0x0000);     //needle pivot cover
  tft.fillCircle(cx, cy, 2, C_NAVY);     //inner dot to hide needle base
}

//homepage plant icon - decorative only
void drawPlantIcon(int cx, int cy, uint16_t col) {
  tft.fillRect(cx - 12, cy - 16, 24, 16, C_PINK);    //plant pot body fill
  tft.drawRect(cx - 12, cy - 16, 24, 16, col);        //plant pot body outline
  tft.fillRect(cx - 14, cy - 17, 28,  4, C_PINK);     //pot rim fill
  tft.drawRect(cx - 14, cy - 17, 28,  4, col);         //pot rim outline
  int sb = cy - 17, st = sb - 34;                      //stem bottom and top positions
  tft.drawLine(cx,     sb, cx,     st, col);            //plant stem left pixel
  tft.drawLine(cx + 1, sb, cx + 1, st, col);            //plant stem right pixel for thickness
  int lbx = cx + 1, lby = st + 18;                     //leaf starting position

  //draw leaf using circles that grow then shrink along a curved path using sin curve
  for (int s = 0; s <= 7; s++) {
    float t  = (float)s / 7.0f;
    int lx = lbx + (int)(t * 24), ly = lby - (int)(t * 20);
    int lr = (int)(10.0f * sinf(t * PI));   //radius peaks at midpoint of the leaf
    if (lr > 1) tft.fillCircle(lx, ly, lr, col);
  }
  tft.drawLine(lbx, lby, lbx + 22, lby - 18, C_NAVY);  //erase leaf midrib with background color
}

//home screen
void drawHome() {
  tft.fillScreen(C_NAVY);
  uint16_t br = C_DIM;

  //draw decorative corner brackets
  tft.drawLine(2,  2,  20,  2, br); tft.drawLine(2,   2,   2,  20, br);    //top left
  tft.drawLine(300,2, 318,  2, br); tft.drawLine(318,  2, 318,  20, br);   //top right
  tft.drawLine(2, 238, 20, 238,br); tft.drawLine(2,  220,   2, 238, br);   //bottom left
  tft.drawLine(300,238,318,238,br); tft.drawLine(318,220, 318, 238, br);   //bottom right

  //draw centered title text
  tft.setTextColor(C_WHITE); tft.setTextSize(3);
  tft.setCursor(160 - (8*18)/2, 68);  tft.print("AZARIA'S"); //8 chars * 18px wide / 2 to center
  tft.setCursor(160 - (7*18)/2, 100); tft.print("WEATHER");
  tft.setCursor(160 - (7*18)/2, 132); tft.print("STATION");
  tft.drawLine(108, 164, 212, 164, C_DIM);    //decorative underline below title

  //draw the four sensor icons in each corner area
  drawThermometerIcon(62, 100, C_CYAN);     //top left - temperature
  drawRaindropIcon(258, 96, C_LBLUE);       //top right - humidity
  drawPressureIcon(62, 200, C_YELLOW);      //bottom left - pressure
  drawPlantIcon(258, 210, C_LIME);          //bottom right - soil
}

//page graphics

//draws a large thermometer with liquid fill height based on temperature value
void drawBigThermometer(int cx, int cy, float tempC, uint16_t col) {
  int tubeH = 110;        //total tube height in pixels
  int top   = cy - tubeH; //y coordinate of tube top

  tft.fillRoundRect(cx - 9, top, 18, tubeH, 9, C_DIM);   //tube background
  tft.drawRoundRect(cx - 9, top, 18, tubeH, 9, col);      //tube outline

  //map temperature -10 to 50 degrees onto the tube height, clamped to valid range
  int fillH = constrain((int)map((long)tempC, -10, 50, 0, tubeH - 12), 0, tubeH - 12);
  if (fillH > 0)
    //Y position moves up as fill increases so liquid appears to rise from the bottom
    tft.fillRoundRect(cx - 6, top + 6 + (tubeH - 12 - fillH), 12, fillH, 4, col);

  //draw 6 tick marks on the right side of the tube
  for (int t = 0; t < 6; t++)
    tft.drawLine(cx + 9,  top + 8 + t * 18,
                 cx + 14, top + 8 + t * 18, col);

  tft.fillCircle(cx, cy + 16, 18, C_DIM);    //bulb background
  tft.drawCircle(cx, cy + 16, 18, col);      //bulb outline
  tft.fillCircle(cx, cy + 16, 12, col);      //bulb fill
}

//draws a large raindrop shape for the humidity panel
void drawBigDrop(int cx, int cy, uint16_t col) {
  //draw pointed top of drop with lines decreasing in width from bottom to top
  for (int i = 0; i <= 44; i++) {
    int w = (int)((1.0f - (float)i / 44.0f) * 26.0f);
    if (w > 0) tft.drawLine(cx - w, cy - 26 - i, cx + w, cy - 26 - i, col);
  }
  tft.fillCircle(cx, cy-18, 26, col);          //round bottom of drop
  tft.fillCircle(cx - 8, cy - 8, 6, 0xC71F);  //shadow highlight
  tft.fillCircle(cx - 8, cy - 8, 3, 0xEEFF);  //bright spot for 3D look
}

//draws a semicircular pressure gauge with a live needle that moves with pressure value
void drawBigGauge(int cx, int cy, float hpa, uint16_t col) {
  int R = 56;   //radius of the gauge in pixels

  //fill the semicircle background
  for (int y = 0; y <= R; y++) {
    int hw = (int)sqrtf((float)(R*R - y*y));
    tft.drawLine(cx - hw, cy - y, cx + hw, cy - y, C_DIM);
  }

  //draw the arc border by plotting pixels every 1.5 degrees around the semicircle
  for (float a = 0.0f; a <= 180.0f; a += 1.5f) {
    float rad = a * (PI / 180.0f);
    for (int t = 0; t < 2; t++)   //draw 2 pixels thick for visible border
      tft.drawPixel(cx + (int)((R-t)*cosf(rad)), cy - (int)((R-t)*sinf(rad)), col);
  }

  tft.drawLine(cx - R, cy, cx + R, cy, col);   //flat base line of gauge

  //draw 5 tick marks evenly spaced 45 degrees apart along the arc
  for (int i = 0; i <= 4; i++) {
    float a  = i * 45.0f * (PI / 180.0f);
    int x1 = cx + (int)((R - 4) * cosf(a));
    int y1 = cy - (int)((R - 4) * sinf(a));
    int x2 = cx + (int)((R - 14)* cosf(a));
    int y2 = cy - (int)((R - 14)* sinf(a));
    tft.drawLine(x1, y1, x2, y2, C_GRAY);
  }

  //calculate needle angle from pressure value
  //norm maps 950-1050 hPa to 0.0-1.0, needle sweeps from left (low) to right (high)
  float norm    = constrain((hpa - 950.0f) / 100.0f, 0.0f, 1.0f);
  float needleA = (1.0f - norm) * PI;   //convert to radians, flipped so low = left

  //calculate needle tip coordinates using trigonometry
  int nx = cx + (int)((R - 14) * cosf(needleA));
  int ny = cy - (int)((R - 14) * sinf(needleA));

  //draw needle twice 1 pixel apart to make it 2px thick
  tft.drawLine(cx, cy, nx, ny, C_WHITE);
  tft.drawLine(cx+1, cy, nx+1, ny, C_WHITE);

  tft.fillCircle(cx, cy, 6, col);      //needle pivot cover circle
  tft.fillCircle(cx, cy, 3, C_NAVY);  //center dot cutout

  //L and H labels at each end of the gauge
  tft.setTextColor(C_DGRAY); tft.setTextSize(1);
  tft.setCursor(cx - R - 8, cy + 4); tft.print("L");
  tft.setCursor(cx + R + 2, cy + 4); tft.print("H");
}

//dht page - shows temperature and humidity from DHT22
void drawDHTPage() {
  tft.fillScreen(C_NAVY);
  tft.drawLine(160, 0, 160, 240, C_DIM);   //vertical divider splitting page into two halves

  //left half - temperature
  tft.setTextColor(C_CYAN); tft.setTextSize(1);
  tft.setCursor(80 - (11*6)/2, 8);         //center the 11 character label over left half
  tft.print("TEMPERATURE");

  drawBigThermometer(80, 152, temperature, C_CYAN);   //draw thermometer with live temperature

  //format temperature as string with 1 decimal place and degree symbol
  char tStr[8]; dtostrf(temperature, 4, 1, tStr);
  char tDisp[12]; snprintf(tDisp, sizeof(tDisp), "%s\xF7""C", tStr);   //\xF7 is degree symbol
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(80 - (int)(strlen(tDisp)*12)/2, 202);  //center text based on its pixel width
  tft.print(tDisp);

  //right half - humidity
  tft.setTextColor(C_LBLUE); tft.setTextSize(1);
  tft.setCursor(240 - (8*6)/2, 8);         //center the 8 character label over right half
  tft.print("HUMIDITY");

  drawBigDrop(240, 112, C_LBLUE);           //draw raindrop graphic

  //format humidity as percentage string
  char hStr[8]; dtostrf(humidity, 4, 1, hStr);
  char hDisp[10]; snprintf(hDisp, sizeof(hDisp), "%s%%", hStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(240 - (int)(strlen(hDisp)*12)/2, 168);
  tft.print(hDisp);

  //humidity bar - outer border is fixed, inner fill scales with humidity value
  tft.drawRect(170, 196, 140, 10, C_LBLUE);                              //fixed border 140px wide
  int bw = (int)(constrain(humidity, 0.0f, 100.0f) / 100.0f * 138.0f);  //fill capped at 138px to stay inside border
  tft.fillRect(171, 197, bw, 8, C_LBLUE);                                //fill proportional to humidity
}

//bmp page - shows pressure and temperature from BMP280
void drawBMPPage() {
  tft.fillScreen(C_NAVY);

  //if sensor was not found at startup, show error and exit early
  if (!bmpOK) {
    tft.setTextColor(C_RED); tft.setTextSize(2);
    tft.setCursor(30, 110); tft.print("BMP280 not found");
    return;
  }

  tft.drawLine(160, 0, 160, 240, C_DIM);   //vertical divider between pressure and temperature

  //left half - pressure gauge
  tft.setTextColor(C_YELLOW); tft.setTextSize(1);
  tft.setCursor(80 - (8*6)/2, 8);
  tft.print("PRESSURE");

  drawBigGauge(80, 162, pressure, C_YELLOW);   //draw gauge with live pressure, needle moves

  //format pressure with no decimal places and hPa unit
  char pStr[8]; dtostrf(pressure, 5, 0, pStr);
  char pDisp[12]; snprintf(pDisp, sizeof(pDisp), "%s hPa", pStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(1);
  tft.setCursor(80 - (int)(strlen(pDisp)*6)/2, 192);
  tft.print(pDisp);

  //right half - BMP280 temperature
  tft.setTextColor(C_ORANGE); tft.setTextSize(1);
  tft.setCursor(240 - (11*6)/2, 8);
  tft.print("TEMPERATURE");

  drawBigThermometer(240, 152, bmpTemp, C_ORANGE);   //BMP has its own temperature sensor

  //format BMP temperature with degree symbol
  char btStr[8]; dtostrf(bmpTemp, 4, 1, btStr);
  char btDisp[12]; snprintf(btDisp, sizeof(btDisp), "%s\xF7""C", btStr);
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(240 - (int)(strlen(btDisp)*12)/2, 202);
  tft.print(btDisp);
}

//soil page - shows moisture level with color coded indicators
void drawSoilPage() {
  tft.fillScreen(C_NAVY);
  uint16_t sc = soilColor(soilPct);   //get color based on moisture level, used throughout page

  //title color changes with moisture level
  tft.setTextColor(sc); tft.setTextSize(1);
  tft.setCursor(160 - (8*6)/2, 10);
  tft.print("MOISTURE");

  //large percentage number also colored by moisture level
  char sStr[6]; snprintf(sStr, sizeof(sStr), "%d%%", soilPct);
  tft.setTextColor(sc); tft.setTextSize(5);
  tft.setCursor(160 - (int)(strlen(sStr)*30)/2, 34);   //center based on character count
  tft.print(sStr);

  //status word changes between DRY, OPTIMAL, and WET with matching threshold boundaries
  const char* status = soilPct > 60 ? "WET"    :
                       soilPct > 30 ? "OPTIMAL" : "DRY";
  tft.setTextColor(C_WHITE); tft.setTextSize(2);
  tft.setCursor(160 - (int)(strlen(status)*12)/2, 90);  //re-centers each time as word length changes
  tft.print(status);

  //colored zone bar - three fixed sections showing dry, optimal, and wet ranges
  int bx = 10, by = 126, bw = 300, bh = 28;
  tft.fillRect(bx,             by, bw*30/100,         bh, C_RED);     //0-30% dry zone
  tft.fillRect(bx + bw*30/100, by, bw*30/100,         bh, C_YELLOW);  //30-60% optimal zone
  tft.fillRect(bx + bw*60/100, by, bw - (bw*60/100),  bh, C_LIME);   //60-100% wet zone
  tft.drawRect(bx, by, bw, bh, C_WHITE);                               //outer border
  tft.drawLine(bx + bw*30/100, by, bx + bw*30/100, by+bh, C_WHITE);   //divider at 30%
  tft.drawLine(bx + bw*60/100, by, bx + bw*60/100, by+bh, C_WHITE);   //divider at 60%

  //triangle pointer marker that moves left and right above the bar with moisture value
  int mx = constrain(bx + (int)(soilPct/100.0f * bw), bx+3, bx+bw-3);  //pixel position clamped to bar width
  tft.fillTriangle(mx-6, by-10, mx+6, by-10, mx, by, C_WHITE);          //downward pointing triangle
  tft.drawLine(mx, by-10, mx, by-16, C_WHITE);                           //tick line above triangle

  //zone labels printed in navy so they appear on top of the colored zones
  tft.setTextColor(C_NAVY); tft.setTextSize(1);
  tft.setCursor(bx + bw*15/100 - 9,  by+10); tft.print("DRY");
  tft.setCursor(bx + bw*45/100 - 15, by+10); tft.print("OPTIMAL");
  tft.setCursor(bx + bw*80/100 - 9,  by+10); tft.print("WET");

  //secondary progress bar below the zone bar, same color as moisture level
  int by2 = by + bh + 20;
  tft.drawRect(bx, by2, bw, 14, C_DGRAY);                           //fixed border
  int fill = (int)(soilPct / 100.0f * (bw-2));                      //fill width scales with percentage
  tft.fillRect(bx+1, by2+1, fill, 12, sc);                          //fill uses same color as rest of page

  //0% and 100% labels at each end of the progress bar
  tft.setTextColor(C_DGRAY); tft.setTextSize(1);
  tft.setCursor(bx,        by2+18); tft.print("0%");
  tft.setCursor(bx+bw-18,  by2+18); tft.print("100%");

  tft.drawLine(bx, 24, bx+bw, 24, C_DIM);   //thin divider line below the moisture percentage number
}
