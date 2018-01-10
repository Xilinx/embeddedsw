#ifndef FREERTOS_BUFFER_H_
#define FREERTOS_BUFFER_H_

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx */
#include "xparameters.h"

// Truly transfer size = RX_REG_OFFSET + 16*MAX_PKT_LEN
#define MAX_PKT_LEN         ( 4096*4 )
#define RX_MAX_BLOCK        0x00060000  // 6*65536 Bytes
#define RX_REG_OFFSET       0x00001000  // 4096 Bytes

// set to your MM2S/S2MM address
#define MEM_BASE_ADDR       0x10000000

// must be 32bit aligned
#define TX_BUFFER_BASE      (MEM_BASE_ADDR + 0x01000000)
#define RX_BUFFER_BASE      (MEM_BASE_ADDR + 0x02000000)
#define RX_BUFFER_HIGH      (MEM_BASE_ADDR + 0x08000000)

// TCP TX buffers
#define TCP_TX_BASE         RX_BUFFER_HIGH
#define TCP_TX_HIGH         (TCP_TX_BASE   + 0x01000000)
#define TCP_RX_BASE         TCP_TX_HIGH
#define TCP_RX_HIGH         (TCP_RX_BASE   + 0x01000000)

/*
 * The maximum time to block waiting to obtain the xTxBufferMutex
 * to become available.
 */
#define MFEIT_MAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS ( 19 / portTICK_RATE_MS )

/*
 * implement a simple PING-PONG FIFO.
 */
u8 *setRxBasePtr( void );      // set A-port
u8 *getRxBasePtr( void );      // get A-port
u8 *getValidRxBasePtr( void ); // get B-port
u8 *getRxBD(unsigned int bdNum, unsigned int nBytes);

#endif
