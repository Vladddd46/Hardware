#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdio.h>
#include "driver/dac.h"
#include "driver/ledc.h"
#include <pthread.h>
void error_exit(int error, char *msg) {
	exit(1);
}

void led2() {
	  int v;
     for(v = 0; v < 255; v++) {
    	dac_output_voltage(DAC_CHANNEL_2, v);
    	ets_delay_us(5000);
    }     
    for(v = 255; v > 0; v--) {
    	dac_output_voltage(DAC_CHANNEL_2, v);
     	ets_delay_us(5000);
 	}
}

void app_main() {
  	int error;

  	ledc_timer_config_t ledc_timer;
  	ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
  	ledc_timer.freq_hz         = 100;
  	ledc_timer.duty_resolution = LEDC_TIMER_8_BIT; // 256
  	ledc_timer.timer_num       = LEDC_TIMER_1;
  	error = ledc_timer_config(&ledc_timer);
  	// if (error != ESP_OK) error_exit(error, "line 28") ;

    ledc_channel_config_t ledc_channel;
  	ledc_channel.gpio_num   = 27;
  	ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  	ledc_channel.channel    = LEDC_CHANNEL_1;
  	ledc_channel.intr_type  = LEDC_INTR_FADE_END;
  	ledc_channel.timer_sel  = LEDC_TIMER_1;
  	ledc_channel.duty 		= 0;
  	error = ledc_channel_config(&ledc_channel);
  	// if (error != ESP_OK) error_exit(error, "line 38");

  	error = ledc_fade_func_install(0);
  	// if (error != ESP_OK) error_exit(error, "line 43");

  	pthread_t thread_id; 


  	dac_output_enable(DAC_CHANNEL_2);
  	while(1) {
  		pthread_create(&thread_id, NULL, led2, NULL); 
  		ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 255, 1000);
     	ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
     	
     	ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 0, 1000);
     	ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_WAIT_DONE);
    	pthread_join(thread_id, NULL); 
    }
}