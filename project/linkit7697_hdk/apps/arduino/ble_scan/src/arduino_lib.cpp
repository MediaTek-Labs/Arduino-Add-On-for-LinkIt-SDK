#include "Arduino.h"
#include <LWiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

extern "C"{
#include <bt_system.h>
#include <bt_gap_le.h>

/**
 * @brief   This function is a static callback for the application to listen to the event. Provide a user-defined callback.
 * @param[in] msg     is the callback message type.
 * @param[in] status  is the status of the callback message.
 * @param[in] buf     is the payload of the callback message.
 * @return            The status of this operation returned from the callback.
 */

void scan(void)
{
	bt_hci_cmd_le_set_scan_parameters_t scan_para = {0};
    bt_hci_cmd_le_set_scan_enable_t enable = {BT_HCI_ENABLE, BT_HCI_ENABLE};
   	scan_para.le_scan_type = BT_HCI_SCAN_TYPE_ACTIVE;
    scan_para.own_address_type = BT_HCI_SCAN_ADDR_PUBLIC;
    scan_para.le_scan_interval = 0x0024;
    scan_para.le_scan_window = 0x0011;
    scan_para.scanning_filter_policy = 0x00;
    
    bt_gap_le_set_scan(&enable, &scan_para);
}

void onCNF(void)
{
	bt_bd_addr_t addr = {0x0C, 0x01, 0x02, 0x03, 0x04, 0x05};
	// set random address before advertising    
    bt_gap_le_set_random_address((bt_bd_addr_ptr_t)addr);

    // trigger scan immediately ater power-on
    scan();
}

static char* get_event_type(uint8_t type)
{
    switch (type)
    {
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
            return "ADV_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_DIRECT_IND:
            return "ADV_DIRECT_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_IND:
            return "ADV_SCAN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_NONCONN_IND:
            return "ADV_NONCONN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
            return "SCAN_RSP";
        default:
            return "NULL";
    }
}

void print_adv_report(const bt_gap_le_advertising_report_ind_t *report)
{
	Serial.print("Type:");
	Serial.println(get_event_type(report->event_type));

	Serial.print("RSSI:");
	Serial.println(report->rssi);

	Serial.print("data_length:");
	Serial.println(report->data_length);

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
}



bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	const char* msg_text = NULL;
	
	switch(msg)
	{
    case BT_POWER_ON_CNF:
    	msg_text = "BT_POWER_ON_CNF";
    	onCNF();
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

	Serial.println(msg_text);

    /*Listen all BT event*/
    return BT_STATUS_SUCCESS;
}

}

void setup() {
	Serial.begin(115200);
	Serial.println("Hello");

	Serial.println("Setup done");
}

void loop() {
	#if 1
	delay(1);
	#else
	static int counter = 0;
	delay(3000);
	Serial.print("counter=");
	Serial.println(counter);
	counter++;
	scan();
	#endif
}
