/*
 * Switch on led1/2/3.
 */
#include "wrappers.h"

#define GPIO_LED1 27
#define GPIO_LED2 26
#define GPIO_LED3 33


void app_main() {
	gpio_set_direction_wrapper(GPIO_LED1, GPIO_MODE_OUTPUT);
	gpio_set_direction_wrapper(GPIO_LED2, GPIO_MODE_OUTPUT);
	gpio_set_direction_wrapper(GPIO_LED3, GPIO_MODE_OUTPUT);


	while(true) {
		gpio_set_level_wrapper(GPIO_LED1, 1);
		gpio_set_level_wrapper(GPIO_LED2, 1);
		gpio_set_level_wrapper(GPIO_LED3, 1);
		vTaskDelay(1);
	}
}