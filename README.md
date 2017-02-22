# Arduino Add On for LinkIt-SDK v4

Arduino Add-On for LinkIt 7697 on LinkIt SDK v4. This package allows developers to:
 1. Build LinkIt 7697 Arduino board support package
 2. Build LinkIt SDK example projects with Arduino porting library



## Prerequisite
 * You need a working LinkIt SDK v4.2 public version. To obtain public version of LinkIt SDK v4.2, visit https://docs.labs.mediatek.com/resource/mt7687-mt7697/en/downloads
 * You must build this package on a Linux machine with `make` and `perl` installed.

## Usage - Build LinkIt 7697 Arduino Supporting Package
Copy-n-replace into LinkIt SDK v4 public version.
Use following command to build the board support package:
~~~bash
mkdir out
./mk_ide_pkg.pl -o out -v 1.0.0
~~~
The resulting packages are located in the `out` directory.

## Usage - Build Example Projects of Arduino Porting Library
Copy-n-replace into LinkIt SDK v4 public version.
Then use following command to build blinky project:
~~~bash
cd project/mt7687_hdk/apps/arduino/blink/GCC
make
~~~
 
## Notes
This package is based on Archermind's LinkIt 7687 Arduino package, with some modifications for MT7697 and LinkIt 7697 board.