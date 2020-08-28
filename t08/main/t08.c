/*
 * ADXL345 control via SPI.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "wrappers.h"
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
    // sample of each axis is represented in twos complement.
    // and as ESP32 is little endian, `accs` does not need swapping.
    // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/spi_master.html#transactions-with-integers-other-than-uint8-t
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

static void do_sound() {
    int counter = 0;
    while(counter != 1000) {
        int v;
        for(v = 0; v < 255; v+= 30) {
        if (dac_output_voltage(DAC_CHANNEL_1, v) != ESP_OK) 
            ESP_LOGI("dac_output_voltage ", "%s", "some error occured.");
        }
        for(v = 255; v > 0; v-= 30) {
            if (dac_output_voltage(DAC_CHANNEL_1, v) != ESP_OK) 
                ESP_LOGI("dac_output_voltage ", "%s", "some error occured.");
        }
        ets_delay_us(1);
        counter +=1;
    }
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

        printf("%d %d %d\n", x,y,z);
 		if ((x >= 246 && x <= 300)|| (x <= -263 && x >= -300)|| (y >= 254 && y <= 300) || (y <= -250 && y >= -300) ) {
	 		gpio_set_level_wrapper(GPIO_LED1, 1);
	 		gpio_set_level_wrapper(GPIO_LED2, 1);
	 		gpio_set_level_wrapper(GPIO_LED3, 1);
            do_sound();
 		}
 		else {
 			gpio_set_level_wrapper(GPIO_LED1, 0);
	 		gpio_set_level_wrapper(GPIO_LED2, 0);
	 		gpio_set_level_wrapper(GPIO_LED3, 0);
 		}

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
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



void app_main (void) {
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
}



