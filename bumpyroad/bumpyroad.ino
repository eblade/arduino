/**************************************************************************/
/*! 
    @file     trianglewave.pde
    @author   Adafruit Industries
    @license  BSD (see license.txt)

    This example will generate a triangle wave with the MCP4725 DAC.   

    This is an example sketch for the Adafruit MCP4725 breakout board
    ----> http://www.adafruit.com/products/935
 
    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

void setup(void) {
    Serial.begin(9600);
    Serial.println("Hello!");
    pinMode(LED_BUILTIN, OUTPUT);

    // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
    // For MCP4725A0 the address is 0x60 or 0x61
    // For MCP4725A2 the address is 0x64 or 0x65
    dac.begin(0x62);


    randomSeed(analogRead(0));
    Serial.println("Starting");
}

uint32_t baseLine = 3000;
int32_t diff = 0;

void loop(void) {
    uint32_t chance;
    //uint32_t baseLineReading;
    //baseLineReading = analogRead(1);
    //baseLine = baseLineReading * 8;
    chance = random(1000);
    if (chance == 0) {
        diff = 1000;
    }
    dac.setVoltage(baseLine + diff, false);
    if (diff > 0) {
        diff--;
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}
