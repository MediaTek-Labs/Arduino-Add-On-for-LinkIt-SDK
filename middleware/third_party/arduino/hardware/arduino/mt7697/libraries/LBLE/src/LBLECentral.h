/*

*/

#ifndef LBLECENTRAL_H
#define LBLECENTRAL_H

#include <inttypes.h>
#include <vector>
#include <WString.h>
#include <LBLE.h>

extern "C"
{
#include "utility/ard_ble.h"
}

extern "C" bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
extern "C" void ard_ble_central_onCentralEvents(bt_msg_type_t msg, bt_status_t status, void *buff);

// This class allows LinkIt 7697 to act as a BLE central device.
// It can scan nearby BLE peripherals, checking if they are beacons or 
// devices that provides GATT services.
// Currently, this class does not provide functions to connect to remote 
// peripheral devices.
class LBLECentral
{
public:
	LBLECentral();

	/* Scan all nearby BLE peripherals.
	 * Note that it may took sometime for peripherals to get scanned.
	 *
	 * This puts the system into a scanning statie.
	 * The system keeps updating the peripheral list in the background.
	 *
	 * To get the scanned peripheral, call getPeripheralCount() and use
	 * methods such as `getPeripheralName` or `isBeacon`
	 * 
	 * The return value is the number of the peripherals currently found.
	 */
	void scan();

	/* Stop scanning nearby BLE peripherals. 
	 * The list of discovered peripherals are still kept but will no longer update.
	 */
	void stopScan();

	// Retrive the number of peripherals currently discovered
	int getPeripheralCount();

	// Get scanned peripheral information by index,
	// ranging from 0 to [getRemotePeripheralCount() - 1]
	String getAddress(int index);
	String getName(int index);
	int32_t getRSSI(int index);
	int32_t getTxPower(int index);
	LBLEUuid getServiceUuid(int index) const;
	String getManufacturer(int index) const;

	// Advertisement Flags may be:
	//	BT_GAP_LE_AD_FLAG_LIMITED_DISCOVERABLE     (0x01)
	//  BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE     (0x02)
	//	BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED	   (0x04)
	//	
	//	There are also other flags, including:
	//	0x08: Bluetooth BR/EDR Controller
	//	0x10: Bluetooth BR/EDR Host
	//	which are not used by BLE-only devices.
	uint8_t  getAdvertisementFlag(int index)const;

	bool isIBeacon(int index) const;
	bool getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower)const;
	
protected:
	// Event handlers and required data structure
	static std::vector<bt_gap_le_advertising_report_ind_t> g_peripherals_found;
	static void processAdvertisement(const bt_gap_le_advertising_report_ind_t *report);
	
	// Accessibility Modifiers
	friend bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
	friend void ard_ble_central_onCentralEvents(bt_msg_type_t msg, bt_status_t status, void *buff);
};

class LBLEAdvertisements
{
public:
	LBLEAdvertisements(bt_gap_le_advertising_report_ind_t& adv,
					   bt_gap_le_advertising_report_ind_t& resp);

	virtual bool isValid() const;

	// Get the Bluetooth device address (MAC address) of the advertising device.
	String getAddress() const;

	// Get local name boardcasted. This may be empty (NULL).
	String getName() const;

	// Get transmitted Tx power of the advertisement
	int8_t getTxPower() const;

	// Get RSSI during scan
	int8_t getRSSI() const;

	LBLEUuid getServiceUuid() const;

	String getManufacturer() const;

	uint8_t getAdvertisementFlag()const;

	// returns false if this advertisment is not of iBeacon format.
	bool getIBeaconInfo(LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower) const;

	

	// Copy the "original" advertisement packet data
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawManufacturerData(uint8_t* dstBuf, uint32_t bufLen);

	// Copy the "original" advertisement packet data
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawAdvertisementData(uint8_t* dstBuf, uint32_t bufLen);
	
	// Copy the "original" scan response packet data.
	// Note that scan response is optional and may be empty (all zero).
	//
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawScanResponseData(uint8_t* dstBuf, uint32_t bufLen);

	static const uint32_t MAX_ADV_DATA_LEN = 0x1F;

private:
	
	bt_gap_le_advertising_report_ind_t& adv_data;
	bt_gap_le_advertising_report_ind_t& resp_data;

	// searches if a specific AD Type field exists in advertisement or scan response data.
	// copy the content if found, and returns the length of the AD data found.
	// otherwise 0 is returned.
	//
	// param type: one of the advertisement type in Bluetooth specification.
	//			   refer to bt_gap_le.h for macro constants like BT_GAP_LE_AD_TYPE_FLAG.
	uint32_t getAdvDataWithType(uint8_t type, uint8_t* dstBuf, uint32_t bufLen) const;

	static uint32_t getAdvDataWithTypeFromPayload(uint8_t type, 
												  uint8_t* dstBuf, uint32_t bufLen, 
												  const bt_gap_le_advertising_report_ind_t& payload);
};

class LBLEiBeacon
{
public:

};

#endif
