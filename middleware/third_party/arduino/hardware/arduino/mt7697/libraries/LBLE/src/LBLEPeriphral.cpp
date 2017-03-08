/*

*/
#include <Arduino.h>
#include <LBLE.h>
#include "LBLEPeriphral.h"

extern "C"
{
#include "utility/ard_ble.h"
}


LBLEAdvertisementData::LBLEAdvertisementData()
{

}

void LBLEAdvertisementData::configIBeaconInfo(const LBLEUuid& uuid, 		
				  uint16_t major, 
				  uint16_t minor, 
				  int8_t txPower)
{
	// remove all existing AD
	m_advDataList.clear();

	// populate AD according to iBeacon spec
	LBLEAdvDataItem item;

	// Advtertisement type flag
	item.clear();
	item.adType = BT_GAP_LE_AD_TYPE_FLAG;
	item.adData[0] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
	item.adDataLen = 1;
	m_advDataList.push_back(item);

	// Manufacturer data
	item.clear();
	item.adType = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
	item.adDataLen = 0x19;	// always 25 bytes of manufacturer payload
	
	item.adData[0] = 0x4c;	// Apple vendor id(0x004c)
	item.adData[1] = 0x00;	// Apple vendor id(0x004c)

	item.adData[2] = 0x02;  // iBeacon type
    item.adData[3] = 0x15;  // iBeacon type

    uuid.toRawBuffer(item.adData + 4, 16);  // 16 bytes of user-specified UUID

    (*(uint16_t*)(item.adData + 20)) = major;	// 2 byte Major number
    (*(uint16_t*)(item.adData + 22)) = minor;	// 2 byte Minor number
    item.adData[24] = (uint8_t)txPower;		// 1 byte TxPower

    m_advDataList.push_back(item);
}

uint32_t LBLEAdvertisementData::getPayload(uint8_t* buf, uint32_t bufLength) const
{
	// input check
	int sizeRequired = 0;
	for(uint32_t i = 0; i < m_advDataList.size(); ++i)
	{
		// data_len + (type + length)
		sizeRequired += (m_advDataList[i].adDataLen + 2);
	}

	if(NULL == buf || bufLength < sizeRequired)
	{
		return sizeRequired;
	}

	// populate the buffer
	uint8_t *cursor = buf;
	for(uint32_t i = 0; i < m_advDataList.size(); ++i)
	{
		const LBLEAdvDataItem& item = m_advDataList[i];
		// AD length
		cursor[0] = item.adDataLen + 1;
		// AD type
		cursor[1] = item.adType;
		// Ad content;
		memcpy(cursor + 2, item.adData, item.adDataLen);

		cursor += item.adDataLen + 2;
	}

	return sizeRequired;
}

void LBLEPeripheral::advertise(const LBLEAdvertisementData& advData)
{
	// enable advertisement
    bt_hci_cmd_le_set_advertising_enable_t enable;
    enable.advertising_enable = BT_HCI_ENABLE;

    // advertisement parameters
    bt_hci_cmd_le_set_advertising_parameters_t adv_param;
    adv_param.advertising_interval_min = 0x00C0;
    adv_param.advertising_interval_max = 0x00D0;
	// TODO: for iBeacon-only devices, 
    // you may want to set advertising type to BT_HCI_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED
    adv_param.advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    adv_param.own_address_type = BT_ADDR_RANDOM;
    adv_param.advertising_channel_map = 7;
    adv_param.advertising_filter_policy = 0;

    // populate the advertisement data
    bt_hci_cmd_le_set_advertising_data_t hci_adv_data = {0};
    hci_adv_data.advertising_data_length = advData.getPayload(hci_adv_data.advertising_data, 
    													      sizeof(hci_adv_data.advertising_data));

    // start broadcasting
    bt_gap_le_set_advertising(&enable, &adv_param, &hci_adv_data, NULL);

    // TODO: we better wait for the BT_GAP_LE_SET_ADVERTISING_CNF event.
}
	
void LBLEPeripheral::stopAdvertise()
{
	// disable advertisement
    bt_hci_cmd_le_set_advertising_enable_t enable;
    enable.advertising_enable = BT_HCI_DISABLE;

    // start broadcasting
    bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);

    // TODO: we better wait for the BT_GAP_LE_SET_ADVERTISING_CNF event.
}

