#include <Arduino.h>
#include <WString.h>
#include <IPAddress.h>
#include "LWiFi.h"
#include "utility/wifi_drv.h"
#include "variant.h"
extern "C" {
#include "log_dump.h"
#include "delay.h"
#if 1
#include "wifi_api.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "dhcpd.h"
#include "ethernetif.h"
#endif
}

extern bool wifi_ready(void);

struct SoftAPConfig {
    IPAddress m_softAP_localIP;
    IPAddress m_softAP_subnet;
    IPAddress m_softAP_gateway;

    SoftAPConfig():
        m_softAP_localIP(10, 10, 10, 1),
        m_softAP_subnet(255, 255, 255, 0),
        m_softAP_gateway(10, 10, 10, 1) {
    }

    void getTCPIPConfig(lwip_tcpip_config_t& tcpip_config) {
        tcpip_config.ap_addr.addr = (uint32_t)m_softAP_localIP;
        tcpip_config.ap_mask.addr = (uint32_t)m_softAP_subnet;
        tcpip_config.ap_gateway.addr = (uint32_t)m_softAP_gateway;
        
        return;
    }

    void getDHCPIPRange(IPAddress& start, IPAddress& end) {
        IPAddress min = (uint32_t)m_softAP_localIP & (uint32_t)m_softAP_subnet;
        // cut the address range in half - we assume that the local IP is in the
        // 1st half of the subnet....
        uint8_t range = (~((uint8_t)m_softAP_subnet[3]));
        start = m_softAP_localIP;
        start[3] = start[3] + 1;

        // make sure we don't exceed the boundary of max STA count and the subnet mask.
        end = start;
        uint8_t maxStation = 4;
        if(wifi_connection_get_max_sta_number(&maxStation) >= 0 && (maxStation != 0)) {
            pr_debug("max station=%d\n", maxStation);
            end[3] = std::min(start[3] + maxStation - 1, (start[3] & m_softAP_subnet[3]) + range - 1);
        } else {
            end[3] = (start[3] & m_softAP_subnet[3]) + range - 1;
        }

        pr_debug("getDHCPIPRange=(%s, %s)\n", start.toString().c_str(), end.toString().c_str());
    }

    void getDHCPDConfig(dhcpd_settings_t& dhcpd_settings) {
        // Network settings
        strcpy((char *)dhcpd_settings.dhcpd_server_address, m_softAP_localIP.toString().c_str());
        strcpy((char *)dhcpd_settings.dhcpd_netmask, m_softAP_subnet.toString().c_str());
        strcpy((char *)dhcpd_settings.dhcpd_gateway, m_softAP_gateway.toString().c_str());

        // IP address range
        IPAddress start, end;
        getDHCPIPRange(start, end);
        strcpy((char *)dhcpd_settings.dhcpd_ip_pool_start, start.toString().c_str());
        strcpy((char *)dhcpd_settings.dhcpd_ip_pool_end, end.toString().c_str());

        // DNS settings seems necessary, otherwise dhcpd fails to init
        IPAddress PRIMARY_DNS(8, 8, 8, 8);
        IPAddress SECONDARY_DNS(8, 8, 4, 4);
        strcpy((char *)dhcpd_settings.dhcpd_primary_dns, PRIMARY_DNS.toString().c_str());
        strcpy((char *)dhcpd_settings.dhcpd_secondary_dns, SECONDARY_DNS.toString().c_str());
    }
};

static SoftAPConfig g_softAPConfig;

void setupWiFiConfig(const char* ssid, const char* passphrase, int channel) {
    // SSID
    const size_t ssid_length = std::min((size_t)WIFI_MAX_LENGTH_OF_SSID, strlen(ssid));
    wifi_config_set_ssid(WIFI_PORT_AP, (unsigned char*)ssid, ssid_length);

    // Passphrase & security mode
    if(NULL == passphrase) {
        // Open network
        wifi_config_set_security_mode(WIFI_PORT_AP, WIFI_AUTH_MODE_OPEN, WIFI_ENCRYPT_TYPE_WEP_DISABLED);
    } else {
        // WPA-PSK2 encrypted
        wifi_config_set_security_mode(WIFI_PORT_AP, WIFI_AUTH_MODE_WPA2_PSK, WIFI_ENCRYPT_TYPE_AES_ENABLED);
        const size_t password_length = std::min((size_t)WIFI_LENGTH_PASSPHRASE, strlen(passphrase));
        wifi_config_set_wpa_psk_key(WIFI_PORT_AP, (unsigned char*)passphrase, password_length);
    }
}

