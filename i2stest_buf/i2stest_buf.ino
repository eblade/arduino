#include <Wire.h>
#include <I2S.h>
//#include <avdweb_AnalogReadFast.h>

#define SAMPLERATE 44100  // sample rate in Hz
#define BITS_PER_SAMPLE 16

//#define ADC_BUFFER_SIZE 32
//uint16_t adcbuf[ADC_BUFFER_SIZE];
//uint8_t adcbuf_c = 0;

float midi[] = {
    8.175798915643707,
    8.661957218027252,
    9.177023997418987,
    9.722718241315029,
    10.300861153527185,
    10.913382232281371,
    11.562325709738575,
    12.249857374429665,
    12.978271799373285,
    13.75,
    14.56761754744031,
    15.433853164253879,
    16.351597831287414,
    17.323914436054505,
    18.354047994837973,
    19.445436482630058,
    20.60172230705437,
    21.826764464562743,
    23.12465141947715,
    24.49971474885933,
    25.95654359874657,
    27.5,
    29.13523509488062,
    30.867706328507758,
    32.70319566257483,
    34.64782887210901,
    36.70809598967595,
    38.890872965260115,
    41.20344461410874,
    43.653528929125486,
    46.2493028389543,
    48.99942949771866,
    51.91308719749314,
    55.0,
    58.27047018976124,
    61.7354126570155,
    65.40639132514966,
    69.29565774421802,
    73.41619197935188,
    77.78174593052023,
    82.4068892282175,
    87.30705785825097,
    92.4986056779086,
    97.99885899543733,
    103.82617439498628,
    110.0,
    116.54094037952248,
    123.470825314031,
    130.8127826502993,
    138.59131548843604,
    146.83238395870376,
    155.56349186104046,
    164.813778456435,
    174.61411571650194,
    184.9972113558172,
    195.99771799087466,
    207.65234878997256,
    220.0,
    233.0818807590449,
    246.94165062806212,
    261.6255653005986,
    277.1826309768721,
    293.66476791740763,
    311.1269837220809,
    329.62755691286986,
    349.22823143300394,
    369.9944227116344,
    391.9954359817492,
    415.3046975799452,
    440.0,
    466.1637615180898,
    493.88330125612424,
    523.2511306011972,
    554.3652619537442,
    587.3295358348153,
    622.2539674441618,
    659.2551138257397,
    698.4564628660079,
    739.9888454232688,
    783.9908719634984,
    830.6093951598904,
    880.0,
    932.3275230361796,
    987.7666025122485,
    1046.5022612023945,
    1108.7305239074883,
    1174.6590716696305,
    1244.5079348883237,
    1318.5102276514795,
    1396.9129257320158,
    1479.9776908465376,
    1567.9817439269968,
    1661.2187903197807,
    1760.0,
    1864.6550460723593,
    1975.533205024497,
    2093.004522404789,
    2217.4610478149766,
    2349.318143339261,
    2489.0158697766474,
    2637.020455302959,
    2793.8258514640315,
    2959.955381693075,
    3135.9634878539937,
    3322.4375806395615,
    3520.0,
    3729.3100921447212,
    3951.0664100489917,
    4186.009044809578,
    4434.922095629955,
    4698.636286678519,
    4978.031739553295,
    5274.040910605921,
    5587.65170292806,
    5919.91076338615,
    6271.926975707992,
    6644.875161279119,
    7040.0,
    7458.6201842894425,
    7902.132820097983,
    8372.018089619156,
    8869.84419125991,
    9397.272573357039,
    9956.06347910659,
    10548.081821211843,
    11175.30340585612,
    11839.8215267723,
};


// 'threshold' is the Debounce Adjustment factor for the Rotary Encoder.
//
// The threshold value I'm using limits it to 100 half pulses a second
//
// My encoder has 12 pulses per 360deg rotation and the specs say
// it is rated at a maximum of 100rpm.
//
// This threshold will permit my encoder to reach 250rpm so if it was connected
// to a motor instead of a manually operated knob I
// might possibly need to adjust it to 25000. However, this threshold
// value is working perfectly for my situation
//
volatile unsigned long threshold = 1000;


