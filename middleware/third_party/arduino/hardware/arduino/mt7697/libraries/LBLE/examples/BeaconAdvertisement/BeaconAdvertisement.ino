/*
  This example configures LinkIt 7697 to send iBeacon-compatbile advertisement data.

  You should be able to search this device with iOS or Android iBeacon tools.

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
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  // configure our advertisement data as iBeacon.
  LBLEAdvertisementData beaconData;

  // This is a common AirLocate example UUID.
  LBLEUuid uuid("E2C56DB5-DFFB-48D2-B060-D0F5A71096E0");
  beaconData.configAsIBeacon(uuid, 01, 02, -40);

  Serial.print("Start advertising iBeacon with uuid=");
  Serial.println(uuid);

  // start advertising it
  LBLEPeripheral.advertise(beaconData);
}

void loop() {
  // The underlying framework will advertise periodically.
  // we simply wait here.
  //
  // You can use iBeacon apps such as
  // "Locate Beacon" by Radius Networks on iOS devices
  // to locate this beacon.
  delay(3000);
}
