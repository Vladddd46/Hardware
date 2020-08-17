
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <strings.h>
#include <string.h>

#define GPIO_NUM_17  17
#define GPIO_NUM_16  16
#define BUF_SIZE 1024


void app_main() {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, BUF_SIZE, 0, 0, NULL, 0);

    char *msg = "\e[41mRED\e[0m \e[42mGREEN\e[0m \e[44mBLUE\e[0m \e[0mDEFAULT\r\n";
    uart_write_bytes(UART_NUM_1, msg, strlen(msg));
}