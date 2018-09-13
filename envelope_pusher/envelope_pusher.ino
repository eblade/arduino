#include <Wire.h>
#include <stdlib.h>
#include <SSD1306AsciiWire.h>
#include <Adafruit_MCP4725.h>

// Display
SSD1306AsciiWire oled;

uint8_t col[2]; // Columns for ADC values.
uint8_t rows; // Rows per line.

// DAC
Adafruit_MCP4725 dac1;
Adafruit_MCP4725 dac2;

// Wiring
#define BUTTONS 4
#define BUTTON_1 4
#define BUTTON_2 7
#define BUTTON_3 8
#define BUTTON_4 12
#define OUTPUT_1 5
#define OUTPUT_2 6
#define OUTPUT_3 9
#define OUTPUT_4 10
#define KNOBS 4
#define BUTTON_TRIG 2
#define INPUT_TRIG 3

// Settings
#define ANALOG_SENSITIVITY 10

// State
uint16_t knob[] = { 0, 0, 0, 0 };
uint8_t button[] = { 0, 0, 0, 0 };


void setup() {
    // Setup display
    Wire.begin();
    oled.begin(&Adafruit128x64, 0x3C);
    oled.displayRemap(true); // rotate 180 deg
    oled.setFont(System5x7);
    oled.clear();
    oled.println("Envelope Pusher");
    oled.println("A: 9999 C: 9999");
    oled.println("B: 9999 D: 9999");
    oled.println("A: 9999 C: 9999");
    oled.println("B: 9999 D: 9999");
    // Calculate columns for ADC values.  No RAM is used by strings.
    // Compiler replaces strlen() calc with 6 and 17.
    col[0] = oled.fieldWidth(strlen("A: "));
    col[1] = oled.fieldWidth(strlen("B: 9999 D: "));
    rows = oled.fontRows();

    // Setup DACs
    dac1.begin(0x62);
    dac2.begin(0x63);

    // Buttons should be INPUT_PULLUP
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
    pinMode(BUTTON_4, INPUT_PULLUP);
    pinMode(BUTTON_TRIG, INPUT_PULLUP);

    // Digital Outputs should be OUTPUT
    pinMode(OUTPUT_1, OUTPUT);
    pinMode(OUTPUT_2, OUTPUT);
    pinMode(OUTPUT_3, OUTPUT);
    pinMode(OUTPUT_4, OUTPUT);

    // Digital Input shold be INPUT
    pinMode(INPUT_TRIG, INPUT);
}

uint16_t drawer = 0;
void loop() {
    readKnobs();
    readButtons();

    uint8_t i;
    for (i = 0; i < KNOBS; i++) {
        oled.clearField(col[i%2], 1 + rows*(i/2), 4);
        oled.print(knob[i]);
    }
    for (i = 0; i < KNOBS; i++) {
        oled.clearField(col[i%2], 3 + rows*(i/2), 4);
        oled.print(button[i]);
    }
}

uint16_t abs_diff(uint16_t a, uint16_t b) {
    return a > b ? a - b : b - a;
}

void readKnobs() {
    for (uint8_t i = 0; i < KNOBS; i++) {
        uint16_t readValue = analogRead(i);
        if (abs_diff(readValue, knob[i]) > ANALOG_SENSITIVITY) knob[i] = readValue;
    }
}

void readButtons() {
    button[0] = digitalRead(BUTTON_1);
    button[1] = digitalRead(BUTTON_2);
    button[2] = digitalRead(BUTTON_3);
    button[3] = digitalRead(BUTTON_4);
}
