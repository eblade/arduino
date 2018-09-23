#include <Wire.h>
#include <stdlib.h>
#include <SSD1306AsciiWire.h>
#include <Adafruit_MCP4725.h>
#include <FastADC.h>

// Display
SSD1306AsciiWire oled;

//uint8_t col[2]; // Columns for ADC values.
//uint8_t rows; // Rows per line.

// ADC
//FastADC(analog, 1, true /*init*/);
#define ADC_MAX 1023

// DAC
Adafruit_MCP4725 dacA;
Adafruit_MCP4725 dacB;
#define DAC_MAX 4095

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

// Modes
#define MODE_DEBUG 0
#define MODE_R 1
//volatile uint8_t mode = 1;

// State
//uint16_t knob[] = { 0, 0, 0, 0 };
//uint8_t button[] = { 0, 0, 0, 0 };
//volatile uint16_t clockA = 0;
volatile bool rising = false;
volatile uint16_t a = 0;
volatile uint16_t pA_attack = 0;
volatile uint16_t pA_release = 0;
volatile uint16_t b = 0;
volatile uint16_t pB_attack = 0;
volatile uint16_t pB_release = 0;
//volatile uint16_t rA = 0;


void setup() {
    // Setup display
    Wire.begin();
    TWBR = 12; // 400 khz
    oled.begin(&Adafruit128x64, 0x3C);
    oled.displayRemap(true); // rotate 180 deg
    oled.setFont(System5x7);
    oled.clear();
    oled.println("Envelope Pusher");
    //oled.println("A: 9999 C: 9999");
    //oled.println("B: 9999 D: 9999");
    //oled.println("A: 9999 C: 9999");
    //oled.println("B: 9999 D: 9999");
    // Calculate columns for ADC values.  No RAM is used by strings.
    // Compiler replaces strlen() calc with 6 and 17.
    //col[0] = oled.fieldWidth(strlen("A: "));
    //col[1] = oled.fieldWidth(strlen("B: 9999 D: "));
    //rows = oled.fontRows();

    // set up timer1 at 1kHz
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 60; // Counter comparison level
    TCCR1B |= (1 << WGM12); // turn on CTC mode
    TCCR1B |= (1 << CS12); // prescaler at 1024
    TIMSK1 |= (1 << OCIE1A); // enable time compare interrupt


    // Setup DACs
    dacA.begin(0x63);
    dacB.begin(0x62);

    // Buttons should be INPUT_PULLUP
    //pinMode(BUTTON_1, INPUT_PULLUP);
    //pinMode(BUTTON_2, INPUT_PULLUP);
    //pinMode(BUTTON_3, INPUT_PULLUP);
    //pinMode(BUTTON_4, INPUT_PULLUP);
    //pinMode(BUTTON_TRIG, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(BUTTON_TRIG), trig, FALLING);

    // Digital Outputs should be OUTPUT
    //pinMode(OUTPUT_1, OUTPUT);
    //pinMode(OUTPUT_2, OUTPUT);
    //pinMode(OUTPUT_3, OUTPUT);
    //pinMode(OUTPUT_4, OUTPUT);

    // Digital Input should be INPUT
    pinMode(INPUT_TRIG, INPUT);
    attachInterrupt(digitalPinToInterrupt(INPUT_TRIG), change, CHANGE);

    // Analog inputs should use the FastADC lib
    //analog.reference(0, DEFAULT);
    //analog.reference(1, DEFAULT);
    //analog.reference(2, DEFAULT);
    //analog.reference(3, DEFAULT);
    //analog.handle(0, DEFAULT, handleKnob0);
    //analog.handle(1, DEFAULT, handleKnob1);
    //analog.handle(2, DEFAULT, handleKnob2);
    //analog.handle(3, DEFAULT, handleKnob3);
}

void loop() {
    //readButtons();

    pA_attack = (ADC_MAX - analogRead(0)) >> 2 + 1;
    pA_release = (ADC_MAX - analogRead(1)) >> 2 + 1;
    pB_attack = (ADC_MAX - analogRead(2)) >> 2 + 1;
    pB_release = (ADC_MAX - analogRead(3)) >> 2 + 1;

    //oled.clearField(0, 1, 5);
    //oled.print(a);

    //a += 100;
    //if (a > 4000) a = 0;

    //a = pA_attack;
    dacA.setVoltage(a, false);
    dacB.setVoltage(b, false);
    //dac1.setVoltage(analog.read(0) << 2, false);
    //dac2.setVoltage(knob[1] << 2, false);

    //uint8_t i;
    //for (i = 0; i < KNOBS; i++) {
    //    oled.clearField(col[i%2], 1 + rows*(i/2), 4);
    //    oled.print(knob[i]);
    //}
    //for (i = 0; i < KNOBS; i++) {
    //    oled.clearField(col[i%2], 3 + rows*(i/2), 4);
    //    oled.print(button[i]);
    //}
    //oled.clearField(col[0], 5, 5);
    //oled.print(clockA);
    //oled.clearField(col[1], 5, 5);
    //oled.print(a);

    //delay(10);
}

//uint16_t r(uint16_t t, uint16_t r) {
//    int16_t a = r == 0 ? 0 : 4096. - (4096. * (float)t) / ((float)(r >> 2));
//    return a >= 0 ? (uint16_t)a : 0;
//}
//
//uint16_t abs_diff(uint16_t a, uint16_t b) {
//    return a > b ? a - b : b - a;
//}
//
//void handleKnob0(uint16_t value, uint16_t dt) { handleKnob(0, value); }
//void handleKnob1(uint16_t value, uint16_t dt) { handleKnob(1, value); }
//void handleKnob2(uint16_t value, uint16_t dt) { handleKnob(2, value); }
//void handleKnob3(uint16_t value, uint16_t dt) { handleKnob(3, value); }
//
//void handleKnob(uint8_t n, uint16_t value) {
//    knob[n] = value;
//    //if (abs_diff(value, knob[n]) > ANALOG_SENSITIVITY) knob[n] = value;
//}
//
//void readButtons() {
//    button[0] = digitalRead(BUTTON_1);
//    button[1] = digitalRead(BUTTON_2);
//    button[2] = digitalRead(BUTTON_3);
//    button[3] = digitalRead(BUTTON_4);
//}

ISR(TIMER1_COMPA_vect) {
    if (rising) {
        attack(&a, pA_attack);
        attack(&b, pB_attack);
    } else {
        release(&a, pA_release);
        release(&b, pB_release);
    }
}

void attack(uint16_t *value, uint16_t attack) {
    if ((*value) < DAC_MAX) {
        if ((*value) <= (DAC_MAX - attack)) {
            (*value) += attack;
        } else {
            (*value) = DAC_MAX;
        }
     }
}

void release(uint16_t *value, uint16_t release) {
        if ((*value) > 0) {
            if ((*value) >= release) {
                (*value) -= release;
            } else {
                (*value) = 0;
            }
        }
}

void change() {
    if (digitalRead(INPUT_TRIG) == HIGH) {
        rising = true;
    } else {
        rising = false;
    }
}

