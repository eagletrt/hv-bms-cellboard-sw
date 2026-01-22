#!/usr/bin/bash

echo "Uploading bootloader..."

pio run -t clean
pio run -e bootloader -t upload
