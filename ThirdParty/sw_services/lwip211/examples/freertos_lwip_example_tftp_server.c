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
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "xil_printf.h"
#include "freertos_lwip_example_tftpserver_common.h"
#include "lwip/sys.h"
#include "FreeRTOS.h"

#define FILES_TO_CREATE 3

#define TFTP_TEST_FIL_DATA \
	"----- This is a test file for TFTP server application -----"

extern struct netif server_netif;

void print_app_header()
{
        xil_printf("\r\n%20s %6d $ tftp -i %s PUT/GET <source-file>\r\n",
                        "Tftp server", TFTP_PORT,
                        inet_ntoa(server_netif.ip_addr));
}

static err_t tftp_send_packet(int sd, struct sockaddr_in *to, char *buf,
		int buflen)
{
	err_t err;

	err = sendto(sd, buf, buflen, 0, (struct sockaddr *)to, sizeof *to);
	if (err < 0)
		xil_printf("socket: send error\r\n");

	return err;
}

/* construct and send an error packet to client */
static int tftp_send_error_packet(int sd, struct sockaddr_in *to,
		tftp_errorcode err)
{
	char buf[TFTP_MAX_ERR_MSG_LEN] = {0};
	/* total packet length */
	int len = TFTP_PACKET_HDR_LEN + strlen(tftp_errorcode_string[err]) + 1;

	tftp_set_opcode(buf, TFTP_ERROR);
	tftp_set_errcode(buf, err);
	tftp_set_errmsg(buf, tftp_errorcode_string[err]);

	return tftp_send_packet(sd, to, buf, len);
}

/* construct and send a data packet */
static int tftp_send_data_packet(int sd, struct sockaddr_in *to, int block,
		char *buf, int buflen)
{
	char packet[TFTP_MAX_MSG_LEN] = {0};

	tftp_set_opcode(packet, TFTP_DATA);
	tftp_set_block_value(packet, block);
	tftp_set_data(packet, buf, buflen);

	return tftp_send_packet(sd, to, packet, buflen + TFTP_PACKET_HDR_LEN);
}

static int tftp_send_ack_packet(int sd, struct sockaddr_in *to, int block)
{
	char packet[TFTP_MAX_ACK_LEN] = {0};

	tftp_set_opcode(packet, TFTP_ACK);
	tftp_set_block_value(packet, block);

	return tftp_send_packet(sd, to, packet, TFTP_MAX_ACK_LEN);
}

static int tftp_read_process(int sd, struct sockaddr_in *to, char *fname)
{
	FIL r_fil;
	FRESULT Res;
	u16_t block = 1;
	err_t err;
	tftp_errorcode errcode;

	Res = f_open(&r_fil, fname, FA_READ);
	if (Res) {
		xil_printf("TFTP_RRQ: Unable to open file: %s\r\n", fname);
		tftp_send_error_packet(sd, to, TFTP_ERR_FILE_NOT_FOUND);
		return -1;
	}

	while (1) {
		char buf[TFTP_DATA_PACKET_LEN];
		char ack_pkt[TFTP_MAX_ACK_LEN + 1];
		u32_t len;
		u8 retry_data = 0;
		u16_t recv_len;

		/* Read data and send packet */
		Res = f_read(&r_fil, buf, TFTP_DATA_PACKET_MSG_LEN, &len);
		if (Res) {
			xil_printf("TFTP_RRQ: File read error\r\n");
			break;
		}

send_again:
		err = tftp_send_data_packet(sd, to, block, buf, len);
		if (err < 0)
			break;

		/* Wait for the ack packet */
		recv_len = recv(sd, ack_pkt, TFTP_MAX_ACK_LEN, 0);
		if (recv_len <= 0) {
			xil_printf("TFTP_RRQ: socket recv error\r\n");
			break;
		}

		/* Check for error response */
		if (tftp_get_opcode(ack_pkt) == TFTP_ERROR) {
			errcode = tftp_get_errcode(ack_pkt);
			xil_printf("TFTP_RRQ: Error code %d: %s", errcode,
					tftp_errorcode_string[errcode]);
			break;
		}

		/* Check for invalid opcode */
		if (tftp_get_opcode(ack_pkt) != TFTP_ACK) {
			xil_printf("TFTP_RRQ: Invalid opcode\r\n");
			tftp_send_error_packet(sd, to,
					TFTP_ERR_ACCESS_VIOLATION);
			break;
		}

		if ((recv_len == TFTP_MAX_ACK_LEN) &&
				(tftp_get_block_value(ack_pkt) == block)) {
			block++;
		} else {
			if (retry_data++ < TFTP_MAX_RETRY_COUNT) {
				goto send_again;
			} else {
				xil_printf("TFTP_RRQ: Max retry done\r\n");
				break;
			}
		}

		if (len < TFTP_DATA_PACKET_MSG_LEN) {
			xil_printf("TFTP_RRQ: Transfer completed\r\n\r\n");
			break;
		}
	}

	f_close(&r_fil);
	return 0;
}

