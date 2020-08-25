/*
 * ADXL345 control via SPI.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "wrappers.h"
#include "get_dht11_data.h"


#define DHT11_POWER 2
#define DHT11_DATA  4

void app_main () {
	while(1) {
		char *temperature = get_dht11_data(DHT11_POWER, DHT11_DATA, 0);
		char *humidity    = get_dht11_data(DHT11_POWER, DHT11_DATA, 1);
		ets_printf("temperature = %s\n", temperature);
		ets_printf("humidity    = %s\n", humidity);

		
		free(temperature);
		free(humidity);
		vTaskDelay(1);
	}
}	
