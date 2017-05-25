#include "utility/wl_definitions.h"
#include "wifi_api.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "ethernetif.h"

#include "log_dump.h"

// declared in Arduino core's "variant.h"
extern void set_wifi_ready(void);
extern bool wifi_ready(void);

static ip_addr_t _hostIpAddr;
static uint8_t hostIpAddrFound = 0;
#define DNS_MAX_NAME_LENGTH	256
void foundHostByName(const char *name, ip_addr_t *ipaddr, void *callback_arg)
{
	_hostIpAddr.addr = (ipaddr)?ipaddr->addr:0xffffffffUL;
	if (_hostIpAddr.addr != 0xffffffffUL) {
		pr_debug("foundHostByName: Found Host: name=%s ip=0x%x <%s>\r\n", name, _hostIpAddr.addr, inet_ntoa(_hostIpAddr.addr));
		hostIpAddrFound = 1;
	} else
		pr_debug("foundHostByName error, failed get server-ip\r\n");
}

int req_reply_host_by_name(const char *buf, size_t len)
{
	char hostName[DNS_MAX_NAME_LENGTH];
	if(len < sizeof(hostName)){
		memcpy(hostName, buf, len);
		hostName[len]='\0';
	}else{
		return -1;
	}
	_hostIpAddr.addr = 0xffffffffUL;
	hostIpAddrFound = 0;
	err_t err = dns_gethostbyname(hostName, &_hostIpAddr, foundHostByName, NULL);
	if(err == ERR_OK) {
		pr_debug("Found Host: name=%s ip=0x%x <%s>\r\n", hostName, _hostIpAddr.addr, inet_ntoa(_hostIpAddr.addr));
		hostIpAddrFound = 1;
		return 1;
	}
	return -1;
}
int8_t req_HostByName(const char* aHostname)
{
	int8_t ret = req_reply_host_by_name(aHostname, strlen(aHostname));
	return ret;
}

int8_t get_HostByName(uint32_t *ipaddr)
{
	*ipaddr = (hostIpAddrFound)?_hostIpAddr.addr : 0xffffffffUL;
	return (*ipaddr == 0xffffffffUL)? 0 : 1;
}

static uint8_t befor_beginfunc = 1;
static ip_addr_t static_ip, static_gw, static_netmask;
static sta_ip_mode_t ipmode_flag = STA_IP_MODE_DHCP;
//static SemaphoreHandle_t ip_ready;
static volatile uint8_t ip_ready = 0;
#define CONFIG_IP	1
#define CONFIG_DNS	2

static void setNetbase_info(char type, uint32_t ip, uint32_t gw, uint32_t netmask)
{
	if (type == CONFIG_DNS) {
		ip_addr_t dns_1, dns_2;
		if (ip) {
			memcpy(&dns_1, &ip, 4);
			dns_setserver(0, &dns_1);
		}
		if (gw) {
			memcpy(&dns_2, &gw, 4);
			dns_setserver(1, &dns_2);
		}
		return;
	}

	if (befor_beginfunc) {		// befor  begin()
		pr_debug("befor_beginfunc \r\n");
		ipmode_flag = STA_IP_MODE_STATIC;

		if (ip)
			static_ip.addr= ip;
		if (gw)
			static_gw.addr= gw;
		else
			static_gw.addr= ((ip & 0x00FFFFFFUL) | 0x01000000UL);
		if (netmask)
			static_netmask.addr= netmask;
		else
			static_netmask.addr = 0x00FFFFFFUL;
	} else {					//after begin()
		//request change  net_info  ip / netmask / gw / dns
		ip_addr_t temp_ip, temp_gw, temp_netmask;

		if (ip)
			temp_ip.addr = ip;
		if (gw)
			temp_gw.addr = gw;
		else
			temp_gw.addr = ((ip & 0x00FFFFFFUL) | 0x01000000UL);
		if (netmask)
			temp_netmask.addr = netmask;
		else
			temp_netmask.addr = 0x00FFFFFFUL;

		struct netif *sta_if;
		sta_if = netif_find_by_type(NETIF_TYPE_STA);

		netif_set_addr(sta_if, &temp_ip, &temp_netmask, &temp_gw);
		pr_debug("reset ip/gw/netmask\r\n");
	}

	return;
}

