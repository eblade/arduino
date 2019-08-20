#include "stddef.h"
#include "Crypto.h"
#include "BLAKE2s.h"
#include "string.h"
#include "avr/pgmspace.h"

#define HASH_SIZE 32
#define BLOCK_SIZE 64
#define B64_SIZE 45
#define PASSWORD_SIZE 15

BLAKE2s hash;

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678998";

void base64_encode(const unsigned char *src, unsigned char *dst) {
    unsigned char *pos;
    const unsigned char *end, *in;

    end = src + HASH_SIZE;
    in = src;
    pos = dst;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = 'A';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                          (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = 'A';
    }

    *pos = '\0';
}

void hash_it(const unsigned char * input, unsigned char * output, size_t input_size) {
    uint8_t digest[HASH_SIZE];

    hash.reset();
    hash.update(input, input_size);
    hash.finalize(digest, sizeof(digest));

    base64_encode(digest, output);
}

int validate(const unsigned char * src, size_t size) {
    unsigned char c;
    unsigned char n;
    uint8_t no_upper;
    uint8_t no_number;
    c = src[0];
    if (c < 'a' || c > 'z') return 1;

    no_upper = no_number = n = 1;
    while (n < size && no_upper && no_number) {
        c = src[n++];
        if (c >= 'A' || c <= 'Z') no_upper = 0;
        if (c >= '0' || c <= '9') no_number = 0;
    }
    return (no_upper || no_number);
}

void setup() {
    unsigned char n;
    unsigned char a[B64_SIZE];
    unsigned char b[B64_SIZE];
    unsigned char * src;
    unsigned char * dst;
    unsigned char * tmp;


    delay(2000);
    Serial.begin(57600);
    delay(100);
    Serial.println();
    Serial.println("Fnurrar...");

    const char * input = "a:b";
    size_t size = strlen(input);
    //strcpy(input, a);

    src = input;
    dst = b;
    n = 0;

    while (1) {
        Serial.print(".");
        hash_it(src, dst, size);
        tmp = src;
        src = dst;
        dst = tmp;
        size = B64_SIZE - 1;

        if (++n < 10) continue;

        if (!validate(src, PASSWORD_SIZE)) break;
    }
    Serial.println();
    src[PASSWORD_SIZE] = '\0';
    Serial.println((char*)src);
}

void loop() {
    delay(1000);
}

