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
    FOTA_STATUS_OK = 1,
    FOTA_STATUS_FAILED = 0
};

class FOTARegion
{
public:
    FOTARegion():
        m_offset(0)
    {

    }

public:

    // resets the internal file pointer to beginning of the FOTA region
    void reset() {
        m_offset = 0;
    }

    // the internal file pointer increments automatically
    FotaError write(const uint8_t* buffer, uint32_t length);

protected:
    const size_t m_size = FOTA_LENGTH;    // available length of the flash region
    const size_t m_base = FOTA_BASE;
    const size_t m_blockSize = 4096;
    size_t m_offset;        // the file pointer
};

FotaError FOTARegion::write(const uint8_t* buffer, uint32_t length)
{
    //
    // Check the validity of parameters
    //
    if (buffer == 0 || length == 0) {
        return FOTA_ERROR_BINARY_EMPTY;
    }

    if ((m_offset + length) > m_size) {
        return FOTA_ERROR_BINARY_TOO_LARGE;
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
            return FOTA_ERROR_FLASH_OP;
        }
    }

    uint32_t i = block_idx_start + 1;
    while (i <= block_idx_end) {
        const uint32_t erase_addr = i * m_blockSize;
        IRQLock irq;
        if (hal_flash_erase(erase_addr, HAL_FLASH_BLOCK_4K) < 0) {
            return FOTA_ERROR_FLASH_OP;
        }
        i++;
    }

    //
    // Write data
    //
    do {
        IRQLock irq;
        if (hal_flash_write(addr, (uint8_t *)buffer, length) < 0) {
            return FOTA_ERROR_FLASH_OP;
        }
    } while (false);
    
    // Increment file pointer
    m_offset += length;
    return FOTA_ERROR_NO_ERROR;
}

LHTTPUpdate::LHTTPUpdate(void)
{
    memset(&_httpClient, 0, sizeof(_httpClient));
}

LHTTPUpdate::~LHTTPUpdate(void)
{
}

void LHTTPUpdate::rebootOnUpdate(bool reboot)
{
    _rebootOnUpdate = reboot;
}

int LHTTPUpdate::_fota_http_retrieve_get(const char* get_url)
{
    //
    // send GET request to server
    //
    httpclient_data_t client_data = {
        .is_more = 0,
        .is_chunked = 0,
        .retrieve_len = 0,
        .response_content_len = 0,
        .post_buf_len = 0,
        .response_buf_len = 0,
        .header_buf_len = 0,
        .post_content_type = NULL,
        .post_buf = NULL,
        .response_buf = NULL,
        .header_buf = NULL,
    };

    _fotaBuffer.resize(4 * 1024 + 1);
    client_data.response_buf = (char*)&_fotaBuffer[0];
    client_data.response_buf_len = (int)_fotaBuffer.size();

    HTTPCLIENT_RESULT ret = httpclient_send_request(&_httpClient, (char*)get_url, HTTPCLIENT_GET, &client_data);
    if (ret < 0) {
        pr_debug("[FOTA DL] http client fail to send request \n");
        _lastError = FOTA_ERROR_CONNECTION;
        return FOTA_STATUS_FAILED;
    }

    int count = 0;
    int recv_temp = 0;
    int data_len = 0;

    //
    // get server response & write to flash
    //
    FOTARegion fotaRegion;
    do {
        ret = httpclient_recv_response(&_httpClient, &client_data);
        if (ret < 0) {
            pr_debug("[FOTA DL] http client recv response error, ret = %d \n", ret);
            _lastError = FOTA_ERROR_CONNECTION;
            return FOTA_STATUS_FAILED;
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
        
        pr_debug("[FOTA DL] total data received %u \n", count);

        const FotaError write_ret = fotaRegion.write((const uint8_t*)client_data.response_buf, data_len);
        if (FOTA_ERROR_NO_ERROR != write_ret) {
            pr_debug("[FOTA DL] fail to write flash, write_ret = %d \n", write_ret);
            _lastError = write_ret;
            return FOTA_STATUS_FAILED;
        }

        pr_debug("[FOTA DL] download progrses = %u \n", count * 100 / client_data.response_content_len);
        
    } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    //
    // report back to user
    //
    pr_debug("[FOTA DL] total length: %d \n", client_data.response_content_len);
    if (count != client_data.response_content_len || httpclient_get_response_code(&_httpClient) != 200) {
        pr_debug("[FOTA DL] data received not completed, or invalid error code \r\n");
        _lastError = FOTA_ERROR_BINARY_INCOMPLETE;
        return FOTA_STATUS_FAILED;
    }
    else if (count == 0) {
        pr_debug("[FOTA DL] receive length is zero, file not found \n");
        _lastError = FOTA_ERROR_BINARY_EMPTY;
        return FOTA_STATUS_FAILED;
    }
    else {
        pr_debug("[FOTA DL] download success \n");
        return FOTA_STATUS_OK;
    }
}

int LHTTPUpdate::update(const String& url)
{
    // connect to server
    HTTPCLIENT_RESULT connectResult = httpclient_connect(&_httpClient, (char*)url.c_str());
    
    if (HTTPCLIENT_OK != connectResult) {
        pr_debug("[FOTA DL] http client connect error\n");
        _lastError = FOTA_ERROR_CONNECTION;
        httpclient_close(&_httpClient);
        return FOTA_STATUS_FAILED;
    }
    
    // download FOTA bin and write to FOTA flash region
    int ret = _fota_http_retrieve_get(url.c_str());
    pr_debug("[FOTA DL] Download result = %d\n", (int)ret);

    if (FOTA_STATUS_OK == ret) {
        // continue...cd
        //Trigger a FOTA update.
        fota_ret_t ret;
        ret = fota_trigger_update();
        if (ret == FOTA_TRIGGER_SUCCESS && _rebootOnUpdate) {
            pr_debug("[FOTA] reboot for update");
            hal_wdt_software_reset();
        }
        return FOTA_STATUS_OK;
    } else {
        return FOTA_STATUS_FAILED;
    }
}

// returns one of the reasons in enum FotaError.
int LHTTPUpdate::getLastError(void)
{
    return _lastError;
}

// returns string of error reason
String LHTTPUpdate::getLastErrorString(void)
{
    switch (getLastError()) {
    case FOTA_ERROR_NO_ERROR:
        return "No error";
    case FOTA_ERROR_CONNECTION:
        return "Connection failed";
    case FOTA_ERROR_BINARY_TOO_LARGE:
        return "Binary too large";
    case FOTA_ERROR_BINARY_EMPTY:
        return "Binary is empty";
    case FOTA_ERROR_BINARY_INCOMPLETE:
        return "Binary download incomplete";
    case FOTA_ERROR_FLASH_OP:
        return "Flash write failed";
    default:
        return "Unknown";
    }
}