#include "driver/gpio.h"
#include "driver/i2c.h"
#include "sh1106.h"
#include "wrappers.h"
#include <string.h>

#define SDA_PIN  21
#define SCL_PIN  22



// Initialize I2C driver.
void init_i2c_driver() {
    i2c_config_t i2c_config = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = SDA_PIN,
        .scl_io_num       = SCL_PIN,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000
    };
    i2c_param_config(SH1106_DEFAULT_PORT,   &i2c_config);
    i2c_driver_install(SH1106_DEFAULT_PORT, I2C_MODE_MASTER, 0, 0, 0);
}



void app_main() {
    sh1106_t display;

    gpio_set_direction_wrapper(32, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(32, 1);

    init_i2c_driver();
    init_display(&display);
    
    sh1106_t *display1 = &display;
    display_print(&display1, "Hello World!");
    sh1106_update(&display);
}
