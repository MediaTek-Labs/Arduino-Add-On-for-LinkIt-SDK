#include <Arduino.h>
#include <constants.h>
#include <pin_mux.h>
#include <arduino_pins.h>
#include <hal_platform.h>
#include "hal_spi_master.h"
#include "hal_gpio.h"
#include "syslog.h"

#include "SPI.h"

#define GPIO_FUNC_INDEX     (8)
#define SPI_FUNC_INDEX      (7)
#define PHASE_MASK          (0x01)
#define POLARITY_MASK       (0x10)

#define DEBUG_LOG(x)        LOG_I(common, x)

SPIClass SPI;

SPISettings::SPISettings(uint32_t clockFrequency, BitOrder bitOrder, SPIDataMode dataMode, bool fullDuplex)
{
    clock          = clockFrequency;
    bit_order      = bitOrder;
    data_mode      = dataMode;
    is_full_duplex = fullDuplex;
}

/* default SPI setting */
SPISettings::SPISettings()
{
    clock     = 7000000;
    bit_order = MSBFIRST;
    data_mode = SPI_MODE3;
}

/* Initializes the SPI bus by setting SCK,
   MOSI, and SS to outputs, pulling SCK and
   MOSI low, and SS high */
void SPIClass::begin()
{
    if (hal_gpio_init(HAL_GPIO_29) != HAL_GPIO_STATUS_OK ||
        hal_gpio_init(HAL_GPIO_31) != HAL_GPIO_STATUS_OK ||
        hal_gpio_init(HAL_GPIO_32) != HAL_GPIO_STATUS_OK)
    {
        DEBUG_LOG("Pin init failed.");
        return;
    }

    hal_pinmux_set_function(HAL_GPIO_29, GPIO_FUNC_INDEX);
    hal_pinmux_set_function(HAL_GPIO_31, GPIO_FUNC_INDEX);
    /* CS pin MUST be configured as GPIO */
    hal_pinmux_set_function(HAL_GPIO_32, GPIO_FUNC_INDEX);

    /* set pin direction for MOSI / SCK / CS */
    hal_gpio_set_direction(HAL_GPIO_29, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_direction(HAL_GPIO_31, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_direction(HAL_GPIO_32, HAL_GPIO_DIRECTION_OUTPUT);

    /* pulling SCK and MOSI low, and SS high */
    hal_gpio_set_output(HAL_GPIO_29, HAL_GPIO_DATA_LOW);
    hal_gpio_set_output(HAL_GPIO_31, HAL_GPIO_DATA_LOW);
    hal_gpio_set_output(HAL_GPIO_32, HAL_GPIO_DATA_HIGH);

    if (hal_gpio_deinit(HAL_GPIO_29) != HAL_GPIO_STATUS_OK ||
        hal_gpio_deinit(HAL_GPIO_31) != HAL_GPIO_STATUS_OK)
    {
        DEBUG_LOG("Pin deinit failed.");
        return;
    }

    /* switch to SPI pinmux */
    hal_pinmux_set_function(HAL_GPIO_29, SPI_FUNC_INDEX);
    hal_pinmux_set_function(HAL_GPIO_30, SPI_FUNC_INDEX);
    hal_pinmux_set_function(HAL_GPIO_31, SPI_FUNC_INDEX);
}

/* Disables the SPI bus (leaving pin modes unchanged) */
void SPIClass::end()
{
    hal_spi_master_deinit(HAL_SPI_MASTER_0);
}

/* Initializes the SPI bus using the defined SPISettings */
void SPIClass::beginTransaction(SPISettings settings)
{
    hal_spi_master_config_t config;

    config.clock_frequency = settings.clock;
    config.slave_port      = HAL_SPI_MASTER_SLAVE_0;
    config.bit_order       = (settings.bit_order == MSBFIRST)? HAL_SPI_MASTER_MSB_FIRST: HAL_SPI_MASTER_LSB_FIRST;
    config.phase           = ((settings.data_mode & PHASE_MASK) == 0)? HAL_SPI_MASTER_CLOCK_PHASE0: HAL_SPI_MASTER_CLOCK_PHASE1;
    config.polarity        = ((settings.data_mode & POLARITY_MASK) == 0)? HAL_SPI_MASTER_CLOCK_POLARITY0: HAL_SPI_MASTER_CLOCK_POLARITY1;

    if (hal_spi_master_init(HAL_SPI_MASTER_0, &config) != HAL_SPI_MASTER_STATUS_OK)
    {
        DEBUG_LOG("SPI init failed.");
        return;
    }

    /* keep track of the current mode */
    m_data_mode      = settings.data_mode;
    m_is_full_duplex = settings.is_full_duplex;
    m_bit_order      = settings.bit_order;
}

/* Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus */
void SPIClass::endTransaction(void)
{
    /* low idle */
    if ((m_data_mode & POLARITY_MASK) == 0)
    {
        hal_gpio_set_output(HAL_GPIO_32, HAL_GPIO_DATA_LOW);
    }
    /* high idle */
    else
    {
        hal_gpio_set_output(HAL_GPIO_32, HAL_GPIO_DATA_HIGH);
    }
}

uint8_t SPIClass::transfer(uint8_t data)
{
    hal_spi_master_send_and_receive_config_t config;
    uint8_t buffer[2];

    config.send_data = &data;
    config.send_length = sizeof(uint8_t);
    config.receive_buffer = buffer;
    config.receive_length = sizeof(buffer);

    if (hal_spi_master_send_and_receive_polling(HAL_SPI_MASTER_0, &config) != HAL_SPI_MASTER_STATUS_OK)
    {
        DEBUG_LOG("SPI transfer failed.");
    }

    /* decide which data to be returned */
    if (m_is_full_duplex)
    {
        return buffer[0];
    }
    else
    {
        return buffer[1];
    }

    return 0;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    hal_spi_master_send_and_receive_config_t config;
    uint8_t buffer[4];

    config.send_data = (uint8_t *)&data;
    config.send_length = sizeof(uint16_t);
    config.receive_buffer = buffer;
    config.receive_length = sizeof(buffer);

    if (hal_spi_master_send_and_receive_polling(HAL_SPI_MASTER_0, &config) != HAL_SPI_MASTER_STATUS_OK)
    {
        DEBUG_LOG("SPI transfer16 failed.");
    }

    /* decide which data to be returned */
    if (m_is_full_duplex)
    {
        return buffer[0];
    }
    else
    {
        return buffer[2];
    }

    return 0;
}

void SPIClass::transfer(void *buf, size_t count)
{
    if (hal_spi_master_send_polling(HAL_SPI_MASTER_0, (uint8_t *)buf, count) != HAL_SPI_MASTER_STATUS_OK)
    {
        DEBUG_LOG("Data sent failed.");
    }
}

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
    /* ToDo */
}



