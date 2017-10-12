# Arduino board support package for LinkIt 7697 
[![GitHub release](https://img.shields.io/github/release/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK.svg)](https://github.com/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK/releases) [![Github All Releases](https://img.shields.io/github/downloads/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK/total.svg)](https://docs.labs.mediatek.com/resource/linkit7697-arduino/en/setup-arduino-ide-for-linkit-7697) [![Travis](https://img.shields.io/travis/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK.svg)](https://travis-ci.org/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK)

This is the **source code** for Arduino Add-On package for LinkIt 7697. To install the board support package, visit this [resource page](https://docs.labs.mediatek.com/resource/linkit7697-arduino/en/resources).
 
The source code is an add-on to LinkIt SDK v4.x. To build the board support package, you need following prerequisites:

## Build Prerequisites
 * You need a working LinkIt SDK v4.6 public version. To obtain public version of LinkIt SDK v4.6, visit https://docs.labs.mediatek.com/resource/mt7687-mt7697/en/downloads
 * You must build this package on a Linux machine with `make` and `perl` installed.

## Building the Board Supporting Package
 * Copy LinkIt SDK v4 public version into the source repo. We want to keep existing files, since this is an add-on package. We assume the SDK is unpacked in `Arduino-Add-On-for-LinkIt-SDK`.
    ```bash
    git clone --depth 1 https://github.com/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK.git
    tar --keep-old-files -zxf LinkIt_SDK_V4.6.0.tar.gz -C Arduino-Add-On-for-LinkIt-SDK
    ```
 * Use following command to build the board support package:
    ```bash
    mkdir out
    ./mk_ide_pkg.pl -o out -v 1.0.0
    ```
Where the version number `1.0.0` can be assigned by yourself. The resulting packages are located in the `out` directory.
You can also refer to the Travis CI config file `.travis.yml` as an example on the building process.

## Notes
This package is based on [Archermind](https://github.com/archermind)'s LinkIt 7687 Arduino package, with some modifications for MT7697 and LinkIt 7697 board.

## Overview

There are 2 major parts in this add-on package:
 
 * a **middleware** module of LinkIt SDK that provides an Arduino porting layer. Most of the porting layer codes are availble in the BSP(board supporting package) as source code format. The porting layer is located in `/middleware/third_party/arduino/hardware/arduino/mt7697/`.
 * a **project** for LinkIt 7697 HDK that builds all the required hardware drivers, FreeRTOS kernel, Wi-Fi and Bluetooth frameworks into a library. The binary library is then copied to the final BSP package. The path to the project is `/project/linkit7697_hdk/apps/arduino/arduino_lib`
