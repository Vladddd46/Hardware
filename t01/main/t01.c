/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <driver/gpio.h>

#define GPIO_LED1 27
#define GPIO_LED2 26
#define GPIO_LED3 33

void app_main() {
  gpio_set_direction(GPIO_LED1, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_LED2, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_LED3, GPIO_MODE_OUTPUT);

  while(true) {
    gpio_set_level(GPIO_LED1, 1);
    gpio_set_level(GPIO_LED2, 1);
    gpio_set_level(GPIO_LED3, 1);
  }
}