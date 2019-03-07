
#include "Wire.h"

#define ROW_COUNT 5
#define COL_COUNT 4

#define ROW0 9 // blue (13-16)
#define ROW1 8 // white (9-12)
#define ROW2 7 // grey (5-8)
#define ROW3 6 // purple (1-4)
#define ROW4 5 // orange (LEADERS)

#define COL0 18 // A0 yellow (3)
#define COL1 19 // A1 red (2)
#define COL2 20 // A2 brown (1)
#define COL3 21 // A3 green (0)

#define ROW_LEADER 0
#define ROW_A 1
#define ROW_B 2
#define ROW_C 3
#define ROW_D 4

#define LEADER_PLAY_PAUSE 0
#define LEADER_RECORD 1
#define LEADER_CONFIG 2
#define LEADER_CHANNEL_SELECT 3

#define SM_IDLE 0
#define SM_CONFIG 1
#define SM_CHANNEL_SELECT 2


//char row_pins[ROW_COUNT] = {ROW0};
//char row_pins[ROW_COUNT] = {ROW0, ROW1, ROW2, ROW3, ROW4};
char row_pins[ROW_COUNT] = {ROW4, ROW3, ROW2, ROW1, ROW0};
//char col_pins[COL_COUNT] = {COL0, COL1, COL2};
//char col_pins[COL_COUNT] = {COL0, COL1, COL2, COL3};
char col_pins[COL_COUNT] = {COL3, COL2, COL1, COL0};

unsigned char state[ROW_COUNT][COL_COUNT] = {};
unsigned char sm_state = SM_IDLE;

void setup() {
    Serial.begin(9600);
    Serial1.begin(31250);

    for (char i=0; i<COL_COUNT; i++) {
        pinMode(col_pins[i], OUTPUT);
        digitalWrite(col_pins[i], LOW);
    }
    for (char i=0; i<ROW_COUNT; i++) {
        pinMode(row_pins[i], INPUT);
    }
}

void loop() {
    delay(10);
    scan();
}

void scan() {
    for (char col=0; col<COL_COUNT; col++) {
        digitalWrite(col_pins[col], HIGH);
        for (char row=0; row<ROW_COUNT; row++) {
            bool pressed = digitalRead(row_pins[row]) == HIGH;
            if (pressed) {
                if (state[row][col] == 0) {
                    state[row][col] = 1;
                } else if (state[row][col] == 1) {
                    state[row][col] = 2;
                    press(row, col);
                }
            } else {
                if (state[row][col] == 1) {
                    state[row][col] = 0;
                } else if (state[row][col] == 2) {
                    state[row][col] = 3;
                } else if (state[row][col] == 3) {
                    state[row][col] = 0;
                }
            }
        }
        digitalWrite(col_pins[col], LOW);
    }
}

void press(unsigned char row, unsigned char col) {
    Serial.print("> ");
    Serial.print(row, DEC);
    Serial.print(", ");
    Serial.println(col, DEC);
    if (row == ROW_LEADER) {
        switch (col) {
            case LEADER_PLAY_PAUSE:
                press_play_pause();
                break;
            case LEADER_RECORD:
                press_record();
                break;
            case LEADER_CONFIG:
                press_config();
                break;
            case LEADER_CHANNEL_SELECT:
                press_channel_select();
                break;
        }
    } else {
        press_number((row - 1) * 4 + col);
    }
}

void press_play_pause() {
    Serial.println("PLAY PAUSE");
    sm_state = SM_IDLE;
}

void press_record() {
    Serial.println("RECORD");
    sm_state = SM_IDLE;
}

void press_config() {
    Serial.println("CONFIG");
    sm_state = SM_CONFIG;
}

void press_channel_select() {
    Serial.println("CHANNEL SELECT");
    sm_state = SM_CHANNEL_SELECT;
}

void press_number(unsigned char number) {
    Serial.print("NUMBER ");
    Serial.println(number, DEC);
    sm_state = SM_IDLE;
    //send(0xC0, number);
    send(0x90, 48+number, 0x7F);
    delay(100);
    send(0x90, 48+number, 0x00);
}

void send(unsigned char cmd, unsigned char value) {
    Serial1.write(cmd);
    Serial1.write(value);
}

void send(unsigned char cmd, unsigned char a, unsigned b) {
    Serial1.write(cmd);
    Serial1.write(a);
    Serial1.write(b);
}
