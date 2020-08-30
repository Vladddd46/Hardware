#include "driver/gpio.h"
#include "driver/i2c.h"
#include "sh1106.h"
#include "wrappers.h"
#include <string.h>

#define OLED_ENABLE 32

void app_main() {
    sh1106_t display;

    gpio_set_direction_wrapper(OLED_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(OLED_ENABLE, 1);

    init_i2c_driver();
    sh1106_init(&display);
    sh1106_clear(&display);

    sh1106_t *display1 = &display;
    print_str_in_line(&display1, "     Hello World!", 3);
    sh1106_update(&display);
}
