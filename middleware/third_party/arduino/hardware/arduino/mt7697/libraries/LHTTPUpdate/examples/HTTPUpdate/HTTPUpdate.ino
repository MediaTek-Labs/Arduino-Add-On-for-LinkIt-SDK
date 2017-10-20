#include <LWiFi.h>
#include <LHTTPUpdate.h>

char ssid[] = "yourapssid";      // your WPA2 network SSID (AP name)
char pass[] = "yourpassword";    // your WPA2 network password

// URL to the FOTA(Firmware-over-the-air) binary.
//
// The FOTA binary is a compressed application binary.
// Download the "FOTA ROM Package" Tool from
// https://docs.labs.mediatek.com/resource/mt7687-mt7697/en/downloads#Downloads-Tools
// to convert the application binary built by Ardino IDE to FOTA binary.
//
// To get the application binary of your sketch,
// select "Sketch > Export Compiled Binary" from the IDE menu.
char fotaBinaryURL[] = "http://download.labs.mediatek.com/resource/fota_example.bin";

void setup() {
    Serial.begin(9600);

    // Show our firmware name - so that we know update succeeded or not.
    Serial.println("HTTPUpdate example begins.");
    
    // attempt to connect to Wifi network:
    int status = WL_IDLE_STATUS;
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
    }
    Serial.println("Connected to wifi");

    Serial.println("Begin downloading new firmware");

    LHTTPUpdate fotaUpdater;

    // change the "false" below to "true" to reboot immediately in the update() call.
    fotaUpdater.rebootOnUpdate(false);

    // Connect to URL, download the FOTA binary and store it to on-board flash.
    // After reboot, the new firmware will be decompressed from the FOTA binary and applied.
    if (fotaUpdater.update(fotaBinaryURL)) {
        Serial.println("Press RST key to reboot to updated firmware version.");
        Serial.println("After reboot, it may take a while to update the firmware.");
        Serial.println("The USR LED lights up during the update process.");
    } else {
        Serial.println("Update Failed - check your internet connection and FOTA binary.");
        Serial.print("Possible reason: ");
        Serial.println(fotaUpdater.getLastErrorString());
    }

}

void loop() {
    delay(10);
}