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
	    WiFiServer::begin()   startServer
	    Start listening to a specific port

	    param port[in]       listening Port
	    param sock[in]       get by WiFiClass::getSocket()
	    param protMode[in]   udp = 1, tcp = 0
	*/
	void start_server_tcp(uint16_t port, uint8_t sock, uint8_t protMode);

	/*
	    WiFiServer::status()  getServerState
	    current server status

	    param _sock[in]  get by WiFiClass::getSocket()

	    return 1 : if server within listen , will return 1 LISTEN
	       other : lpcb->state
	    if server start listen failed, should return 0 CLOSED
	*/
	uint8_t get_state_tcp(uint8_t _sock);

	/*
	    WiFiClient::write()   setp 1:sendData
	    Send data to the current client

	    param sock[in]   get by WiFiClass::getSocket()
	    param data[in]   data pointer
	    param len[in]    data len   strlen(data)

	    return : WL_SUCCESS   1   success
	             WL_FAILURE  -1   failed
	*/
	uint16_t send_data_tcp(uint8_t sock, const uint8_t *data, uint16_t len);

	/*
	    setp 2:checkDataSent
	    Check whether the data is sent to complete

	    param sock[in]   get by WiFiClass::getSocket()

	    ret   1: send success
	          0: fail
	*/
	int8_t data_sent_tcp(uint8_t _sock);


	/*
	    WiFiClient::connect()   startClient
	    Connect to a specific server

	    param _addr[in]   server ipaddress
	    param port[in]     server Port
	    param sock[in]    get by WiFiClass::getSocket()
	    param protMode[in]   udp = 1, tcp = 0

	    return  1:  success
	           -1:  failed
	*/
	int start_client_tcp(uint32_t _addr, uint16_t port, uint8_t sock, uint8_t protMode);

	/*
	    WiFiClient::stop()   stopClient
	    close a specific port client

	    param sock[in]    get by WiFiClass::getSocket()
	*/
	void stop_client_tcp(uint8_t sock);


	/*
	    WiFiClient::read(buf, size)
	    get [datalen] of data
	    note: should call this function after avail_data_tcp(), but length of geted data
	          are not necessarily equal to avail length (get len <= avail len)

	    param sock[in]        get by WiFiClass::getSocket()
	    param *_data[out]     data pointer
	    param *_dataLen[in]   gets the length of the data

	    return datalen >0
	*/
	uint16_t get_databuf_tcp(uint8_t sock, uint8_t *_data, uint16_t *_dataLen);

	/*
	    WiFiClient::read()    getData
	    get one byte data

	    param _sock[in]   get by WiFiClass::getSocket()
	    param *data[out]  data pointer
	    param peek[in]    =1, only look data(1byte) , no free (until all is finished)
	                      =0, get data(1byte) and free data

	    return  data
	*/
	uint8_t get_data_tcp(uint8_t _sock, uint8_t *data, uint8_t peek);

	/*
	    WiFiClient::available()      availData
	    Get the length of the readable data
	    note: get_databuf should after this function, but length of data
	          is not necessarily equal to this length (get len <= avail len)

	    param _sock[in]    get by WiFiClass::getSocket()

	    return  datalen > 0
	                  0  no data
	*/
	uint16_t avail_data_tcp(uint8_t _sock);

	/*
	    WiFiClient::status()   getClientstate
	    if client connected return ESTABLISHED(4), otherwise others

	    param _sock[in]    get by WiFiClass::getSocket()
	*/
	uint8_t get_client_state_tcp(uint8_t _sock);


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


	/*
	    WiFiUdp::endPacket()     sendUdpData
	    Send cached data

	    param _sock[in]    get by WiFiClass::getSocket()

	    return  1: success
	           -1: failed
	*/
	int8_t send_data_udp(uint8_t _sock);

	/*
	    WiFiUdp::write()       insertDataBuf
	    take the data is stored in the cache

	    param _sock[in]    get by WiFiClass::getSocket()
	    param *data[in]    data pointer
	    param _len[in]      data len  strlen(data)

	    return  0: success
	           -1: failed
	*/
	int8_t insert_data(uint8_t sock, const uint8_t *data, uint16_t _len);

	/*
	    WiFiUdp::remoteIP() / remotePort()    getRemoteData
	    Gets the ip / port of the remote UDP connection

	    param _sock[in]    get by WiFiClass::getSocket()
	    param *ip[in]      4 byte
	    param *port[in]    2 byte
	*/
	void get_reply_remote_data(uint8_t sock, uint8_t *ip, uint8_t *port);


	int8_t get_ip_setting(uint8_t *ip, uint8_t *mask, uint8_t *gwip);

	/*
	    WiFiClass::begin()    wifiSetNetwork
	    Connect to an open type AP (no password)

	    param *ssid[in]       AP SSID
	    param ssid_len[in]    strlen(ssid)

	    return >=0: success
	            <0: failed
	*/
	int8_t set_net(char* ssid, uint8_t ssid_len);

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
	int8_t set_key(char *ssid, uint8_t ssid_len, uint8_t key_idx, const char *key, uint8_t len);

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
	int8_t set_passphrase(char* ssid, uint8_t ssid_len, const char *passphrase, uint8_t len);

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
	int8_t *get_curr_ssid(char *_ssid);

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
