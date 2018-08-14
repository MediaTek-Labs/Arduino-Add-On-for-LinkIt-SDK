#include <Arduino.h>
#include "LRTC.h"
#include <hal_rtc.h>
#include <time.h>

LRTCClass LRTC;

#define YEAR_BASE   (2000)

LRTCClass::LRTCClass()
{

}

void LRTCClass::begin()
{
    if (hal_rtc_init() != HAL_RTC_STATUS_OK)
    {
        Serial.println("RTC init failed");
    }
}

void LRTCClass::get()
{
    hal_rtc_time_t time;

    if (hal_rtc_get_time(&time) != HAL_RTC_STATUS_OK)
    {
        Serial.println("RTC get time failed");
        return;
    }

    m_year   = time.rtc_year + YEAR_BASE;
    m_month  = time.rtc_mon;
    m_day    = time.rtc_day;
    m_hour   = time.rtc_hour;
    m_minute = time.rtc_min;
    m_second = time.rtc_sec;
}

void LRTCClass::set(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second)
{
    hal_rtc_time_t time;
    hal_rtc_status_t ret;

    memset(&time, 0, sizeof(time));

    time.rtc_year = year - YEAR_BASE;
    time.rtc_mon  = month;
    time.rtc_day  = day;
    time.rtc_hour = hour;
    time.rtc_min  = minute;
    time.rtc_sec  = second;

    ret = hal_rtc_set_time(&time);

    if (ret != HAL_RTC_STATUS_OK)
    {
        Serial.print(ret);
        Serial.println(") RTC set time failed");
        return;
    }

    m_year   = year;
    m_month  = month;
    m_day    = day;
    m_hour   = hour;
    m_minute = minute;
    m_second = second;
}

int32_t LRTCClass::year()
{
    return m_year;
}

int32_t LRTCClass::month()
{
    return m_month;
}

int32_t LRTCClass::day()
{
    return m_day;
}

int32_t LRTCClass::hour()
{
    return m_hour;
}

int32_t LRTCClass::minute()
{
    return m_minute;
}

int32_t LRTCClass::second()
{
    return m_second;
}

time_t LRTCClass::epoch()
{
    tm t;
    time_t epoch = 0;
    t.tm_sec = m_second;
    t.tm_min = m_minute;
    t.tm_hour = m_hour % 24;
    t.tm_mday = m_day;
    // C99 tm_month ranges from 0-11
    t.tm_mon = m_month - 1;
    // tm_year is years from 1900
    t.tm_year = m_year - 1900;

    // no day-light saving time
    t.tm_isdst = 0;

    // These are ignored by mktime
    t.tm_wday = 0;
    t.tm_yday = 0;
    
    epoch = mktime(&t);

    if(-1 == epoch) {
        return 0;
    } else {
        return (uint64_t)epoch;
    }
}