#!/usr/bin/bash

pio run -t clean
pio run -e base
pio run -e bin2srec

./.pio/build/bin2srec/program -o 8003000 -a 4 ./.pio/build/base/firmware.bin > ./binaries/cellboard.srec
