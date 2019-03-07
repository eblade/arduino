#include <Wire.h>

#define PIN_GATE PIN2
#define PIN_LED0 PIN4
#define PIN_LED1 PIN5
#define PIN_LED2 PIN6
#define PIN_LED3 PIN7
#define PIN_LEDG 12

unsigned char state = 0;
unsigned char output = 0;

void setup() {
    pinMode(PIN_GATE, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_GATE), gate_rising, RISING);

    pinMode(PIN_LED0, OUTPUT);
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_LED3, OUTPUT);
    pinMode(PIN_LEDG, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 0);
}

void loop() {
    digitalWrite(PIN_LED0, state && (output == 0));
    digitalWrite(PIN_LED1, state && (output == 1));
    digitalWrite(PIN_LED2, state && (output == 2));
    digitalWrite(PIN_LED3, state && (output == 3));
    digitalWrite(PIN_LEDG, state);
    state = 0;
    delay(10);
}

void gate_rising() {
    state = 1;
    output++;
    output %= 4;
    digitalWrite(PIN_LEDG, 1);
}

