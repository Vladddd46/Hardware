/*
 * Switch on led1/2/3.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <driver/gpio.h>
#include "esp_log.h"

#define GPIO_LED1 27
#define GPIO_LED2 26
#define GPIO_LED3 33

void gpio_set_direction_wrapper(int gpio, int mode) {
	if (gpio_set_direction(gpio, mode) != ESP_OK) {
		ESP_LOGI("gpio_set_direction: ", "%s", "some error occured.");
		exit(1);
	}
}



void gpio_set_level_wrapper(int gpio, int level) {
	if (gpio_set_level(gpio, level) != ESP_OK)
		ESP_LOGI("gpio_set_level ", "%s", "some error occured.");
}



void app_main() {
	gpio_set_direction_wrapper(GPIO_LED1, GPIO_MODE_OUTPUT);
	gpio_set_direction_wrapper(GPIO_LED2, GPIO_MODE_OUTPUT);
	gpio_set_direction_wrapper(GPIO_LED3, GPIO_MODE_OUTPUT);


	while(true) {
		gpio_set_level_wrapper(GPIO_LED1, 1);
		gpio_set_level_wrapper(GPIO_LED2, 1);
		gpio_set_level_wrapper(GPIO_LED3, 1);
	}
}