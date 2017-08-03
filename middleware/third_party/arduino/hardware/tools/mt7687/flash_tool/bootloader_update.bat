@ECHO OFF
echo Update Bootloader...
"%~dp0upload.exe" -c %1 -n "%~dp0da97.bin" -t ldr -f "%~dp0mt7697_bootloader.bin"
echo ---------------------
echo Update N9 Firmware...
"%~dp0upload.exe" -c %1 -n "%~dp0da97.bin" -t n9 -f "%~dp0WIFI_RAM_CODE_MT76X7_in_flash.bin"
echo ---------------------
echo Finished.
