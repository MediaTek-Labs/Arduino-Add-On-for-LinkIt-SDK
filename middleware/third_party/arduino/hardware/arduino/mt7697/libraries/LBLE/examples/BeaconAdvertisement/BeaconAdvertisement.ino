/*
  This example configures LinkIt 7697 to send iBeacon-compatbile advertisement data.
	
  You should be able to search this device with iOS or Android iBeacon tools.

  created Mar 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

LBLEPeripheral device;

void setup() {
	//Initialize serial and wait for port to open:
	Serial.begin(9600);

	// Initialize BLE subsystem
	Serial.println("BLE begin");
	LBLE.begin();
	while(!LBLE.ready())
	{
		delay(100);
	}
	Serial.println("BLE ready");

	Serial.println("Prepare data");
	// configure our advertisement data as iBeacon.
	LBLEAdvertisementData beaconData;
	beaconData.configIBeaconInfo(LBLEUuid("74278BDA-B644-4520-8F0C-720EAF059935"),
						   32,
						   11,
						   -40);
	
	Serial.println("Start advertising");
	// start advertising it
	device.advertise(beaconData);
}

void loop() {
	// the underlying framework will advertise periodically.
	// we simply wait here.
	delay(3000);
	Serial.println("Advertising");
}
