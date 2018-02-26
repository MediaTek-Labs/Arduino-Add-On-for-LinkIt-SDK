/*
  This example connects to a neary BLE peripheral device selected by user,
  and list all the services and characteristics of the device.

  The scrip then tries to read values from each characteristic and prints them.

  created Feb 2018 by MediaTek Labs
*/

#include <LBLE.h>
#include <LBLECentral.h>

LBLEClient client;

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
    LBLECentral.scan();
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

int searching = 1;

enum AppState
{
    SEARCHING,    // We scan nearby devices and provide a list for user to choose from
    CONNECTING,   // User has choose the device to connect to
    CONNECTED     // We have connected to the device
};

void loop() {
    static AppState state = SEARCHING;
    static LBLEAddress serverAddress;

    // check if we're forcefully disconnected.
    if(state == CONNECTED)
    {
        if(!client.connected())
        {
            Serial.println("disconnected from remote device");
            state = SEARCHING;
        }
    }

    switch(state)
    {
    case SEARCHING:
    {
        // wait for a while
        Serial.println("state=SEARCHING");
        for(int i = 0; i < 10; ++i)
        {
            delay(1000);
            Serial.print(".");
        }
        // enumerate advertisements found.
        Serial.print("Peripherals found = ");
        Serial.println(LBLECentral.getPeripheralCount());
        Serial.println("idx\taddress\t\t\tflag\tRSSI");
        for (int i = 0; i < LBLECentral.getPeripheralCount(); ++i) {
            printDeviceInfo(i);
        }

        #if 0
        int toConnect = 0;
        for (int i = 0; i < LBLECentral.getPeripheralCount(); ++i) {
          printDeviceInfo(i);
          if(LBLECentral.getAddress(i) == String("21:8C:C4:C2:15:3C(PUB)")) {
            Serial.println("found!");
            toConnect = i;
          }
        }
        const int idx = toConnect;
        #else
        // waiting for user input
        Serial.println("Select the index of device to connect to: ");
        while(!Serial.available())
        {
            delay(100);
        }

        const int idx = Serial.parseInt();
        #endif

        if(idx < 0 || idx >= LBLECentral.getPeripheralCount())
        {
            Serial.println("wrong index, keep scanning devices.");
        }
        else
        {
            serverAddress = LBLECentral.getBLEAddress(idx);
            Serial.print("Connect to device with address ");
            Serial.println(serverAddress);
            // we must stop scan before connecting to devices
            LBLECentral.stopScan();
            state = CONNECTING;
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
    }
    break;
    case CONNECTED:
    {
        Serial.println("state=CONNECTED");

        // display all services of the remote device
        const int serviceCount = client.getServiceCount();
        Serial.println("available services = ");
        for(int s = 0; s < serviceCount; ++s)
        {
            Serial.print("\t - ");
            const String serviceName = client.getServiceName(s);
            if(serviceName.length())
            {
                Serial.print("[");
                Serial.print(serviceName);
                Serial.print("] ");
            }
            Serial.println(client.getServiceUuid(s));

            // enumerate all characteristics in the service
            const int deviceCharacteristicCount = client.getCharacteristicCount(s);
            for(int c = 0; c < deviceCharacteristicCount; ++c) {
                // print UUID
                Serial.print("\t\t");
                const LBLEUuid uuid = client.getCharacteristicUuid(s, c);
                Serial.println(uuid);

                // Special case to treat 0x2A00 (Device Name)
                if(uuid == LBLEUuid(0x2A00)){
                  Serial.print("\t\t\t");
                  Serial.println(client.readCharacteristicString(s, c));
                } else {
                  // print content
                  Serial.print("\t\t\t");
                  LBLEValueBuffer b = client.readCharacteristic(s, c);
                  for(auto i : b){
                    Serial.print(i, HEX);
                  }
                  Serial.println();
                }
            }
        }

        client.disconnect();
        Serial.println("disconnected from remote device");

        // enter idle state.
        while(true)
        {
            delay(100);
        }
    }
    break;
    }
}
