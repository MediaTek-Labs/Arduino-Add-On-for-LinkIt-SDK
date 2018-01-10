// As of LinkIt SDK v4.2.1, we cannot pass user data to BLE framework in attribute read/write callback.
// This file provides a statically allocated table to workaround this problem.
#ifndef ARD_BT_ATTR_CALLBACK_H
#define ARD_BT_ATTR_CALLBACK_H

#include "ard_ble.h"

#define ARD_BT_MAX_ATTR_CALLBACK_NUM (60)   // 60 attributes ~= 20 Characteristics

#define ARD_BT_CB_TYPE_VALUE_CALLBACK  (0)
#define ARD_BT_CB_TYPE_CCCD_CALLBACK   (1)

// Call this function to allocate a callback function that you can pass to for BLE framework
// that will call ard_bt_callback_trampoline
//
// @param user_data pointer to user data
// @param int callback type, can be ARD_BT_CB_TYPE_VALUE_CALLBACK or ARD_BT_CB_TYPE_CCCD_CALLBACK
bt_gatts_rec_callback_t ard_bt_alloc_callback_slot(void* user_data, int callback_type);

// Define the function body. This function is called when the callback returned from ard_bt_alloc_callback_slot() is called, 
// with the user_data you pass when calling ard_bt_alloc_callback_slot.
extern uint32_t ard_bt_callback_trampoline(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset, void* user_data, int callback_type);


#endif // ARD_BT_ATTR_CALLBACK_H