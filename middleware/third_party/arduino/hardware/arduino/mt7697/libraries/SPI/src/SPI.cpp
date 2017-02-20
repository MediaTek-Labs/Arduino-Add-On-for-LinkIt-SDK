#include <Arduino.h>
#include <constants.h>
#include <pin_mux.h>
#include <arduino_pins.h>
#include <hal_platform.h>
#include "hal_spi_master.h"
#include "hal_gpio.h"
#include "syslog.h"

#include "SPI.h"

//#define DEBUG_LOG(x)        Serial.println(x)
#define DEBUG_LOG(x)

SPIClass SPI;

SPISettings::SPISettings(uint32_t clockFrequency, BitOrder bitOrder, SPIDataMode dataMode)
{
    clock          = clockFrequency;
    bit_order      = bitOrder;
    data_mode      = dataMode;
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

}

/* Disables the SPI bus (leaving pin modes unchanged) */
void SPIClass::end()
{

}

/* Initializes the SPI bus using the defined SPISettings */
void SPIClass::beginTransaction(SPISettings settings)
{
    if (m_spi == NULL)
    {
        RHSPIDataMode   mode;
        RHSPIFrequency  freq;
        RHSPIBitOrder   order;

        /* decide bit order */
        order = (settings.bit_order == LSBFIRST)? BitOrderLSBFirst: BitOrderMSBFirst;

        /* decide the SPI mode */
        mode  = (RHSPIDataMode)settings.data_mode;

#if 1
        /* best effort for SW SPI */
        freq = Frequency16MHz;
#else
        if (settings.clock >= 16000000)
        {
            freq = Frequency16MHz;
        }
        else if (settings.clock >= 8000000)
        {
            freq = Frequency8MHz;
        }
        else if (settings.clock >= 4000000)
        {
            freq = Frequency4MHz;
        }
        else if (settings.clock >= 2000000)
        {
            freq = Frequency2MHz;
        }
        else
        {
            freq = Frequency1MHz;
        }
#endif
        m_spi = new RHSoftwareSPI(freq, order, mode);

        if (m_spi == NULL)
        {
            DEBUG_LOG("allocate the new SPI instance failed.");
        }

        m_spi->begin();
    }
}

/* Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus */
void SPIClass::endTransaction(void)
{
    if (m_spi != NULL)
    {
        delete m_spi;

        m_spi = NULL;
    }
}

uint8_t SPIClass::transfer(uint8_t data)
{
    if (m_spi != NULL)
    {
        return m_spi->transfer(data);
    }

    return 0;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    uint8_t     buffer[2];
    uint16_t    result = 0;

    buffer[1] = transfer((data >> 8) & 0xff);
    buffer[0] = transfer(data & 0xff);

    result = ((buffer[1] << 8) | buffer[0]) & 0xffff;

    return result;
}

void SPIClass::transfer(void *buf, size_t count)
{
    if (m_spi != NULL)
    {
        int32_t i;

        for (i = count - 1; i >= 0; i--)
        {
            transfer(*(((uint8_t *)buf) + i));
        }
    }
}


