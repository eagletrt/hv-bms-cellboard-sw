#!/usr/bin/bash

pio run -t clean
pio run -e base


srec_cat ./.pio/build/base/firmware.bin \
  -binary \
  -offset 0x08003000 \
  -o ./binaries/cellboard.srec \
  -Motorola \
  -address-length=4

