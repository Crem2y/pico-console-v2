#!/bin/bash

echo "Uploading..."

picotool load build/pico_southbridge.uf2 -x

echo "Uploading Complete!"
