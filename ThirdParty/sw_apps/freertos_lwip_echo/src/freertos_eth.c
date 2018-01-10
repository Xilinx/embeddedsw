/* liubenyuan <liubenyuan@gmail.com> */

#include "freertos_eth.h"
#include "xil_printf.h"

/* enable debug when code is not stable */
#define DBG_ETH 1

/*---------------------------------------------------------------------------*/

/* chunk based read/write */
int chunk_size(int len, const int chunk_size) {
    if (len < chunk_size)
        return len;
    else
        return chunk_size;
}
/*---------------------------------------------------------------------------*/

/* static TX buffer */
static TX_BUF_STRUCT dout;

/*
 * MFEIT write, if data > BUF_SIZE, you should be divided into small segments.
 * @input : socket, data, length
 * @output : 0 : success, -1 : failed
 */
int mfeit_write(long skt, char *data, int n)
{

    /* if socket is invalid */
    if (skt == -1) {
#if(DBG_ETH)
        xil_printf("MFEIT-WRITE : invalid socket.\n");
#endif
        return -1;
    }

    /*
     * as we are making a local copy, avoid TX buffer over-flow
     */
    if (n > BUF_SIZE) {
        n = BUF_SIZE;
#if(DBG_ETH)
        xil_printf("MFEIT-WRITE : TX Buffer Overflow, n = %d\n", n);
#endif
    }

    /* build transmit TB */
    dout.len = n;
    memcpy(dout.buf, data, n);

    /* build LFTP headers
     * and transmit TX data packet by packet */
    int len = n + sizeof(size_t);
    char *p = (char *)&dout;
    int iResult = 0;
    int tcp_tx_len;

    /* send chunk by chunk to peer */
    tcp_tx_len = chunk_size(len, TCP_SND_BUF);
    do {

        /* transmit chunk by chunk */
        iResult = lwip_send(skt, p, tcp_tx_len, 0);

        if (iResult > 0) {
            len       -= iResult;
            p         += iResult;
            tcp_tx_len = chunk_size(len, TCP_SND_BUF);
        } else {
#if(DBG_ETH)
            xil_printf("MFEIT-WRITE : TX failed at %d\n", len);
#endif
            return -1;
        }
    } while (tcp_tx_len > 0);

    /* should not happen */
    if (len != 0) {
#if(DBG_ETH)
        xil_printf("MFEIT-WRITE : packet are not fully transmitted, remaining %d\n", len);
#endif
        return -1;
    }

    /* everything is OK */
    return len;
}
/*---------------------------------------------------------------------------*/

/* MFEIT read */
int mfeit_read(long skt, char *data)
{

    /* if socket invalid */
    if (skt == -1) {
#if(DBG_ETH)
        xil_printf("MFEIT-READ : invalid read socket.\n");
#endif
        return -1;
    }

    /* receive length from peer */
    unsigned int len;
    int iResult = lwip_recv(skt, (char *)&len, sizeof(size_t), 0);
    if ( iResult == -1 ) {
#if(DBG_ETH)
        xil_printf("MFEIT-READ : receive TCP header failed.\n");
#endif
        return -1;
    }

    /* receive data chunk by chunk from peer */
    char *p = data;
    int tcp_rx_len;

    tcp_rx_len = chunk_size( len, TCP_MSS );
    do {

        /* receive data chunk by chunk */
        iResult = lwip_recv(skt, p, tcp_rx_len, 0);

        if (iResult > 0) {
            len -= iResult;
            p   += iResult;
            tcp_rx_len = chunk_size( len, TCP_MSS );
        } else {
#if(DBG_ETH)
            xil_printf("MFEIT-READ : RX failed at %d\n", len);
#endif
            return -1;
        }
    } while (tcp_rx_len > 0);

    /* should not happen */
    if (len != 0) {
#if(DBG_ETH)
        xil_printf("MFEIT-READ : packet are not fully received, remaining %d\n", len);
#endif
        return -1;
    }

    /* everything is OK */
    return len;
}
/*---------------------------------------------------------------------------*/

