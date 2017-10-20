#ifndef __LHTTPUPDATE_H__
#define __LHTTPUPDATE_H__

#include <Arduino.h>
#include <WString.h>
#include <vector>
extern "C" {
#include "httpclient.h"
}

enum FotaError {
    FOTA_ERROR_NO_ERROR = 0,
    FOTA_ERROR_CONNECTION = -100,  //< Internet connection failed
    FOTA_ERROR_BINARY_TOO_LARGE,  //< FOTA is not initialized yet.
    FOTA_ERROR_BINARY_EMPTY,        //< FOTA is not initialized yet.
    FOTA_ERROR_BINARY_INCOMPLETE,        //< FOTA is not initialized yet.
    FOTA_ERROR_FLASH_OP,         //< Flash operation failed.
};

class LHTTPUpdate
{
public:
    LHTTPUpdate(void);

    ~LHTTPUpdate(void);

    // URL to the FOTA(Firmware-over-the-air) binary.
    //
    // The FOTA binary is a compressed application binary.
    // Download the "FOTA ROM Package" Tool from
    // https://docs.labs.mediatek.com/resource/mt7687-mt7697/en/downloads#Downloads-Tools
    // to convert the application binary built by Ardino IDE to FOTA binary.
    //
    // To get the application binary of your sketch,
    // select "Sketch > Export Compiled Binary" from the IDE menu.
    //
    // returns  1: success, 0: error
    int update(const String& url);

    // pass true to automatically reboot after a successful update() call.
    void rebootOnUpdate(bool reboot);

    // returns one of the reasons in enum FotaError.
    int getLastError(void);

    // returns string of error reason
    String getLastErrorString(void);

protected:
    // returns 1: success, 0: error
    int _fota_http_retrieve_get(const char* get_url);

protected:
    httpclient_t _httpClient;
    std::vector<uint8_t> _fotaBuffer;
    bool _rebootOnUpdate = false;
    int _lastError = FOTA_ERROR_NO_ERROR;
};

#endif // #ifndef __LHTTPUPDATE_H__