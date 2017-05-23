/*
  wifi_drv.cpp - Library for LinkIt 7697 HDK.
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "utility/wifi_drv.h"

extern "C" {
#include "log_dump.h"
#include "delay.h"
#include "utility/ard_mtk.h"
}

// Array of data to cache the information related to the networks discovered
char 	WiFiDrv::_networkSsid[][WL_SSID_MAX_LENGTH] = {{0}} ;
int32_t WiFiDrv::_networkRssi[WL_NETWORKS_LIST_MAXNUM] = { 0 };
uint8_t WiFiDrv::_networkEncr[WL_NETWORKS_LIST_MAXNUM] = { 0 };

// Cached values of retrieved data
char 	WiFiDrv::_ssid[] = {0};
uint8_t	WiFiDrv::_bssid[] = {0};
uint8_t WiFiDrv::_mac[] = {0};
uint8_t WiFiDrv::_localIp[] = {0};
uint8_t WiFiDrv::_subnetMask[] = {0};
uint8_t WiFiDrv::_gatewayIp[] = {0};
// Firmware version
char    WiFiDrv::fwVersion[] = "1.1.0";


void WiFiDrv::getNetworkData(uint8_t *ip, uint8_t *mask, uint8_t *gwip)
{
	get_ip_setting(ip, mask, gwip);
}

void WiFiDrv::wifiDriverInit()
{
}

int8_t WiFiDrv::wifiSetNetwork(const char* ssid, uint8_t ssid_len)
{
	return set_net(ssid, ssid_len);
}

int8_t WiFiDrv::wifiSetPassphrase(const char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t len)
{
	return set_passphrase(ssid, ssid_len, passphrase, len);
}


int8_t WiFiDrv::wifiSetKey(const char* ssid, uint8_t ssid_len, uint8_t key_idx, const char *key, const uint8_t len)
{
	return set_key(ssid, ssid_len, key_idx, key, len);
}

void WiFiDrv::config(uint8_t validParams, uint32_t local_ip, uint32_t gateway, uint32_t subnet)
{
	set_ip_config(local_ip, gateway, subnet);
}

void WiFiDrv::setDNS(uint8_t validParams, uint32_t dns_server1, uint32_t dns_server2)
{
	set_dns_config(dns_server1, dns_server2);
}

int8_t WiFiDrv::disconnect()
{
	return net_disconnect();
}

uint8_t WiFiDrv::getConnectionStatus()
{
	return get_conn_status();
}

uint8_t* WiFiDrv::getMacAddress()
{
	get_mac(_mac);
	return _mac;
}

void WiFiDrv::getIpAddress(IPAddress& ip)
{
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	ip = _localIp;
}

void WiFiDrv::getSubnetMask(IPAddress& mask)
{
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	mask = _subnetMask;
}

void WiFiDrv::getGatewayIP(IPAddress& ip)
{
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	ip = _gatewayIp;
}

char* WiFiDrv::getCurrentSSID()
{
	get_curr_ssid(_ssid);
	return _ssid;
}

uint8_t* WiFiDrv::getCurrentBSSID()
{
	get_curr_bssid(_bssid);
	return _bssid;
}

int32_t WiFiDrv::getCurrentRSSI()
{
	return get_curr_rssi();
}

uint8_t WiFiDrv::getCurrentEncryptionType()
{
	return get_curr_enct();
}

int8_t WiFiDrv::startScanNetworks()
{
	return start_scan_net();
}


uint8_t WiFiDrv::getScanNetworks()
{
	return get_reply_scan_networks();
}

char* WiFiDrv::getSSIDNetoworks(uint8_t networkItem)
{
	get_idx_ssid(networkItem, _networkSsid[networkItem]);
	return _networkSsid[networkItem];
}

uint8_t WiFiDrv::getEncTypeNetowrks(uint8_t networkItem)
{
	return get_idx_enct(networkItem);
}

int32_t WiFiDrv::getRSSINetoworks(uint8_t networkItem)
{
	return get_idx_rssi(networkItem);
}

uint8_t WiFiDrv::reqHostByName(const char* aHostname)
{
	return req_HostByName(aHostname);
}

int WiFiDrv::getHostByName(IPAddress& aResult)
{
	uint32_t _ipaddr;
	int8_t ret = -1;

	ret = get_HostByName(&_ipaddr);
	aResult = _ipaddr;
	return ret;
}

int WiFiDrv::getHostByName(const char* aHostname, IPAddress& aResult)
{
	if (reqHostByName(aHostname)) {
		const unsigned long start = millis();
		uint16_t retry = 0;
		// 10-second timeout for DNS query
		while ((millis() - start) < 10000) {
			if(getHostByName(aResult)) {
				return 1;
			}
			delay(50);
			retry++;
		}
		pr_debug("getHostByName timeout, retry %d times", retry)
	} else {
		return 0;
	}
	return 0;
}

char* WiFiDrv::getFwVersion()
{
	return fwVersion;
}

WiFiDrv wiFiDrv;
