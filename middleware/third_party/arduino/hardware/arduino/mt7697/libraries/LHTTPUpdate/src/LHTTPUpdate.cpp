#include "LHTTPUpdate.h"
#include <mutex>

extern "C" {
#include <FreeRTOS.h>
#include <task.h>
#include <hal_flash.h>
#include <flash_map.h>
#include <httpclient.h>
#include <hal_wdt.h>
#include <fota.h>
#include "log_dump.h"
}

class IRQLock
{
public:
    IRQLock() {
        taskDISABLE_INTERRUPTS();
    }

    ~IRQLock() {
        taskENABLE_INTERRUPTS();
    }
    
    // Copy constructor is deleted.
    IRQLock( const IRQLock& ) = delete;
};

enum FotaStatus {
    FOTA_STATUS_OK,
    FOTA_STATUS_IS_EMPTY,
    FOTA_STATUS_IS_FULL,
    FOTA_STATUS_ERROR_INVALD_PARAMETER, //< supplied parameter is not valid.
    FOTA_STATUS_ERROR_BLOCK_ALIGN,      //< partition not block-aligned.
    FOTA_STATUS_ERROR_OUT_OF_RANGE,     //< the partition ran out-of-space.
    FOTA_STATUS_ERROR_UNKNOWN_ID,       //< the partition is not known.
    FOTA_STATUS_ERROR_NOT_INITIALIZED,  //< FOTA is not initialized yet.
    FOTA_STATUS_ERROR_FLASH_OP,         //< Flash operation failed.
};

class FOTARegion
{
public:
    FOTARegion():
        m_offset(0),
        m_size(FOTA_LENGTH),
        m_base(FOTA_BASE),
        m_blockSize(4096)
    {

    }

public:

    // resets the internal file pointer to beginning of the FOTA region
    int reset() {
        m_offset = 0;
    }

    // the internal file pointer increments automatically
    FotaStatus write(const uint8_t* buffer, uint32_t length);

protected:
    const size_t m_size;    // available length of the flash region
    const size_t m_base;   
    const size_t m_blockSize;  
    size_t m_offset;        // the file pointer
};

FotaStatus FOTARegion::write(const uint8_t* buffer, uint32_t length)
{
    //
    // Check the validity of parameters
    //
    if (buffer == 0 || length == 0) {
        return FOTA_STATUS_ERROR_INVALD_PARAMETER;
    }

    if ((m_offset + length) > m_size) {
        return FOTA_STATUS_ERROR_OUT_OF_RANGE;
    }

    // 
    // Erase blocks: if the write is to the block boundary, erase the block
    //
    const uint32_t addr = m_base + m_offset;
    const uint32_t block_idx_start = addr / m_blockSize;
    const uint32_t block_idx_end = (addr + length - 1) / m_blockSize;

    if ((addr % m_blockSize) == 0) {
        IRQLock irq;
        if (hal_flash_erase(addr, HAL_FLASH_BLOCK_4K) < 0) {
            return FOTA_STATUS_ERROR_FLASH_OP;
        }
    }

    int i = block_idx_start + 1;
    while (i <= block_idx_end) {
        const uint32_t erase_addr = i * m_blockSize;
        IRQLock irq;
        if (hal_flash_erase(erase_addr, HAL_FLASH_BLOCK_4K) < 0) {
            return FOTA_STATUS_ERROR_FLASH_OP;
        }
        i++;
    }

    //
    // Write data
    //
    do{
        IRQLock irq;
        if (hal_flash_write(addr, (uint8_t *)buffer, length) < 0) {
            return FOTA_STATUS_ERROR_FLASH_OP;
        }
    } while(false);
    
    // Increment file pointer
    m_offset += length;
    return FOTA_STATUS_OK;
}

LHTTPUpdate::LHTTPUpdate(void)
{
    memset(&m_httpClient, 0, sizeof(m_httpClient));
}

LHTTPUpdate::~LHTTPUpdate(void)
{
}

int32_t LHTTPUpdate::_fota_http_retrieve_get(const char* get_url)
{
    int32_t ret = HTTPCLIENT_ERROR_CONN;

    m_fotaBuffer.resize(4 * 1024 + 1);
    httpclient_data_t client_data = {0};
    client_data.response_buf = (char*)&m_fotaBuffer[0];
    client_data.response_buf_len = (int)m_fotaBuffer.size();

    ret = httpclient_send_request(&m_httpClient, (char*)get_url, HTTPCLIENT_GET, &client_data);
    if (ret < 0) {
        pr_debug("[FOTA DL] http client fail to send request \n");
        return ret;
    }

    uint32_t count = 0;
    uint32_t recv_temp = 0;
    uint32_t data_len = 0;

    FOTARegion fotaRegion;

    do {
        ret = httpclient_recv_response(&m_httpClient, &client_data);
        if (ret < 0) {
            pr_debug("[FOTA DL] http client recv response error, ret = %d \n", ret);
            return ret;
        }

        if (recv_temp == 0)
        {
            recv_temp = client_data.response_content_len;
        }

        pr_debug("[FOTA DL] retrieve_len = %d \n", client_data.retrieve_len);
        
        data_len = recv_temp - client_data.retrieve_len;
        pr_debug("[FOTA DL] data_len = %u \n", data_len);
        
        count += data_len;
        recv_temp = client_data.retrieve_len;
        
        //vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
        pr_debug("[FOTA DL] total data received %u \n", count);

        auto write_ret = fotaRegion.write((const uint8_t*)client_data.response_buf, data_len);
        if (FOTA_STATUS_OK != write_ret) {
            pr_debug("[FOTA DL] fail to write flash, write_ret = %d \n", write_ret);
            return ret;
        }

        pr_debug("[FOTA DL] download progrses = %u \n", count * 100 / client_data.response_content_len);
        
    } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    pr_debug("[FOTA DL] total length: %d \n", client_data.response_content_len);
    if (count != client_data.response_content_len || httpclient_get_response_code(&m_httpClient) != 200) {
        pr_debug("[FOTA DL] data received not completed, or invalid error code \r\n");
        return -1;
    }
    else if (count == 0) {
        pr_debug("[FOTA DL] receive length is zero, file not found \n");
        return -2;
    }
    else {
        pr_debug("[FOTA DL] download success \n");
        return ret;
    }
        

}

LHTTPUpdateResult LHTTPUpdate::update(const String& url, const String& currentVersion)
{
    // connect to server
    int ret = httpclient_connect(&m_httpClient, (char*)url.c_str());
    
    if (!ret) {
        // download FOTA bin and write to FOTA flash region
        ret = _fota_http_retrieve_get(url.c_str());
    }else {
        pr_debug("[FOTA DL] http client connect error. \r");
    }
    
    pr_debug("[FOTA DL] Download result = %d \r\n", (int)ret);
    httpclient_close(&m_httpClient);

    if (ret == FOTA_UPDATE_OK) {
        // continue...cd
        //Trigger a FOTA update.
        fota_ret_t ret;
        ret = fota_trigger_update();
        if (ret == FOTA_TRIGGER_SUCCESS && _rebootOnUpdate)
        {
            pr_debug("[FOTA] reboot for update")
            // reboot according to user's will
            hal_wdt_software_reset();
        }
    }

    return FOTA_UPDATE_FAILED;
}

