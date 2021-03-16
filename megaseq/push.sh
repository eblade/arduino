#!/bin/bash +x

avrdude -p m328p -c usbtiny -U flash:w:$(find /tmp/arduino_build_* -name '*.ino.hex' -print0 | xargs -0 ls -1 -t | head -1)


# Set CLVDIV8
# avrdude -p m328p -c usbtiny -U lfuse:w:0x62:m

# Unset
# avrdude -p m328p -c usbtiny -U lfuse:w:0xE2:m
