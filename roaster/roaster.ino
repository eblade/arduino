/*
Zerocross från AC dimmer modul kopplad till D2 med interrupt flagga.
Startar interrupt timer för att aktivera triac efter viss tid som bestäms av värdet i OCR1A register.

Värdet av OCR1A register styrs av PI(D) rutin som räknas ut från faktisk temperatur (thermocouple) och önskat värde Setpoint

Majoriteten av resterande kod är för LCD display rutin, läsa en rotary encoder och PWM för fläktkontroll.
*/


#include <Wire.h>                     // Library for I2C communication
#include <max6675.h>                  // Library for Thermocouple
#include <stdlib.h>
#include <SSD1306AsciiAvrI2c.h>

//U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);
SSD1306AsciiAvrI2c oled;


//ROBOTDYN AC DIMMER
#define ZERO_CROSS 2
#define TRIAC_GATE 3             //initialase port for dimmer: name(PinNumber);

//RobotDyn MOSFET
#define FAN 5
unsigned int fanDuty = 255, lastFanDuty;


//MAX6675
#define THERMO_CLK 6
#define THERMO_CS  7
#define THERMO_DO  8
MAX6675 thermocouple(THERMO_CLK, THERMO_CS, THERMO_DO);

//Display updates
#define U_TEMP_IN 0x01
#define U_TEMP_SET 0x02
#define U_FAN 0x04
#define U_HEAT 0x08
#define U_SAMPLES 0x10
#define U_KP 0x20
#define U_KI 0x40
uint8_t displayUpdate = 0xff;
uint16_t nSamples = 0;

#define VALUE_COLUMN 90
#define LED_RUNNING 4
#define SW_STOP_MODE 9
#define SW_START_SET 10
#define PARAMETER A1
#define VALUE A0

// States and such
#define STATE_IDLE 0
#define STATE_RUNNING 1
unsigned char state = STATE_IDLE;
unsigned int lastA = 0, lastB = 0;

/*working variables*/
unsigned long previousTime, elapsedTime, currentTime;
unsigned int sampleTime = 500;
float tempIn, tempOut = 0;
float tempSetPoint = 100;
float pTerm, iTerm, dTerm, lastSetpoint = 1;
float kp = 30.0, ki = 1.2;
float lastkp = 0.0, lastki = 0.0;
float outMin = 10, outMax = 557;
unsigned int power;

void setup() {
  pinMode(ZERO_CROSS, INPUT_PULLUP);
  pinMode(TRIAC_GATE, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(LED_RUNNING, OUTPUT);
  pinMode(SW_START_SET, INPUT_PULLUP);
  pinMode(SW_STOP_MODE, INPUT_PULLUP);

  OCR1A = 100;      //initialize the comparator
  TIMSK1 = 0x03;    //enable comparator A and overflow interrupts
  TCCR1A = 0x00;    //timer control registers set for normal operation, timer disabled
  TCCR1B = 0x00;    //timer control registers set for normal operation, timer disabled

  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS), zeroCrossClock, RISING); /* flytta till loop? (med detach) om setpoint under x grade hoppa över*/

  init_lcd();
}

void init_lcd() {

    oled.begin(&Adafruit128x64, 0x3C);
    oled.setFont(System5x7);
    oled.clear();
    oled.println("Temperature: ");
    oled.println("Set point: ");
    oled.println("Fan duty: ");
    oled.println("Heat power: ");
    oled.println("Samples: ");
    oled.println("kP: ");
    oled.println("kI: ");

    update_lcd();
}

void prepare_lcd(unsigned int row) {
    oled.setRow(row);
    oled.setCol(VALUE_COLUMN);
    oled.print("      ");
    oled.setCol(VALUE_COLUMN);
}

