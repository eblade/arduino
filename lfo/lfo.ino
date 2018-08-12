#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>

// Setup cap touch over I2C
Adafruit_FT6206 ctp = Adafruit_FT6206();

// Setup display over I2C
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define COLOR_BACK ILI9341_BLACK
#define COLOR_WAVE ILI9341_GREEN

TS_Point lastPoint = TS_Point();
bool lastTouched = 0;

unsigned char wave[320] = { 127 };


void setup() {
  Serial.begin(115200);
  Serial.println(F("Start"));

  tft.begin();

  if (!ctp.begin(40)) { // pass sensitivity
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Touchscreen started.");

  tft.fillScreen(COLOR_BACK);

  for (int i = 0; i < 320; i++) {
    wave[i] = 127;
    tft.drawPixel(127, i, COLOR_WAVE);
  }
}

void loop() {
  if (!ctp.touched()) {
    lastTouched = 0;
    return;
  }

  TS_Point p = ctp.getPoint();
 
  p.x = map(p.x, 0, 240, 240, 0);
  p.y = map(p.y, 0, 320, 320, 0);

  if (p.x == lastPoint.x && p.y == lastPoint.y) return;

  wave[p.y] = p.x;

  tft.drawLine(0, p.y, 239, p.y, COLOR_BACK);
  tft.drawPixel(p.x, p.y, COLOR_WAVE);

  if (lastTouched) {
    unsigned char y1 = wave[lastPoint.y];
    unsigned char y2 = wave[p.y];
    int_16t d = p.y - lastPoint.y;
    int_16t s = (y1 - y2) / d;
    int_16t dy = y2 - y1;
    for (int_16t i = lastPoint.y + 1; i < p.y; i++) {
      wave[i] = 
    }
  }

  lastTouched = 1;
  lastPoint.x = p.x;
  lastPoint.y = p.y;
}
