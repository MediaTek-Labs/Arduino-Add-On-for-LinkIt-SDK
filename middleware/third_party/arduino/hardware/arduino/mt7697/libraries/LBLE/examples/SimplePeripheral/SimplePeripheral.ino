/*
  This example configures LinkIt 7697 to act as a simple GATT server with 1 custom +attribute.

  created Mar 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

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

	// TODO: insert a simple GATT service with only 1 attribute

	// configure our advertisement data.
	// In this case, we simply create an advertisement that represents an
	// connectable device with a device name "Simple BLE".
	LBLEAdvertisementData advertisement;
	advertisement.configAsConnectableDevice("UNIQLO");

	// configure our device's Generic Access Profile's device name
	// TODO: This is currently not working.
	LBLEPeripheral.setName("LinkIt Simple BLE");

	/*
	LBLEService s = service LBLEPeripheral.addService(LBLEUuid(0xABCD));
	s.addAttribute(LBLEUuid(0xABCD), 0);
	*/

	// start advertisment
	LBLEPeripheral.advertise(advertisement);
}

void loop() {
	// The underlying framework will advertise periodically.
	// we simply wait here.
	// 
	// You can use iOS/Android apps such as
	// "BLE Finder" to scan this peripheral and check the
	// services it provides.
	delay(3000);
}
