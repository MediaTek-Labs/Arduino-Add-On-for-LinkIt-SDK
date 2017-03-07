/*
  This example scans nearby BLE peripherals and prints the peripherals found to Serial.

  created Mar 2017
*/

#include <LBLE.h>
#include <LBLECentral.h>

LBLECentral scanner;
	
void setup() {
	//Initialize serial and wait for port to open:
	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	// Initialize BLE subsystem
	LBLE.begin();
	while(!LBLE.ready())
	{
		delay(100);
	}
	Serial.println("BLE ready");

	// start scanning nearby advertisements
	scanner.scan();
}

void loop() {
	// wait 6 seconds
	delay(6000);

	// enumerate advertisements found.
	Serial.println(scanner.getPeripheralCount());
	for(int i = 0; i < scanner.getPeripheralCount(); ++i)
	{
		Serial.print("Addr: ");
		Serial.println(scanner.getAddress(i));
		Serial.print("RSSI: ");
		Serial.println(scanner.getRSSI(i));
		Serial.print("Name: ");
		Serial.println(scanner.getName(i));
		Serial.print("UUID: ");
		if(!scanner.getServiceUuid(i).isEmpty())
		{
			Serial.println(scanner.getServiceUuid(i));
		}
		else
		{
			Serial.println();
		}
		Serial.print("Manu: ");
		Serial.println(scanner.getManufacturer(i));

		if(scanner.isIBeacon(i))
		{
			LBLEUuid uuid;
			uint16_t major = 0, minor = 0;
			uint8_t txPower = 0;
			scanner.getIBeaconInfo(i, uuid, major, minor, txPower);

			Serial.println("iBeacon->");
			Serial.print("    UUID: ");
			Serial.println(uuid);
			Serial.print("    Major: ");
			Serial.println(major);
			Serial.print("    Minor: ");
			Serial.println(minor);
			Serial.print("    txPower: ");
			Serial.println(txPower);
		}

		Serial.println("---");
	}
	
	Serial.println("----------------------");
}
