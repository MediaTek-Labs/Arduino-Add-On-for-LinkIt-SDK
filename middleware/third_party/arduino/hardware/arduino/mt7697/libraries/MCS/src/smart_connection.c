#ifdef MTK_SMARTCONNECT_HDK
#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include "os.h"

static int startSmart = 0;

void smart_connection_init () {
    hal_pinmux_set_function(HAL_GPIO_0, 8);
    hal_gpio_data_t data_pull_up;
    hal_gpio_data_t data_pull_down;
    hal_gpio_status_t ret;
    hal_pinmux_status_t ret_pinmux_status;

    ret = hal_gpio_init(HAL_GPIO_0);

    /* set pin to work in GPIO mode.*/
    ret_pinmux_status = hal_pinmux_set_function(HAL_GPIO_0, 8);

    /* set direction of GPIO is input.*/
    ret = hal_gpio_set_direction(HAL_GPIO_0, HAL_GPIO_DIRECTION_INPUT);

    /* configure the pull state to pull-up.*/
    ret = hal_gpio_pull_up(HAL_GPIO_0);

    /* get input data of the pin for further validation.*/
    ret = hal_gpio_get_input(HAL_GPIO_0, &data_pull_up);

    if (1 == data_pull_up && 0 == startSmart) {
        startSmart = 1;
        char param[0] = "connect\0";
        _smart_config_test(1, param);
    }

    /* configure the pull state to pull-down.*/
    ret = hal_gpio_pull_down(HAL_GPIO_0);

    /* get input data of the pin for further validation.*/
    ret = hal_gpio_get_input(HAL_GPIO_0, &data_pull_down);

    // printf("down: %d\n", data_pull_down);
    // printf("up: %d\n", data_pull_up);
    ret = hal_gpio_deinit(HAL_GPIO_0);
}

void smart_connection_task(void *parameter) {
  for (;;) {
    smart_connection_init();
    vTaskDelay(200);
  }
}

void smart_config_if_enabled()
{
    xTaskCreate(smart_connection_task, "smartConnectionConnect", 2048, NULL, 2, NULL);
}

#else

void smart_config_if_enabled()
{

}

#endif
