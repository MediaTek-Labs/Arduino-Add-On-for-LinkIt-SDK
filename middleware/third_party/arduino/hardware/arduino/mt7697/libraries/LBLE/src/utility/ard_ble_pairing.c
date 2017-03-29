// This file implements the callback required by BT framework
// that are used to determine pairing & bonding policies.

#include "ard_ble.h"
#include "bt_hci.h"
#include "bt_gap_le.h"

/**
 * @brief     This API invoked by the SDK process should be implemented by the application. The application should return the Local Configuration field.
 * @return    The loacl configuration pointer, please set the local key and secure connection mode flag. The pointer should not be NULL and it must be a global variable.
 */
bt_gap_le_local_config_req_ind_t *bt_gap_le_get_local_config(void)
{
    static bt_gap_le_local_config_req_ind_t local_config = {0};
    static bt_gap_le_local_key_t local_key = {
        .encryption_info.ltk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc8, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .master_id = {
            .ediv = 0x1005,
            .rand = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 },
        },
        .identity_info.irk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .signing_info.csrk = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf }
    };

    local_config.local_key_req = &local_key;
    local_config.sc_only_mode_req = false;
 
    return &local_config;
}

/**
 * @brief   This API invoked by the SDK process should be implemented by the application. The application should return the Bonding Information field.
 * @param[in] remote_addr The address of the remote device to be bonded.
 * @return                The Bonding Information pointer, please set a pointer to the connection bonding information. The pointer should not be NULL and it must be a global variable.
 */
bt_gap_le_bonding_info_t *bt_gap_le_get_bonding_info(const bt_addr_t __attribute__ ((unused))remote_addr)
{
    static bt_gap_le_bonding_info_t  bonding_info = {0};
    return &bonding_info;
}

/**
 * @brief   This API invoked by the SDK process should be implemented by the application. The application should return the Pairing Configuration field.
 * @param[in] ind         Bonding start indication structure. Application should set the pairing_config_req variable to a global variable.
 * @return    #BT_STATUS_SUCCESS, the application set the pairing configuration successfully.
 *            #BT_STATUS_OUT_OF_MEMORY, out of memory.
 */
bt_status_t bt_gap_le_get_pairing_config(bt_gap_le_bonding_start_ind_t *ind)
{
    static bt_gap_le_smp_pairing_config_t pairing_config = {
        .maximum_encryption_key_size = 16,
        .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
        .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
        .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
        .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
        .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    };

    ind->pairing_config_req = pairing_config;

    return BT_STATUS_SUCCESS;
}


