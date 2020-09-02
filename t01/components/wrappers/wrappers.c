#include "wrappers.h"


void gpio_set_direction_wrapper(int gpio, int mode) {
	if (gpio_set_direction(gpio, mode) != ESP_OK) {
		ESP_LOGI("gpio_set_direction: ", "%s", "some error occured.");
		exit(1);
	}
}




void gpio_set_level_wrapper(int gpio, int level) {
	if (gpio_set_level(gpio, level) != ESP_OK) {
		ESP_LOGI("gpio_set_level ", "%s", "some error occured.");
		exit(1);
	}
}
