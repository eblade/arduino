#define ENC0_PINA 2
#define ENC0_PINB 4


volatile uint8_t enc0_value = 0;
volatile uint32_t enc0_reads = 0;

void setup() {
    pinMode(ENC0_PINA, INPUT);
    digitalWrite(ENC0_PINA, HIGH); // pull-up
    pinMode(ENC0_PINB, INPUT);
    digitalWrite(ENC0_PINB, HIGH); // pull-up

    attachInterrupt(digitalPinToInterrupt(ENC0_PINA), doEncoder, CHANGE);
    Serial.begin(9600);
    Serial.println("start");
}

void loop() {
    Serial.println(enc0_value, DEC);
    Serial.println(enc0_reads, DEC);
    Serial.println("-----");
    delay(1000);
}

void doEncoder() {
    if (digitalRead(ENC0_PINA) == digitalRead(ENC0_PINB)) {
        enc0_value++;
    } else {
        enc0_value--;
    }
    enc0_reads++;
    delay(20);
}
