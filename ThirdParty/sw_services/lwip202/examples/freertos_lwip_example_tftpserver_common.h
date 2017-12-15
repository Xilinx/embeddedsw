/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef __TFTP_COMMON_H_
#define __TFTP_COMMON_H_

#include "ff.h"
#include "lwip/ip.h"

#define TFTP_THREAD_STACKSIZE		1024

#define TFTP_PORT			69

#define TFTP_MAX_MSG_LEN		600

#define TFTP_DATA_PACKET_MSG_LEN	512
#define TFTP_PACKET_HDR_LEN		4
#define TFTP_DATA_PACKET_LEN		(TFTP_DATA_PACKET_MSG_LEN + TFTP_PACKET_HDR_LEN)
#define TFTP_MAX_ACK_LEN		4
#define TFTP_MIN_DATA_PACKET_LEN	4
#define TFTP_MAX_ERR_MSG_LEN		30

#define TFTP_MAX_RETRY_COUNT		3

/* tftp packet offset */
#define OPCODE_OFFSET			0
#define FIL_NAME_OFFSET			2
#define ERRCODE_OFFSET			2
#define BLOCK_OFFSET			2
#define DATA_OFFSET			4

/* Packet form macro */
#define tftp_get_opcode(buf) \
		ntohs(*((u16_t *)(buf + OPCODE_OFFSET)))

#define tftp_set_opcode(buf, opcode) \
		*((u16_t *)(buf + OPCODE_OFFSET)) = htons(opcode)

#define tftp_get_filename(buf, fname) \
		strcpy(fname, buf + FIL_NAME_OFFSET)

#define tftp_set_filename(buf, fname) \
		strcpy(buf + FIL_NAME_OFFSET, fname)

#define tftp_set_mode(buf, mode, offset) \
		strcpy(buf + offset, mode)

#define tftp_get_block_value(buf) \
		ntohs(*((u16_t *)(buf + BLOCK_OFFSET)))

#define tftp_set_block_value(buf, value) \
		*((u16_t *)(buf + BLOCK_OFFSET)) = htons(value)

#define tftp_get_errcode(buf) \
		ntohs(*((u16_t *)(buf + ERRCODE_OFFSET)))

#define tftp_set_errcode(buf, err) \
		*((u16_t *)(buf + ERRCODE_OFFSET)) = htons(err)

#define tftp_set_errmsg(buf, errmsg) \
		strcpy(buf + DATA_OFFSET, errmsg)

#define tftp_set_data(pkt, buf, len) \
		memcpy(pkt + DATA_OFFSET, buf, len)

typedef enum {
	TFTP_RRQ = 1,
	TFTP_WRQ = 2,
	TFTP_DATA = 3,
	TFTP_ACK = 4,
	TFTP_ERROR = 5
} tftp_opcode;

/* tftp_errorcode error strings */
char *tftp_errorcode_string[] = {
        "not defined",
        "file not found",
        "access violation",
        "disk full",
        "illegal operation",
        "unknown transfer id",
        "file already exists",
        "no such user",
};

typedef enum {
        TFTP_ERR_NOTDEFINED,
        TFTP_ERR_FILE_NOT_FOUND,
        TFTP_ERR_ACCESS_VIOLATION,
        TFTP_ERR_DISKFULL,
        TFTP_ERR_ILLEGALOP,
        TFTP_ERR_UNKNOWN_TRANSFER_ID,
        TFTP_ERR_FILE_ALREADY_EXISTS,
        TFTP_ERR_NO_SUCH_USER,
} tftp_errorcode;

typedef struct {
	/* last block read */
	char data[TFTP_MAX_MSG_LEN];
	struct sockaddr_in from;
} tftp_connection_args;

#endif /* __TFTP_COMMON_H_ */
