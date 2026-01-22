#!/usr/bin/bash

pio run -t clean
pio run -e base

./build_tools/bin2srec -a 0x8003000 -i ./.pio/build/base/firmware.bin -o binaries/cellboard.srec
