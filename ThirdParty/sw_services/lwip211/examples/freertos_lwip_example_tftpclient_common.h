/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#ifndef __TFTP_COMMON_H_
#define __TFTP_COMMON_H_

#include "ff.h"
#include "lwip/ip.h"

/* Host ip which is running tftp daemon application */
#define TFTP_SERVER_IP			"10.10.70.101"

#define TFTP_THREAD_STACKSIZE		1024

#define TFTP_PORT			69

#define TFTP_MAX_MSG_LEN		600

#define TFTP_DATA_PACKET_MSG_LEN	512
#define TFTP_PACKET_HDR_LEN		4
#define TFTP_DATA_PACKET_LEN		(TFTP_DATA_PACKET_MSG_LEN + TFTP_PACKET_HDR_LEN)
#define TFTP_MAX_ACK_LEN		4
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
