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

#include <string.h>

#include "lwip/sockets.h"
#include "xil_printf.h"
#include "freertos_lwip_example_tftpclient_common.h"
#include "lwip/sys.h"

#define FILE_TO_TRANSFER 3

#define TFTP_TEST_FIL_DATA \
	"----- This is a test file for TFTP client application -----"

extern struct netif server_netif;

void print_app_header()
{
	xil_printf("\r\nTftp client app gets 3 files(sample#.txt) from host"
			" and put 3 files(sample#.txt) on host.\r\n"
			"Make sure host(%s) is running tftp daemon app\r\n\r\n",
			TFTP_SERVER_IP);
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

static void tftp_prepare_request(char *buf, char *file, tftp_opcode op,
		char *mode)
{
	u16_t offset;

	tftp_set_opcode(buf, op);
	tftp_set_filename(buf, file);

	offset = FIL_NAME_OFFSET + strlen(file) + 1;
	tftp_set_mode(buf, mode, offset);
}

static void tftp_handle_write_req_resp(int sd, char *fname)
{
	int rcv_len;
	char buf[TFTP_DATA_PACKET_MSG_LEN];
	FIL fil;
	FRESULT Res;
	u32_t len;
	tftp_opcode op;
	u16_t block = 0;
	u8_t retries = 0;
	tftp_errorcode errcode;
	err_t err;
	u8_t last_pkt_sent = 0;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	/* Open file to write; Overwrite the file if already exists */
	Res = f_open(&fil, fname, FA_READ);
	if (Res) {
		xil_printf("TFTP_WRQ: Unable to open file to read");
		return;
	}

	/* Receive packets from server */
	while (1) {
		/* Wait for the response from server */
		rcv_len = recvfrom(sd, buf, TFTP_MAX_ACK_LEN, 0,
				(struct sockaddr *)&from, &fromlen);
		if (rcv_len <= 0) {
			xil_printf("TFTP_WRQ: receive error\r\n");
			f_close(&fil);
			return;
		}

		/* Verify the opcode in packet */
		op = tftp_get_opcode(buf);

		switch (op) {
		case TFTP_ACK:
			/* Verify block number */
			if (tftp_get_block_value(buf) != block) {
				if (retries++ < TFTP_MAX_RETRY_COUNT) {
					/* Send previous packet */
					err = tftp_send_data_packet(sd, &from,
							block, buf, len);
					if (err < 0) {
						f_close(&fil);
						return;
					}

					continue;
				}
				xil_printf("TFTP_WRQ: Max retry done\r\n");
				f_close(&fil);
				return;
			}

			/* If ack is of the last packet, display the stats */
			if (last_pkt_sent) {
				xil_printf("TFTP_WRQ: Transfer completed\r\n\r\n");
				f_close(&fil);
				return;
			}
			/* Write data to file */
			Res = f_read(&fil, buf, TFTP_DATA_PACKET_MSG_LEN, &len);
			if (Res) {
				xil_printf("TFTP_WRQ: File read error\r\n");
				tftp_send_error_packet(sd, &from,
						TFTP_ERR_ACCESS_VIOLATION);
				f_close(&fil);
				return;
			}

			/* Send ACK packet */
			err = tftp_send_data_packet(sd, &from, ++block, buf,
							len);
			if (err < 0) {
				f_close(&fil);
				return;
			}

			/* If received packet length is less than maximum data
			 * packet length then we have received whole file */
			if (len < TFTP_DATA_PACKET_MSG_LEN)
				last_pkt_sent++;

			break;
		case TFTP_ERROR:
			/* Print the received error message */
			errcode = tftp_get_errcode(buf);
			xil_printf("TFTP_WRQ: Error code %d: %s\r\n\r\n", errcode,
					tftp_errorcode_string[errcode]);
			f_close(&fil);
			return;
		default:
			/* Send error packet */
			tftp_send_error_packet(sd, &from,
					TFTP_ERR_ACCESS_VIOLATION);
			f_close(&fil);
			return;
		}
		/* Set retries to 0 for new packet */
		retries = 0;
	}
}

static void tftp_handle_read_req_resp(int sd, char *fname)
{
	int rcv_len;
	char buf[TFTP_DATA_PACKET_LEN];
	FIL fil;
	FRESULT Res;
	u32_t len;
	tftp_opcode op;
	u16_t block = 1;
	u8_t retries = 0;
	tftp_errorcode errcode;
	err_t err;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	/* Open file to write; Overwrite the file if already exists */
	Res = f_open(&fil, fname, FA_CREATE_ALWAYS | FA_WRITE);
	if (Res) {
		xil_printf("TFTP_RRQ: Unable to open file to write");
		return;
	}

	/* Receive packets from server */
	while (1) {
		/* Wait for the response from server */
		rcv_len = recvfrom(sd, buf, TFTP_DATA_PACKET_LEN, 0,
				(struct sockaddr *)&from, &fromlen);
		if (rcv_len <= 0) {
			xil_printf("TFTP_RRQ: receive error\r\n");
			f_close(&fil);
			return;
		}

		/* Verify the opcode in packet */
		op = tftp_get_opcode(buf);

		switch (op) {
		case TFTP_DATA:
			/* Verify block number */
			if (tftp_get_block_value(buf) != block) {
				if (retries++ < TFTP_MAX_RETRY_COUNT) {
					/* Send ACK packet for last block */
					err = tftp_send_ack_packet(sd, &from,
							(u16_t) block - 1);
					if (err < 0) {
						f_close(&fil);
						return;
					}

					continue;
				}

				xil_printf("TFTP_WRQ: Max retry done\r\n");
				f_close(&fil);
				return;
			}

			/* Write data to file */
			Res = f_write(&fil, buf + TFTP_PACKET_HDR_LEN,
					rcv_len - TFTP_PACKET_HDR_LEN,
					&len);
			if (Res ||
				(len != (rcv_len - TFTP_PACKET_HDR_LEN))) {
				xil_printf("TFTP_RRQ: File write error\r\n");
				tftp_send_error_packet(sd, &from,
						TFTP_ERR_DISKFULL);
				f_close(&fil);
				return;
			}

			/* Send ACK packet */
			err = tftp_send_ack_packet(sd, &from, block++);
			if (err < 0) {
				f_close(&fil);
				return;
			}

			/* If received packet length is less than maximum data
			 * packet length then we have received whole file */
			if (rcv_len < TFTP_DATA_PACKET_LEN) {
				xil_printf("TFTP_RRQ: Transfer completed\r\n\r\n");
				f_close(&fil);
				return;
			}

			break;
		case TFTP_ERROR:
			/* Print the received error message */
			errcode = tftp_get_errcode(buf);
			xil_printf("TFTP_RRQ: Error code %d: %s\r\n\r\n",
					errcode,
					tftp_errorcode_string[errcode]);
			f_close(&fil);
			return;
		default:
			/* Send error packet */
			tftp_send_error_packet(sd, &from,
					TFTP_ERR_ACCESS_VIOLATION);
			f_close(&fil);
			return;
		}
		/* Set retries to 0 for new packet */
		retries = 0;
	}
}

static void tftp_transfer_client_data(int sd)
{
	err_t err;
	char *buf;
	char fname[20];
	char *mode = "netascii";
	u16_t len;
	u8_t file_count = 0;
	struct sockaddr_in server;
	tftp_opcode op = TFTP_WRQ;

	/* Set server address */
	server.sin_family = AF_INET;
	err = inet_aton(TFTP_SERVER_IP, &(server.sin_addr.s_addr));
	if (!err) {
		xil_printf("Invalid server ip address: %s\r\n", TFTP_SERVER_IP);
		return;
	}
	server.sin_port = htons(TFTP_PORT);

	/* Prepare "tftp put" request and send to tftp server */
	do {
		/* Put file(sample#.txt) request */
		sprintf(fname, "sample%d.txt", ++file_count);

		/* len = opcode(2) + filename + '\0' + mode + '\0' */
		len = 2 + strlen(fname) + strlen(mode) + 2;

		buf = (char *) mem_malloc(len);
		if (!buf) {
			xil_printf("Failed to allocate request buf\r\n");
			return;
		}

		tftp_prepare_request(buf, fname, op, mode);

		tftp_send_packet(sd, &server, buf, len);

		mem_free(buf);

		if (op == TFTP_WRQ) {
			xil_printf("TFTP_WRQ: tftp put %s\r\n", fname);
			tftp_handle_write_req_resp(sd, fname);

			if (file_count == FILE_TO_TRANSFER) {
				file_count = 0;
				op = TFTP_RRQ;
			}
		} else {
			xil_printf("TFTP_RRQ: tftp get %s\r\n", fname);
			tftp_handle_read_req_resp(sd, fname);
		}
	} while (file_count < FILE_TO_TRANSFER);
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

	return 0;
}

static void init_tftp_client()
{
	int sd;
	struct sockaddr_in addr;
	int ret;

	/* Create files(sample#.txt) in file system */
	ret = tftp_create_test_file();
	if (ret < 0) {
		return;
	}

	/* Create UDP socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		xil_printf("Failed to create socket; err: %d\r\n", sd);
		vTaskDelete(NULL);
		return;
	}

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(0);

	if (bind(sd, (struct sockaddr *)&addr, sizeof addr) < 0)  {
		xil_printf("Failed to bind\r\n");
		goto cleanup;
	}

	/* Send request to server */
	tftp_transfer_client_data(sd);

cleanup:
	close(sd);
	vTaskDelete(NULL);
}

void start_application()
{
	sys_thread_new("tftpc", init_tftp_client, 0, TFTP_THREAD_STACKSIZE,
			DEFAULT_THREAD_PRIO);
}
