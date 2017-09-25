/*
  WiFi.cpp - Library for LinkIt 7697 HDK.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <WString.h>
#include <IPAddress.h>
#include "LWiFi.h"
#include "utility/wifi_drv.h"

extern "C" {
#include "log_dump.h"
#include "delay.h"
#include "wifi_api.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "dhcpd.h"
#include "ethernetif.h"
}

#define AP_IPADDR     IPAddress(10, 10, 10, 1)
#define AP_NETMASK    IPAddress(255, 255, 255, 0)
#define AP_GATEWAY    IPAddress(10, 10, 10, 1)
#define PRIMARY_DNS   IPAddress(8, 8, 8, 8)
#define SECONDARY_DNS IPAddress(8, 8, 4, 4)
#define IP_POOL_START IPAddress(10, 10, 10, 2)
#define IP_POOL_END   IPAddress(10, 10, 10, 10)

extern bool wifi_ready(void);
extern void set_wifi_ready(void);

String ip2String(const IPAddress& _address)
{
	String p;
	size_t n = 0;
	for (int i =0; i < 3; i++)
	{
		p += String(_address[i], DEC);
		p += String('.');
	}
	p += String(_address[3], DEC);
	return p;
}

//////////////////////////////////////////////////////////////////////
//	WiFiClass
//////////////////////////////////////////////////////////////////////
WiFiClass::WiFiClass()
{
	// Driver initialization
	init();
}

void WiFiClass::init()
{
	WiFiDrv::wifiDriverInit();
}

char* WiFiClass::firmwareVersion()
{
	return WiFiDrv::getFwVersion();
}

uint8_t WiFiClass::waitForWiFiConnection()
{
	const uint32_t maxWaitMillis = WL_MAX_CONNECTION_WAIT_SECONDS * 1000;
	const uint32_t startTime = millis();
	uint8_t status = WL_IDLE_STATUS;
	do{
		status = WiFiDrv::getConnectionStatus();
		delay(100);
	} while ((status != WL_CONNECTED) && ((millis() - startTime) < maxWaitMillis));

	return status;
}

int WiFiClass::begin(const char* ssid)
{
	uint8_t status = WL_IDLE_STATUS;

	if (WiFiDrv::wifiSetNetwork(ssid, strlen(ssid)) != WL_FAILURE)
	{
		status = waitForWiFiConnection();
	}
	else
	{
		status = WL_CONNECT_FAILED;
	}
	return status;
}

int WiFiClass::begin(const char* ssid, uint8_t key_idx, const char *key)
{
	uint8_t status = WL_IDLE_STATUS;

	// set encryption key
	if (WiFiDrv::wifiSetKey(ssid, strlen(ssid), key_idx, key, strlen(key)) != WL_FAILURE)
	{
		status = waitForWiFiConnection();
	}
	else
	{
		status = WL_CONNECT_FAILED;
	}
	return status;
}

int WiFiClass::begin(const char* ssid, const char *passphrase)
{
	uint8_t status = WL_IDLE_STATUS;

	// set passphrase
	if (WiFiDrv::wifiSetPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase)) != WL_FAILURE)
	{
		status = waitForWiFiConnection();
	}
	else
	{
		status = WL_CONNECT_FAILED;
	}
	return status;
}

void WiFiClass::config(IPAddress local_ip)
{
	WiFiDrv::config(1, (uint32_t)local_ip, 0, 0);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server)
{
	WiFiDrv::config(1, (uint32_t)local_ip, 0, 0);
	WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
	WiFiDrv::config(2, (uint32_t)local_ip, (uint32_t)gateway, 0);
	WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
	WiFiDrv::config(3, (uint32_t)local_ip, (uint32_t)gateway, (uint32_t)subnet);
	WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
}

void WiFiClass::setDNS(IPAddress dns_server1)
{
	WiFiDrv::setDNS(1, (uint32_t)dns_server1, 0);
}

void WiFiClass::setDNS(IPAddress dns_server1, IPAddress dns_server2)
{
	WiFiDrv::setDNS(2, (uint32_t)dns_server1, (uint32_t)dns_server2);
}

int WiFiClass::disconnect()
{
	return WiFiDrv::disconnect();
}

uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
	uint8_t* _mac = WiFiDrv::getMacAddress();
	memcpy(mac, _mac, WL_MAC_ADDR_LENGTH);
	return mac;
}

IPAddress WiFiClass::localIP()
{
	IPAddress ret;
	WiFiDrv::getIpAddress(ret);
	return ret;
}

IPAddress WiFiClass::subnetMask()
{
	IPAddress ret;
	WiFiDrv::getSubnetMask(ret);
	return ret;
}

IPAddress WiFiClass::gatewayIP()
{
	IPAddress ret;
	WiFiDrv::getGatewayIP(ret);
	return ret;
}

char* WiFiClass::SSID()
{
	return WiFiDrv::getCurrentSSID();
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid)
{
	uint8_t* _bssid = WiFiDrv::getCurrentBSSID();
	memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
	return bssid;
}

int32_t WiFiClass::RSSI()
{
	return WiFiDrv::getCurrentRSSI();
}

uint8_t WiFiClass::encryptionType()
{
	return WiFiDrv::getCurrentEncryptionType();
}


int8_t WiFiClass::scanNetworks()
{
	uint8_t attempts = 10;
	uint8_t numOfNetworks = 0;

	if (WiFiDrv::startScanNetworks() == WL_FAILURE)
		return WL_FAILURE;
	do
	{
		delay(2000);
		numOfNetworks = WiFiDrv::getScanNetworks();
	}
	while (( numOfNetworks == 0)&&(--attempts>0));
	return numOfNetworks;
}

char* WiFiClass::SSID(uint8_t networkItem)
{
	return WiFiDrv::getSSIDNetworks(networkItem);
}

int32_t WiFiClass::RSSI(uint8_t networkItem)
{
	return WiFiDrv::getRSSINetworks(networkItem);
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem)
{
	return WiFiDrv::getEncTypeNetworks(networkItem);
}

uint8_t WiFiClass::status()
{
	return WiFiDrv::getConnectionStatus();
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult)
{
	return WiFiDrv::getHostByName(aHostname, aResult);
}

static int32_t _wifi_ready_handler(wifi_event_t event,
		uint8_t *payload,
		uint32_t length)
{
	if (event == WIFI_EVENT_IOT_INIT_COMPLETE) {
		set_wifi_ready();
		pr_debug(" wifi_ready_handler from AP mode exec\r\n");
	}

	return 0;
}



int WiFiClass::softAP(const char* ssid, const char* password)
{
	if(!wifi_ready())
	{
		pr_debug("wifi_init in SOFT_AP mode")
		wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE , _wifi_ready_handler);

		wifi_config_t config = {0};
		config.opmode = WIFI_MODE_AP_ONLY;
		strncpy((char *)config.ap_config.ssid, ssid, WIFI_MAX_LENGTH_OF_SSID);
		config.ap_config.ssid_length = strlen((char *)config.ap_config.ssid);
		config.ap_config.auth_mode = WIFI_AUTH_MODE_OPEN;
		config.ap_config.encrypt_type = WIFI_ENCRYPT_TYPE_WEP_DISABLED;
		config.ap_config.channel = 6;

		pr_debug("calling wifi_init\n");
		wifi_init(&config, NULL);

		pr_debug("calling lwip_tcpip_init\n");
		lwip_tcpip_config_t tcpip_config = {{0}, {0}, {0}, {0}, {0}, {0}};		
		tcpip_config.ap_addr.addr = (uint32_t)AP_IPADDR;
		tcpip_config.ap_mask.addr = (uint32_t)AP_NETMASK;
		tcpip_config.ap_gateway.addr = (uint32_t)AP_GATEWAY;
		lwip_tcpip_init(&tcpip_config, config.opmode);

		pr_debug("calling while(wifi_ready)\n");
		while (!wifi_ready())
		{
			delay(100);
		}
		
		pr_debug("wifi ready. call dhcpd_start()\n");
		dhcpd_settings_t dhcpd_settings = {{0},{0},{0},{0},{0},{0},{0}};
		strcpy((char *)dhcpd_settings.dhcpd_server_address, ip2String(AP_IPADDR).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_netmask, ip2String(AP_NETMASK).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_gateway, ip2String(AP_GATEWAY).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_primary_dns, ip2String(PRIMARY_DNS).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_secondary_dns, ip2String(SECONDARY_DNS).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_ip_pool_start, ip2String(IP_POOL_START).c_str());
		strcpy((char *)dhcpd_settings.dhcpd_ip_pool_end, ip2String(IP_POOL_END).c_str());
		netif *ap_if = netif_find_by_type(NETIF_TYPE_AP);
		if(ap_if) {
			netif_set_default(ap_if);
			netif_set_link_up(ap_if);
			dhcpd_start(&dhcpd_settings);
			pr_debug("dhcpd_start() called\n");
		} else {
			pr_debug("netif_find_by_type(NETIF_TYPE_AP) failed!\n");
		}
	
		return 1;
	}
	else
	{
		return 0;
	}
}

IPAddress WiFiClass::softAPIP() {
	return AP_IPADDR;
}

int WiFiClass::softAP_maxClient()
{
	uint8_t number = 0;
	const uint8_t status = wifi_connection_get_max_sta_number(&number);
	return number;
}
	
WiFiClass WiFi;