/* write the file in fs */
static int tftp_write_process(int sd, struct sockaddr_in *to, char *fname)
{
	FIL w_fil;
	FRESULT Res;
	u16 block = 0;
	u16_t data_len;
	u32_t len;
	err_t err;
	tftp_errorcode errcode;
	u8_t retries = 0;

	Res = f_open(&w_fil, fname, FA_CREATE_ALWAYS | FA_WRITE);
	if (Res) {
		xil_printf("Unable to open file %s for writing %d\r\n", fname,
				Res);
		tftp_send_error_packet(sd, to, TFTP_ERR_DISKFULL);
		return -1;
	}

	/* Send ack packet with block 0; Notify client send data packets */
	err= tftp_send_ack_packet(sd, to, block);
	if (err < 0) {
		f_close(&w_fil);
		return -1;
	}


	/* Wait for the data packets;
	 * on receiving data packet, write data to file and send ack packets */
	while (1) {
		char buf[TFTP_DATA_PACKET_LEN];

		data_len = recv(sd, buf, TFTP_DATA_PACKET_LEN, 0);
		if (data_len <= 0) {
			xil_printf("TFTP_WRQ: socket rcv error\r\n");
			break;
		}

		/* Check for error response */
		if (tftp_get_opcode(buf) == TFTP_ERROR) {
			errcode = tftp_get_errcode(buf);
			xil_printf("TFTP_WRQ: Error code %d: %s\r\n", errcode,
					tftp_errorcode_string[errcode]);
			break;
		}

		/* Check for invalid opcode */
		if (tftp_get_opcode(buf) != TFTP_DATA) {
			xil_printf("TFTP_WRQ: Invalid opcode\r\n");
			tftp_send_error_packet(sd, to,
					TFTP_ERR_ACCESS_VIOLATION);
			break;
		}

		/* Validate the packet */
		if ((data_len < TFTP_MIN_DATA_PACKET_LEN) ||
			tftp_get_block_value(buf) != (u16_t) (block + 1)) {
			if (retries++ < TFTP_MAX_RETRY_COUNT) {
				/* send previous block ack again */
				err = tftp_send_ack_packet(sd, to, block);
				if (err < 0)
					break;

				continue;
			}
			xil_printf("TFTP_WRQ: Max retry done\r\n");
			break;
		}

		/* Write data to file */
		Res = f_write(&w_fil, buf + TFTP_PACKET_HDR_LEN,
				data_len - TFTP_PACKET_HDR_LEN, &len);
		if (Res || (len != data_len - TFTP_PACKET_HDR_LEN)) {
			xil_printf("TFTP_WRQ: File write error\r\n");
			tftp_send_error_packet(sd, to, TFTP_ERR_DISKFULL);
			break;
		}

		err = tftp_send_ack_packet(sd, to, ++block);
		if (err < 0)
			break;

		if (len < TFTP_DATA_PACKET_MSG_LEN) {
			xil_printf("TFTP_WRQ: Transfer completed \r\n\r\n");
			break;
		}
		/* Set retries to 0 for new packet */
		retries = 0;
	}

	f_close(&w_fil);
	return 0;
}

