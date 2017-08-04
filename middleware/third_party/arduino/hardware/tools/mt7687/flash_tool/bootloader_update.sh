#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo Update Bootloader...
"$DIR/upload.py" -c $1 -n "$DIR/da97.bin" -t ldr -f "$DIR/mt7697_bootloader.bin"
echo ---------------------
echo Update N9 Firmware...
"$DIR/upload.py" -c $1 -n "$DIR/da97.bin" -t n9 -f "$DIR/WIFI_RAM_CODE_MT76X7_in_flash.bin"
echo ---------------------
echo Finished.
