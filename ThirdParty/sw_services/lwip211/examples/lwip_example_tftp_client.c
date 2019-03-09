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

#include <string.h>

#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip_example_tftpclient_common.h"
#include "xil_printf.h"

#define FILE_TO_TRANSFER 3

#define TFTP_TEST_FIL_DATA \
	"----- This is a test file for TFTP client application -----"

/* tftp_errorcode error strings */
static char *tftp_errorcode_string[] = {
        "not defined",
        "file not found",
        "access violation",
        "disk full",
        "illegal operation",
        "unknown transfer id",
        "file already exists",
        "no such user",
};

static struct udp_pcb *pcb;
static tftp_cli_connection_args tftp_arg;
static u8_t tftp_get_done, tftp_put_done;
volatile u8_t tftp_in_process;

extern struct netif server_netif;

void print_app_header()
{
	xil_printf("\r\nTftp client app gets 3 files(sample#.txt) from host"
			" and put 3 files(sample#.txt) on host.\r\n"
			"Make sure host(%s) is running tftp daemon app\r\n\r\n",
			TFTP_SERVER_IP);
}

static void tftp_prepare_request(char *buf, char *file, tftp_opcode op,
		char *mode)
{
	u16_t offset;

	tftp_set_opcode(buf, op);
	tftp_set_filename(buf, file);

	offset = FIL_NAME_OFFSET + strlen(file) + 1;
	tftp_set_mode(buf, mode, offset);
}

static void tftp_client_send_req(tftp_opcode opcode, ip_addr_t *ip)
{
	err_t err;
	struct pbuf *pbuf;
	char fname[20];
	char *mode = "netascii";
	u16_t len;

	if (tftp_arg.file_count == FILE_TO_TRANSFER) {
		if (opcode == TFTP_WRQ) {
			tftp_put_done = 1;
			/* we are done with write requests; start with read */
			tftp_arg.file_count = 0;
			opcode = TFTP_RRQ;
		} else {
			tftp_get_done = 1;
			return;
		}
	}

	/* Put file(sample#.txt) request */
	sprintf(fname, "sample%d.txt", ++tftp_arg.file_count);

	/* len = opcode(2) + filename + '\0' + mode + '\0' */
	len = 2 + strlen(fname) + strlen(mode) + 2;

	/* Allocate pbuf */
	pbuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (!pbuf) {
		xil_printf("Unable to alloc pbuf. Memory error\r\n");
		return;
	}

	tftp_prepare_request(pbuf->payload, fname, opcode, mode);

	/* block is 1 for all read requests, 0 for write */
	tftp_arg.block = (opcode == TFTP_RRQ);

	err = udp_sendto(pcb, pbuf, ip, TFTP_PORT);

	if (err != ERR_OK) {
		xil_printf("Failed to send tftp request for %s\r\n", fname);
	} else {
		xil_printf("TFTP_REQ: tftp %s %s\r\n",
				(opcode == TFTP_RRQ) ? "get" : "put", fname);
		tftp_in_process = 1;
	}

	pbuf_free(pbuf);
}

void transfer_data(ip_addr_t *ip)
{
	if (!tftp_put_done)
		tftp_client_send_req(TFTP_WRQ, ip);
	else if (!tftp_get_done)
		tftp_client_send_req(TFTP_RRQ, ip);
}

/* construct and send error packet */
static void tftp_send_error_packet(struct udp_pcb *pcb, ip_addr_t *addr,
		int port, tftp_errorcode err)
{
	err_t ret;
	struct pbuf *pbuf;
	u16_t len;

	/* len = opcode(2) + errcode(2) + msg + '\0' */
	len = TFTP_PACKET_HDR_LEN + strlen(tftp_errorcode_string[err]) + 1;

	pbuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (!pbuf) {
		xil_printf("Failed to allocate pbuf. Memory error\r\n");
		return;
	}

	tftp_set_opcode(pbuf->payload, TFTP_ERROR);
	tftp_set_errcode(pbuf->payload, err);
	tftp_set_errmsg(pbuf->payload, tftp_errorcode_string[err]);

	ret = udp_sendto(pcb, pbuf, addr, port);
	if (ret != ERR_OK)
		xil_printf("Failed to send TFTP_ERROR packet\r\n");

	pbuf_free(pbuf);
}

