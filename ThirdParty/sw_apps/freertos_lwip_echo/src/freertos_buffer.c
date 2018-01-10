#include "freertos_buffer.h"

/* PRINT_ will print the received data */
#define PRINT_DBG_DMA 0
#define DBG_DMA_TX    1

/* prepare a PING-PONG buffer */
const u8 *rx_a_ptr         =(u8 *)RX_BUFFER_BASE;
const u8 *rx_b_ptr         =(u8 *)(RX_BUFFER_BASE + RX_MAX_BLOCK);
static u8 *rxbd            =(u8 *)RX_BUFFER_BASE;

/* should we invalidate these memory */
char *tcp_tx_ptr           =(char *)TCP_TX_BASE;
char *tcp_rx_ptr           =(char *)TCP_RX_BASE;

/* Semaphore used to guard the TX buffer. */
static xSemaphoreHandle xmTxBufferMutex = NULL;

/*---------------------------------------------------------------------------*/
/*
 * toggle PING-PONG for MFEIT
 * you may add a semaphore to control the toggling of
 * the RX_Base_ptr, in case TCP-IP is reading a buffer.
 */
static unsigned int dma_nOverride = 0;
u8 *setRxBasePtr()
{
    if (rxbd == rx_a_ptr)
        rxbd = (u8 *)rx_b_ptr;
    else
        rxbd = (u8 *)rx_a_ptr;

    /* Finished with the TX buffer. Produce new. */
    if (xSemaphoreTake( xmTxBufferMutex,0 ) != pdPASS)
        ;
    else {
        /* if the producer takes the mutex, it means that the
         * consumer failed to process the message, an override
         * may happens due to TCP/IP traffic */
        dma_nOverride ++;
#if(DBG_DMA_TX)
        xil_printf("(%8d) Producer override Consumer\n", dma_nOverride);
#endif
    }

    /* regardless of whether DMA succeed or fail,
     * always produce a new RX BD */
    xSemaphoreGive( xmTxBufferMutex );

    return rxbd;
}
/*---------------------------------------------------------------------------*/

/* get a valid ptr for writing sTCB */
u8 *getRxBasePtr()
{
    // Xil_DCacheInvalidateRange((u32)rxbd, RX_REG_OFFSET);
    return rxbd;
}
/*---------------------------------------------------------------------------*/

/* get RX valid base_ptr for Transmission */
u8 *getValidRxBasePtr()
{

    /* it will wait here if no valid ptr has been generated */
    while (xSemaphoreTake( xmTxBufferMutex,
                           MFEIT_MAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS ) != pdPASS ) {
        ;
    }

    if (rxbd == rx_a_ptr)
        return (u8 *)rx_b_ptr;
    else
        return (u8 *)rx_a_ptr;

}
/*---------------------------------------------------------------------------*/

/*
 * get RX BD. bdNum should be [0, ..., max_switch - 1]
 */
u8 *getRxBD(unsigned int bdNum, unsigned int nBytes)
{

    u8 *this_rxbd = rxbd + RX_REG_OFFSET + bdNum * nBytes;

    /*
     * Invalidate the DestBuffer before receiving the data,
     * in case the Data Cache is enabled.
     */
    // Xil_DCacheInvalidateRange((u32)this_rxbd, nBytes);

    return this_rxbd;
}
/*---------------------------------------------------------------------------*/

