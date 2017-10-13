/*
  WiFi.h - Library for LinkIt 7697 HDK.
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

#ifndef WiFi_h
#define WiFi_h

#include <inttypes.h>

extern "C" {
#include "utility/wl_definitions.h"
}

#include "IPAddress.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "TLSClient.h"

class WiFiClass
{
private:

    uint8_t waitForWiFiConnection();
    static void init();
public:
    WiFiClass();

    /*
     * Get firmware version
     */
    static char* firmwareVersion();


    /* Start Wifi connection for OPEN networks
     *
     * param ssid: Pointer to the SSID string.
     */
    int begin(const char* ssid);

    /* Start Wifi connection with WEP encryption.
     * Configure a key into the device. The key type (WEP-40, WEP-104)
     * is determined by the size of the key (5 bytes for WEP-40, 13 bytes for WEP-104).
     *
     * param ssid: Pointer to the SSID string.
     * param key_idx: The key index to set. Valid values are 0-3.
     * param key: Key input buffer.
     */
    int begin(const char* ssid, uint8_t key_idx, const char* key);

    /* Start Wifi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     */
    int begin(const char* ssid, const char *passphrase);

    /* Change Ip configuration settings disabling the dhcp client
     *
     * param local_ip: 	Static ip configuration
     */
    void config(IPAddress local_ip);

    /* Change Ip configuration settings disabling the dhcp client
     *
     * param local_ip: 	Static ip configuration
     * param dns_server:     IP configuration for DNS server 1
     */
    void config(IPAddress local_ip, IPAddress dns_server);

    /* Change Ip configuration settings disabling the dhcp client
     *
     * param local_ip: 	Static ip configuration
     * param dns_server:     IP configuration for DNS server 1
     * param gateway : 	Static gateway configuration
     */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);

    /* Change Ip configuration settings disabling the dhcp client
     *
     * param local_ip: 	Static ip configuration
     * param dns_server:     IP configuration for DNS server 1
     * param gateway: 	Static gateway configuration
     * param subnet:		Static Subnet mask
     */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

    /* Change DNS Ip configuration
     *
     * param dns_server1: ip configuration for DNS server 1
     */
    void setDNS(IPAddress dns_server1);

    /* Change DNS Ip configuration
     *
     * param dns_server1: ip configuration for DNS server 1
     * param dns_server2: ip configuration for DNS server 2
     *
     */
    void setDNS(IPAddress dns_server1, IPAddress dns_server2);

    /*
     * Disconnect from the network
     *
     * return: one value of wl_status_t enum
     */
    int disconnect(void);

    /*
     * Get the interface MAC address.
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* macAddress(uint8_t* mac);

    /*
     * Get the interface IP address.
     *
     * return: Ip address value
     */
    IPAddress localIP();

    /*
     * Get the interface subnet mask address.
     *
     * return: subnet mask address value
     */
    IPAddress subnetMask();

    /*
     * Get the gateway ip address.
     *
     * return: gateway ip address value
     */
    IPAddress gatewayIP();

    /*
     * Return the current SSID associated with the network
     *
     * return: ssid string
     */
    char* SSID();

    /*
     * Return the current BSSID associated with the network.
     * It is the MAC address of the Access Point
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* BSSID(uint8_t* bssid);

    /*
     * Return the current RSSI /Received Signal Strength in dBm)
     * associated with the network
     *
     * return: signed value
     */
    int32_t RSSI();

    /*
     * Return the Encryption Type associated with the network
     *
     * return: one value of wl_enc_type enum
     */
    uint8_t	encryptionType();

    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int8_t scanNetworks();

    /*
     * Return the SSID discovered during the network scan.
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: ssid string of the specified item on the networks scanned list
     */
    char*	SSID(uint8_t networkItem);

    /*
     * Return the encryption type of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
     */
    uint8_t	encryptionType(uint8_t networkItem);

    /*
     * Return the RSSI of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: signed value of RSSI of the specified item on the networks scanned list
     */
    int32_t RSSI(uint8_t networkItem);

    /*
     * Return Connection status.
     *
     * return: one of the value defined in wl_status_t
     */
    uint8_t status();

    /*
     * Resolve the given hostname to an IP address.
     * param aHostname: Name to be resolved
     * param aResult: IPAddress structure to store the returned IP address
     * result: 1 if aIPAddrString was successfully converted to an IP address,
     *          else error code
     */
    int hostByName(const char* aHostname, IPAddress& aResult);


    /////////////////////////////////////////////////////////////////////////////
    //  SoftAP APIs
    /////////////////////////////////////////////////////////////////////////////
public:
    /*
	 * Start an SoftAP with given SSID and Password. 
	 * If passpharse is non-null, WPA-PSK encryption are used.
	 * Pass NULL to passphrase to create an open network (unencrypted).
     * 
     * By default, the AP is created with a subnet "10.10.10.x".
     * and the AP's IP address is "10.10.10.1". Use `softAPConfig` to change this.
     * 
     * return: true if SoftAP created successfully. false otherwise.
     */
	bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 6);
    
    /*
     * Configures the DHCPD behavior of the Soft AP.
     * This must be called before softAP(ssid, passphrase, channel) is called.
     * 
     * If the softAP is already running, call softAPdisconnect() and then softAP() again
     * for the setting to take effect.
     *
     * return: true means configuration updated successfully
     */
    bool softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);

    /*
     * Configures the DHCPD behavior of the Soft AP.
     *
     * return: true if SoftAP created successfully. false otherwise.
     */
    bool softAPdisconnect(bool wifioff = false);

    /*
     * Returns the IP address of the created soft AP.
     *
     * return: IPAddress of the Soft AP.
     */
	IPAddress softAPIP();
    
    /*
     * Returns the number of connected STA client
     *
     * return: number of STA client connected to this AP
     */
	uint8_t softAPgetStationNum();
    
    /*
     * Get the MAC address of the AP.
     * 
     * param mac is a pointer to an `uint8_t` array with length `WL_MAC_ADDR_LENGTH`
     *
     * return: pointer to the mac address buffer if successful. NULL otherwise.
     */
    uint8_t* softAPmacAddress(uint8_t* mac);

    /*
     * Get the printable representation of the MAC address of the AP.
     *
     * return: printable string of the MAC address
     */
    String softAPmacAddress(void);

// Internal Use
public:
    friend class WiFiClient;
	friend class WiFiServer;
};

extern WiFiClass WiFi;

#endif
