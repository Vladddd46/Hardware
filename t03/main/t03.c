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
#define LCD_3   33 // just for testing goals. (used in task_led3_blink() function)

// LED1
static int pressed_status_1 = 0;
static bool flash_status_1 = false;
// LED2
static int pressed_status_2 = 0;
static bool flash_status_2 = false;




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
                gpio_set_level(LCD_1, 1);
            }
            else if(pressed_status_1 == 0) {
                flash_status_1 = false;
                gpio_set_level(LCD_1, 0);
            }
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}



void sw2_task(void *param) {
    while(1) {
        if(gpio_get_level(SW2) == 1) {
            if(pressed_status_2 < 5 && flash_status_2 == false) {
                pressed_status_2++;
            }
            else if(pressed_status_2 > 0 && flash_status_2 == true) {
                pressed_status_2--;
            }
        }
        else {
            if(pressed_status_2 >= 5) {
                flash_status_2 = true;
                gpio_set_level(LCD_2, 1);
            }
            else if(pressed_status_2 == 0) {
                flash_status_2 = false;
                gpio_set_level(LCD_2, 0);
            }
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}


// // testing function. Makes led3 blink.
// void task_led3_blink(void *param) {
//     int error = 0;
//     // timer configuration.
//     ledc_timer_config_t ledc_timer;
//     ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
//     ledc_timer.freq_hz         = 100;
//     ledc_timer.duty_resolution = LEDC_TIMER_8_BIT; // 256
//     ledc_timer.timer_num       = LEDC_TIMER_1;
//     error = ledc_timer_config(&ledc_timer);
//     if (error != ESP_OK) ESP_LOGI("line 37 ", "%s", "some error occured");

//     // chanel configuration.
//     ledc_channel_config_t ledc_channel;
//     ledc_channel.gpio_num   = LCD_3;
//     ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
//     ledc_channel.channel    = LEDC_CHANNEL_1;
//     ledc_channel.intr_type  = LEDC_INTR_FADE_END;
//     ledc_channel.timer_sel  = LEDC_TIMER_1;
//     ledc_channel.duty     = 0;

//     error = ledc_channel_config(&ledc_channel);
//     if (error != ESP_OK) ESP_LOGI("line 49 ", "%s", "some error occured");
//     error = ledc_fade_func_install(0);
//     if (error != ESP_OK) ESP_LOGI("line 51 ", "%s", "some error occured");
//     while(1) {
//         // ascending.
//         ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 255, 1000);
//         ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
//         // descending.
//         ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 0, 1000);
//         ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
//         vTaskDelay(1);
//     }
//     vTaskDelete(NULL);
// }




void app_main() {
    if (gpio_set_direction(LCD_1, GPIO_MODE_OUTPUT) != ESP_OK) ESP_LOGI("line 116 ", "%s", "some error occured");
    if (gpio_set_direction(LCD_2, GPIO_MODE_OUTPUT) != ESP_OK) ESP_LOGI("line 117 ", "%s", "some error occured");
    // if (gpio_set_direction(LCD_3, GPIO_MODE_OUTPUT) != ESP_OK) ESP_LOGI("line 118 ", "%s", "some error occured"); // for test goals
    if (gpio_set_direction(SW1,   GPIO_MODE_INPUT) != ESP_OK) ESP_LOGI("line 119 ", "%s", "some error occured");
    if (gpio_set_direction(SW2,   GPIO_MODE_INPUT) != ESP_OK) ESP_LOGI("line 120 ", "%s", "some error occured");

    xTaskCreate(sw1_task, "led1", 2048, NULL, 5, NULL);
    xTaskCreate(sw2_task, "led2", 2048, NULL, 5, NULL);
    // xTaskCreate(task_led3_blink, "led3", 2048, NULL, 5, NULL); // just for testing goals.
}