static void tftp_request_process(void *_arg)
{
	tftp_connection_args *arg = (tftp_connection_args *)_arg;
	tftp_opcode op = tftp_get_opcode(arg->data);
	char fname[512];
	int sd;
	struct sockaddr_in cli;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		xil_printf("TFTP_REQ: Failed to create socket; err: %d\r\n",
				sd);
		mem_free(arg);
		return;
	}

	memset(&cli, 0, sizeof cli);
	cli.sin_family = AF_INET;
	cli.sin_addr.s_addr = INADDR_ANY;
	cli.sin_port = 0;

	/* bind to port 0 to receive next available free port */
	if (bind(sd, (struct sockaddr *)&cli, sizeof cli) < 0)  {
		xil_printf("TFTP_REQ: Unable to bind\r\n");
		goto cleanup;
	}

	switch (op) {
	case TFTP_RRQ:
		/* get file name from request payload */
		strcpy(fname, arg->data + FIL_NAME_OFFSET);
		printf("TFTP RRQ (read request): %s\r\n", fname);
		tftp_read_process(sd, &arg->from, fname);
		break;
	case TFTP_WRQ:
		/* get file name from request payload */
		strcpy(fname, arg->data + FIL_NAME_OFFSET);
		printf("TFTP WRQ (write request): %s\r\n", fname);
		tftp_write_process(sd, &arg->from, fname);
		break;
	default:
		/* send a generic access violation message */
		tftp_send_error_packet(sd, &arg->from,
				TFTP_ERR_ACCESS_VIOLATION);
		printf("TFTP unknown request op: %d\r\n", op);
		break;
	}

cleanup:
	close(sd);
	mem_free(arg);
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
	} while (file_count <= FILES_TO_CREATE);

	return 0;
}

static void init_tftp_server()
{
	int sd;
	struct sockaddr_in server;
	int len, ret;

	/* create test file for 'tftp get' request from client */
	ret = tftp_create_test_file();
	if (ret) {
		xil_printf("Unable to create test file\r\n");
		return;
	}

	/* Create UDP socket to listen tftp client requests */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		xil_printf("Failed to create socket; err: %d\r\n", sd);
		vTaskDelete(NULL);
		return;
	}

	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(TFTP_PORT);

	if (bind(sd, (struct sockaddr *)&server, sizeof server) < 0)  {
		xil_printf("Failed to bind at port %d\r\n", TFTP_PORT);
		goto cleanup;
	}

	while (1) {
		tftp_connection_args *args;

		/* create a new args structure for every incoming request.
		 * This args structure is passed on to the child thread which is
		 * expected to free this memory */
		args = (tftp_connection_args *) mem_malloc(sizeof *args);
		if (args == NULL) {
			xil_printf("Error allocating memory for connection "
					"arguments\r\n");
			goto cleanup;
		}

		/* Wait for a connection request */
		ret = recvfrom(sd, &args->data, TFTP_MAX_MSG_LEN, 0,
				(struct sockaddr *)&args->from,
				(socklen_t *)&len);
		if (ret < 0) {
			xil_printf("Error in recvfrom(); err: %d\r\n", ret);
			goto cleanup;
		}

		/* serve the client */
		tftp_request_process(args);
	}

cleanup:
	close(sd);
	vTaskDelete(NULL);
}

void start_application()
{
	sys_thread_new("tftpd", init_tftp_server, 0, TFTP_THREAD_STACKSIZE,
			DEFAULT_THREAD_PRIO);
}
