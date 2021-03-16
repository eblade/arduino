#define INT_CLOCK_PIN 0
#define STEP_OUT_PIN 1
#define EXT_CLOCK_IN_PIN 2
#define SEQ_RESET_PIN 3
#define SEQ_DIR_PIN 7
#define SEQ_ZERO_PIN 8
#define UNUSED_PIN_1 9
#define UNUSED_PIN_2 10
#define UNUSED_PIN_3 11 // MOSI
#define UNUSED_PIN_4 12 // MISO
#define UNUSED_PIN_5 13 // SCK

#define SEQ_DIR_FORWARDS 1
#define SEQ_DIR_BACKWARDS 0

// 74HC595 PINS
#define LATCH_PIN 4
#define DATA_PIN 5
#define CLOCK_PIN 6

// Analog input pins
#define EXT_POS_5V_PIN A0
#define RATE_5V_PIN A1
#define INT_POS_5V_PIN A2
#define PULSE_LENGTH_PIN A3

int8_t step = 0;
uint8_t direction = SEQ_DIR_FORWARDS;
uint16_t stepOutCycles = 0;
int16_t pulseLen = 5;
int16_t extPosRaw = 0;
int8_t extPos = -1;
int16_t intRate = 0;
int16_t intRateCycles = 1;
int16_t intRateCyclesOn = 0;


void setup() {
  // Set up clock-related pins
  pinMode(INT_CLOCK_PIN, OUTPUT);
  pinMode(STEP_OUT_PIN, OUTPUT);
  pinMode(EXT_CLOCK_IN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(EXT_CLOCK_IN_PIN), handleExtClockIn, RISING);
  pinMode(SEQ_RESET_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SEQ_RESET_PIN), handleReset, RISING);
  pinMode(SEQ_DIR_PIN, INPUT);
  pinMode(SEQ_ZERO_PIN, INPUT);

  // Set up unused pins
  pinMode(UNUSED_PIN_1, INPUT_PULLUP);
  pinMode(UNUSED_PIN_2, INPUT_PULLUP);
  pinMode(UNUSED_PIN_3, INPUT_PULLUP);
  pinMode(UNUSED_PIN_4, INPUT_PULLUP);
  pinMode(UNUSED_PIN_5, INPUT_PULLUP);

  // Set up shift register pins
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
}

void loop() {
  if (stepOutCycles > 0) {
    digitalWrite(STEP_OUT_PIN, HIGH);
    stepOutCycles--;
  } else {
    digitalWrite(STEP_OUT_PIN, LOW);
  }
  direction = digitalRead(SEQ_DIR_PIN);
  if (digitalRead(SEQ_ZERO_PIN) == HIGH) {
    step = -1;
  }
  pulseLen = analogRead(PULSE_LENGTH_PIN);
  extPosRaw = analogRead(EXT_POS_5V_PIN);
  extPos = map(extPosRaw, 200, 1023, -1, 15);
  intRate = 1100 - analogRead(RATE_5V_PIN);
  if (--intRateCycles == 0) {
    intRateCyclesOn = 5;
    intRateCycles = intRate;
  }
  if (intRateCyclesOn > 0) {
    digitalWrite(INT_CLOCK_PIN, HIGH);
    intRateCyclesOn--;
  } else {
    digitalWrite(INT_CLOCK_PIN, LOW);
  }
  updateOutputs();
}

void updateOutputs() {
  digitalWrite(LATCH_PIN, LOW);
  if (step == -1) {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
  } else if (step < 8) {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 1<<step);
  } else {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 1<<(step-8));
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void handleExtClockIn() {
  if (extPos > -1) {
    step = extPos;
  } else {
    switch (direction) {
      case SEQ_DIR_FORWARDS:
        step++;
        if (step>=16) {
          step = 0;
        }
        break;
      case SEQ_DIR_BACKWARDS:
        step--;
        if (step<0) {
          step = 15;
        }
        break;
    }
  }
  stepOutCycles = pulseLen;
}

void handleReset() {
  step = 0;
}