void ip_ready_callback(struct netif *netif)
{
	if (!ip4_addr_isany_val(netif->ip_addr)) {
		pr_debug("%s ip4_addr_isany_val success\r\n", __func__);
		ip_ready = 1;
		if (NULL != inet_ntoa(netif->ip_addr)) {
			pr_debug("************************\r\n");
			pr_debug("DHCP got IP:%s\r\n", inet_ntoa(netif->ip_addr));
			pr_debug("DHCP got NETMASK:%s\r\n", inet_ntoa(netif->netmask));
			pr_debug("DHCP got GW:%s\r\n", inet_ntoa(netif->gw));
			pr_debug("************************\r\n");

		} else {
			pr_debug("DHCP got Failed\r\n");
		}
	} else {
		pr_debug("%s ip4_addr_isany_val fail\r\n", __func__);
	}
}


/**
 * @brief  wifi connected will call this callback function. set lwip status in this function
 * @param[in] wifi_event_t event: not used.
 * @param[in] uint8_t *payload: not used.
 * @param[in] uint32_t length: not used.
 * @retval None
 */
static int32_t _wifi_station_port_secure_event_handler(wifi_event_t event,
		uint8_t *payload,
		uint32_t length)
{
	pr_debug("%s   wifi connected\r\n", __func__);
	struct netif *sta_if;
	//xSemaphoreGive(ip_ready);
	//xSemaphoreTake(ip_ready, portMAX_DELAY);
	if(ipmode_flag != STA_IP_MODE_DHCP)
		ip_ready = 1;
	sta_if = netif_find_by_type(NETIF_TYPE_STA);
	netif_set_link_up(sta_if);

	return 0;
}

/**
 * @brief  wifi disconnected will call this callback function. set lwip status in this function
 * @param[in] wifi_event_t event: not used.
 * @param[in] uint8_t *payload: not used.
 * @param[in] uint32_t length: not used.
 * @retval None
 */
static int32_t _wifi_station_disconnected_event_handler(wifi_event_t event,
		uint8_t *payload,
		uint32_t length)
{
	struct netif *sta_if;
	pr_debug("%s   wifi disconnected\r\n", __func__);
	ip_ready = 0;
	sta_if = netif_find_by_type(NETIF_TYPE_STA);
	netif_set_link_down(sta_if);
	if(ipmode_flag == STA_IP_MODE_DHCP)
		netif_set_addr(sta_if, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);

	return 0;
}

static int32_t _wifi_ready_handler(wifi_event_t event,
		uint8_t *payload,
		uint32_t length)
{
	if (event == WIFI_EVENT_IOT_INIT_COMPLETE) {
		set_wifi_ready();

		pr_debug(" wifi_ready_handler exec\r\n");
	}

	return 0;
}

void lwip_init_start(uint8_t opmode)
{
	befor_beginfunc = 0;		//Indicate begin() aready exec
	//ip_ready = xSemaphoreCreateBinary();

	struct netif *sta_if;
	sta_if = netif_find_by_type(NETIF_TYPE_STA);

	switch (opmode) {
		case WIFI_MODE_STA_ONLY:
		case WIFI_MODE_REPEATER:
			wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE , _wifi_ready_handler);
			wifi_connection_register_event_handler(WIFI_EVENT_IOT_PORT_SECURE, _wifi_station_port_secure_event_handler);
			wifi_connection_register_event_handler(WIFI_EVENT_IOT_DISCONNECTED, _wifi_station_disconnected_event_handler);

			if (ipmode_flag == STA_IP_MODE_DHCP) {
				netif_set_default(sta_if);
				netif_set_status_callback(sta_if, ip_ready_callback);
				dhcp_start(sta_if);
				pr_debug("STA mode dhcp\r\n");
			} else {
				pr_debug("STA mode static\r\n");
				netif_set_addr(sta_if, &static_ip, &static_netmask, &static_gw);
			}

			break;
		case WIFI_MODE_AP_ONLY:
			/*......*/
			break;
	}
}

