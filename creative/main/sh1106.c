#include "driver/i2c.h"
#include "sh1106.h"
#include <unistd.h>
#include <stdlib.h>
#include "font6x8.h"

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


void oled_print(char *str) {
    sh1106_t display;
    init_display(&display);

    sh1106_t *display1 = &display;
    display_print(&display1, str);
    sh1106_update(&display);
}


/*
 * Initialize display structure 
 * and fills all pixels with 0 value.
 */
void init_display(sh1106_t *display) {
    display->addr = SH1106_DEFAULT_ADDR;
    display->port = SH1106_DEFAULT_PORT;
    sh1106_init(display);
    for (uint8_t y = 0; y < 64; y++) {
        for (uint8_t x = 0; x < 132; x++) {
            sh1106_set(display, x, y, 0);
        }
    }
}



void sh1106_init(sh1106_t *display) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // инициализация сссылки на комманды
    i2c_master_start(cmd); // стартовый бит

    /*
     * Бит мода(чтение/запись) является частью адресса.
     * slave adderess - представляет собой 7-битное число.
     * 00111100 - адресс.
     * 00000000 - I2C_MASTER_WRITE
     * (display->addr << 1) => 01111000
     * 01111000 | 00000000 => 0111100(0) - бит для мода (чтение/запись)
     */
    i2c_master_write_byte(cmd, (display->addr << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, 0x00, true); // command stream
    i2c_master_write_byte(cmd, 0xAE, true); // off
    i2c_master_write_byte(cmd, 0xD5, true); // clock div
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_write_byte(cmd, 0xA8, true); // multiplex
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0x8D, true); // charge pump
    i2c_master_write_byte(cmd, 0x14, true);
    i2c_master_write_byte(cmd, 0x10, true); // high column
    i2c_master_write_byte(cmd, 0xB0, true);
    i2c_master_write_byte(cmd, 0xC8, true);
    i2c_master_write_byte(cmd, 0x00, true); // low column
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_write_byte(cmd, 0x40, true);
    i2c_master_write_byte(cmd, 0xA1, true); // segment remap
    i2c_master_write_byte(cmd, 0xA6, true);
    i2c_master_write_byte(cmd, 0x81, true); //  Настройка контраста.
    i2c_master_write_byte(cmd, 0x96, true); //  | контраст = от 00 до FF
    i2c_master_write_byte(cmd, 0xAF, true); // on

    
    i2c_master_stop(cmd); // стоп бит
    i2c_master_cmd_begin(display->port, cmd, 10/portTICK_PERIOD_MS); // выполнение коммандной ссылки
    i2c_cmd_link_delete(cmd); // удаление коммандной ссылки
}

void sh1106_write_page(sh1106_t *display, uint8_t page) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (display->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x80, true); // single command
    i2c_master_write_byte(cmd, 0xB0 + page, true);
    i2c_master_write_byte(cmd, 0x40, true); // data stream
    i2c_master_write(cmd, display->pages[page], 128, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(display->port, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void sh1106_update(sh1106_t *display) {
    for (uint8_t i = 0; i < 8; i++) {
        if (display->changes & (1 << i)) {
            sh1106_write_page(display, i);
        }
    }
    display->changes = 0x0000;
}

void sh1106_clear(sh1106_t *display) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 128; j++) {
            display->pages[i][j] = 0x00;
        }
    }
    display->changes = 0xffff;
}

void sh1106_fill(sh1106_t *display) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 128; j++) {
            display->pages[i][j] = 0xff;
        }
    }
    display->changes = 0xffff;
}

void sh1106_set(sh1106_t *display, uint8_t x, uint8_t y, bool s) {
    const uint8_t i = y / 8;
    if (s) {
        display->pages[i][x] |= (1 << (y % 8));
    } else {
        display->pages[i][x] &= ~(1 << (y % 8));
    }
    display->changes |= (1 << i);
}


bool sh1106_get(sh1106_t *display, uint8_t x, uint8_t y) {
    return display->pages[y / 8][x] & (1 << (y % 8));
}