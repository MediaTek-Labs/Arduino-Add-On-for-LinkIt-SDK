#include "Arduino.h"
#include <LWiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "utility/wifi_drv.h"
#include <LBLE.h>
#include <LBLECentral.h>

extern "C"{
#include <bt_system.h>
#include <bt_gap_le.h>

int led = 7;
char key = '0';
//volatile int ble_init = 0;
int ble_id = 0;

LBLECentral scanner;

/**
 * @brief   This function is a static callback for the application to listen to the event. Provide a user-defined callback.
 * @param[in] msg     is the callback message type.
 * @param[in] status  is the status of the callback message.
 * @param[in] buf     is the payload of the callback message.
 * @return            The status of this operation returned from the callback.
 */

void scan(void)
{
    /* start scan */
    scanner.scan();

    delay(3500);

    for(int i = 0; i < scanner.getPeripheralCount(); i++)
    {
        Serial.print(i);
        Serial.print(") ");

        Serial.print("address: ");
        Serial.print(scanner.getAddress(i).c_str());

        Serial.print(" manufacturer: ");
        Serial.print(scanner.getManufacturer(i).c_str());
        Serial.println("");

        delay(100);
    }

    /* stop scan */
    scanner.stopScan();

    delay(500);
}

void onCNF(void)
{
	//bt_bd_addr_t addr = {0x0C, 0x01, 0x02, 0x03, 0x04, 0x05};
	// set random address before advertising
    //bt_gap_le_set_random_address((bt_bd_addr_ptr_t)addr);

    // trigger scan immediately ater power-on
    scan();
}

static char* get_event_type(uint8_t type)
{
    switch (type)
    {
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
            return "undirected advertising";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_DIRECT_IND:
            return "directed advertising";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_IND:
            return "Scannable undirected advertising";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_NONCONN_IND:
            return "Non connectable undirected advertising";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
            return "Scan response";
        default:
            return "NULL";
    }
}

void print_adv_report(const bt_gap_le_advertising_report_ind_t *report)
{
    //Serial.print("Discover type: ");
	//Serial.println(get_event_type(report->event_type));

	//Serial.print("RSSI:");
	//Serial.println(report->rssi);

	//Serial.print("data_length: ");
	//Serial.println(report->data_length);

    Serial.print(ble_id++);
    Serial.print(") ");

    Serial.print("address: ");
    for (int i = BT_BD_ADDR_LEN - 1; i >= 0; i--)
    {
        Serial.print(report->address.addr[i], HEX);

        if (i != 0)
        {
            Serial.print(":");
        }
        else
        {
            Serial.print(" ");
        }
    }

    Serial.print("type: ");
    Serial.println(get_event_type(report->event_type));

#if 0
	if(report->data_length > 64)
	{
		return;
	}

	int cursor = 0;
	int ad_data_len = 0;
	int ad_data_type = 0;
    uint8_t buff[64] = {0};
    while (cursor < report->data_length) {
        ad_data_len = report->data[cursor];
        Serial.print("AD len=");
        Serial.println(ad_data_len);

        /* Error handling for data length over 30 bytes. */
        if (ad_data_len >= 0x1F || ad_data_len <= 0) {
            return;
        }

        ad_data_type = report->data[cursor+1];
        Serial.print("AD Data Type=");
        Serial.println(ad_data_type);
        switch(ad_data_type)
        {
        case BT_GAP_LE_AD_TYPE_NAME_SHORT:
        case BT_GAP_LE_AD_TYPE_NAME_COMPLETE:
        	Serial.print("name=");
        	strncpy((char*)buff, (const char*)(report->data + cursor + 2), ad_data_len - 2);
        	Serial.println((const char*)buff);
        	break;
        default:
        	break;
        }

        cursor+=ad_data_len;
    }
#endif
}

#if 0
bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	const char* msg_text = NULL;

	switch(msg)
	{
    case BT_POWER_ON_CNF:
    	msg_text = "BT_POWER_ON_CNF";
    	//onCNF();
        ble_init = 1;
        break;

    case BT_GAP_LE_SET_SCAN_CNF:
    	msg_text = "BT_GAP_LE_SET_SCAN_CNF";
    	break;

    case BT_GAP_LE_ADVERTISING_REPORT_IND:
    	msg_text = "BT_GAP_LE_ADVERTISING_REPORT_IND";
    	print_adv_report((bt_gap_le_advertising_report_ind_t*)buff);
    	break;

	default:
		break;
	}

	//Serial.println(msg_text);

    /*Listen all BT event*/
    return BT_STATUS_SUCCESS;
}
#endif

}

