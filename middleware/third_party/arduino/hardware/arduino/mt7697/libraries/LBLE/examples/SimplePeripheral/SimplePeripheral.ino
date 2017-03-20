/*
  This example configures LinkIt 7697 to act as a simple GATT server with 1 custom +attribute.

  created Mar 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

LBLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214");
LBLECharacteristicInt switchCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", LBLE_READ | LBLE_WRITE);

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

	ledService.addAttribute(switchCharacteristic);
	LBLEPeripheral.addService(ledService);

	// start the GATT server
	LBLEPeripheral.begin();

	// start advertisment
	LBLEPeripheral.advertise(advertisement);

	// Initialize GPIO
	pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
	// The underlying framework will advertise periodically.
	// we simply wait here.
	// 
	// You can use iOS/Android apps such as
	// "BLE Finder" to scan this peripheral and check the
	// services it provides.
	delay(200);

	if(switchCharacteristic.isWritten())
	{
		const char value = switchCharacteristic.getValue();
		Serial.print("sketch->");
		Serial.println(value);
		switch(value)
		{
		case 'O':
			digitalWrite(LED_BUILTIN, HIGH);
			break;
		case 'C':
			digitalWrite(LED_BUILTIN, LOW);
			break;
		default:
			Serial.print("not O, C->");
			Serial.println(value);
			break;
		}
	}
}
