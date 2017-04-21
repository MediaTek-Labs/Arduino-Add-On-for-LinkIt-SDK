/*
  This example configures LinkIt 7697 to send iBeacon-compatbile advertisement data.

  You should be able to search this device with iOS or Android iBeacon tools.

  created Mar 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  // Initialize BLE subsystem
  Serial.println("BLE begin");
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  // configure our advertisement data as iBeacon.
  LBLEAdvertisementData beaconData;

  // make an Eddystone-URL beacon that board casts
  // https://www.asp.net/learn
  beaconData.configAsEddystoneURL(EDDY_HTTPS_WWW, "asp", EDDY_DOT_NET_SLASH, "learn");

  Serial.print("Start advertising Eddystone-URL");

  // start advertising it
  LBLEPeripheral.advertiseAsBeacon(beaconData);
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
