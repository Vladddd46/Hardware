#include "get_dht11_data.h"

/*
 * Gets temperature and humidity value from
 * sensor DHT11.
 * Arguments:
 *  int dht11_power_pin
 *  int dht11_data_pin
 *  int mode  - 0 to get temperature/ 1 to get humidity.
 * In case of any error, returns NULL. 
 */



static int wait_status(int time, _Bool status, int dht11_data_pin) {
    int count = 0;
    while (gpio_get_level(dht11_data_pin) == status){
        if (count > time)
            return -1;
        ets_delay_us(1);
        count++;
    }
    if (count == 0)
        return -1;
    return count;
}



// Makes DHT11 send data.
static int preparing_for_receiving_data(int gpio) {
    // turning on data pin.
    gpio_set_direction_wrapper(gpio,  GPIO_MODE_OUTPUT); // Data pin.
    gpio_set_level_wrapper(gpio,  1);
    ets_delay_us(1500 * 1000);

    // turning off pin to the ground for 18 miliseconds.
    gpio_set_level_wrapper(gpio, 0);
    ets_delay_us(18000);
    // turning on pin to the ground for 30 microseconds.
    gpio_set_level_wrapper(gpio, 1);
    ets_delay_us(30);
    gpio_set_direction_wrapper(gpio, GPIO_MODE_INPUT);

    // Responses from server that confirm sensor is ready to send data.
    if (wait_status(80, 0, gpio) == -1) {
        printf("%s\n", "Wrong response from server: must have being returning 0 during 80 microseconds");
        return -1;
        }
    if (wait_status(80, 1, gpio) == -1) {
        printf("%s\n", "Wrong response from server: must have being returning 1 during 80 microseconds");
        return -1;
    }
    return 1;
}



int get_dht11_data(int dht11_power_pin, int dht11_data_pin, int mode) {
    gpio_set_direction_wrapper(dht11_power_pin, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(dht11_power_pin, 1);

    int res = 0;
    uint8_t data[5];
    bzero(&data, sizeof(data));

    if (preparing_for_receiving_data(dht11_data_pin) == -1) 
        return -1;

    // Getting data.
    for (int i = 1, j = 0; i < 41; i++) {
        if ((res = wait_status(50, 0, dht11_data_pin)) == -1) {
            printf("%s\n", "Error during sending data.");
             break;
        }
        if ((res = wait_status(70, 1, dht11_data_pin)) == -1) {
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

    // Checking checksum
    if (data[0] + data[1] + data[2] + data[3] != data[4]) {
        printf("%s\n", "Invalid checksum");
        return -1;
    }

    // data cast.
    // char temperature[10];
    // bzero(temperature, 10);
    // char humidity[10];
    // bzero(humidity, 10);
    // itoa(data[2], temperature, 10);
    // itoa(data[0], humidity, 10);
        
    if (mode == 0)
        return data[2];
    else if (mode == 1)
        return data[0];
    else 
        return -1;
}

