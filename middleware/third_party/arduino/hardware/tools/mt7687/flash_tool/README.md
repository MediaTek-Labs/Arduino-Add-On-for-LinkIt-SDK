#MT76x7 Uploader
This tool provides the functionality to flash the bootloader and the firmware of the MT76x7 platform, which includes the firmware of both CM4 and the N9 processors.

Official GitHub repository: https://github.com/MediaTek-Labs/mt76x7-uploader

##Usage
```
-c COM_PORT      COM port, can be COM1, COM2, ..., COMx
-f BIN_FILE      path of the bin file to be uploaded
-n DA_FILE       path of the DA file to be used. The default file is da97.bin for mt7697 and da87.bin for mt7687
-p PLATFORM_NAME platform to be flashed (mt7687 | mt7697). The default platform is mt7697
-t FLASH_TARGET  target to be flashed (cm4 | ldr | n9)
```
##Example
Windows:
```
upload.exe -c COM24 -f sample.bin -t cm4 -p mt7687
```
Linux/macOS:
```
python ./upload.py -c /dev/tty.usbmodem1412 -f sample.bin -t cm4 -p mt7687
```