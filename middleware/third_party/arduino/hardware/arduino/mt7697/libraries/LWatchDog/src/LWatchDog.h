#ifndef __LWATCHDOG_H__
#define __LWATCHDOG_H__

#include <Arduino.h>


// A Watch Dog is basically a hardware supported timer
// that reboots the whole system after a given period
// of time. To keep the system alive, the user
// must call the reset function (`feed()` in this class)
// periodically.
class LWatchDogClass
{
public:
    // Begin the watch dog and start the count down
    // for `secondsBeforeReboot`.
    // 
    // If feed() is not called for a period of
    // `secondsBeforeReboot`, the system reboots
    // automatically.
    // 
    // The default timer is 10 seconds.
    // 
    // returns 1 if the watch dog start successfully.
    // returns 0 if the watch dog fails to start.
    int begin(uint32_t secondsBeforeReboot = 10);

    // Call this method to reset the reboot timer set with `begin()`
    void feed();

    // Stop the watch dog.
    void end();

public:
    // Constructor
    LWatchDogClass();

    // Destructor
    ~LWatchDogClass();
};

// The global watch dog object
extern LWatchDogClass LWatchDog;
#endif // __LWATCHDOG_H__
