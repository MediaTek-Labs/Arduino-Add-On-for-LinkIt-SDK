/*
  This example configures LinkIt 7697 to act as a simple GATT server with 1 characteristic.
  The GATT server notifies changes of the characteristic periodically.
  
  created Aug 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

// Define a simple GATT service with only 1 characteristic
LBLEService counterService("4e38e0c3-ab04-4c5d-b54a-852900379bb3");
LBLECharacteristicInt counterCharacteristic("4e38e0c4-ab04-4c5d-b54a-852900379bb3", LBLE_READ | LBLE_WRITE);

int blink_status = 0;;

void setup() {

  // Initialize LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  blink_status = 0;
  digitalWrite(LED_BUILTIN, blink_status);

  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  Serial.print("Device Address = [");
  Serial.print(LBLE.getDeviceAddress());
  Serial.println("]");

  // configure our advertisement data.
  // In this case, we simply create an advertisement that represents an
  // connectable device with a device name
  LBLEAdvertisementData advertisement;
  advertisement.configAsConnectableDevice("GATT");

  // Configure our device's Generic Access Profile's device name
  // Ususally this is the same as the name in the advertisement data.
  LBLEPeripheral.setName("GATT Test");

  // Add characteristics into counterService
  counterService.addAttribute(counterCharacteristic);

  // Add service to GATT server (peripheral)
  LBLEPeripheral.addService(counterService);

  // start the GATT server - it is now 
  // available to connect
  LBLEPeripheral.begin();

  // start advertisment
  LBLEPeripheral.advertise(advertisement);
}

void loop() {
  delay(1000);
  blink_status = !blink_status;

  Serial.print("conected=");
  Serial.println(LBLEPeripheral.connected());

  if(LBLEPeripheral.connected())
  {
    // increment the value
    const int newValue = counterCharacteristic.getValue() + 1;
    counterCharacteristic.setValue(newValue);

    // broadcasting value changes to all connected central devices
    LBLEPeripheral.notifyAll(counterCharacteristic);
  }
}
