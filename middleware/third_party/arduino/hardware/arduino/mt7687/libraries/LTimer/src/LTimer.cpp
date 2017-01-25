#include <Arduino.h>

#include "LTimer.h"
#include <hal_gpt.h>

#define HAL_ID(id)  (_hal_id[id])

static hal_gpt_port_t _hal_id[] = {HAL_GPT_0, HAL_GPT_1};

LTimer::LTimer(LTimerID timerId)
{
    m_timerId = timerId;
};

/* initilize a timer */
LTimerStatus LTimer::begin()
{
    hal_gpt_status_t         ret;
    hal_gpt_running_status_t running_status = HAL_GPT_STOPPED;

    ret = hal_gpt_get_running_status(HAL_ID(m_timerId), &running_status);

    if (ret != HAL_GPT_STATUS_OK || running_status != HAL_GPT_STOPPED)
    {
        return LTIMER_ERROR_PORT;
    }

    if (hal_gpt_init(HAL_ID(m_timerId)) != HAL_GPT_STATUS_OK)
    {
        return LTIMER_ERROR_PORT;
    }

    return LTIMER_OK;
}

/* start the timer
   [IN] timeoutMS    - timeout time in milliseconds
   [IN] timerMode    - oneshot or repeat mode
   [IN] callbackFunc - the callback function when the timer is expired
   [IN] userData     - data to be passed when the callback is invoked
*/
LTimerStatus LTimer::start(
    uint32_t            timeoutMS,
    LTimerMode          timerMode,
    ltimer_callback_t   callbackFunc,
    void                *userData)
{
    hal_gpt_status_t ret;

    ret = hal_gpt_register_callback(HAL_ID(m_timerId), callbackFunc, userData);

    if (ret != HAL_GPT_STATUS_OK)
    {
        return LTIMER_INVALID_PARAM;
    }

    ret = hal_gpt_start_timer_ms(
        HAL_ID(m_timerId),
        timeoutMS,
        (timerMode == LTIMER_ONESHOT_MODE)? HAL_GPT_TIMER_TYPE_ONE_SHOT: HAL_GPT_TIMER_TYPE_REPEAT);

    if (ret != HAL_GPT_STATUS_OK)
    {
        return LTIMER_INVALID_PARAM;
    }

    return LTIMER_OK;
}

/* stop the timer */
LTimerStatus LTimer::stop()
{
    hal_gpt_status_t ret;

    ret = hal_gpt_stop_timer(HAL_ID(m_timerId));

    return (ret == HAL_GPT_STATUS_OK)? LTIMER_OK: LTIMER_ERROR;
}

/* release the timer */
LTimerStatus LTimer::end()
{
    hal_gpt_status_t ret;

    ret = hal_gpt_deinit(HAL_ID(m_timerId));

    return (ret == HAL_GPT_STATUS_OK)? LTIMER_OK: LTIMER_ERROR;
}
