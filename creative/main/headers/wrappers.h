#include <stdio.h>
#include "driver/gpio.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "esp_log.h"
#include "driver/dac.h"

void gpio_set_direction_wrapper(int gpio, int mode);
void gpio_set_level_wrapper(int gpio, int level);
void gpio_set(int gpio, int mode, int level);
void dac_output_enable_wrapper(int dac_channel);