int8_t get_ip_setting(uint8_t *ip, uint8_t *mask, uint8_t *gwip)
{
	struct netif *iface;
	iface = netif_find_by_type(NETIF_TYPE_STA);

	if (iface == NULL) {
		return -1;
	}

	//if (dhcp_supplied_address(iface))
	//	pr_debug("mode:      dhcp\r\n");
	//else
	//	pr_debug("mode:      static\r\n");

	if (dhcp_supplied_address(iface)) {
		struct dhcp *d = iface->dhcp;
		memcpy(ip, &(d->offered_ip_addr), sizeof(ip_addr_t));
		memcpy(mask, &d->offered_sn_mask, sizeof(ip_addr_t));
		memcpy(gwip, &d->offered_gw_addr, sizeof(ip_addr_t));
		//pr_debug("%x %x %x\r\n", d->offered_ip_addr.addr, d->offered_sn_mask.addr, d->offered_gw_addr.addr);
	} else {
		memcpy(ip, &iface->ip_addr, sizeof(ip_addr_t));
		memcpy(mask, &iface->netmask, sizeof(ip_addr_t));
		memcpy(gwip, &iface->gw, sizeof(ip_addr_t));
		//pr_debug("%x %x %x\r\n", d->offered_ip_addr.addr, d->offered_sn_mask.addr, d->offered_gw_addr.addr);
	}

	return 0;
}

int8_t set_net(const char* ssid, uint8_t ssid_len)
{
	if (ssid_len == 0)
		return WL_FAILURE;

	if (!wifi_ready()) {

		wifi_config_t config = {0};

		config.opmode = WIFI_MODE_STA_ONLY;

		strcpy((char *)config.sta_config.ssid, ssid);
		config.sta_config.ssid_length = ssid_len;

		wifi_init(&config, NULL);
		lwip_tcpip_init(NULL, config.opmode);

	} else {

		uint8_t port = WIFI_PORT_STA;
		wifi_auth_mode_t auth = WIFI_AUTH_MODE_OPEN;
		wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_WEP_DISABLED;

		if (wifi_config_set_ssid(port, (uint8_t*)ssid, ssid_len) < 0) {
			pr_debug("wifi_config_set_ssid failed\r\n");
			goto err;
		}

		if (wifi_config_set_security_mode(port, auth, encrypt) < 0) {
			pr_debug("wifi_config_set_security_mode failed\r\n");
			goto err;
		}

		if (wifi_config_reload_setting() < 0) {
			pr_debug("wifi_config_reload_setting failed\r\n");
			goto err;
		}

	}

	lwip_init_start(WIFI_MODE_STA_ONLY);

	return WL_SUCCESS;
err:
	return WL_FAILURE;
}

int8_t set_key(const char *ssid, uint8_t ssid_len, uint8_t key_idx, const char *key, uint8_t len)
{
	if (ssid_len == 0 || len == 0)
		return WL_FAILURE;

	if (!wifi_ready()) {

		wifi_config_t config = {0};
		wifi_config_ext_t config_ext = {0};

		config.opmode = WIFI_MODE_STA_ONLY;

		strcpy((char *)config.sta_config.ssid, ssid);
		config.sta_config.ssid_length = ssid_len;

		strcpy((char *)config.sta_config.password, key);
		config.sta_config.password_length = len;

		config_ext.sta_wep_key_index_present = 1;
		config_ext.sta_wep_key_index = key_idx;

		wifi_init(&config, &config_ext);
		lwip_tcpip_init(NULL, config.opmode);

	} else {

		uint8_t port = WIFI_PORT_STA;
		wifi_auth_mode_t auth = WIFI_AUTH_MODE_OPEN;
		wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_WEP_ENABLED;
		const char *password = key;
		wifi_wep_key_t wep_key;
		wep_key.wep_tx_key_index = key_idx;
		strncpy((char*)wep_key.wep_key[0], password, len);
		wep_key.wep_key[0][len] = '\0';
		wep_key.wep_key_length[0] = len;


		if (wifi_config_set_ssid(port, (uint8_t*)ssid, ssid_len) < 0) {
			pr_debug("wifi_config_set_ssid failed\r\n");
			goto err;
		}

		if (wifi_config_set_security_mode(port, auth, encrypt) < 0) {
			pr_debug("wifi_config_set_security_mode failed\r\n");
			goto err;
		}

		if (wifi_config_set_wep_key(port, &wep_key) < 0) {
			pr_debug("wifi_config_set_wep_key failed\r\n");
			goto err;
		}

		if (wifi_config_reload_setting() < 0) {
			pr_debug("wifi_config_reload_setting failed\r\n");
			goto err;
		}

	}

	lwip_init_start(WIFI_MODE_STA_ONLY);

	return WL_SUCCESS;
err:
	return WL_FAILURE;
}

