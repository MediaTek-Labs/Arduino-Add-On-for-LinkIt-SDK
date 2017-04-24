/*
  This example configures LinkIt 7697 to send Eddyston-URL advertisement data.

  You should be able to search this beacon with tools such as "Beacon Tools" on iOS or 
  "Physical Web" app on Android.

  created April 2017
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
  // Note 1: You can obmit the suffix and tail part, e.g.
  //        https://goo.gl/Aq18zF
  //        can be constructed with
  //        configAsEddystoneURL(EDDY_HTTPS, "goo.gl/Aq18zF");
  // Note 2: Note that total url length must not exceed 17 bytes.
  beaconData.configAsEddystoneURL(EDDY_HTTPS, "asp", EDDY_DOT_NET_SLASH, "learn");

  Serial.print("Start advertising Eddystone-URL");

  // start advertising it
  LBLEPeripheral.advertiseAsBeacon(beaconData);
}

void loop() {
  // The underlying framework will advertise periodically.
  // we simply wait here.
  //
  // You should be able to search this beacon with tools such as "Beacon Tools" on iOS or 
  // "Physical Web" app on Android.
  delay(3000);
}
