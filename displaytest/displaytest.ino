#include <Wire.h>
//#include "U8glib.h"
#include <stdlib.h>
#include <SSD1306AsciiWire.h>

//U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);
SSD1306AsciiWire oled;

void setup() {
    Wire.begin();
    //Serial.begin(9600);
    //Serial.println("Init");
    oled.begin(&Adafruit128x64, 0x3C);
    oled.setFont(utf8font10x16);
    oled.clear();
    //oled.print("Starting...");
}

void loop() {
    draw();
}

uint16_t counter = 0;

void draw() {
    //oled.setFont(u8g_font_helvB10);
    //oled.drawStr(0, 14, "Parameter");
    counter++;
    //uint16_t value = analogRead(0);
    char buffer[5];
    //itoa(value, buffer, 10);
    itoa(counter, buffer, 10);
    //oled.drawStr(0, 40, buffer);
    oled.setRow(5);
    oled.setCol(1);
    oled.print(buffer);
    oled.print("  ");
}
