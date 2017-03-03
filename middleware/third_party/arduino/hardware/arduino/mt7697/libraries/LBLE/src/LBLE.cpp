#include "LBLE.h"

extern "C" {
#include "utility/ard_ble.h"
}

LBLEClass::LBLEClass()
{
}

int LBLEClass::begin()
{
	return ard_ble_begin();
}

int LBLEClass::ready()
{
	return ard_ble_is_ready();
}

LBLEClass LBLE;
