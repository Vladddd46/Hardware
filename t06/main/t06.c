#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <strings.h>
#include <string.h>
#include "driver/dac.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define GPIO_NUM_5   5
#define GPIO_NUM_25  25

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
    if (dac_output_enable(DAC_CHANNEL_1) != ESP_OK) ESP_LOGI("dac_output_enable ", "%s", "some error occured.");
    gpio_set_direction_wrapper(GPIO_NUM_5, GPIO_MODE_INPUT);
    gpio_set_level_wrapper(GPIO_NUM_5, 1);

     while(1) {
        int v;
        for(v = 0; v < 255; v+= 30) {
          if (dac_output_voltage(DAC_CHANNEL_1, v) != ESP_OK) 
              ESP_LOGI("dac_output_voltage ", "%s", "some error occured.");
          ets_delay_us(5000);
        }
    }
}