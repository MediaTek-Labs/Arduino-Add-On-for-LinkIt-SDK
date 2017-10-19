#ifndef __LHTTPUPDATE_H__
#define __LHTTPUPDATE_H__

#include <Arduino.h>
#include <WString.h>
#include <vector>
extern "C" {
#include "httpclient.h"
}

enum LHTTPUpdateResult {
    FOTA_UPDATE_OK,
    FOTA_NO_UPDATES,
    FOTA_UPDATE_FAILED,
};

class LHTTPUpdate
{
public:
    LHTTPUpdate(void);
    ~LHTTPUpdate(void);

    void rebootOnUpdate(bool reboot)
    {
        _rebootOnUpdate = reboot;
    }

    // This function is deprecated, use rebootOnUpdate and the next one instead
    LHTTPUpdateResult update(const String& url, const String& currentVersion = "");

    int getLastError(void);
    String getLastErrorString(void);

protected:
    int32_t _fota_http_retrieve_get(const char* get_url);

protected:
    // LHTTPUpdateResult handleUpdate(HTTPClient& http, const String& currentVersion, bool spiffs = false);
    // bool runUpdate(Stream& in, uint32_t size, String md5, int command = U_FLASH);
    int _lastError;
    bool _rebootOnUpdate = true;
    httpclient_t m_httpClient;
    std::vector<uint8_t> m_fotaBuffer;
};

#endif // #ifndef __LHTTPUPDATE_H__