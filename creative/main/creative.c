#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <pthread.h>
#include "get_dht11_data.h"
#include "general.h"
#include "wrappers.h"
#include "sh1106.h"
#include "peripherals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/dac.h"

#define GPIO_LED1 27
#define GPIO_LED2 26
#define GPIO_LED3 33

#define GPIO_NUM_5   5
#define GPIO_NUM_25  25

// Power for accelerometr.
#define GPIO_EN_ACCEL 23

// @brief DMA channel is not used.
#define DMA_CHAN  0

#define PIN_NUM_MISO  12
#define PIN_NUM_MOSI  13
#define PIN_NUM_CLK   14
#define PIN_NUM_CS    15


/** @brief ADXL345 register read flag. */
#define ADXL345_REG_READ_FLAG  0x80u
/** @brief ADXL345 register multibyte flag. */
#define ADXL345_REG_MB_FLAG    0x40u
/** @brief ADXL345 register: DATAX0. */
#define ADXL345_REG_DATAX0     0x32u
/** @brief ADXL345 register: BW_RATE. */
#define ADXL345_REG_BW_RATE    0x2Cu
/** @brief ADXL345 register: POWER_CTL. */
#define ADXL345_REG_POWER_CTL  0x2Du

/** @brief ADXL345 POWER_CTL flag: Measure. */
#define ADXL345_POWER_CTL_MEASURE  0x08u

/** @brief ADXL345 delay to update (200ms). */
#define ADXL345_UPDATE_DELAY  (200u / portTICK_PERIOD_MS)

#define GPIO_BUTTON1 39
#define GPIO_BUTTON2 18
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_LED1)    | (1ULL<<GPIO_LED2))
#define GPIO_INPUT_PIN_SEL   ((1ULL<<GPIO_BUTTON1) | (1ULL<<GPIO_BUTTON2))
#define ESP_INTR_FLAG_DEFAULT 0

#define OLED_ENABLE 32

static xQueueHandle gpio_evt_queue = NULL;
static pthread_mutex_t mutex;
static int screen_num  = 0;
static int temperature = 0;
static int humidity    = 0;
static int reverse     = 0;
static int refresh     = 0;
/*
 * ISR, which handles button press.
 * Sets global variable "screen_num" to
 * the corresponding screen.
 */
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;

    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void switch_button_handler(void* arg) {
    uint32_t io_num;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == GPIO_BUTTON1) {
                if (screen_num == 0)
                    screen_num += 1;
                make_beep();
            }
            else if (io_num == GPIO_BUTTON2) {
                if (screen_num == 1)
                    screen_num -= 1;
                make_beep();
            }
        }
    }
}



/*
 * Gets data from dht11 depending on 
 * the current screen state.
 * Places result in 
 * global vars. temperature/humidity. 
 */
static void dht11_data_checker() {
    int res;

    while(1) {
        if (screen_num == 0) {
            res = get_dht11_data(2, 4, 1);
            if (res != -1) humidity    = res;
        }
        else if (screen_num == 1) {
            res = get_dht11_data(2, 4, 0);
            if (res != -1) temperature = res;
        }
        vTaskDelay(100);
    }
}



/*
 * Initialize temperature and humidity values.
 */
static void dht11_data_init() {
    int counter = 0;
    int res;

    while(counter < 5) {
        res = get_dht11_data(2, 4, 0);
        if (res == -1) {
            counter++;
            continue;
        }
        temperature = res;

        res = get_dht11_data(2, 4, 1);
        if (res == -1) {
            counter++;
            continue;
        }
        humidity = res;
        return;
    }
    printf("Error while dht11_init\n");
}

