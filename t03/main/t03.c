/*
 * Control LED_1 and LED_2, using 
 * button1 and button2.
 */
#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <stdbool.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/dac.h"
#include "esp_log.h"

#define SW1     39
#define SW2     18
#define LCD_1   26
#define LCD_2   27

// LED1
static int  pressed_status_1 = 0;
static bool flash_status_1 = false;
// LED2
static int  pressed_status_2 = 0;
static bool flash_status_2 = false;

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


void sw1_task(void *param) {
    while(1) {
        if (gpio_get_level(SW1) == 1) {
            if(pressed_status_1 < 5 && flash_status_1 == false)
                pressed_status_1++;
            else if(pressed_status_1 > 0 && flash_status_1 == true)
                pressed_status_1--;
        }
        else {
            if(pressed_status_1 >= 5) {
                flash_status_1 = true;
                gpio_set_level_wrapper(LCD_1, 1);
            }
            else if(pressed_status_1 == 0) {
                flash_status_1 = false;
                gpio_set_level_wrapper(LCD_1, 0);
            }
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}



void sw2_task(void *param) {
    while(1) {
        if(gpio_get_level(SW2) == 1) {
            if(pressed_status_2 < 5 && flash_status_2 == false)
                pressed_status_2++;
            else if(pressed_status_2 > 0 && flash_status_2 == true)
                pressed_status_2--;
        }
        else {
            if(pressed_status_2 >= 5) {
                flash_status_2 = true;
                gpio_set_level_wrapper(LCD_2, 1);
            }
            else if(pressed_status_2 == 0) {
                flash_status_2 = false;
                gpio_set_level_wrapper(LCD_2, 0);
            }
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}



void app_main() {
    gpio_set_direction_wrapper(LCD_1, GPIO_MODE_OUTPUT);
    gpio_set_direction_wrapper(LCD_2, GPIO_MODE_OUTPUT);
    gpio_set_direction_wrapper(SW1,   GPIO_MODE_INPUT);
    gpio_set_direction_wrapper(SW2,   GPIO_MODE_INPUT);

    xTaskCreate(sw1_task, "led1", 2048, NULL, 5, NULL);
    xTaskCreate(sw2_task, "led2", 2048, NULL, 5, NULL);
}