/* construct and send ack packet */
static void tftp_send_ack_packet(struct udp_pcb *pcb, ip_addr_t *addr, int port,
		u16_t block)
{
	err_t err;
	struct pbuf *pbuf;

	pbuf = pbuf_alloc(PBUF_TRANSPORT, TFTP_MAX_ACK_LEN, PBUF_POOL);
	if (!pbuf) {
		xil_printf("Failed to allocate pbuf. Memory error\r\n");
		return;
	}

	tftp_set_opcode(pbuf->payload, TFTP_ACK);
	tftp_set_block_value(pbuf->payload, block);


	err = udp_sendto(pcb, pbuf, addr, port);
	if (err != ERR_OK)
		xil_printf("Failed to send TFTP_ACK packet\r\n");

	pbuf_free(pbuf);
}

/* construct and send data packet */
static void tftp_send_data_packet(struct udp_pcb *pcb, ip_addr_t *addr,
		int port)
{
	err_t err;
	struct pbuf *pbuf;

	pbuf = pbuf_alloc(PBUF_TRANSPORT,
			tftp_arg.data_len + TFTP_PACKET_HDR_LEN, PBUF_POOL);
	if (!pbuf) {
		xil_printf("Failed to allocate pbuf. Memory error\r\n");
		return;
	}

	tftp_set_opcode(pbuf->payload, TFTP_DATA);
	tftp_set_block_value(pbuf->payload, tftp_arg.block);
	tftp_set_data(pbuf->payload, tftp_arg.data, tftp_arg.data_len);


	err = udp_sendto(pcb, pbuf, addr, port);
	if (err != ERR_OK)
		xil_printf("Failed to send TFTP_ACK packet\r\n");

	pbuf_free(pbuf);
}

/* Send TFTP_ACK packet on receiving TFTP_DATA from host machine */
static void tftp_handle_read_req_resp(struct udp_pcb *pcb, struct pbuf *p,
		ip_addr_t *ip, u16_t port)
{
	FRESULT Res;
	static u8_t file_open = 0;
	static char fname[20];
	char *buf = (char *)p->payload;
	u32_t n;

	/* Verify block value */
	if (tftp_get_block_value(p->payload) != tftp_arg.block) {
		tftp_send_ack_packet(pcb, ip, port, tftp_arg.block);
		return;
	}

	/* Open file on the first ack */
	if (!file_open) {
		sprintf(fname, "sample%d.txt", tftp_arg.file_count);

		/* Overwrite the file if already exists */
		Res = f_open(&tftp_arg.fil, fname, FA_CREATE_ALWAYS | FA_WRITE);
		if (Res) {
			xil_printf("TFTP_RRQ: Unable to open file %s\r\n",
					fname);
			tftp_send_error_packet(pcb, ip, port,
					TFTP_ERR_DISKFULL);
			tftp_in_process = 0;
			return;
		}
		file_open = 1;
	}

	/* write data to file */
	f_write(&tftp_arg.fil, buf + TFTP_PACKET_HDR_LEN,
			p->len - TFTP_PACKET_HDR_LEN, &n);
	if (n != p->len - TFTP_PACKET_HDR_LEN) {
		xil_printf("TFTP_RRQ: Write to file error\r\n");
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_DISKFULL);

		/* Cleanup */
		f_close(&tftp_arg.fil);
		file_open = 0;
		tftp_in_process = 0;
		return;
	}

	/* send_ack_packet */
	tftp_send_ack_packet(pcb, ip, port, tftp_arg.block);
	tftp_arg.block++;

	/* If the data packet length is less than maximum data packet length,
	 * that means we have received whole file. Do cleanup */
	if (p->len < TFTP_DATA_PACKET_MSG_LEN) {
		xil_printf("TFTP_RRQ: Transfer completed\r\n\r\n");

		/* Cleanup */
		f_close(&tftp_arg.fil);
		file_open = 0;
		tftp_in_process = 0;
	}
}

/* Send next packet on receiving TFTP_ACK from host machine for previous
 * TFTP_DATA packet */
