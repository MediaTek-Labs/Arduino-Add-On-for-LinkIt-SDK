/*
*/

#ifndef __LRTC_H__
#define __LRTC_H__

#include <Arduino.h>

/***
***/

class LRTCClass
{
public:
    LRTCClass();

    int32_t year();
    int32_t month();
    int32_t day();
    int32_t hour();
    int32_t minute();
    int32_t second();

    void begin();
    void get();
    void set(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second);

private:
    int32_t m_year;
    int32_t m_month;
    int32_t m_day;
    int32_t m_hour;
    int32_t m_minute;
    int32_t m_second;
};

extern LRTCClass LRTC;

#endif
