/*
  Adapter layer from Arduino to LinkIt SDK API
*/
#ifndef _ARD_BLE_H_
#define _ARD_BLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bt_debug.h>
#include <bt_type.h>
#include <bt_uuid.h>
#include <bt_system.h>
#include <bt_hci.h>
#include <bt_gap_le.h>
#include <bt_gatt.h>
#include <bt_gatts.h>
#include <log_dump.h>

// we supress missing filed initializer warnings 
// because GCC incorrectly complains the BT_GATTS_NEW_PRIMARY_SERVICE_16
// macros failing to initialize the "perm" atrribute - which it does.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces"

extern int ard_ble_begin(void);
extern int ard_ble_is_ready(void);
extern void generate_random_device_address(bt_bd_addr_t addr);

// implemented by LBLE.cpp
extern void ard_ble_postAllEvents(bt_msg_type_t msg, bt_status_t status, void *buff);

#ifdef __cplusplus
}
#endif

#endif
