/*
  This example scans nearby BLE peripherals and prints the peripherals found to Serial.

  created Mar 2017
*/

#include <LBLE.h>


// Start scan
// LBLECentral scanner;
	
void setup() {
	//Initialize serial and wait for port to open:
	Serial.begin(9600);

  Serial.println("start init BLE");

	// Initialize BLE subsystem
	LBLE.begin();
	while(!LBLE.ready())
	{
		delay(100);
	}

  Serial.println("BLE init done");
	// scanner.startScan();
}

void loop() {
	// scan for existing networks:
	Serial.println("Scanning available networks...");
	delay(1000);
}