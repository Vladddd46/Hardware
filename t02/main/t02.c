#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "driver/dac.h"
#include "driver/ledc.h"
#include <pthread.h>
#include "esp_log.h"

void task_led_on_dac(void *param) {
    while(1) {
        int v;
        for(v = 0; v < 255; v++) {
          dac_output_voltage(DAC_CHANNEL_2, v);
          ets_delay_us(5000);
        }     
        for(v = 255; v > 0; v--) {
          dac_output_voltage(DAC_CHANNEL_2, v);
          ets_delay_us(5000);
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}

void task_led_on_pwm(void *param) {
    // timer configuration.
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
    ledc_timer.freq_hz         = 100;
    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT; // 256
    ledc_timer.timer_num       = LEDC_TIMER_1;
    if(ledc_timer_config(&ledc_timer) != ESP_OK) 
        ESP_LOGI("ledc_timer_config ", "%s", "some error occured");

    // chanel configuration.
    ledc_channel_config_t ledc_channel;
    ledc_channel.gpio_num   = 27;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.channel    = LEDC_CHANNEL_1;
    ledc_channel.intr_type  = LEDC_INTR_FADE_END;
    ledc_channel.timer_sel  = LEDC_TIMER_1;
    ledc_channel.duty     = 0;
    if (ledc_channel_config(&ledc_channel) != ESP_OK) 
        ESP_LOGI("ledc_channel_config ", "%s", "some error occured");
    if (ledc_fade_func_install(0) != ESP_OK) 
        ESP_LOGI("ledc_fade_func_install ", "%s", "some error occured");

    while(1) {
        // ascending.
        ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 255, 1000);
        ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
        // descending.
        ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 0, 1000);
        ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}


void app_main() {
    dac_output_enable(DAC_CHANNEL_2);

    xTaskCreate(task_led_on_dac, "led_on_dac", 2048, NULL, 5, NULL);
    xTaskCreate(task_led_on_pwm, "led_on_pwm", 2048, NULL, 5, NULL);
}
