#!/bin/bash +x

avrdude -p m328p -c usbtiny -U flash:w:$(find /tmp/arduino_build_* -name '*.ino.hex' -print0 | xargs -0 ls -1 -t | head -1)