void sh1106_init2(sh1106_t *display) {
    // Filling display structure with 
    display->addr = SH1106_DEFAULT_ADDR;
    display->port = SH1106_DEFAULT_PORT;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // command link initialize.
    i2c_master_start(cmd); // start bit.

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
    i2c_master_write_byte(cmd, 0xAE, true); // turn display off
    i2c_master_write_byte(cmd, 0xD5, true); // clock div
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_write_byte(cmd, 0xA8, true); // multiplex
    i2c_master_write_byte(cmd, 0x3F, true);
    i2c_master_write_byte(cmd, 0x8D, true); // charge pump
    i2c_master_write_byte(cmd, 0x14, true);
    i2c_master_write_byte(cmd, 0x10, true); // high column
    i2c_master_write_byte(cmd, 0xB0, true);
    // i2c_master_write_byte(cmd, 0xC8, true);
    i2c_master_write_byte(cmd, 0x00, true); // low column
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_write_byte(cmd, 0x40, true);
    // i2c_master_write_byte(cmd, 0xA1, true); // segment remap

    i2c_master_write_byte(cmd, 0xC0, true); // need delete
    i2c_master_write_byte(cmd, 0xA0, true); // need delete
    i2c_master_write_byte(cmd, 0xA7, true); // normal/reverse display (A6 = светится когда 1 и A7 = наоборот )
    i2c_master_write_byte(cmd, 0x81, true); // Настройка контраста.
    i2c_master_write_byte(cmd, 0xFF, true); //  | контраст = от 00 до FF
    i2c_master_write_byte(cmd, 0xAF, true); // on

    i2c_master_stop(cmd); // stop bit.
    i2c_master_cmd_begin(display->port, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}


/*
 * Draws screen page depending on screen_num.
 */
static void drawer(void) {
    sh1106_t display;
    sh1106_init(&display);
    sh1106_clear(&display);
    sh1106_t *display1 = &display;

    char res[50];
    while(1) {
        pthread_mutex_lock(&mutex);
        bzero(res, 50);
        sh1106_clear(&display);

        if (reverse == 1 && refresh) {
            sh1106_init2(&display);
            refresh = 0;
        }
        else if (reverse == 0 && refresh) {
            sh1106_init(&display);
            refresh = 0;
        }

        if (screen_num == 0)
            sprintf(res, "    humidity: %d %%", humidity);
        else if (screen_num == 1)
            sprintf(res, "   temperature: %d C", temperature);

        print_str_in_line(&display1, res, 3);
        sh1106_update(&display);

        pthread_mutex_unlock(&mutex);
        vTaskDelay(10);
    }
}



/*
 * Darws waiting page, while dht11 needs
 * to process data.
 */
static void wait_page_draw() {
    sh1106_t display;
    sh1106_init(&display);
    sh1106_clear(&display);

    sh1106_t *display1 = &display;
    print_str_in_line(&display1, "        Wait...", 3);
    sh1106_update(&display);
}

/**
 * @brief Writes a given value in a specified register of an ADXL345.
 *
 * In a 4-wire SPI transaction, transmission and receiving happen in parallel
 * as shown below.
 *
 * ```
 * Transmit: address --> t.tx_data[0]
 * Receive:  N/A     --> t.rx_data[0]
 *
 * where t is spi_transaction_t and
 * t.tx_data[0] = value
 * t.rx_data[0] = do not care
 * ```
 *
 * This function blocks until the transmission and receiving end.
 *
 * @param[in] spi
 *
 *   Handle of the ADXL345 where the register value is to be set.
 *
 * @param[in] address
 *
 *   Address of the register to be written.
 *
 * @param[in] value
 *
 *   Value to be written to the register associated with `address`.
 */
static void adxl345_write(spi_device_handle_t spi, uint8_t address, uint8_t value) {
    esp_err_t ret;

    spi_transaction_t trans = {
        .flags     = SPI_TRANS_USE_RXDATA,
        .cmd       = address,
        .tx_buffer = &value,
        .length    = 8 // in bits
    };
    ret = spi_device_polling_transmit(spi, &trans);
    assert(ret == ESP_OK);
}

/**
 * @brief Reads the latest acceleration from the ADXL345.
 *
 * Undefined if `accs` does not point to a block smaller than
 * `sizeof(int16_t) * 3`
 *
 * @param[in] spi
 *
 *   Handle of the ADXL345 from which the latest acceleration is to be read.
 *
 * @param[out] accs
 *
 *   Buffer to receive acceleration.
 *   - `[0]`: x-acceleration
 *   - `[1]`: y-acceleration
 *   - `[2]`: z-acceleration
 */
static void adxl345_read_acceleration(spi_device_handle_t spi, int16_t* accs) {
    esp_err_t ret;
    uint8_t tx_buffer[3u * sizeof(uint16_t)]; // a dummy buffer
    spi_transaction_t trans = {
        .cmd = ADXL345_REG_READ_FLAG |
            ADXL345_REG_MB_FLAG |
            ADXL345_REG_DATAX0,
        .length = sizeof(tx_buffer) * 8, // in bits
        .tx_buffer = tx_buffer,
        .rx_buffer = accs
    };
    ret = spi_device_polling_transmit(spi, &trans);
    assert(ret == ESP_OK);
}


/**
 * @brief Starts sampling of the ADXL345.
 *
 * This function blocks for 200ms after sending a start command.
 *
 * @param[in] spi
 *
 *   Handle of the ADXL345 to start.
 */
static void adxl345_start(spi_device_handle_t spi) {
    adxl345_write(spi, ADXL345_REG_POWER_CTL, ADXL345_POWER_CTL_MEASURE);
    vTaskDelay(ADXL345_UPDATE_DELAY);
}

spi_bus_config_t bus_config_init() {
    spi_bus_config_t bus_config = {
        .miso_io_num        = PIN_NUM_MISO,
        .mosi_io_num        = PIN_NUM_MOSI,
        .sclk_io_num        = PIN_NUM_CLK,
        .quadwp_io_num      = -1,
        .quadhd_io_num      = -1
    };
    return bus_config;
}



spi_device_interface_config_t device_config_init() {
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 1000000,          // 1Mbps. nearest clock will be chosen.
        .mode           = 3,                // CPOL=1, CPHA=1
        .spics_io_num   = PIN_NUM_CS,
        .command_bits   = 8,                // ADXL345 always takes 1+7 bit command (address).
        .queue_size     = 1 
    };

    return device_config;
}

