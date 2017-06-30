# Arduino board support package for LinkIt 7697 
[![GitHub release](https://img.shields.io/github/release/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK.svg)](https://github.com/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK/releases) [![Github All Releases](https://img.shields.io/github/downloads/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK/total.svg)](https://docs.labs.mediatek.com/resource/linkit7697-arduino/en/setup-arduino-ide-for-linkit-7697) [![Travis](https://img.shields.io/travis/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK.svg)](https://travis-ci.org/MediaTek-Labs/Arduino-Add-On-for-LinkIt-SDK)
---

This is the **source code** for Arduino Add-On package for LinkIt 7697. To install the board support package, visit this [resource page](https://docs.labs.mediatek.com/resource/linkit7697-arduino/en/resources).
 
The source code is an add-on to LinkIt SDK v4.x. To build the board support package, you need following prerequisites:

## Build Prerequisites
 * You need a working LinkIt SDK v4.3 public version. To obtain public version of LinkIt SDK v4.3, visit https://docs.labs.mediatek.com/resource/mt7687-mt7697/en/downloads
 * You must build this package on a Linux machine with `make` and `perl` installed.

## Building the Board Supporting Package
 * Copy-n-replace into LinkIt SDK v4 public version.
 * Use following command to build the board support package:
~~~bash
mkdir out
./mk_ide_pkg.pl -o out -v 1.0.0
~~~
Where the version number `1.0.0` can be assigned by yourself. The resulting packages are located in the `out` directory.
You can also refer to the Travis CI config file `.travis.yml` as an example on the building process.

## Notes
This package is based on [Archermind](https://github.com/archermind)'s LinkIt 7687 Arduino package, with some modifications for MT7697 and LinkIt 7697 board.