int8_t set_passphrase(const char* ssid, uint8_t ssid_len, const char *passphrase, uint8_t len)
{
	if (ssid_len == 0 || len == 0)
		return WL_FAILURE;

	if (!wifi_ready()) {

		wifi_config_t config = {0};

		config.opmode = WIFI_MODE_STA_ONLY;

		strcpy((char *)config.sta_config.ssid, ssid);
		config.sta_config.ssid_length = ssid_len;

		strcpy((char *)config.sta_config.password, passphrase);
		config.sta_config.password_length = len;

		wifi_init(&config, NULL);
		lwip_tcpip_init(NULL, config.opmode);

	} else {

		uint8_t port = WIFI_PORT_STA;
		wifi_auth_mode_t auth = WIFI_AUTH_MODE_WPA2_PSK;
		wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_AES_ENABLED;

		if (wifi_config_set_ssid(port, (uint8_t*)ssid, ssid_len) < 0) {
			pr_debug("wifi_config_set_ssid failed\r\n");
			goto err;
		}

		if (wifi_config_set_security_mode(port, auth, encrypt) < 0) {
			pr_debug("wifi_config_set_security_mode failed\r\n");
			goto err;
		}

		if (wifi_config_set_wpa_psk_key(port, (uint8_t*)passphrase, len) < 0) {
			pr_debug("wifi_config_set_wpa_psk_key failed\r\n");
			goto err;
		}

		if (wifi_config_reload_setting() < 0) {
			pr_debug("wifi_config_reload_setting failed\r\n");
			goto err;
		}

	}

	lwip_init_start(WIFI_MODE_STA_ONLY);

	return WL_SUCCESS;
err:
	return WL_FAILURE;
}



int8_t get_conn_status(void)
{
	uint8_t link_status = 0;
	int8_t ret = WL_DISCONNECTED;

	if (!wifi_ready())
		return WL_DISCONNECTED;

	wifi_connection_get_link_status(&link_status);

	if (link_status == WIFI_STATUS_LINK_CONNECTED && ip_ready) {	//wifi have connected and have ip
		pr_debug("link=%d, the station is connecting to an AP router.\r\n", link_status);
		ret = WL_CONNECTED;
	} else if (link_status == WIFI_STATUS_LINK_DISCONNECTED) {
		pr_debug("link=%d, the station doesn't connect to an AP router.\r\n", link_status);
		ret = WL_DISCONNECTED;
	}

	return ret;
}

void set_ip_config(uint32_t local_ip, uint32_t gateway, uint32_t subnet)
{
	setNetbase_info(CONFIG_IP, local_ip, gateway, subnet);
}

void set_dns_config(uint32_t dns_server1, uint32_t dns_server2)
{
	//dns_print();
	setNetbase_info(CONFIG_DNS, dns_server1, dns_server2, 0);
	//dns_print();
}

int8_t net_disconnect(void)
{
	uint8_t link_status = 0;

	wifi_connection_get_link_status(&link_status);
	if (link_status == WIFI_STATUS_LINK_CONNECTED) {
		if (wifi_connection_disconnect_ap() < 0) {
			pr_debug("disconnect fail\r\n");
			return WL_FAILURE;
		}
		pr_debug("disconnect success\r\n");
	} else if (link_status == WIFI_STATUS_LINK_DISCONNECTED){
		pr_debug("already disconnect\r\n");
	}

	return WL_SUCCESS;
}
static void byte_swap(uint8_t *val1, uint8_t *val2)
{
	*val1 ^= *val2;
	*val2 ^= *val1;
	*val1 ^= *val2;
}
uint8_t *get_mac(uint8_t *_mac)
{
	const int32_t ret = wifi_config_get_mac_address(WIFI_PORT_STA, _mac);
	if(ret < 0)
	{
		pr_debug("wifi_config_get_mac_address failed with %d", ret);
	}

	byte_swap(&_mac[0], &_mac[5]);
	byte_swap(&_mac[1], &_mac[4]);
	byte_swap(&_mac[2], &_mac[3]);

	pr_debug("get mac %02x:%02x:%02x:%02x:%02x:%02x\r\n", _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);

	return _mac;
}

