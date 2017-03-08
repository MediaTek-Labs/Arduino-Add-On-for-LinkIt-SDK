/*
  Adapter layer from Arduino to LinkIt SDK API
*/
#ifndef _ARD_BLE_H_
#define _ARD_BLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bt_hci.h>
#include <bt_gap_le.h>
#include <bt_debug.h>
#include <bt_system.h>
#include <bt_type.h>
#include <bt_uuid.h>

extern int ard_ble_begin(void);
extern int ard_ble_is_ready(void);

extern int ard_ble_wait_for_event(bt_msg_type_t event);
extern void ard_ble_central_onCentralEvents(bt_msg_type_t msg, bt_status_t status, void *buff);

#ifdef __cplusplus
}
#endif

#endif
