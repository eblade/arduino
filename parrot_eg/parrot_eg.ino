#include <Wire.h>
#include <Adafruit_MCP4725.h>

#define MEM_SIZE 512

Adafruit_MCP4725 dac;

uint16_t value;
uint16_t gate;
uint16_t counter = 0;
uint16_t memory[MEM_SIZE];

uint16_t position;
bool playing;
bool recording;
bool will_record;

void setup() {
    Serial.begin(9600);
    Serial.println("Init");
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN2, INPUT);
    dac.begin(0x62);

    for (int i = 0; i < MEM_SIZE; i++) {
        memory[i] = 511;
    }
    playing = false;
    recording = false;
    will_record = false;
}

void loop() {
    delay(3);
    if (playing) {
        dac.setVoltage(memory[position++] * 4, false);
        if (position >= MEM_SIZE) {
            playing = false;
            Serial.println("Done playing.");
        }
        if (!will_record && digitalRead(PIN2)) {
            Serial.println("Will record!");
            will_record = true;
        }
    } else if (recording) {
        memory[position] = (uint16_t)analogRead(0);
        dac.setVoltage(memory[position] * 4, false);
        position++;
        if (position >= MEM_SIZE) {
            recording = false;
            Serial.println("Done recording.");
        }
    } else {
        if (digitalRead(PIN2)) {
            Serial.println("Recording...");
            position = 0;
            recording = true;
        } else if (analogRead(1) > 800) {
            if (will_record) {
                Serial.println("Recording...");
                position = 0;
                will_record = false;
                recording = true;
            } else {
                Serial.println("Playing...");
                position = 0;
                playing = true;
            }
        }
    }
}