uint8_t *get_curr_ssid(char *_ssid)
{
	uint8_t ssid_len = 0;
	memset(_ssid, 0, WL_SSID_MAX_LENGTH);
	wifi_config_get_ssid(WIFI_PORT_STA, (uint8_t*)_ssid, &ssid_len);
	//pr_debug("current ssid %s  len = %d\r\n", _ssid, ssid_len);
	return (uint8_t*)_ssid;
}

uint8_t *get_curr_bssid(uint8_t *_bssid)
{
	wifi_config_get_bssid(_bssid);
	//pr_debug("bssid %02x:%02x:%02x:%02x:%02x:%02x\r\n", _bssid[0], _bssid[1], _bssid[2],
	//		_bssid[3], _bssid[4], _bssid[5]);

	byte_swap(&_bssid[0], &_bssid[5]);
	byte_swap(&_bssid[1], &_bssid[4]);
	byte_swap(&_bssid[2], &_bssid[3]);

	return _bssid;
}

int32_t get_curr_rssi(void)
{
	int8_t rssi = 0;

	wifi_connection_get_rssi(&rssi);
	return rssi;
}

static uint8_t encrypt_map_arduino(uint8_t encrypt_type)
{
	switch(encrypt_type)
	{
		case WIFI_ENCRYPT_TYPE_WEP_ENABLED:
			return ENC_TYPE_WEP;
		case WIFI_ENCRYPT_TYPE_WEP_DISABLED:
			return ENC_TYPE_NONE;
		case WIFI_ENCRYPT_TYPE_TKIP_ENABLED:
			return ENC_TYPE_TKIP;
		case WIFI_ENCRYPT_TYPE_AES_ENABLED:
			return ENC_TYPE_CCMP;
		case WIFI_ENCRYPT_TYPE_TKIP_AES_MIX:
			return ENC_TYPE_TKIP_AES;
		default:
			return ENC_TYPE_AUTO;
	}
}

uint8_t get_curr_enct(void)
{
	wifi_auth_mode_t auth_mode;
	wifi_encrypt_type_t encrypt_type;
	wifi_config_get_security_mode(WIFI_PORT_STA, &auth_mode, &encrypt_type);
	return encrypt_map_arduino(encrypt_type);
}


static char    Ssid[WL_NETWORKS_LIST_MAXNUM][WL_SSID_MAX_LENGTH];
static int32_t Rssi[WL_NETWORKS_LIST_MAXNUM] = {0};
static uint8_t Encr[WL_NETWORKS_LIST_MAXNUM] = {0};
static uint8_t getscan = 0;
//QueueHandle_t MsgQueue;
static wifi_scan_list_item_t _ap_list[WL_NETWORKS_LIST_MAXNUM];

int8_t ssid_checkout(char *ssid, int8_t i)
{
	int j;
	if (i > 0) {
		for (j = 0; j<=i-1; j++) {
			if (strcmp(Ssid[j], ssid) == 0)
				return 0;
		}
	}
	return 1;
}

int32_t get_scan_list(wifi_event_t event, uint8_t *payload, uint32_t length)
{
	wifi_scan_list_item_t *ptr = _ap_list;
	uint8_t status = 0, number = WL_NETWORKS_LIST_MAXNUM;
	int8_t i = 0;

	if (event == WIFI_EVENT_IOT_SCAN_COMPLETE) {
		for (i = 0; i < WL_NETWORKS_LIST_MAXNUM; i++) {
			memset(Ssid[i], 0, sizeof(char)*WL_SSID_MAX_LENGTH);
			Encr[i] = 0;
			Rssi[i] = 0;
		}
		i = 0;
		while (number--)
		{
			if (strlen((char*)ptr->bssid) != 0 && ssid_checkout((char*)ptr->ssid, i)) {
#if 0
				pr_debug("%d\r\n", i);
				pr_debug("    rssi:%d\r\n", ptr->rssi);
				pr_debug("    ssid=%s, ssid_len=%d\r\n", ptr->ssid, ptr->ssid_length);
				pr_debug("    channel=%d\r\n", ptr->channel);
				pr_debug("    central_channel=%d\r\n", ptr->central_channel);
				pr_debug("    bssid=%02X:%02X:%02X:%02X:%02X:%02X\r\n", ptr->bssid[0],
						ptr->bssid[1],
						ptr->bssid[2],
						ptr->bssid[3],
						ptr->bssid[4],
						ptr->bssid[5]);
				pr_debug("    wps=%d\r\n", ptr->wps_element);
				pr_debug("    auth_mode=%d\r\n", ptr->auth_mode);
				pr_debug("    encrypt_type=%d\r\n", ptr->encrypt_type);
#endif
				Rssi[i] = ptr->rssi;
				memcpy(Ssid[i], ptr->ssid, ptr->ssid_length);
				Encr[i] = (uint8_t)ptr->encrypt_type;
				++i;
			}

			ptr++;
		}
		//xQueueSend(MsgQueue, &number, 0);
		getscan = i;
	}
	return status;
}

