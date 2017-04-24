// This file statically defines the required attributes for BLE GAP.
#include "ard_ble.h"

const bt_uuid_t CLI_BT_SIG_UUID_DEVICE_NAME =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_DEVICE_NAME);
const bt_uuid_t CLI_BT_SIG_UUID_APPEARANCE =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_APPEARANCE);
const bt_uuid_t CLI_BT_SIG_UUID_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS);
const bt_uuid_t CLI_BT_SIG_UUID_SERIAL_NUMBER =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SERIAL_NUMBER);
const bt_uuid_t CLI_BT_SIG_UUID_CENTRAL_ADDRESS_RESOLUTION =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION);

//Declare every record here
//service collects all bt_gatts_service_rec_t
//IMPORTAMT: handle:0x0000 is reserved, please start your handle from 0x0001
//GAP 0x0001
static char g_gatts_device_name[256]={"LinkIt 7697"};

void ard_bt_set_gatts_device_name(const char* device_name)
{
    if(NULL == device_name)
    {
        return;
    }
    
    strncpy(g_gatts_device_name, device_name, sizeof(g_gatts_device_name) - 1);
    return;
}

static uint32_t bt_if_gap_dev_name_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    uint32_t str_size = strlen(g_gatts_device_name);
    uint32_t buf_size = sizeof(g_gatts_device_name);
    uint32_t copy_size;

    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            copy_size = (str_size> offset)?(str_size-offset):0;
            if (size==0){
                return str_size;
            }
            copy_size = (size > copy_size)? copy_size:size;
            memcpy(data, g_gatts_device_name+offset, copy_size);
            return copy_size;
        case BT_GATTS_CALLBACK_WRITE:
            copy_size = (size > buf_size)? buf_size:size;
            memcpy(g_gatts_device_name, data, copy_size);
            return copy_size;
        default:
            return BT_STATUS_SUCCESS;
    }
}

uint16_t gap_appearance=0x1234;//GAP appearance
static uint32_t bt_if_gap_appearance_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (rw == BT_GATTS_CALLBACK_WRITE){
        if (size != sizeof(gap_appearance)){ //Size check
            return 0;
        }
        gap_appearance = *(uint16_t*)data;
    }else {
        if (size!=0){
            uint16_t *buf = (uint16_t*) data;
            *buf = gap_appearance;
        }
    }
    return sizeof(gap_appearance);
}
BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_gap_primary_service, BT_GATT_UUID16_GAP_SERVICE);
BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_dev_name, BT_GATT_CHARC_PROP_READ, 0x0003, BT_SIG_UUID16_DEVICE_NAME);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_gap_dev_name, CLI_BT_SIG_UUID_DEVICE_NAME, BT_GATTS_REC_PERM_READABLE, bt_if_gap_dev_name_callback);
/* For BQB test TC_GAR_SR_BV_07_C & TC_GAR_SR_BV_08_C*/
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(bt_if_gap_dev_name_user_description,
                BT_GATTS_REC_PERM_READABLE, bt_if_gap_dev_name_callback);
/* For BQB test TC_GAR_SR_BI_01_C */
/* This test characteristic can not read and write */
BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_serial_number, 0,
                0x0006, BT_SIG_UUID16_SERIAL_NUMBER);

#define MY_VENDOR_SERIAL_NUMBER       "01-32-588"
BT_GATTS_NEW_CHARC_VALUE_STR16(bt_if_gap_serial_number, CLI_BT_SIG_UUID_SERIAL_NUMBER,
                0, 9, MY_VENDOR_SERIAL_NUMBER);
/* For BQB test TC_GAR_SR_BI_28_C */
//can not read and write.
BT_GATTS_NEW_CHARC_USER_DESCRIPTION_STR16(bt_if_gap_serial_number_user_description,
                0,
                8, "MediaTek");
BT_GATTS_NEW_CHARC_16_WRITABLE(bt_if_gap_char4_appearance, BT_GATT_CHARC_PROP_READ, 0x0009, BT_SIG_UUID16_APPEARANCE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_gap_appearance, CLI_BT_SIG_UUID_APPEARANCE,
                BT_GATTS_REC_PERM_READABLE|BT_GATTS_REC_PERM_WRITABLE, bt_if_gap_appearance_callback);

BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_ppcp, BT_GATT_CHARC_PROP_READ, 0x000B, BT_SIG_UUID16_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS);
BT_GATTS_NEW_CHARC_VALUE_HALFW8_WRITABLE(bt_if_gap_ppcp, CLI_BT_SIG_UUID_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS,
                BT_GATTS_REC_PERM_READABLE, 8, 0x0580,0x0c80,0x0010,0x0333);
BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_central_address_resolution, BT_GATT_CHARC_PROP_READ, 0x000D, BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION);
BT_GATTS_NEW_CHARC_VALUE_UINT8_WRITABLE(bt_if_central_address_resolution, CLI_BT_SIG_UUID_CENTRAL_ADDRESS_RESOLUTION, BT_GATTS_REC_PERM_READABLE, 1);

static const bt_gatts_service_rec_t *bt_if_gap_service_rec[] = {
    (const bt_gatts_service_rec_t*) &bt_if_gap_primary_service,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_dev_name,
    (const bt_gatts_service_rec_t*) &bt_if_gap_dev_name,
    (const bt_gatts_service_rec_t*) &bt_if_gap_dev_name_user_description,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_serial_number,
    (const bt_gatts_service_rec_t*) &bt_if_gap_serial_number,
    (const bt_gatts_service_rec_t*) &bt_if_gap_serial_number_user_description,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_appearance,
    (const bt_gatts_service_rec_t*) &bt_if_gap_appearance,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_ppcp,
    (const bt_gatts_service_rec_t*) &bt_if_gap_ppcp,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_central_address_resolution,
    (const bt_gatts_service_rec_t*) &bt_if_central_address_resolution
    };

const bt_gatts_service_t bt_if_gap_service = {
    .starting_handle = 0x0001,
    .ending_handle = 0x000D,
    .required_encryption_key_size = 7,
    .records = bt_if_gap_service_rec
    };


///////////////////////

const bt_uuid_t CLI_BT_SIG_UUID_SERVICE_CHANGED =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SERVICE_CHANGED);

BT_GATTS_NEW_PRIMARY_SERVICE_16(gatt_primary_service, BT_GATT_UUID16_GATT_SERVICE);
BT_GATTS_NEW_CHARC_16(gatt_char4_service_changed, BT_GATT_CHARC_PROP_READ|BT_GATT_CHARC_PROP_NOTIFY|BT_GATT_CHARC_PROP_INDICATE,
                0x0013, BT_SIG_UUID16_SERVICE_CHANGED);
BT_GATTS_NEW_CHARC_VALUE_UINT32_WRITABLE(gatt_service_changed, CLI_BT_SIG_UUID_SERVICE_CHANGED,
                             0x2, 0x0001050F);

static const bt_gatts_service_rec_t *gatt_service_rec[] = {
    (const bt_gatts_service_rec_t*) &gatt_primary_service,
    (const bt_gatts_service_rec_t*) &gatt_char4_service_changed,
    (const bt_gatts_service_rec_t*) &gatt_service_changed
    };

const bt_gatts_service_t bt_if_gatt_service_ro = {
    .starting_handle = 0x0011,
    .ending_handle = 0x0013,
    .required_encryption_key_size = 7,
    .records = gatt_service_rec
};


//////////////////////////////////////////////////
////// from iot_sdk_demo, smartconnection data transfer service
//
///////////////////////////
#define BLE_SMTCN_SERVICE_UUID        (0x18AA) // Data Transfer Service
#define BLE_SMTCN_CHAR_UUID           (0x2AAA)
#define BLE_SMTCN_SSID_LEN            32
#define BLE_SMTCN_IP_LEN              17
#define BLE_SMTCN_CHAR_VALUE_HANDLE   0x0016
const bt_uuid_t BLE_SMTCN_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(BLE_SMTCN_CHAR_UUID);

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_dtp_primary_service, BLE_SMTCN_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_if_dtp_char,
                      BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_INDICATE, BLE_SMTCN_CHAR_VALUE_HANDLE, BLE_SMTCN_CHAR_UUID);


static uint32_t ble_smtcn_charc_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    return 0;
}

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_dtp_char_value, BLE_SMTCN_CHAR_UUID128,
                    BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ble_smtcn_charc_value_callback);

static uint32_t ble_smtcn_client_config_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    return 0;
}

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_if_dtp_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 ble_smtcn_client_config_callback);

static const bt_gatts_service_rec_t *bt_if_ble_smtcn_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_dtp_primary_service,
    (const bt_gatts_service_rec_t *) &bt_if_dtp_char,
    (const bt_gatts_service_rec_t *) &bt_if_dtp_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_dtp_client_config
};

const bt_gatts_service_t bt_if_ble_smtcn_service = {
    .starting_handle = 0x0014,
    .ending_handle = 0x0017,
    .required_encryption_key_size = 0,
    .records = bt_if_ble_smtcn_service_rec
};