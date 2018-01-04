/*
  This example scans nearby BLE peripherals and prints the peripherals found.

  created Mar 2017 by MediaTek Labs
*/

#include <LBLE.h>
#include <LBLECentral.h>

void setup() {
  //Initialize serial
  Serial.begin(9600);

  // Initialize BLE subsystem
  Serial.println("BLE begin");
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(10);
  }

  Serial.println("BLE ready, start scan (wait 10 seconds)");
  LBLECentral.scan();
  for(int i = 0; i < 10; ++i)
  {
    delay(1000);
    Serial.print(".");
  }

  // list advertisements found.
  Serial.print("Total ");
  Serial.print(LBLECentral.getPeripheralCount());
  Serial.println(" devices found:");
  Serial.println("idx\taddress\t\t\tflag\tRSSI");
  for (int i = 0; i < LBLECentral.getPeripheralCount(); ++i) {
    printDeviceInfo(i);
  }
  LBLECentral.stopScan();
  Serial.println("------scan stopped-------");
}

void loop() {
  // do nothing
  delay(3000);
}

void printDeviceInfo(int i) {
  Serial.print(i);
  Serial.print("\t");
  Serial.print(LBLECentral.getAddress(i));
  Serial.print("\t");
  Serial.print(LBLECentral.getAdvertisementFlag(i), HEX);
  Serial.print("\t");
  Serial.print(LBLECentral.getRSSI(i));
  Serial.print("\t");
  const String name = LBLECentral.getName(i);
  Serial.print(name);
  if(name.length() == 0)
  {
    Serial.print("(Unknown)");
  }
  Serial.print(" by ");
  const String manu = LBLECentral.getManufacturer(i);
  Serial.print(manu);
  Serial.print(", service: ");
  if (!LBLECentral.getServiceUuid(i).isEmpty()) {
    Serial.print(LBLECentral.getServiceUuid(i));
  } else {
    Serial.print("(no service info)");
  }

  if (LBLECentral.isIBeacon(i)) {
    LBLEUuid uuid;
    uint16_t major = 0, minor = 0;
    int8_t txPower = 0;
    LBLECentral.getIBeaconInfo(i, uuid, major, minor, txPower);

    Serial.print(" ");
    Serial.print("iBeacon->");
    Serial.print("  UUID: ");
    Serial.print(uuid);
    Serial.print("\tMajor:");
    Serial.print(major);
    Serial.print("\tMinor:");
    Serial.print(minor);
    Serial.print("\ttxPower:");
    Serial.print(txPower);
  }

  Serial.println();
}
