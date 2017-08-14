#include "LWatchDog.h"

extern "C"
{
#include <hal_wdt.h>
}

// The global singleton class for watch dog.
LWatchDogClass LWatchDog;

int LWatchDogClass::begin(uint32_t secondsBeforeReboot)
{
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = secondsBeforeReboot;
    const hal_wdt_status_t initStat = hal_wdt_init(&wdt_config);
    const hal_wdt_status_t enableStat = hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
    if(HAL_WDT_STATUS_OK == initStat && HAL_WDT_STATUS_OK == enableStat)
    {
        feed();
        return 1;
    }
    else
    {
        return 0;
    }
}

void LWatchDogClass::feed()
{
    hal_wdt_feed(HAL_WDT_FEED_MAGIC);  //feed the WDT regularly.
}

// Stop the watch dog.
void LWatchDogClass::end()
{
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    hal_wdt_deinit();
}

// Constructor
LWatchDogClass::LWatchDogClass()
{

}

// Destructor
LWatchDogClass::~LWatchDogClass()
{
    if(hal_wdt_get_enable_status())
    {
        end();
    }
}