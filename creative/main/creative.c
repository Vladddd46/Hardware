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



static int screen_num = 0;

static void switch_handler(void* arg) {
    uint32_t io_num;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == GPIO_BUTTON1) {
                if (screen_num == 0)
                    screen_num += 1;
            }
            else if (io_num == GPIO_BUTTON2) {
                if (screen_num == 1)
                    screen_num -= 1;
            }
        }
    }
}

static void checker(void *s) {
    sh1106_t display;
    sh1106_init(&display);
    sh1106_clear(&display);

    sh1106_t *display1 = &display;
    // printf("%d\n", screen_num);
    while(1) {


        printf("%d\n", screen_num);
        if (screen_num == 0) {
            sh1106_clear(&display);
            char *x = get_dht11_data(2, 4, 1);
            char res[50];
            bzero(res, 50);
            sprintf(res, "   humidity: %s %%", x);
            print_str_in_line(&display1, res, 3);
        }
        else if (screen_num == 1) {
            sh1106_clear(&display);
            char *x = get_dht11_data(2, 4, 0);
            char res[50];
            bzero(res, 50);
            sprintf(res, "   temperature: %s C", x);
            print_str_in_line(&display1, res, 3);
        }
        sh1106_update(&display);
        vTaskDelay(1);
    }
}

void app_main(void) {
    // Oled enable
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

    xTaskCreate(switch_handler, "switch_handler", 4048, NULL, 10, NULL);
    xTaskCreate(checker, "checker", 4048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_BUTTON1, gpio_isr_handler, (void*) GPIO_BUTTON1);
    gpio_isr_handler_add(GPIO_BUTTON2, gpio_isr_handler, (void*) GPIO_BUTTON2);
}