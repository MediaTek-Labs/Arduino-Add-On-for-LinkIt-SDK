/*
  This example scans nearby BLE peripherals and prints the peripherals found.

  created Mar 2017 by MediaTek Labs
*/

#include <LBLE.h>
#include <LBLECentral.h>

// loop() will connect to devices who support this service.
// Note: 0x180A is the Heart Rate service
const uint16_t SERVICE_TO_CONNECT = 0x180A;   

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
  LBLECentral.scan();
}

void printDeviceInfo(int i) {
  Serial.print("Addr: ");
  Serial.println(LBLECentral.getAddress(i));
  Serial.print("RSSI: ");
  Serial.println(LBLECentral.getRSSI(i));
  Serial.print("Name: ");
  Serial.println(LBLECentral.getName(i));
  Serial.print("UUID: ");
  if (!LBLECentral.getServiceUuid(i).isEmpty()) {
    Serial.println(LBLECentral.getServiceUuid(i));
  } else {
    Serial.println();
  }
  Serial.print("Flag: ");
  Serial.println(LBLECentral.getAdvertisementFlag(i), HEX);
  Serial.print("Manu: ");
  Serial.println(LBLECentral.getManufacturer(i));

  if (LBLECentral.isIBeacon(i)) {
    LBLEUuid uuid;
    uint16_t major = 0, minor = 0;
    uint8_t txPower = 0;
    LBLECentral.getIBeaconInfo(i, uuid, major, minor, txPower);

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
      Serial.println("state=SEARCHING");

      // enumerate advertisements found.
      Serial.println(LBLECentral.getPeripheralCount());
      const LBLEUuid searchId((uint16_t)SERVICE_TO_CONNECT);
      for (int i = 0; i < LBLECentral.getPeripheralCount(); ++i) {
        // find any heartrate device
        const LBLEUuid serviceId = LBLECentral.getServiceUuid(i);
        if(serviceId == searchId)
        {
          serverAddress = LBLECentral.getBLEAddress(i);
          Serial.print("Device found & connect to address ");
          Serial.println(serverAddress);
          // we should stop scan before connecting to devices
          LBLECentral.stopScan();
          state = CONNECTING;
        }
      }
    }
    break;
  case CONNECTING:
  {
    Serial.println("state=CONNECTING");
    client.connect(serverAddress);
    if(client.connected())
    {
      state = CONNECTED;
    }
    else
    {
      Serial.println("can't connect");
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
    Serial.println("state=CONNECTED");

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
