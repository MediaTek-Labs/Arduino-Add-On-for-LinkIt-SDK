#include "Arduino.h"
#include "LWiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
#include "WiFiUdp.h"

int led = 7;
int val = 0;

IPAddress ip(192, 168, 1, 111);
IPAddress dns(8, 8, 8, 8);  //Google dns

WiFiServer server(80);
WiFiClient client;
WiFiUDP Udp;
int setup_flag = 1;

char key = '0';

char ssid[] = "MediaTek_Labs";
//char ssid[] = "lovelive";
char pass[] = "12345678";

static void printMacAddress()
{
	// the MAC address of your Wifi shield
	byte mac[6];

	// print your MAC address:
	WiFi.macAddress(mac);
	Serial.print("MAC: ");
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

static void printEncryptionType(int thisType)
{
	// read the encryption type and print out the name:
	switch (thisType) {
	case ENC_TYPE_WEP:
		Serial.println("WEP");
		break;
	case ENC_TYPE_TKIP:
		Serial.println("WPA");
		break;
	case ENC_TYPE_NONE:
		Serial.println("None");
		break;
	case ENC_TYPE_CCMP:
		Serial.println("WPA2 (AES)");
		break;
	case ENC_TYPE_TKIP_AES:
		Serial.println("TKIP AES MIX");
		break;
	default:
		Serial.println("Unknown");
	}
}

static void listNetworks(void)
{
	// scan for nearby networks:
	Serial.println("** Scan Networks **");
	int numSsid = WiFi.scanNetworks();
	if (numSsid == -1) {
		Serial.println("Couldn't get a wifi connection");
		while (true);
	}

	// print the list of networks seen:
	Serial.print("number of available networks:");
	Serial.println(numSsid);

	// print the network number and name for each network found:
	for (int thisNet = 0; thisNet < numSsid; thisNet++) {
		Serial.print(thisNet);
		Serial.print(") ");
		Serial.print(WiFi.SSID(thisNet));
		Serial.print("\tSignal: ");
		Serial.print(WiFi.RSSI(thisNet));
		Serial.print(" dBm");
		Serial.print("\tEncryption: ");
		int encr = WiFi.encryptionType(thisNet);
		printEncryptionType(encr);
	}
}

static void printBSSID(void)
{
	byte bssid[6];
	WiFi.BSSID(bssid);
	Serial.print("BSSID: ");
	Serial.print(bssid[5],HEX);
	Serial.print(":");
	Serial.print(bssid[4],HEX);
	Serial.print(":");
	Serial.print(bssid[3],HEX);
	Serial.print(":");
	Serial.print(bssid[2],HEX);
	Serial.print(":");
	Serial.print(bssid[1],HEX);
	Serial.print(":");
	Serial.println(bssid[0],HEX);
}

static void printIp_info(void)
{
	IPAddress local_ip;
	IPAddress subnet;
	IPAddress gateway;
	Serial.print("LOCALIP: ");
	local_ip = WiFi.localIP();
	Serial.println(local_ip);
	subnet = WiFi.subnetMask();
	Serial.print("NETMASK: ");
	Serial.println(subnet);
	gateway = WiFi.gatewayIP();
	Serial.print("GATEWAY: ");
	Serial.println(gateway);
}

void setup_WIFI_Setting_and_Info(void)
{

	WiFi.config(ip);
	WiFi.begin(ssid, pass);
	printMacAddress();

	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	printBSSID();

	long rssi = WiFi.RSSI();
	Serial.print("RSSI:");
	Serial.println(rssi);

	byte encryption = WiFi.encryptionType();
	Serial.print("Encryption Type:");
	Serial.println(encryption,HEX);

	WiFi.setDNS(dns);
    Serial.print("Dns configured.");

	Serial.println("Scanning available networks...");
	listNetworks();

	printIp_info();

	Serial.print("INADDR_NONE: ");
	Serial.println(INADDR_NONE);
}

void loop_WIFI_Setting_and_Info(void)
{
	delay(4000);
	Serial.println(WiFi.status());
}

void setup_WIFI_Web_Server(void)
{
	WiFi.begin(ssid, pass);

	server.begin();
	printIp_info();
	Serial.println("Waiting client connect...");
}

void loop_WIFI_Web_Server(void)
{
	WiFiClient client = server.available();
	if (client) {
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();

				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 5");  // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					// output the value of each analog input pin
					static int number = 0;
					for (int i = 0; i < 6; i++) {
						client.print("number ");
						client.print(" is ");
						client.print(number++);
						client.print(".   Hello~");
						client.println("<br />");
					}
					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				}
				else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);

		// close the connection:
		client.stop();
	}
}

