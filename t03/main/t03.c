#include <stdio.h>
#include "driver/gpio.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "driver/timer.h"

void app_main() {
    const int button_one = 39;
    const int button_two = 18; // Could be 19
    const int led_one = 26;
    const int led_two = 27;
    int pressed_status_one = false;
    bool flash_status_one = false;
    int pressed_status_two = false;
    bool flash_status_two = false;

    gpio_set_direction(button_one, GPIO_MODE_INPUT);
    gpio_set_direction(button_two, GPIO_MODE_INPUT);
    gpio_set_direction(led_one, GPIO_MODE_OUTPUT);
    gpio_set_direction(led_two, GPIO_MODE_OUTPUT);

    while(true) {
        // First button and led
        if(gpio_get_level(button_one) == 1) {
            if(pressed_status_one < 40 && flash_status_one == false) {
                pressed_status_one++;
            }
            else if(pressed_status_one > 0 && flash_status_one == true) {
                pressed_status_one--;
            }
        }
        else {
            if(pressed_status_one >= 40) {
                flash_status_one = true;
                gpio_set_level(led_one, 1);
            }
            else if(pressed_status_one == 0) {
                flash_status_one = false;
                gpio_set_level(led_one, 0);
            }
        }
        // Second button and led
        if(gpio_get_level(button_two) == 1) {
            if(pressed_status_two < 40 && flash_status_two == false) {
                pressed_status_two++;
            }
            else if(pressed_status_two > 0 && flash_status_two == true) {
                pressed_status_two--;
            }
        }
        else {
            if(pressed_status_two >= 40) {
                flash_status_two = true;
                gpio_set_level(led_two, 1);
            }
            else if(pressed_status_two == 0) {
                flash_status_two = false;
                gpio_set_level(led_two, 0);
            }
        }
    }
}