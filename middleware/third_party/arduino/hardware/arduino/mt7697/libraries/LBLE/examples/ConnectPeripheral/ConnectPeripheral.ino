/*
  This example scans nearby BLE peripherals and prints the peripherals found.

  created Mar 2017 by MediaTek Labs
*/

#include <LBLE.h>
#include <LBLECentral.h>

// loop() will connect to devices who support this service.
// Note: 0x180A is the Heart Rate service
const uint16_t SERVICE_TO_CONNECT = 0x180A;   

LBLECentral central;
LBLEClient client;

void setup() {
  //Initialize serial
  Serial.begin(115200);

  // Initialize BLE subsystem
  Serial.println("BLE begin");
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(10);
  }
  Serial.println("BLE ready");

  // start scanning nearby advertisements
  central.scan();
}

void printDeviceInfo(int i) {
  Serial.print("Addr: ");
  Serial.println(central.getAddress(i));
  Serial.print("RSSI: ");
  Serial.println(central.getRSSI(i));
  Serial.print("Name: ");
  Serial.println(central.getName(i));
  Serial.print("UUID: ");
  if (!central.getServiceUuid(i).isEmpty()) {
    Serial.println(central.getServiceUuid(i));
  } else {
    Serial.println();
  }
  Serial.print("Flag: ");
  Serial.println(central.getAdvertisementFlag(i), HEX);
  Serial.print("Manu: ");
  Serial.println(central.getManufacturer(i));

  if (central.isIBeacon(i)) {
    LBLEUuid uuid;
    uint16_t major = 0, minor = 0;
    uint8_t txPower = 0;
    central.getIBeaconInfo(i, uuid, major, minor, txPower);

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

int searching = 1;

enum AppState
{
  SEARCHING,
  CONNECTING,
  CONNECTED
};

void loop() {
  static AppState state = SEARCHING;
  static LBLEAddress serverAddress;
  switch(state)
  {
  case SEARCHING:
    {
      // wait for a while
      delay(3000);

      // enumerate advertisements found.
      Serial.println(central.getPeripheralCount());
      const LBLEUuid searchId((uint16_t)SERVICE_TO_CONNECT);
      for (int i = 0; i < central.getPeripheralCount(); ++i) {
        // find any heartrate device
        const LBLEUuid serviceId = central.getServiceUuid(i);
        if(serviceId == searchId)
        {
          state = CONNECTING;
          serverAddress = central.getBLEAddress(i);
          Serial.print("Device found & connect to address");
          Serial.println(serverAddress);
        }
      }
    }
    break;
  case CONNECTING:
  {
    Serial.println("Connecting...");
    client.connect(serverAddress);
    if(client.connected())
    {
      state = CONNECTED;
    }
    client.discoverServices();
    const int serviceCount = client.getServiceCount();
    Serial.println("available services = ");
    for(int i = 0; i < serviceCount; ++i)
    {
      Serial.println(client.getServiceUuid(i));
    }
  }
  break;
  case CONNECTED:
  {
    Serial.println("yeah! connected");

    // read the "Battery Level" characteristic
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.battery_level.xml
    LBLEValueBuffer value = client.readCharacterstic(LBLEUuid((uint16_t)0x2A29));
    if(!value.empty())
    {
      Serial.print("manufacturer=");
      Serial.println((char*)&value[0]);
    }
    

    value = client.readCharacterstic(LBLEUuid((uint16_t)0x2A19));
    if(!value.empty())
    {
      Serial.print("battery=");
      Serial.println(value[0]);
    }

    // delay for 10 seconds.
    delay(10000);
  }
  break;
  }
}