void setup_WIFI_Udp_Chat(void)
{
	WiFi.begin(ssid, pass);

	unsigned int localPort = 2390;
	Udp.begin(localPort);

	printIp_info();
}

void loop_WIFI_Udp_Chat(void)
{
	char  ReplyBuffer[] = "acknowledged   ";
	char  ReplyBuffer1[] = "complete!";
	char packetBuffer[255];
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if (packetSize) {
		Serial.print("Received packet of size ");
		Serial.println(packetSize);
		Serial.print("From ");
		IPAddress remoteIp = Udp.remoteIP();

		Serial.print(remoteIp);
		Serial.print(", port ");
		Serial.println(Udp.remotePort());

		// read the packet into packetBufffer
		int len = Udp.read(packetBuffer, 255);
		if (len > 0) {
			packetBuffer[len] = 0;
		}
		Serial.println("Contents:");
		Serial.println(packetBuffer);

		// send a reply, to the IP address and port that sent us the packet we received
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write(ReplyBuffer);
		Udp.write(ReplyBuffer1);
		Udp.endPacket();
	}
}

void setup_WIFI_Web_Client(void)
{
	WiFi.begin(ssid, pass);

	printIp_info();

	//char server1[] = "www.baidu.com";
	char server1[] = "apis.map.qq.com";
	Serial.println("\nStarting connection to server...");
#if 0
	// if you get a connection, report back via serial:
	if (client.connect(server1, 80)) {
		Serial.println("connected to server (POST)");
		// Make a HTTP request:
		client.print("POST ");
		client.print("http://www.baidu.com");
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.print("baidu.com");
		client.println();
		client.println("Connection: keep-alive");
		client.println();
	}
#else
	if (client.connect(server1, 80)) {
		Serial.println("connected to server (GET)");
		client.println("GET /ws/location/v1/ip?ip=61.135.17.68&key=6MABZ-VFKAF-DITJ6-JRPZN-OUOFJ-ULBWQ HTTP/1.1");
		client.println("Host: apis.map.qq.com");
		client.println();
	}
#endif
}

void loop_WIFI_Web_Client(void)
{
	// if there are incoming bytes available
	// from the server, read them and print them:
	while (client.available()) {
		char c = client.read();
		Serial.write(c);
	}
	// if the server's disconnected, stop the client:
	if (!client.connected()) {
		Serial.println();
		Serial.println("disconnecting from server.");
		client.stop();

		// do nothing forevermore:
		//while(true);
		delay(3000);
	}
}

struct Testing {
	int  id;
	char *label;
	void (*setup)(void);
	void (*loop)(void);
};

struct Testing list[] = {
	{
		.id    = 0,
		.label = "WIFI_Setting_and_Info",
		.setup = setup_WIFI_Setting_and_Info,
		.loop  = loop_WIFI_Setting_and_Info,
	},
	{
		.id    = 1,
		.label = "WIFI_Web_Server",
		.setup = setup_WIFI_Web_Server,
		.loop  = loop_WIFI_Web_Server,
	},
	{
		.id    = 2,
		.label = "WIFI_Web_Client",
		.setup = setup_WIFI_Web_Client,
		.loop  = loop_WIFI_Web_Client,
	},
	{
		.id    = 3,
		.label = "WIFI_Udp_Chat",
		.setup = setup_WIFI_Udp_Chat,
		.loop  = loop_WIFI_Udp_Chat,
	},
};

void setup() {
	//pinMode(led, OUTPUT);
	Serial.begin(115200);
	int i = 0;
	for (; i < 4; i++) {
		Serial.print("'");
		Serial.print(i);
		Serial.print("'  ");
		Serial.println(list[i].label);
	}
	Serial.println("Please input '0'-'3' to choose test function:");
}

void loop() {
	//digitalWrite(led, val);
	//val = !val;
	//delay(1000);
	if (setup_flag) {
#if 1
		while (!Serial.available());
		key = Serial.read();

		Serial.print("<-----");
		Serial.print(list[key - '0'].label);
		Serial.println("----->");
		list[key - '0'].setup();

#else
		Serial.print("<-----");
		Serial.print(list[key - '0'].label);
		Serial.println("----->");
		list[key - '0'].setup();
#endif
		setup_flag = 0;
	}

	list[key - '0'].loop();
}
