#include "driver/gpio.h"
#include "driver/i2c.h"
#include "sh1106.h"
#include "wrappers.h"
#include "font6x8.h"
#include <string.h>


#define SDA_PIN  21
#define SCL_PIN  22
#define I2C_ADDR SH1106_DEFAULT_ADDR
#define I2C_PORT SH1106_DEFAULT_PORT

/*
 * Prints character on specified page and position.
 * gage must be >= 0 && <= 7
 * position must be >= 0 && <= 126
 */
void print_char(sh1106_t **display, char c, int page, int position) {
    // args validation.
    if (page < 0 || page > 7 || position < 0 || position > 126) 
        return;

    // index of line, which represent character 'c' in font6x8 array
    int index_in_font = (c - 32) * 6;

    sh1106_t *display1 = *display;
    display1->pages[page][position + 0] = font6x8[index_in_font];
    display1->pages[page][position + 1] = font6x8[index_in_font + 1];
    display1->pages[page][position + 2] = font6x8[index_in_font + 2];
    display1->pages[page][position + 3] = font6x8[index_in_font + 3];
    display1->pages[page][position + 4] = font6x8[index_in_font + 4];
    display1->pages[page][position + 5] = font6x8[index_in_font + 5];
}



/*
 * Prints str. in a specified line(page).
 * Line(page) must be >= 0 &&  <= 7 
 * line over 22char will be cut.
 */
void print_str_in_line(sh1106_t **display, char *str, int page) {
    if (page < 0 || page > 7)
        return;

    int pos = 0;
    for (int i = 0; str[i]; ++i) {
        print_char(display, str[i], 0, pos);
        pos += 6;

        if (pos >= 126)
            break;
    }
}



/*
 * Prints string on display. Starts printing from top left corver(0,0)
 * If string is larger, than 176 chars, it will. be cut.
 */
void display_print(sh1106_t **display, char *str) {
    int line = 0;
    int pos  = 0;

    for (int i = 0; str[i]; ++i) {
        print_char(display, str[i], line, pos);
        
        pos += 6;
        if (pos >= 126) {
            line += 1;
            pos = 0;
        }

        if (line >= 7)
            break;
    }
}



/*
 * Initialize display structure 
 * and fills all pixels with 0 value.
 */
void init_display(sh1106_t *display) {
    display->addr = I2C_ADDR;
    display->port = I2C_PORT;
    sh1106_init(display);
    for (uint8_t y = 0; y < 64; y++) {
        for (uint8_t x = 0; x < 132; x++) {
            sh1106_set(display, x, y, 0);
        }
    }
}


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
    i2c_param_config(I2C_PORT,   &i2c_config);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
}

void app_main() {
    sh1106_t display;

    gpio_set_direction_wrapper(32, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(32, 1);

    init_i2c_driver();
    init_display(&display);
    
    sh1106_t *display1 = &display;
    display_print(&display1, "Spasiba Yura!!!");
    sh1106_update(&display);
}
