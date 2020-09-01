#include "accelerometer.h"

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
void adxl345_write(spi_device_handle_t spi, uint8_t address, uint8_t value) {
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
void adxl345_read_acceleration(spi_device_handle_t spi, int16_t* accs) {
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
void adxl345_start(spi_device_handle_t spi) {
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
