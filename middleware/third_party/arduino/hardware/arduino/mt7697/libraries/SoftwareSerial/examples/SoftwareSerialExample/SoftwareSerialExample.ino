/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 2  (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)


 Note:
 Not all pins on LinkIt 7697 support change interrupts,
 so only the following can be used for RX:
 2, 3

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 modified Dec 2017
 by MediaTek Labs

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>

// On LinkIt 7697, the RX pin must be one of the EINT pins, such as P2 and P3.
// There are no limitations on TX pin.
SoftwareSerial mySerial(2, 11); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
}

void loop() { // run over and over
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}