int lcd_update = 0;
void update_lcd() {
    if (lcd_update++ < 1000) {
        return;
    }
    lcd_update = 0;

    if (displayUpdate & U_TEMP_IN) {
        prepare_lcd(0);
        oled.print(tempIn);
        displayUpdate &= ~U_TEMP_IN;
    }
    if (displayUpdate & U_TEMP_SET) {
        prepare_lcd(1);
        oled.print(tempSetPoint);
        displayUpdate &= ~U_TEMP_SET;
    }
    if (displayUpdate & U_FAN) {
        prepare_lcd(2);
        oled.print(fanDuty);
        displayUpdate &= ~U_FAN;
    }
    if (displayUpdate & U_HEAT) {
        prepare_lcd(3);
        oled.print(tempOut);
        displayUpdate &= ~U_HEAT;
    }
    if (displayUpdate & U_SAMPLES) {
        prepare_lcd(4);
        oled.print(nSamples);
        displayUpdate &= ~U_SAMPLES;
    }
    if (displayUpdate & U_KP) {
        prepare_lcd(5);
        oled.print(kp);
        displayUpdate &= ~U_KP;
    }
    if (displayUpdate & U_KI) {
        prepare_lcd(6);
        oled.print(ki);
        displayUpdate &= ~U_KI;
    }
}


bool _blink = 0;
bool blink() {
    return _blink = !_blink;
}

void zeroCrossClock() { //zero cross detect
  digitalWrite(TRIAC_GATE, HIGH); //set TRIAC gate to high
  TCCR1B = 0x04; //start timer with divide by 256 input
  TCNT1 = 0;   //reset timer - count from zero, compare to OCR1A, trigger timer1_compa_vect
}

ISR(TIMER1_COMPA_vect) {    //comparator match
  //digitalWrite(LED_RUNNING, blink());
  digitalWrite(TRIAC_GATE, LOW); //set TRIAC gate to high
  TCNT1 = 65535 - 3;    //trigger pulse width, set to 16bit max value - 64ms, triac latching time, overflow to OVF which turnsoff.
}

ISR(TIMER1_OVF_vect) {    //timer1 overflow
  digitalWrite(TRIAC_GATE, LOW); //turn off TRIAC gate
  TCCR1B = 0x00;          //disable timer stopd unintended triggers
}

int compute() {

  float error = tempSetPoint - tempIn;
  pTerm = kp * error;
  iTerm += ((ki * sampleTime / 1000.0) * error);
  iTerm = constrain(iTerm, outMin, outMax);

  return pTerm + iTerm;
}

void readA() {
    unsigned int newA = analogRead(PARAMETER);
    if (abs(lastA - newA) < 16) {
        return;
    }
    lastA = newA;
    fanDuty = map(newA, 0, 1023, 25, 255);
    analogWrite(FAN, fanDuty);
    displayUpdate |= U_FAN;
}

void readB() {
    unsigned int newB = analogRead(VALUE);
    if (abs(lastB - newB) < 16) {
        return;
    }
    lastB = newB;
    tempSetPoint = map(newB, 0, 1023, 40, 400);
    displayUpdate |= U_TEMP_SET;
}

void loop() {
    digitalWrite(LED_RUNNING, blink());
    currentTime = millis();
    elapsedTime = (currentTime - previousTime);

    if (digitalRead(SW_START_SET) == LOW) {
        state = STATE_RUNNING;
        digitalWrite(LED_RUNNING, HIGH);
    }

    if (elapsedTime >= sampleTime) {
        previousTime = currentTime;
        float tempInPrev = tempIn;
        tempIn = thermocouple.readCelsius();
        if (abs(tempIn - tempInPrev) > 0.5) {
            displayUpdate |= U_TEMP_IN;
        }

        tempOut = constrain(compute(), outMin, outMax);
        unsigned int powerLast = power;
        power = map(tempOut, outMax, outMin, 100, 0);
        if (power != powerLast) {
            displayUpdate |= U_HEAT;
        }
        OCR1A = (unsigned int)tempOut;
        //OCR1A = 10;
        nSamples++;
        displayUpdate |= U_SAMPLES;

        readA();
        readB();
    }

    update_lcd();
}