// 'rotaryHalfSteps' is the counter of half-steps. The actual
// number of steps will be equal to rotaryHalfSteps / 2
//
volatile long rotaryHalfSteps = 120;


// Working variables for the interrupt routines
//
volatile unsigned long int0time = 0;
volatile unsigned long int1time = 0;
volatile uint8_t int0signal = 0;
volatile uint8_t int1signal = 0;
volatile uint8_t int0history = 0;
volatile uint8_t int1history = 0;


void setup() {
    //Serial.begin(115200);
    //Serial.println("I2S sine wave tone");

    // start I2S at the sample rate with 16-bits per sample
    if (!I2S.begin(I2S_PHILIPS_MODE, SAMPLERATE, BITS_PER_SAMPLE)) {
        //Serial.println("Failed to initialize I2S!");
        while (1); // do nothing
    }

    // calculate midi table
    //float midi[127];
    //int a = 440; // a is 440 hz...
    //for (int x = 0; x < 127; ++x) {
    //   midi[x] = (a / 32.) * (2. ^ ((x - 9) / 12));
    //}

    //analogReadResolution(10);

    //uint16_t start = analogReadFast(A0);
    //for (uint8_t i = 0; i<=ADC_BUFFER_SIZE; i++) {
    //    adcbuf[i] = 0;
    //}

    pinMode(7, INPUT_PULLUP);

    // Rotary Encoder A
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), int0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(3), int1, CHANGE);
}


float frequency = 440.;
uint32_t amplitude = 10000.;
uint32_t sample = 0;
uint8_t buffer[I2S_BUFFER_SIZE];

bool on = true;


void int0() {
    if (micros() - int0time < threshold) {
        return;
    }
    int0history = int0signal;
    int0signal = digitalRead(2);
    if (int0history == int0signal) {
        return;
    }
    int0time = micros();
    if (int0signal == int1signal) {
        rotaryHalfSteps++;
    } else {
        rotaryHalfSteps--;
    }
}

void int1() {
    if (micros() - int1time < threshold) {
        return;
    }
    int1history = int1signal;
    int1signal = digitalRead(3);
    if (int1history == int1signal) {
        return;
    }
    int1time = micros();
}

void loop() {
    uint32_t samples_to_write;

    if (!on) {
        return;
    }

    long midi_value = (rotaryHalfSteps / 2);
    midi_value %= 128;
    frequency = midi[midi_value];
    //Serial.println(midi_value);

    if (samples_to_write = I2S.availableForWrite()) {
        samples_to_write >>= 1; // 2 bytes per sample
        uint16_t* stream = (uint16_t*)buffer;
        float period = SAMPLERATE / frequency;
        uint16_t value;
        uint32_t sample_in_buffer = 0;
        while (sample_in_buffer < samples_to_write) {
            if (sample >= period) {
                sample = 0;
            }
            value = (uint16_t)((float)(amplitude * sample) / period);

            // Stereo
            stream[sample_in_buffer] = value;
            stream[sample_in_buffer+1] = value;
            sample++;
            sample_in_buffer += 2;
        }

        I2S.write(buffer, samples_to_write << 1);
    } else {

        if (!digitalRead(7)) {
            on = false;
            return;
        }

        //adcbuf[adcbuf_c++] = analogReadFast(A0);
        //if (adcbuf_c == ADC_BUFFER_SIZE) {
        //    adcbuf_c = 0;
        //}
        //uint32_t avalue = adcbuf[0];
        //for (uint8_t i = 1; i<=ADC_BUFFER_SIZE; i++) {
        //    avalue += adcbuf[i];
        //}
        //avalue /= ADC_BUFFER_SIZE;
        //Serial.println(avalue >> 3);
        //frequency = midi[avalue >> 3];
    }
}


