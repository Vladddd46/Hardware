#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "get_dht11_data.h"
#include "general.h"
#include "wrappers.h"
#include "sh1106.h"
// #include "font6x8.h"

#define GPIO_LED1 27
#define GPIO_LED2 26

#define GPIO_BUTTON1 39
#define GPIO_BUTTON2 18
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_LED1) | (1ULL<<GPIO_LED2))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_BUTTON1) | (1ULL<<GPIO_BUTTON2))
#define ESP_INTR_FLAG_DEFAULT 0

#define left  0
#define right 1

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;

    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}



int screen_num = 0;
int refresh    = 0;

// Initialize I2C driver.
void init_i2c_driver() {
    i2c_config_t i2c_config = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = 21, //SDA_PIN,
        .scl_io_num       = 22, //SCL_PIN,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000
    };
    i2c_param_config(SH1106_DEFAULT_PORT,   &i2c_config);
    i2c_driver_install(SH1106_DEFAULT_PORT, I2C_MODE_MASTER, 0, 0, 0);
}


static void switch_screen(int direction) {
    sh1106_t display;
    init_display(&display);
    sh1106_t *display1 = &display;

    if (direction == left) {
        if (screen_num > -1)
            screen_num -= 1;
        if (screen_num == 0) {
            display_print(&display1, "screen 0!");
            sh1106_update(&display);
        }
        if (screen_num == -1) {
            display_print(&display1, "screen -1!");
            sh1106_update(&display);
        }
    }

    if (direction == right) {
        if (screen_num < 1)
        screen_num += 1;

        if (screen_num == 0) {
            display_print(&display1, "screen 0!");
            sh1106_update(&display);
        }
        if (screen_num == 1) {
            display_print(&display1, "screen 1!");
            sh1106_update(&display);
            // oled_print("screen 1");
        }
    }
}


static void gpio_task_example(void* arg) {
    uint32_t io_num;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == GPIO_BUTTON1)
                switch_screen(right);
            else if (io_num == GPIO_BUTTON2)
                switch_screen(left);
        }
    }
}

void app_main(void) {
    gpio_set_direction_wrapper(32, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(32, 1);
    init_i2c_driver();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_BUTTON1, gpio_isr_handler, (void*) GPIO_BUTTON1);
    gpio_isr_handler_add(GPIO_BUTTON2, gpio_isr_handler, (void*) GPIO_BUTTON2);
}