#include "Tone.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "pins_arduino.h"
#include "hal_pwm.h"
#include "delay.h"

#include "log_dump.h"

#define PWM_SRC_CLOCK				HAL_PWM_CLOCK_32KHZ
#define PWM_MAX_FREQUENCY			(6500ul)
#define PWM_MIN_FREQUENCY			(10ul)
#ifndef HAL_PWM_MAX
#define HAL_PWM_MAX HAL_PWM_MAX_CHANNEL
#endif

#include "hal_gpt.h"

typedef struct gpt_channel {
	hal_gpt_port_t		gpt_port;
	pin_desc_t		*pin_desc;

	// The following member is just used to speed up access.
	hal_pwm_channel_t	pwm_channel;
} gpt_channel_t;

static gpt_channel_t gpt_channel_tab[] = {
	[HAL_GPT_0] = {HAL_GPT_0, NULL, HAL_PWM_MAX},
	[HAL_GPT_1] = {HAL_GPT_1, NULL, HAL_PWM_MAX}
};

#define max_gpt_channel			(sizeof(gpt_channel_tab)/sizeof(gpt_channel_t))

static gpt_channel_t *pin_get_gpt_channel(pin_desc_t *pin_desc)
{
	size_t i;

	for (i=0; i< max_gpt_channel; i++) {
		if (gpt_channel_tab[i].pin_desc == pin_desc)
			return &gpt_channel_tab[i];
	}

	return NULL;
}

static void free_gpt_channel(gpt_channel_t *gpt_channel)
{
	hal_gpt_running_status_t	status;

	if (hal_gpt_get_running_status(gpt_channel->gpt_port, &status)
							== HAL_GPT_STATUS_OK) {
		if (status != HAL_GPT_STOPPED) {
			hal_gpt_stop_timer(gpt_channel->gpt_port);
		}
		else{
			pr_debug("GPT Channel is stop.\r\n");
		}	

		hal_gpt_deinit(gpt_channel->gpt_port);
	} else {
		pr_debug("Fail to get GPT Channel state\r\n");
	}

	gpt_channel->pwm_channel= hal_pwm_channel_t(-1);
	gpt_channel->pin_desc	= NULL;

	return ;
}

static void gpt_channel_timer_handler(void *user_data)
{
	gpt_channel_t			*gpt_channel = (gpt_channel_t *)user_data;
	hal_pwm_running_status_t	status;

	if (hal_pwm_get_running_status(gpt_channel->pwm_channel, &status)
							!= HAL_PWM_STATUS_OK)
		return ;

	if (status == HAL_PWM_BUSY)
		hal_pwm_stop(gpt_channel->pwm_channel);

	free_gpt_channel(gpt_channel);

	return ;
}

static gpt_channel_t *alloc_gpt_channel(pin_desc_t *pin_desc)
{
	gpt_channel_t			*gpt_channel;
	int						pwm_channel = -1;
	hal_gpt_running_status_t	status;
	size_t				i;

	if (pin_desc == NULL)
		return NULL;

	pwm_channel = pin_get_pwm_channel(pin_desc);
	if (pwm_channel == -1)
		return NULL;

	for (i=0; i< max_gpt_channel; i++) {
		gpt_channel = &gpt_channel_tab[i];

		// pr_debug("alloc_gpt: GPT Channel->gpt_port: %d\r\n", gpt_channel->gpt_port);
		// pr_debug("alloc_gpt: GPT Channel->pwm_channel: %d\r\n", gpt_channel->pwm_channel);
		// pr_debug("alloc_gpt: GPT Channel->pin_desc: %p\r\n", gpt_channel->pin_desc);

		if (hal_gpt_get_running_status(gpt_channel->gpt_port,
						&status) != HAL_GPT_STATUS_OK) {
			// pr_debug("alloc_gpt: Fail to get GPT Channel state\r\n");
			continue;
		}

		if (status != HAL_GPT_STOPPED) {
			// pr_debug("alloc_gpt: GPT Channel is Running.\r\n");
			continue;
		}

		if (hal_gpt_init(gpt_channel->gpt_port) != HAL_GPT_STATUS_OK) {
			// pr_debug("alloc_gpt: GPT Channel fail to init.\r\n");
			continue;
		}

		/* Register callback */
		if (hal_gpt_register_callback(gpt_channel->gpt_port,
				gpt_channel_timer_handler, gpt_channel)
				!= HAL_GPT_STATUS_OK) {

			// pr_debug("alloc_gpt: GPT Channel fail to register cb.\r\n");
			hal_gpt_deinit(gpt_channel->gpt_port);
			continue;
		}

		gpt_channel->pwm_channel= (hal_pwm_channel_t)pwm_channel;
		gpt_channel->pin_desc	= pin_desc;

		// pr_debug("alloc_gpt: GPT Channel->gpt_port: %d\r\n", gpt_channel->gpt_port);
		// pr_debug("alloc_gpt: GPT Channel->pwm_channel: %d\r\n", gpt_channel->pwm_channel);
		// pr_debug("alloc_gpt: GPT Channel->pin_desc: %p\r\n", gpt_channel->pin_desc);

		return gpt_channel;
	}

	return NULL;
}

