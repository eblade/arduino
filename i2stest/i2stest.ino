/*
 This example generates a sine wave based tone at a specified frequency
 and sample rate. Then outputs the data using the I2S interface.

 Public Domain
*/

#include <Wire.h>
#include <I2S.h>

#define SAMPLERATE 44100  // sample rate in Hz

//int16_t sinetable[SAMPLERATE / FREQUENCY];
uint32_t sample = 0;

#define PI 3.14159265

void setup() {
    Serial.begin(115200);
    Serial.println("I2S sine wave tone");

    // start I2S at the sample rate with 16-bits per sample
    if (!I2S.begin(I2S_PHILIPS_MODE, SAMPLERATE, 16)) {
        Serial.println("Failed to initialize I2S!");
        while (1); // do nothing
    }

    pinMode(7, INPUT_PULLUP);
}

uint32_t frequency = 440;
uint32_t amplitude = 10000;

void loop() {
    if (sample >= (SAMPLERATE / frequency)) {
        sample = 0;
    }

    if (digitalRead(7)) {
        return;
    }

    frequency = analogRead(A0);

    int32_t y1;
    if (sample > (SAMPLERATE / frequency) / 2) {
        y1 = sample * (amplitude / (SAMPLERATE / 2)) - amplitude / 2;
    } else {
        y1 = amplitude - (sample - (SAMPLERATE / frequency) / 2) * (amplitude / (SAMPLERATE / 2)) - amplitude / 2;
    }

    I2S.write(y1);
    I2S.write(y1);

    sample++;
}

