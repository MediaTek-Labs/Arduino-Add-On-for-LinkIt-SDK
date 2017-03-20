/*
  This example scans nearby BLE peripherals and prints the peripherals found.

  created Mar 2017 by MediaTek Labs
*/

#include <LBLE.h>
#include <LBLECentral.h>

LBLECentral scanner;

void setup() {
  //Initialize serial
  Serial.begin(9600);

  // Initialize BLE subsystem
  Serial.println("BLE begin");
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(10);
  }
  Serial.println("BLE ready");

  // start scanning nearby advertisements
  scanner.scan();
}

void printDeviceInfo(int i) {
  Serial.print("Addr: ");
  Serial.println(scanner.getAddress(i));
  Serial.print("RSSI: ");
  Serial.println(scanner.getRSSI(i));
  Serial.print("Name: ");
  Serial.println(scanner.getName(i));
  Serial.print("UUID: ");
  if (!scanner.getServiceUuid(i).isEmpty()) {
    Serial.println(scanner.getServiceUuid(i));
  } else {
    Serial.println();
  }
  Serial.print("Flag: ");
  Serial.println(scanner.getAdvertisementFlag(i), HEX);
  Serial.print("Manu: ");
  Serial.println(scanner.getManufacturer(i));

  if (scanner.isIBeacon(i)) {
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
}

void loop() {
  // wait for a while
  delay(3000);

  // enumerate advertisements found.
  Serial.println(scanner.getPeripheralCount());
  for (int i = 0; i < scanner.getPeripheralCount(); ++i) {
    Serial.println("---");
    printDeviceInfo(i);
  }

  Serial.println("----------------------");
}
