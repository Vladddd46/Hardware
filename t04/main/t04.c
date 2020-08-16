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

#define DHT11_POWER 2
#define DHT11_DATA  4


int wait_status(int time, _Bool status) {
    int count = 0;
    while (gpio_get_level(DHT11_DATA) == status){
        if (count > time)
            return -1;
        ets_delay_us(1);
        count++;
    }
    if (count == 0)
        return -1;
    return count;
}


void communicate(void *param) {
    int res = 0;
    uint8_t data[5];

    while(1) {
    bzero(&data, sizeof(data));

        // turning on data pin.
        gpio_set_direction(DHT11_DATA,  GPIO_MODE_OUTPUT); // Data pin.
        gpio_set_level(DHT11_DATA,  1);
        ets_delay_us(1500 * 1000);

        // turning off pin to the ground for 18 miliseconds.
        gpio_set_level(DHT11_DATA, 0);
        ets_delay_us(18000);
        // turning on pin to the ground for 30 microseconds.
        gpio_set_level(DHT11_DATA, 1);
        ets_delay_us(30);
        gpio_set_direction(DHT11_DATA, GPIO_MODE_INPUT);

        // Responses from server that confirm sensor is ready to send data.
        if (wait_status(80, 0) == -1) {
            printf("%s\n", "Wrong response from server: must have being returning 0 during 80 microseconds");
            continue;
        }
        if (wait_status(80, 1) == -1) {
            printf("%s\n", "Wrong response from server: must have being returning 1 during 80 microseconds");
            continue;
        }

        // Getting data.
        for (int i = 1, j = 0; i < 41; i++) {
            if ((res = wait_status(50, 0)) == -1) {
                printf("%s\n", "Error during sending data.");
                break;
            }
            if ((res = wait_status(70, 1)) == -1) {
                printf("%s\n", "Error during sending data.");
                break;
            }
            if (res > 28) {
                data[j] <<= 1;
                data[j] += 1;
            }
            else
                data[j] <<= 1;

            if (i % 8 == 0)
                j++;
        }

        if (data[0] + data[1] + data[2] + data[3] != data[4]) {
            printf("%s\n", "Invalid checksum");
            continue;
        }
        printf("Temperature: %d C\n", data[2]);
        printf("Humidity: %d %%\n", data[0]);
        vTaskDelay(5);
    }
    vTaskDelete(NULL);
}


void app_main() {
    gpio_set_direction(DHT11_POWER, GPIO_MODE_OUTPUT); // VCC power.
    gpio_set_level(DHT11_POWER, 1);                    // switch on power.

    xTaskCreate(communicate, "communicate", 2048, NULL, 5, NULL);
}