static void listNetworks(bool show_result)
{
    int ap_count = 0;
    // scan for nearby networks:
    //Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1) {
        Serial.println("Couldn't get a wifi connection");
        while (true);
    }

    if (!show_result)
    {
        return;
    }

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        if (strlen(WiFi.SSID(thisNet)) > 0)
        {
            Serial.print(ap_count++);
            Serial.print(") ");
            Serial.println(WiFi.SSID(thisNet));
            //Serial.print("\tSignal: ");
            //Serial.print(WiFi.RSSI(thisNet));
            //Serial.print(" dBm");
            //Serial.print("\tEncryption: ");
            //int encr = WiFi.encryptionType(thisNet);
            //printEncryptionType(encr);
        }

        delay(100);
    }

    // print the list of networks seen:
    //Serial.print("number of available networks: ");
    //Serial.println(numSsid);
    //Serial.println("");
}

void setup_WIFI_Setting_and_Info(void)
{
    Serial.println("Scanning Wi-Fi networks...");
    Serial.println("");
    listNetworks(true);
}

String readline()
{
    String s = "";

    while (1)
    {
        while (!Serial.available());

        char c = Serial.read();
        Serial.print(c);

        if (c != 0x0A && c!= 0x0D)
        {
            s += c;
        }
        else
        {
            break;
        }
    }

    return s;
}

void loop_gpio(bool on, bool in)
{
    String zero = "0";
    String pin;
    int num;

    Serial.print("Choose pin: ");

    pin = readline();

    Serial.println("");

    num = atoi(pin.c_str());

    bool is_zero = (zero == pin);

    if (!is_zero && num == 0)
    {
        num = -1;
    }

    if (num >= 0)
    {
        pinMode(num, (in == true)? INPUT: OUTPUT);

        Serial.println("");
        Serial.print("P(");
        Serial.print(num);
        Serial.print("): ");

        // GPIO OUT
        if (in == false)
        {
            digitalWrite(num, (on == true)? HIGH: LOW);

            Serial.println((on == true)? "H": "L");
        }
        // GPIO IN
        else
        {
            int level = LOW;

            level = digitalRead(num);

            if (level == HIGH)
            {
                Serial.println("H");
            }
            else
            {
                Serial.println("L");
            }
        }
    }
}

void loop_gpio_out_off(void)
{
    loop_gpio(false, false);
}

void loop_gpio_out_on(void)
{
    loop_gpio(true, false);
}

void loop_gpio_in(void)
{
    loop_gpio(false, true);
}

extern "C"
{
   extern void init_bt_subsys(void);
}

void loop_ble_scan(void)
{
    Serial.println("Scanning BLE devices...");
    Serial.println("");

    ble_id = 0;

    onCNF();
}

void nop(void)
{
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
        .label = "Wi-Fi scan",
        .setup = setup_WIFI_Setting_and_Info,
        .loop  = nop,
    },
    {
        .id    = 1,
        .label = "BLE scan",
        .setup = nop,
        .loop  = loop_ble_scan,
    },
    {
        .id    = 2,
        .label = "GPIO OUT (ON)",
        .setup = nop,
        .loop  = loop_gpio_out_on,
    },
    {
        .id    = 3,
        .label = "GPIO OUT (OFF)",
        .setup = nop,
        .loop  = loop_gpio_out_off,
    },
    {
        .id    = 4,
        .label = "GPIO IN",
        .setup = nop,
        .loop  = loop_gpio_in,
    },
};

void setup() {
	pinMode(led, OUTPUT);
    Serial.begin(115200);
    digitalWrite(led, 0);

    //listNetworks(false);
    //WiFiDrv::startScanNetworks();

    //Serial.print("BLE init");

    LBLE.begin();

    while (!LBLE.ready())
    {
        delay(100);
        //Serial.print(".");
    }

    //Serial.println("Done!");
}

void loop() {
	int i = 0;

    Serial.println("");

    for (; i < sizeof(list) /  sizeof(list[0]); i++) {
        Serial.print("'");
        Serial.print(i);
        Serial.print("'  ");
        Serial.println(list[i].label);
    }

    Serial.print("Please select: ");

    while (!Serial.available());

    key = Serial.read();
    //key = '0';
    Serial.println(key);


    if (key < '0' || key > '5')
    {
        return;
    }

    //Serial.print("[");
    //Serial.print(list[key - '0'].label);
    //Serial.print("] ");

    list[key - '0'].setup();
    list[key - '0'].loop();
}
