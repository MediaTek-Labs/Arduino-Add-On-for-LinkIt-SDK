#ifndef WL_DEFINITIONS_H_
#define WL_DEFINITIONS_H_

// Maximum size of a SSID
#define WL_SSID_MAX_LENGTH		32
// Length of passphrase. Valid lengths are 8-63.
#define WL_WPA_KEY_MAX_LENGTH		63
// Length of key in bytes. Valid values are 5 and 13.
#define WL_WEP_KEY_MAX_LENGTH		13
// Size of a MAC-address or BSSID
#define WL_MAC_ADDR_LENGTH		6
// Size of a MAC-address or BSSID
#define WL_IPV4_LENGTH			4
// Maximum size of a SSID list
#define WL_NETWORKS_LIST_MAXNUM		16
// Default state value for Wifi state field
#define NA_STATE			-1
//Maximum number of attempts to establish wifi connection
#define WL_MAX_CONNECTION_WAIT_SECONDS	10

typedef enum {
	WL_NO_SHIELD		= 255,
	WL_IDLE_STATUS		= 0,
	WL_NO_SSID_AVAIL,
	WL_SCAN_COMPLETED,
	WL_CONNECTED,
	WL_CONNECT_FAILED,
	WL_CONNECTION_LOST,
	WL_DISCONNECTED
} wl_status_t;

/* Encryption modes */
enum wl_enc_type {  /* Values map to 802.11 encryption suites... */
	ENC_TYPE_WEP		= 5,
	ENC_TYPE_TKIP		= 2,
	ENC_TYPE_CCMP		= 4,
	/* ... except these two, 7 and 8 are reserved in 802.11-2007 */
	ENC_TYPE_NONE		= 7,
	ENC_TYPE_AUTO		= 8,
	ENC_TYPE_TKIP_AES	= 9
};


typedef enum {
	WL_FAILURE =		-1,
	WL_SUCCESS =		1,
} wl_error_code_t;

/* Authentication modes */
enum wl_auth_mode {
	AUTH_MODE_INVALID,
	AUTH_MODE_AUTO,
	AUTH_MODE_OPEN_SYSTEM,
	AUTH_MODE_SHARED_KEY,
	AUTH_MODE_WPA,
	AUTH_MODE_WPA2,
	AUTH_MODE_WPA_PSK,
	AUTH_MODE_WPA2_PSK
};

#endif /* WL_DEFINITIONS_H_ */
