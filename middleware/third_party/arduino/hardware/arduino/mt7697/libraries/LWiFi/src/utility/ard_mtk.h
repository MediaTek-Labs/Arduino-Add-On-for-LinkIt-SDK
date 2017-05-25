#ifndef _ARD_MTK_H_
#define _ARD_MTK_H_

#ifdef __cplusplus
extern "C" {
#endif

	/*
           enum wl_tcp_state {
	       CLOSED      = 0,
	       LISTEN      = 1,
	       SYN_SENT    = 2,
	       SYN_RCVD    = 3,
	       ESTABLISHED = 4,
	       FIN_WAIT_1  = 5,
	       FIN_WAIT_2  = 6,
	       CLOSE_WAIT  = 7,
	       CLOSING     = 8,
	       LAST_ACK    = 9,
	       TIME_WAIT   = 10
	    };
	*/

	/*
	    WiFiClass::hostByName()
	    step 1: reqHostByName

	    param *aHostname[in]  URL

	    return  1: success
	           -1: failed
	*/
	int8_t req_HostByName(const char* aHostname);
	// void foundHostByName(const char *name,  ip_addr_t *ipaddr, void *callback_arg);
	// int req_reply_host_by_name(char *buf, uint8_t len);

	/* step 2: getHostByName
	   param *ipaddr[in]  4 btye

	   return 1: success
	          0: failed
	*/
	int8_t get_HostByName(uint32_t *ipaddr);


	int8_t get_ip_setting(uint8_t *ip, uint8_t *mask, uint8_t *gwip);

	/*
	    WiFiClass::begin()    wifiSetNetwork
	    Connect to an open type AP (no password)

	    param *ssid[in]       AP SSID
	    param ssid_len[in]    strlen(ssid)

	    return >=0: success
	            <0: failed
	*/
	int8_t set_net(const char* ssid, uint8_t ssid_len);

	/*
	    WiFiClass::begin()   wifiSetKey
	    Connect to a network that is encrypted with WEP

	    param *ssid[in]       AP SSID
	    param ssid_len[in]    strlen(ssid)
	    param key_idx[in]     WEP encrypted networks can hold up to 4 different keys
	    param *key            a hexadecimal string used as a security code for WEP encrypted networks
	    param len             strlen(key)

	    return >=0: success
	            <0: failed
	*/
	int8_t set_key(const char *ssid, uint8_t ssid_len, uint8_t key_idx, const char *key, uint8_t len);

	/*
	    WiFiClass::begin()     wifiSetPassphrase
	    Connect to an encrypted network(hava password)

	    param *ssid[in]        AP SSID
	    param ssid_len[in]     strlen(ssid)
	    param *passphrase[in]  WPA encrypted networks use a password in the form of a string for security
	    param len[in]          strlen(passphrase)

	    return >=0: success
	            <0: failed
	*/
	int8_t set_passphrase(const char* ssid, uint8_t ssid_len, const char *passphrase, uint8_t len);

	/*
	    WiFiClass::status()         getConnectionStatus
	    Gets the current connection status

	    return WL_CONNECTED     3   connect success
	           WL_DISCONNECTED  6   failed
	*/
	int8_t get_conn_status(void);

	/*
	    WiFiClass::config()
	    Configuration device ipaddree , gatewat, netmask

	    param local_ip[in]   the IP address of the device (array of 4 bytes)
	    param gateway[in]    the IP address of the network gateway (array of 4 bytes)
	    param subnet[in]     the subnet mask of the network (array of 4 bytes)
	*/
	void set_ip_config(uint32_t local_ip, uint32_t gateway, uint32_t subnet);

	/*
	    WiFiClass::setDNS()
	    Configuration device  dnsserver

	    param dns_server1[in]     the address for a DNS server
	    param dns_server2[in]     the address for a DNS server
	*/
	void set_dns_config(uint32_t dns_server1, uint32_t dns_server2);

	/*
	    WiFiClass::disconnect()
	    Disconnect the current connection

	    return  0: success
	           -1: failed
	*/
	int8_t net_disconnect(void);

	/*
	    WiFiClass::macAddress()
	    Gets the MAC Address of your WiFi shield

	    param *_mac[out]     uint8_t 6 btye

	    return mac pointer
	*/
	uint8_t *get_mac(uint8_t *_mac);

	/*
	    WiFiClass::SSID()    getCurrentSSID
	    Gets the SSID of the current network

	    param *_ssid[out]   char[]

	    return ssid pointer
	*/
	uint8_t *get_curr_ssid(char *_ssid);

	/*
	    WiFiClass::BSSID()   getCurrentBSSID
	    Gets the MAC address of the routher you are connected to

	    param *_bssid[out]   uint8_t  6 byte

	    return bssid pointer
	*/
	uint8_t *get_curr_bssid(uint8_t *_bssid);

	/*
	    WiFiClass::RSSI()    getCurrentRSSI
	    Gets the signal strength of the connection to the router

	    return rssi
	*/
	int32_t get_curr_rssi(void);

	/*
	    WiFiClass::encryptionType()    getCurrentEncryptionType
	    Gets the encryption type of the current network

	    return encryption type
	*/
	uint8_t get_curr_enct(void);

	/*
	    WiFiClass::scanNetworks()
	    start scan
	    step 1: startScanNetworks

	    return  >=: success
	            <0: failed
	*/
	int8_t start_scan_net(void);
	/*
	    step 2: Get scan results
	    return  number of discovered networks
	*/
	uint8_t get_reply_scan_networks(void);

	/*
	    WiFiClass::SSID()   getSSIDNetoworks
	    Based on scan, get information about the specified network

	    param networkItem[in]    Less than scan results(get by WiFiClass::scanNetworks)
	*/
	void get_idx_ssid(uint8_t networkItem, char *netssid);
	int32_t get_idx_rssi(uint8_t networkItem);
	uint8_t get_idx_enct(uint8_t networkItem);

	void dns_print(void);

#ifdef __cplusplus
}
#endif

#endif