int8_t start_scan_net(void)
{
	int8_t status = 0, i;
	//uint8_t scannum = 0;

	if (!wifi_ready()) {

		pr_debug("start_scan_net wifi_init\r\n");

		wifi_config_t wifi_config = {0};
		wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE , _wifi_ready_handler);

		wifi_config.opmode = WIFI_MODE_STA_ONLY;

		strcpy((char *)wifi_config.sta_config.ssid, (const char *)" ");
		wifi_config.sta_config.ssid_length = strlen((const char *)wifi_config.sta_config.ssid);

		wifi_init(&wifi_config, NULL);

		lwip_tcpip_init(NULL, wifi_config.opmode);

		while (!wifi_ready());
		pr_debug("start_scan_net wifi_init completion\r\n");
	}

	getscan = 0;

	for (i = 0; i < WL_NETWORKS_LIST_MAXNUM; i++) {
		memset(&(_ap_list[i]), 0, sizeof(wifi_scan_list_item_t));
	}
	if(wifi_connection_scan_init(_ap_list, WL_NETWORKS_LIST_MAXNUM) < 0)
		pr_debug("wifi_connection_scan_init failed\r\n");

	if((status = wifi_connection_register_event_notifier(WIFI_EVENT_IOT_SCAN_COMPLETE,
					get_scan_list)) < 0)
		pr_debug("register failed\r\n");

	if((status = wifi_connection_start_scan(NULL, 0, NULL, 0, 0)) < 0)
		pr_debug("start scan failed\r\n");


	//MsgQueue = xQueueCreate(5, sizeof(uint8_t));
	//xQueueReceive(MsgQueue, &scannum, portMAX_DELAY); // portMAX_DELAY = BLOCk, but can set timeout-timer
	//vQueueDelete(MsgQueue);
	//wifi_connection_unregister_event_notifier(WIFI_EVENT_IOT_SCAN_COMPLETE, get_scan_list);
	return (status >= 0)? WL_SUCCESS : WL_FAILURE;
}

uint8_t get_reply_scan_networks(void)
{
	if(getscan){
		wifi_connection_unregister_event_notifier(WIFI_EVENT_IOT_SCAN_COMPLETE, get_scan_list);
		wifi_connection_stop_scan();
		wifi_connection_scan_deinit();
		return getscan;
	}
	return 0;
}

void get_idx_ssid(uint8_t networkItem, char *netssid)
{
	memset(netssid, 0, WL_SSID_MAX_LENGTH);
	memcpy(netssid, Ssid[networkItem], strlen(Ssid[networkItem]));
}

int32_t get_idx_rssi(uint8_t networkItem)
{
	return Rssi[networkItem];
}

uint8_t get_idx_enct(uint8_t networkItem)
{
	return encrypt_map_arduino(Encr[networkItem]);
}

void dns_print(void)
{
	ip_addr_t dns1, dns2;
	dns1 = dns_getserver(0);
	dns2 = dns_getserver(1);
	struct in_addr temp1, temp2;
	memcpy(&temp1, &dns1, 4);
	memcpy(&temp2, &dns2, 4);
	pr_debug("==  dns [1] %s  ==\r\n", inet_ntoa(temp1));
	pr_debug("==  dns [2] %s  ==\r\n", inet_ntoa(temp2));
}
