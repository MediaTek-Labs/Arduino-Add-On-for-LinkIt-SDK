#include <LWiFi.h>
#include <LHTTPUpdate.h>

char ssid[] = "MediaTek_Labs";      //  your network SSID (name)
char pass[] = "84149961";  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;               // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(117,185,24,248);
char server[] = "download.labs.mediatek.com";   // http://download.labs.mediatek.com/linkit_7697_ascii.txt

void setup() {
    Serial.begin(115200);
    
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
    }
    Serial.println("Connected to wifi");

    Serial.println("begin update...");

    LHTTPUpdate u;
    auto result = u.update("http://download.labs.mediatek.com/resource/fota_test.bin");
}

void loop() {
    
}