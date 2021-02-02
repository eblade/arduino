/*
Zerocross från AC dimmer modul kopplad till D2 med interrupt flagga.
Startar interrupt timer för att aktivera triac efter viss tid som bestäms av värdet i OCR1A register.

Värdet av OCR1A register styrs av PI(D) rutin som räknas ut från faktisk temperatur (thermocouple) och önskat värde Setpoint

Majoriteten av resterande kod är för LCD display rutin, läsa en rotary encoder och PWM för fläktkontroll.
*/


#include <max6675.h>                  // Library for Thermocouple
//#include <SSD1306AsciiAvrI2c.h>

//U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);
//SSD1306AsciiAvrI2c oled;


//ROBOTDYN AC DIMMER
#define ZERO_CROSS 2
#define TRIAC_GATE 3    //initialase port for dimmer: name(PinNumber);

//RobotDyn MOSFET
#define FAN 5
volatile uint8_t fanDuty = 255, lastFanDuty = 255;


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
volatile uint8_t displayUpdate = 0xff;
volatile uint16_t nSamples = 0;

#define VALUE_COLUMN 90
#define LED_RUNNING 4
#define SW_STOP_MODE 9
#define SW_START_SET 10
#define PARAMETER A1
#define VALUE A0

// States and such
#define STATE_IDLE 0
#define STATE_RUNNING 1
uint8_t state = STATE_IDLE;
volatile uint16_t lastA = 0;
volatile uint16_t lastB = 0;

/*working variables*/
const float sampleTime = 250;
volatile float measuredTemp;
volatile uint16_t heat = 0;
volatile float tempSetPoint = 100;
volatile float pTerm, iTerm, dTerm;
const float kp = 8.0, ki = 0.4;
const int16_t outMin = 0, outMax = 247;

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

    //readA();
    //init_lcd();
    analogWrite(FAN, fanDuty);
}

//void init_lcd() {
//    oled.begin(&Adafruit128x64, 0x3C);
//    oled.setFont(System5x7);
//    oled.clear();
//    oled.println("Temperature: ");
//    oled.println("Set point: ");
//    oled.println("Fan duty: ");
//    oled.println("Heat power: ");
//    oled.println("Samples: ");
//    oled.println("kP: ");
//    oled.println("kI: ");
//}

//void prepare_lcd(unsigned int row) {
//    oled.setRow(row);
//    oled.setCol(VALUE_COLUMN);
//    oled.print("      ");
//    oled.setCol(VALUE_COLUMN);
//}

//void update_lcd() {
//    //displayUpdate = 0xff;
//    if (displayUpdate & U_TEMP_IN) {
//        prepare_lcd(0);
//        oled.print((uint16_t)measuredTemp);
//        displayUpdate &= ~U_TEMP_IN;
//    }
//    if (displayUpdate & U_TEMP_SET) {
//        prepare_lcd(1);
//        oled.print((uint16_t)tempSetPoint);
//        displayUpdate &= ~U_TEMP_SET;
//    }
//    if (displayUpdate & U_HEAT) {
//        prepare_lcd(3);
//        oled.print(heat);
//        displayUpdate &= ~U_HEAT;
//    }
//    return;
//    if (displayUpdate & U_FAN) {
//        prepare_lcd(2);
//        oled.print(fanDuty);
//        displayUpdate &= ~U_FAN;
//    }
//   // if (displayUpdate & U_SAMPLES) {
//   //     prepare_lcd(4);
//   //     oled.print(nSamples);
//   //     displayUpdate &= ~U_SAMPLES;
//   // }
//    if (displayUpdate & U_KP) {
//        prepare_lcd(5);
//        oled.print((uint16_t)pTerm);
//        displayUpdate &= ~U_KP;
//    }
//    if (displayUpdate & U_KI) {
//        prepare_lcd(6);
//        oled.print((uint16_t)iTerm);
//        displayUpdate &= ~U_KI;
//    }
//}


volatile bool _blink = 0;
bool blink() {
    return _blink = !_blink;
}

void zeroCrossClock() { //zero cross detect
    TCCR1B = 0x04; //start timer with divide by 256 input
    TCNT1 = 0;   //reset timer - count from zero, compare to OCR1A, trigger timer1_compa_vect
    OCR1A = 290 - heat; // set compare register to happen sooner if heat is higher, though never later than a clock cycle minus some slack
}

ISR(TIMER1_COMPA_vect) {    //comparator match
    digitalWrite(TRIAC_GATE, HIGH); //set TRIAC gate to high
    TCNT1 = 65535 - 3;    //trigger pulse width, set to 16bit max value - 64ms, triac latching time, overflow to OVF which turnsoff.
}

ISR(TIMER1_OVF_vect) {    //timer1 overflow
    digitalWrite(TRIAC_GATE, LOW); //turn off TRIAC gate
    TCCR1B = 0x00;          //disable timer stopd unintended triggers
}

int16_t compute() {
  float error = tempSetPoint - measuredTemp;
  pTerm = kp * error;
  iTerm += ((ki * sampleTime / 1000.0) * error);
  if (iTerm > 170.) iTerm = 170.;
  //iTerm = constrain(iTerm, (float)outMin, (float)outMax);

  return (int16_t)(pTerm + iTerm);
}

//void readA() {
//    uint16_t newA = analogRead(PARAMETER);
//    if (abs(lastA - newA) < 16) {
//        return;
//    }
//    lastA = newA;
//    fanDuty = map(newA, 0, 1023, 25, 255);
//    analogWrite(FAN, fanDuty);
//    displayUpdate |= U_FAN;
//}

void readB() {
    uint16_t newB = analogRead(VALUE);
    if (abs(lastB - newB) < 16) {
        return;
    }
    lastB = newB;
    tempSetPoint = (float)(((uint16_t)map(newB, 0, 1023, 180, 244)) / 5 * 5);
    displayUpdate |= U_TEMP_SET;
}

void loop() {
    bool is_at_set;
    digitalWrite(LED_RUNNING, blink());

    if (digitalRead(SW_START_SET) == LOW) {
        state = STATE_RUNNING;
        digitalWrite(LED_RUNNING, HIGH);
    }

    float tempInPrev = measuredTemp;
    measuredTemp = thermocouple.readCelsius();
    if (abs(measuredTemp - tempInPrev) > 0.5) {
        displayUpdate |= U_TEMP_IN;
    }

    is_at_set = abs(measuredTemp - tempSetPoint) < 4;

    heat = constrain(compute(), outMin, outMax);
    nSamples++;
    displayUpdate |= U_SAMPLES;
    displayUpdate |= U_KI | U_KP;

    //if (digitalRead(SW_STOP_MODE) == LOW) {
    //    readA();
    //}

    readB();

    //update_lcd();
    if (is_at_set) {
        delay(100);
    } else {
        delay(25);
        digitalWrite(LED_RUNNING, blink());
        delay(25);
        digitalWrite(LED_RUNNING, blink());
        delay(25);
        digitalWrite(LED_RUNNING, blink());
        delay(25);
        digitalWrite(LED_RUNNING, blink());
    }
}