bool WiFiClass::softAP(const char* ssid, const char* passphrase, int channel) {
    init_global_connsys();

        // Wi-Fi is now ready - change OpMode immediately
    wifi_config_set_opmode(WIFI_MODE_AP_ONLY);
        wifi_config_set_radio(1);
    setupWiFiConfig(ssid, passphrase, channel);
    wifi_config_reload_setting();

    pr_debug("wifi AP mode ready. initialize IP & DHCPD\n");

        netif *ap_if = netif_find_by_type(NETIF_TYPE_AP);
        if(ap_if) {
            netif_set_default(ap_if);

        lwip_tcpip_config_t tcpip_config = {0};
        g_softAPConfig.getTCPIPConfig(tcpip_config);
        netif_set_addr(ap_if, &tcpip_config.ap_addr, &tcpip_config.ap_mask, &tcpip_config.ap_gateway);
        
            netif_set_link_up(ap_if);

        dhcpd_settings_t dhcpd_settings = {{0},{0},{0},{0},{0},{0},{0}};
        g_softAPConfig.getDHCPDConfig(dhcpd_settings);
            dhcpd_start(&dhcpd_settings);

            pr_debug("dhcpd_start() called\n");
        } else {
            pr_debug("netif_find_by_type(NETIF_TYPE_AP) failed!\n");
        }

        return true;
}

bool WiFiClass::softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    // update settings.
    // these only take effect when softAP() is called.
        g_softAPConfig.m_softAP_localIP = local_ip;
        g_softAPConfig.m_softAP_gateway = gateway;
        g_softAPConfig.m_softAP_subnet = subnet;
        return true;
}

bool WiFiClass::softAPdisconnect(bool wifioff) {
    
    // disconnect all clients
    uint8_t clientCount = 0;
    wifi_sta_list_t* pClientList = NULL;
    if(0 > wifi_connection_get_sta_list(&clientCount, NULL) && clientCount) {
        pClientList = (wifi_sta_list_t*)malloc(clientCount * clientCount);
        wifi_connection_get_sta_list(&clientCount, pClientList);
    }

    if(pClientList) {
        for(uint8_t i = 0; i < clientCount; ++clientCount) {
            wifi_connection_disconnect_sta(pClientList[i].mac_address);
        }
        free(pClientList);
        pClientList = NULL;
    }

    dhcpd_stop();

    // Disable Wi-Fi Soft AP (by swtiching back into STA mode)
    int32_t result = 0;
    result = wifi_config_set_opmode(WIFI_MODE_STA_ONLY);
    pr_debug("wifi_config_set_opmode(WIFI_MODE_STA_ONLY) returns %d\n", result);

    if(wifioff) {
        // turn off radio if the user asks so
        result = wifi_config_set_radio(0);
        pr_debug("wifi_config_set_radio(0) returns %d\n", result);
    }    

    return true;
}

IPAddress WiFiClass::softAPIP() {
    return g_softAPConfig.m_softAP_localIP;
}

uint8_t WiFiClass::softAPgetStationNum() {
    // if we pass NULL to sta_list, we get an error.
    // prepare 10 slots because the MAX STA is 9 as of SDK v4.6.0
    static const uint8_t list_size = 10;
    uint8_t number = list_size;
    wifi_sta_list_t sta_list[list_size] = {0};
    const uint8_t status = wifi_connection_get_sta_list(&number, sta_list);
    pr_debug("wifi_connection_get_sta_list result = %d\n", status);
    if(status >= 0) {
    return number;
    } else {
        return 0;
    }
    
}

uint8_t* WiFiClass::softAPmacAddress(uint8_t* mac) {
    if(NULL == mac) {
        return NULL;
    }
    if(wifi_config_get_mac_address(WIFI_PORT_AP, mac) >= 0) {
        return mac;
    } else {
        return NULL;
    }
}

String WiFiClass::softAPmacAddress(void) {
    uint8_t mac[WIFI_MAC_ADDRESS_LENGTH] = {0};
    if(wifi_config_get_mac_address(WIFI_PORT_AP, mac) >= 0) {
        char macTextBuf[18] = { 0 };
        sprintf(macTextBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(macTextBuf);
    } else {
        return String();
    }
}
