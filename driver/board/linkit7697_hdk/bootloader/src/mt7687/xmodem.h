
#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdint.h>

/**
 * This is the callback function prototype of xmodem_block_rx(). Implement
 * a function in this type and supply it to xmodem_block_rx(). See description
 * of xmodem_block_rx() for more information.
 *
 * @param buf the pointer to the buffer.
 * @param len -1 if RX ended.
 */
typedef void (*xmodem_block_rx_callback_t)(void *ptr, uint8_t *buffer, int len);

/**
 * This is the block-based, callback-enabled implementation of xmodemReceive.
 */
int xmodem_block_rx(void                        *ptr,
                    uint8_t                     *dest,
                    int                         destsz,
                    xmodem_block_rx_callback_t  function_pointer);

#endif
