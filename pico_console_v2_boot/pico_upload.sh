#!/bin/bash

echo "Uploading..."

picotool load build/pico_console_bootloader.uf2 -x

echo "Uploading Complete!"