/**
 * @brief Task that periodically reads accelerations.
 *
 * @param[in] pvParameters
 *
 *   (`spi_device_handle_t`) Handle to the ADXL345 from which the latest
 *   accleration is to be read.
 */
static void accelerator(void* pvParameters) {
    int16_t accs[3];
    int x;
    int y;
    int z;

    spi_device_handle_t spi = (spi_device_handle_t)pvParameters;
    while (1) {
        adxl345_read_acceleration(spi, accs);

        x = (int)accs[0];
        y = (int)accs[1];
        z = (int)accs[2];

        if (y >= 200) {
            if (reverse == 0) {
                refresh = 1;
            }
            reverse = 1;
        }
        else {
            if (reverse == 1) {
                refresh = 1;
            }
            reverse = 0;
        }

        vTaskDelay(100);
    }
}



void app_main(void) {
    spi_device_handle_t spi;

    if (dac_output_enable(DAC_CHANNEL_1) != ESP_OK) ESP_LOGI("dac_output_enable ", "%s", "some error occured.");
    gpio_set_direction_wrapper(GPIO_NUM_5, GPIO_MODE_INPUT);
    gpio_set_level_wrapper(GPIO_NUM_5, 1);
    // Enables led.
    gpio_set_direction_wrapper(GPIO_LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction_wrapper(GPIO_LED2, GPIO_MODE_OUTPUT);
    gpio_set_direction_wrapper(GPIO_LED3, GPIO_MODE_OUTPUT);
    // Enable accelerometer.
    gpio_set_direction_wrapper(GPIO_EN_ACCEL, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(GPIO_EN_ACCEL, 1);

    spi_bus_config_t bus_config                 = bus_config_init();
    spi_device_interface_config_t device_config = device_config_init();

    // initializes the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, DMA_CHAN));
    // attaches the ADXL to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &device_config, &spi));
    // starts sampling
    adxl345_start(spi);

    xTaskCreate(accelerator, "acceleration", 2048u, (void*)spi, 5, 0);


    // Oled enable
    gpio_set_direction_wrapper(OLED_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level_wrapper(OLED_ENABLE, 1);

    init_i2c_driver();
    wait_page_draw();
    dht11_data_init();

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

    xTaskCreate(switch_button_handler, "switch_button_handler", 4048, NULL, 10, NULL);
    xTaskCreate(drawer, "drawer", 4048, NULL, 10, NULL);
    xTaskCreate(dht11_data_checker, "dht11_data_checker", 4048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_BUTTON1, gpio_isr_handler, (void*) GPIO_BUTTON1);
    gpio_isr_handler_add(GPIO_BUTTON2, gpio_isr_handler, (void*) GPIO_BUTTON2);
}