
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <strings.h>
#include <string.h>
#include "driver/dac.h"
#include "driver/ledc.h"

#define GPIO_NUM_5   5
#define GPIO_NUM_25  25

void app_main() {
    dac_output_enable(DAC_CHANNEL_1);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
    gpio_set_level(GPIO_NUM_5, 1);

     while(1) {
        int v;
        for(v = 0; v < 255; v+= 30) {
          dac_output_voltage(DAC_CHANNEL_1, v);
          ets_delay_us(5000);
        }
    }
}