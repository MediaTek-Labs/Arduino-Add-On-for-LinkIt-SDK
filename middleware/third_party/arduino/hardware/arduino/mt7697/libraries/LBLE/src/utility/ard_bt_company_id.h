/*
	This Bluetooth Manufacturer ID Table is converted from 
	https://github.com/dannycabrera/Bluetooth-Company-Identifiers
*/
#ifndef ARD_BT_COMPANY_ID_H
#define ARD_BT_COMPANY_ID_H

#include <stdint.h>

const char* getBluetoothCompanyName(uint16_t companyId);
#endif // ARD_BT_COMPANY_ID_H
