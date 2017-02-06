#ifndef __LFLASH_H__
#define __LFLASH_H__

#ifdef MTK_NVDM_ENABLE

#include <Arduino.h>

typedef enum
{
    LFLASH_RAW_DATA,
    LFLASH_STRING_DATA
} LFlashDataType;

typedef enum
{
    LFLASH_INVALID_PARAMETER  = -5, /**< The user parameter is invalid. */
    LFLASH_ITEM_NOT_FOUND     = -4, /**< The data item wasn't found.  */
    LFLASH_INSUFFICIENT_SPACE = -3, /**< No space is available in the flash. */
    LFLASH_INCORRECT_CHECKSUM = -2, /**< Found a checksum error when reading the data item. */
    LFLASH_ERROR              = -1, /**< An unknown error occurred. */
    LFLASH_OK                 = 0   /**< The operation was successful. */
} LFlashStatus;

class LFlashClass {
public:
    LFlashClass();

    /* initialize the flash module */
    LFlashStatus begin();

    /* write data into the flash
       [IN] sectionName  - The name of the data section
       [IN] propertyName - The name of the property inside the data section specified above
       [IN] dataType     - The data type to be stored
       [IN] buffer       - The buffer which contains the data
       [IN] data_size    - The size of the data
    */
    LFlashStatus write(
        const char      *sectionName,
        const char      *propertyName,
        LFlashDataType  dataType,
        const uint8_t   *buffer,
        uint32_t        data_size);

    /* read data from the flash
       [IN]     sectionName  - The name of the data section
       [IN]     propertyName - The name of the property inside the data section specified above
       [IN]     buffer       - The buffer to contain the data
       [IN/OUT] data_size    - as input: the size of the buffer. as output: the size of the read-out data
    */
    LFlashStatus read(const char *sectionName, const char *propertyName, uint8_t *buffer, uint32_t *size);

private:
};

extern LFlashClass LFlash;

#else
#error "MTK_NVDM_ENABLE not defined."
#endif

#endif
