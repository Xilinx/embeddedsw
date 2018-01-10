/*
 * liubenyuan <liubenyuan@gmail.com>
 */

#ifndef MFEIT_ZYNQ_H_
#define MFEIT_ZYNQ_H_

/* Standard includes. */
#include "stdlib.h"
#include "string.h"

/* lwIP core includes */
#include "lwip/sockets.h"

/*
 * RX/TX should not exceed the limit for data allocation
 * client works best when packet size > 1024
 */

/* maximum data per packet (aggressive) */
#define BUF_SIZE            ( 4*(65536 + 1024) + 4096 )
/* = TCP_SND_BUF, maximum per send command */
#define CHUNK_SIZE          (TCP_SND_BUF)

/* header + data */
typedef struct tx_buf_struct
{
    size_t len;              // (size_t)
    char buf[BUF_SIZE];      // real data content
} TX_BUF_STRUCT;

/* read/write functions */
int chunk_size(int len, const int chunk_size);
int mfeit_write(long skt, char *data, int len);
int mfeit_read(long skt, char *data);

#endif /* MFEIT_ZYNQ_H_ */