#ifdef __cplusplus
}
#endif

void noTone(uint8_t pin)
{
	pin_desc_t			*pin_desc;
	gpt_channel_t			*gpt_channel;
	int		pwm_channel = -1;
	hal_pwm_running_status_t	status;

	// pr_debug("Enter noTone\r\n");
	pin_desc = get_arduino_pin_desc((arduino_pin_t)pin);
	if (pin_desc == NULL)
		return ;

	pwm_channel = pin_get_pwm_channel(pin_desc);
	if (pwm_channel == -1)
		return;

	if (hal_pwm_get_running_status((hal_pwm_channel_t)pwm_channel, &status)
							== HAL_PWM_STATUS_OK) {
		if (HAL_PWM_BUSY == status) {
			hal_pwm_stop((hal_pwm_channel_t)pwm_channel);
		}
	}

	gpt_channel = pin_get_gpt_channel(pin_desc);
	if (NULL != gpt_channel) {
		// pr_debug("noTone: GPT Not NULL\r\n");
		free_gpt_channel(gpt_channel);
	}

	return ;
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration)
{
	pin_desc_t			*pin_desc;
	gpt_channel_t			*gpt_channel;
	int		pwm_channel = -1;
	hal_pwm_running_status_t	status;
	uint32_t			total_count = 0;
	static uint8_t			init_flag = 0;

	if ((frequency < PWM_MIN_FREQUENCY) | (frequency > PWM_MAX_FREQUENCY))
		return ;

	pin_desc = get_arduino_pin_desc((arduino_pin_t)pin);
	if (pin_desc == NULL)
		return ;

	pwm_channel = pin_get_pwm_channel(pin_desc);
	if (pwm_channel == -1)
		return;

	// Step 1: Clear the previous settings
	if (hal_pwm_get_running_status((hal_pwm_channel_t)pwm_channel, &status)
							== HAL_PWM_STATUS_OK) {
		if (HAL_PWM_BUSY == status) {
			hal_pwm_stop((hal_pwm_channel_t)pwm_channel);
		}
	} else
		return ;

	gpt_channel = pin_get_gpt_channel(pin_desc);
	if (NULL != gpt_channel) {
		pr_debug("tone: GPT Not NULL\r\n");
		free_gpt_channel(gpt_channel);
	}

	// Step 2: Prepare the PWM Settings
	// Precondition:
	// 1. PWM Should be stopped.
	if (!pin_enable_pwm(pin_desc))
	{
		pr_debug("pin_enable_pwm fails");
		goto FAIL_PWM;
	}	

	if (!init_flag) {
		hal_pwm_init(PWM_SRC_CLOCK);
			init_flag++;
	}

	if (hal_pwm_set_frequency((hal_pwm_channel_t)pwm_channel, frequency, &total_count)
							!= HAL_PWM_STATUS_OK) {
		pr_debug("tone: Set frequency Fail\r\n");
		goto FAIL_PWM;
	}

	pr_debug("tone: Set frequency: %d, Total_count: %lu\r\n", frequency, total_count);
	if (hal_pwm_set_duty_cycle((hal_pwm_channel_t)pwm_channel, total_count/2)
							!= HAL_PWM_STATUS_OK) {
		pr_debug("tone: Set Duty Cycle Fail\r\n");
		goto FAIL_PWM;
	}

	// Step 3: Prepare the Timer Settings if the duration != 0
	if (duration != 0) {
		gpt_channel = alloc_gpt_channel(pin_desc);

		if (NULL == gpt_channel) {
			pr_debug("tone: Alloc GPT Fail\r\n");
			goto FAIL_PWM;
		}

		if (hal_gpt_start_timer_ms(gpt_channel->gpt_port,
				duration, HAL_GPT_TIMER_TYPE_ONE_SHOT)
							!= HAL_GPT_STATUS_OK) {
			pr_debug("tone: Start Timer Fail\r\n");
			goto FAIL_GPT;
		}
	}

	if (HAL_PWM_STATUS_OK != hal_pwm_start((hal_pwm_channel_t)pwm_channel)) {
		pr_debug("tone: Start PWM Fail\r\n");
		goto FAIL_GPT;
	}

	return ;

FAIL_GPT:
	free_gpt_channel(gpt_channel);
FAIL_PWM:

	pr_debug("tone: Error Tone\r\n");
	return;
}
