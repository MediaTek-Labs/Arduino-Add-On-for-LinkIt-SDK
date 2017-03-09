/*
// A BLE peripheral have 2 major operations:
//	* Advertising itself for BLE central device to scan.
//	* Optinally, it may also providing services consisting of attributes for BLE central to connect to.
*/

#ifndef LBLEPERIPHERAL_H
#define LBLEPERIPHERAL_H

#include <inttypes.h>
#include <vector>
#include <WString.h>
#include <LBLE.h>
#include <vector>

extern "C"
{
#include "utility/ard_ble.h"
}

struct LBLEAdvDataItem
{
	// according to BLE spec, there is no way we can accomodate
	// more than 31 bytes.
	static const uint32_t MAX_ADV_DATA_LEN = 0x1F;

	uint8_t adType;
	uint8_t adData[MAX_ADV_DATA_LEN];
	uint32_t adDataLen;

	void clear()
	{
		adType = 0;
		adDataLen = 0;
		memset(adData, 0, MAX_ADV_DATA_LEN);
	}
};


class LBLEAdvertisementData
{
public:
	LBLEAdvertisementData();

	// This methods RESETS all the advertisement data fields
	// and replace them with iBeacon format (flag + manufacturer data)
	//
	// if you don't know which UUID to use, 
	// use LBLEUuid("74278BDA-B644-4520-8F0C-720EAF059935"),
	// since this is the UUID used by iOS AirLocate example.
	// (https://developer.apple.com/library/content/samplecode/AirLocate/Introduction/Intro.html)
	//
	// major, minor, and txPower are all user defined values.
	void configAsIBeacon(const LBLEUuid& uuid, 		
							  uint16_t major, 
							  uint16_t minor, 
							  int8_t txPower);


	// Convert to a raw advertisement data payload.
	uint32_t getPayload(uint8_t* buf, uint32_t bufLength) const;

private:
	std::vector<LBLEAdvDataItem> m_advDataList;
};

class LBLEPeripheral
{
public:
	// start advertisement
	void advertise(const LBLEAdvertisementData& advertisementData);
	// stop advertisement
	void stopAdvertise();

};


#endif // #ifndef LBLEPERIPHERAL_H