static void tftp_handle_write_req_resp(struct udp_pcb *pcb, struct pbuf *p,
		ip_addr_t *ip, u16_t port)
{
	FRESULT Res;
	static u8_t file_open = 0, last_packet_sent = 0;
	char fname[20];

	/* Open file on the first ack */
	if (!file_open) {
		sprintf(fname, "sample%d.txt", tftp_arg.file_count);

		/* Overwrite the file if already exists */
		Res = f_open(&tftp_arg.fil, fname, FA_READ);
		if (Res) {
			xil_printf("TFTP_WRQ: Unable to open file %s\r\n",
					fname);
			tftp_send_error_packet(pcb, ip, port,
					TFTP_ERR_FILE_NOT_FOUND);
			tftp_in_process = 0;
			return;
		}
		file_open = 1;
	}

	/* Read new data if TFTP_ACK packet for previous block is received */
	if (tftp_arg.block == tftp_get_block_value(p->payload)) {
		/* Do not read more if ACK is for last packet */
		if (last_packet_sent) {
			xil_printf("TFTP_WRQ: Transfer completed\r\n\r\n");

			/* Cleanup */
			f_close(&tftp_arg.fil);
			file_open = 0;
			last_packet_sent = 0;
			tftp_in_process = 0;
			return;
		}

		/* read data from file */
		Res = f_read(&tftp_arg.fil, tftp_arg.data,
				TFTP_DATA_PACKET_MSG_LEN, &tftp_arg.data_len);
		if (Res) {
			xil_printf("TFTP_WRQ: File read error\r\n");
			f_close(&tftp_arg.fil);
			file_open = 0;
			tftp_in_process = 0;
			return;
		}
		tftp_arg.block++;
	}

	/* send data packet */
	tftp_send_data_packet(pcb, ip, port);

	/* If the packet length is less than maximum data packet length then
	 * we have read whole file. Wait for the ACK packet before cleanup */
	if (tftp_arg.data_len < TFTP_DATA_PACKET_MSG_LEN)
		last_packet_sent = 1;
}

/* Handle tftp server responses */
static void tftp_client_response_recv_cb(void *arg, struct udp_pcb *pcb,
		struct pbuf *p, ip_addr_t *ip, u16_t port)
{
	tftp_opcode op = tftp_get_opcode(p->payload);
	tftp_errorcode err;

	switch (op) {
	case TFTP_DATA:
		/* Send ACK packet for DATA packet */
		tftp_handle_read_req_resp(pcb, p, ip, port);
		break;
	case TFTP_ACK:
		/* Send next data packet */
		tftp_handle_write_req_resp(pcb, p, ip, port);
		break;
	case TFTP_ERROR:
		err = tftp_get_errcode(p->payload);
		xil_printf("TFTP_WRQ: Error code %d: %s\r\n", err,
			   tftp_errorcode_string[err]);
		tftp_in_process = 0;
		break;
	default:
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_NOTDEFINED);
		xil_printf("TFTP_WRQ: Invalid opcode received\r\n");
		tftp_in_process = 0;
		break;
	}

	pbuf_free(p);
}

/* Create 3 test files(sample#.txt) for "tftp put" in file system */
static int tftp_create_test_file(void)
{
	FIL fp;
	FRESULT Res;
	u32_t ret_len, data_len;
	u8_t file_count = 1;
	char fname[20];

	do {
		sprintf(fname, "sample%d.txt", file_count++);

		Res = f_open(&fp, fname, FA_CREATE_NEW | FA_WRITE);
		if (Res == FR_EXIST)
			continue;

		if (Res) {
			xil_printf("Unable to open file %s for writing %d\r\n",
					fname, Res);
			return -1;
		}

		data_len = strlen(TFTP_TEST_FIL_DATA);
		Res = f_write(&fp, TFTP_TEST_FIL_DATA, data_len, &ret_len);
		if (Res || ret_len != data_len) {
			xil_printf("File write error\r\n");
			f_close(&fp);
			return -1;
		}

		f_close(&fp);
	} while (file_count <= FILE_TO_TRANSFER);

	return ERR_OK;
}

int start_application()
{
	int err;

	/* Create files(sample#.txt) in file system */
	err = tftp_create_test_file();
	if (err != ERR_OK)
		return err;

	/* Create udp pcb */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("RRQ: Error creating PCB. Memory error\r\n");
		return ERR_MEM;
	}

	/* Bind to any free port */
	err = udp_bind(pcb, IP_ADDR_ANY, 0);
	if (err != ERR_OK) {
		xil_printf("RRQ: No free ports available; err %d\r\n", err);
		udp_remove(pcb);
		return err;
	}

	/* Register receive callback */
	udp_recv(pcb, (udp_recv_fn) tftp_client_response_recv_cb, NULL);

	return ERR_OK;
}
