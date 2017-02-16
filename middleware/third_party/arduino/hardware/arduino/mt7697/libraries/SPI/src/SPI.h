#ifndef __SPI_H__
#define __SPI_H__

#include <Arduino.h>
#include "RHSoftwareSPI.h"

typedef enum
{
    SPI_MODE0 = 0,
    SPI_MODE1 = 1,
    SPI_MODE2 = 2,
    SPI_MODE3 = 3
} SPIDataMode;

class SPISettings {
public:
    SPISettings(uint32_t clockFrequency, BitOrder bitOrder, SPIDataMode dataMode);
    SPISettings();

private:
    uint32_t    clock;
    BitOrder    bit_order;
    SPIDataMode data_mode;

    friend class SPIClass;
};

class SPIClass {
public:
    SPIClass()  { m_spi = NULL; }
    void        begin();
    void        end();
    void        beginTransaction(SPISettings settings);
    void        endTransaction(void);
    uint8_t     transfer(uint8_t data);
    uint16_t    transfer16(uint16_t data);
    void        transfer(void *buf, size_t count);
    //void        usingInterrupt(uint8_t interruptNumber);

    /* Deprecated. Use SPISettings with SPI.beginTransaction() to configure SPI parameters */
#if 0
    void        setBitOrder(uint8_t bitOrder);
    void        setClockDivider(uint8_t clockDiv);
    void        setDataMode(uint8_t dataMode);
#endif

private:
    SPIDataMode m_data_mode;
    BitOrder    m_bit_order;

    RHSoftwareSPI *m_spi;
};

extern SPIClass SPI;

#endif
