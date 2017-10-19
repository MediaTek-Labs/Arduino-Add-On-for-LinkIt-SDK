#include <LWiFi.h>
#include <WiFiClient.h>
// #include <LWebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "LinkItAP";
const char *password = "yourpassword";

const char *sta_ssid = "ap_to_connect_to";
const char *sta_password = "ap_password";

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.print("Configuring access point...");

    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("AP ready.");
    Serial.print("AP Name: ");
    Serial.print(ssid);
    Serial.print("AP MAC=");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("AP Disconnect in 10 seconds...");

    delay(10 * 1000);

    // pass true to shutdown WiFi radio
    // pass false to disable AP mode but keeps Wi-Fi module alive
    WiFi.softAPdisconnect(true);

    int status = WiFi.begin(sta_ssid, sta_password);

    printCurrentNet();
	printWifiData();
}

void loop() {
    delay(1);
}


void printWifiData() {
	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
	Serial.println(ip);

	// print your MAC address:
	byte mac[6];
	WiFi.macAddress(mac);
	Serial.print("MAC address: ");
	Serial.print(mac[5], HEX);
	Serial.print(":");
	Serial.print(mac[4], HEX);
	Serial.print(":");
	Serial.print(mac[3], HEX);
	Serial.print(":");
	Serial.print(mac[2], HEX);
	Serial.print(":");
	Serial.print(mac[1], HEX);
	Serial.print(":");
	Serial.println(mac[0], HEX);

}

void printCurrentNet() {
	// print the SSID of the network you're attached to:
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	// print the MAC address of the router you're attached to:
	byte bssid[6];
	WiFi.BSSID(bssid);
	Serial.print("BSSID: ");
	Serial.print(bssid[5], HEX);
	Serial.print(":");
	Serial.print(bssid[4], HEX);
	Serial.print(":");
	Serial.print(bssid[3], HEX);
	Serial.print(":");
	Serial.print(bssid[2], HEX);
	Serial.print(":");
	Serial.print(bssid[1], HEX);
	Serial.print(":");
	Serial.println(bssid[0], HEX);

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.println(rssi);

	// print the encryption type:
	byte encryption = WiFi.encryptionType();
	Serial.print("Encryption Type:");
	Serial.println(encryption, HEX);
	Serial.println();
}
