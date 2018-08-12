#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_NeoPixel.h>

// NeoPixel settings
const byte neoPin = 11;
const byte neoPixels = 24;
byte neoBright = 16;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(neoPixels, neoPin, NEO_GRB);

// DAC
Adafruit_MCP4725 dac;

// Sequencer variables
bool firstTime = true;
byte readPosition = 0;
byte writePosition = 0;
byte size = neoPixels;
uint16_t lastSizeRead = 0;
uint16_t lastPositionRead = 0;
uint16_t lastValueRead = 0;
uint16_t values[neoPixels];
byte writeBlink = 0;
uint16_t cyclesSinceLastGateRise = 0;
uint16_t cyclesSinceLastResetRise = 0;

bool buildingRamp = false;
byte rampStartPosition = 0;

// Wiring
#define PIN_CON_GATE PIN2
#define PIN_CON_RESET PIN3
#define PIN_CON_RAMP PIN4
#define PIN_BTN_PROGRAM PIN5
#define PIN_BTN_CLEAR PIN6
#define PIN_BTN_RESET PIN7
#define ANALOG_RANGE 2
#define ANALOG_POSITION 1
#define ANALOG_VALUE 0


void setup() {
    ring.begin();
    ring.setBrightness(neoBright);
    ring.show();

    dac.begin(0x62);

    pinMode(PIN_CON_GATE, INPUT); // interrupt for GATE connector
    attachInterrupt(digitalPinToInterrupt(PIN_CON_GATE), step, RISING);
    pinMode(PIN_CON_RESET, INPUT); // interrupt for RESET connector
    attachInterrupt(digitalPinToInterrupt(PIN_CON_RESET), reset, RISING);
    pinMode(PIN_CON_RAMP, INPUT); // input for PGM RAMP connector
    pinMode(PIN_BTN_PROGRAM, INPUT_PULLUP); // input for PGM button
    pinMode(PIN_BTN_CLEAR, INPUT_PULLUP); // input for CLR button
    pinMode(PIN_BTN_RESET, INPUT_PULLUP); // input for RST button

    for (int i = 0; i < neoPixels; i++) {
        values[i] = 0;
    }
}

uint16_t abs_diff(uint16_t a, uint16_t b) {
    return a > b ? a - b : b - a;
}

void loop() {
    bool forceReset = !digitalRead(PIN_BTN_RESET);
    if (forceReset) {
        readPosition = 0;
    }

    bool forceClear = !digitalRead(PIN_BTN_CLEAR);
    if (forceClear) {
        for (int i = 0; i < neoPixels; i++) {
            values[i] = 0;
        }
    }

    bool ramp = digitalRead(PIN_CON_RAMP);
    if (ramp && !buildingRamp) {
        rampStartPosition = readPosition;
        buildingRamp = true;
    } else if (buildingRamp && (!ramp || readPosition >= size)) {
        byte length = readPosition - rampStartPosition;
        for (int i = rampStartPosition; i <= readPosition; i++) {
            byte n = i - rampStartPosition;
            values[i] = n * 2048 / length;
        }
        buildingRamp = false;
    }

    uint16_t sizeRead = analogRead(ANALOG_RANGE);
    if (abs_diff(sizeRead, lastSizeRead) > 8) {
        lastSizeRead = sizeRead;
        size = sizeRead / 42;
    }

    bool movedPosition = false;
    uint16_t positionRead = analogRead(ANALOG_POSITION);
    if (abs_diff(positionRead, lastPositionRead) > 8) {
        lastPositionRead = positionRead;
        writePosition = positionRead / 42;
        movedPosition = true;
    }

    bool movedValue = false;
    uint16_t valueRead = (uint16_t)analogRead(ANALOG_VALUE) << 2;
    if (abs_diff(valueRead, lastValueRead) > 8) {
        lastValueRead = valueRead;
        movedValue = true;
    }

    if (firstTime) {
        movedValue = false;
        firstTime = false;
    }

    bool forceWrite = !digitalRead(PIN_BTN_PROGRAM);
    if ((movedValue && movedPosition) || forceWrite) {
        values[writePosition] = valueRead;
    }

    dac.setVoltage(values[readPosition], false);

    for (int i = 0; i < neoPixels; i++) {
        if (i == writePosition && i != readPosition) {
            ring.setPixelColor(i, ring.Color(writeBlink, values[i] >> 4, writeBlink));
        } else if (i == readPosition) {
            ring.setPixelColor(i, ring.Color(255, 0, i == writePosition ? writeBlink : 0));
        } else if (i < size) {
            ring.setPixelColor(i, ring.Color(0, values[i] >> 4, 20));
        } else {
            ring.setPixelColor(i, ring.Color(0, 0, 0));
        }
    }
    ring.setBrightness(neoBright);
    ring.show();
    writeBlink++;
    cyclesSinceLastGateRise++;
    cyclesSinceLastResetRise++;
}

void step() {
    if (cyclesSinceLastGateRise < 50) return;
    if (++readPosition >= size) readPosition = 0;
    cyclesSinceLastGateRise = 0;
}

void reset() {
    if (cyclesSinceLastResetRise < 50) return;
    readPosition = 0;
    cyclesSinceLastResetRise = 0;
}
