#include <stdio.h>
#include "driver/gpio.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "esp_log.h"
#include "wrappers.h"
#include <string.h>
#include "general.h"

int get_dht11_data(int dht11_power_pin, int dht11_data_pin, int mode);
