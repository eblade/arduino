


void setup() {
    Serial.begin(9600);
}

uint16_t lastValue = 0;
void loop() {
    uint16_t value = analogRead(0);
    //if (abs_diff(value, lastValue) >= 10) {
        Serial.println(value);
    //}
    lastValue = value;
    if (value > 512) {
        digitalWrite(LED_BUILTIN, 0);
    } else {
        digitalWrite(LED_BUILTIN, 1);
    }

}

uint16_t abs_diff(uint16_t a, uint16_t b) {
    return a > b ? a - b : b - a;
}

