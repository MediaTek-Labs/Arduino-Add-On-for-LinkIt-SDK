#ifndef __LTIMER_H__
#define __LTIMER_H__

#include <Arduino.h>

typedef void (*ltimer_callback_t)(void *user_data);

typedef enum
{
    LTIMER_ONESHOT_MODE,
    LTIMER_REPEAT_MODE
} LTimerMode;

typedef enum
{
    LTIMER_OK,
    LTIMER_INVALID_PARAM,
    LTIMER_ERROR_PORT,
    LTIMER_ERROR
} LTimerStatus;

typedef enum
{
    LTIMER_0 = 0,
    LTIMER_1 = 1
} LTimerID;

class LTimer {
public:
    LTimer(LTimerID timerId);

    /* initilize a timer */
    LTimerStatus begin();

    /* start the timer
       [IN] timeoutMS    - timeout time in milliseconds
       [IN] timerMode    - oneshot or repeat mode
       [IN] callbackFunc - the callback function when the timer is expired
       [IN] userData     - data to be passed when the callback is invoked
    */
    LTimerStatus start(
        uint32_t            timeoutMS,
        LTimerMode          timerMode,
        ltimer_callback_t   callbackFunc,
        void                *userData);

    /* stop the timer */
    LTimerStatus stop();

    /* release the timer */
    LTimerStatus end();

private:
    LTimerID m_timerId;
};

#endif
