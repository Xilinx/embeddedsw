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
#include "lwip/udp.h"
#include "lwip_example_tftpserver_common.h"
#include "xil_printf.h"

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

static err_t tftp_send_packet(struct udp_pcb *pcb, ip_addr_t *addr,
		int port, char *buf, int buflen)
{
	err_t err;
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, buflen, PBUF_POOL);
	if (!p) {
		xil_printf("Error allocating pbuf\r\n");
		return ERR_MEM;
	}

	memcpy(p->payload, buf, buflen);

	/* send packet */
	err = udp_sendto(pcb, p, addr, port);
	if (err != ERR_OK)
		xil_printf("UDP send error\r\n");

	/* free pbuf */
	pbuf_free(p);

	return err;
}

/* construct and send an error packet to client */
static int tftp_send_error_packet(struct udp_pcb *pcb, ip_addr_t *addr,
		int port, tftp_errorcode err)
{
	char buf[TFTP_MAX_ERR_MSG_LEN] = {0};
	int len;

	tftp_set_opcode(buf, TFTP_ERROR);
	tftp_set_errcode(buf, err);
	tftp_set_errmsg(buf, tftp_errorcode_string[err]);

	/* total packet length */
	len = TFTP_PACKET_HDR_LEN + strlen(tftp_errorcode_string[err]) + 1;

	return tftp_send_packet(pcb, addr, port, buf, len);
}

/* construct and send a data packet */
static int tftp_send_data_packet(struct udp_pcb *pcb, ip_addr_t *to,
		int to_port, int block, char *buf, int buflen)
{
	char packet[TFTP_MAX_MSG_LEN] = {0};

	tftp_set_opcode(packet, TFTP_DATA);
	tftp_set_block_value(packet, block);
	tftp_set_data(packet, buf, buflen);

	return tftp_send_packet(pcb, to, to_port, packet,
			buflen + TFTP_PACKET_HDR_LEN);
}

static int tftp_send_ack_packet(struct udp_pcb *pcb, ip_addr_t *to,
		int to_port, int block)
{
	char packet[TFTP_MAX_ACK_LEN] = {0};

	tftp_set_opcode(packet, TFTP_ACK);
	tftp_set_block_value(packet, block);

	return tftp_send_packet(pcb, to, to_port, packet, TFTP_MAX_ACK_LEN);
}

static void tftp_cleanup(struct udp_pcb *pcb, tftp_connection_args *args)
{
	/* cleanup the args */
	f_close(&args->fil);
	mem_free(args);

	/* close the connection */
	udp_remove(pcb);
}

static void tftp_send_next_block(struct udp_pcb *pcb,
		tftp_connection_args *args, ip_addr_t *ip, u16_t port)
{
	FRESULT Res;

	Res = f_read(&args->fil, args->data, TFTP_DATA_PACKET_MSG_LEN,
			&args->data_len);
	if (Res) {
		xil_printf("closing connection, err: %d\r\n", args->data_len);
		/* we are done */
		return tftp_cleanup(pcb, args);
	}

	/* send the data */
	tftp_send_data_packet(pcb, ip, port, args->block, args->data,
			args->data_len);

}

static void tftp_server_read_req_recv_cb(void *_args, struct udp_pcb *upcb,
		struct pbuf *p, ip_addr_t *addr, u16_t port)
{
	tftp_connection_args *args = (tftp_connection_args *)_args;

	if ((tftp_get_opcode(p->payload) == TFTP_ACK) &&
		(args->block == tftp_get_block_value(p->payload))) {
		/* increment block # */
		args->block++;
	} else {
		/* we did not receive the expected ACK, so do not update
		 * block #, resend current block */
		xil_printf("TFTP_RRQ: Incorrect ack received, resend\r\n");
		tftp_send_data_packet(upcb, addr, port, args->block, args->data,
				args->data_len);
		pbuf_free(p);
		return;
	}

	pbuf_free(p);

	/* if the last read returned less than the requested number of bytes,
	 * then we've sent the whole file and so we can quit
	 */
	if (args->data_len < TFTP_DATA_PACKET_MSG_LEN) {
		xil_printf("TFTP_RRQ: Transfer completed\r\n\r\n");
		return tftp_cleanup(upcb, args);
	}

	tftp_send_next_block(upcb, args, addr, port);
}

static int tftp_process_read(struct udp_pcb *pcb, ip_addr_t *ip,
		int port, char *fname)
{
	tftp_connection_args *conn;
	FIL r_fil;
	FRESULT Res;

	Res = f_open(&r_fil, fname, FA_READ);
	if (Res) {
		xil_printf("Unable to open file: %s\r\n", fname);
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_FILE_NOT_FOUND);
		udp_remove(pcb);
		return -1;
	}

	conn = mem_malloc(sizeof *conn);
	if (!conn) {
		xil_printf("Unable to allocate memory for tftp conn\r\n");
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_DISKFULL);
		udp_remove(pcb);
		f_close(&r_fil);
		return -1;
	}

	memcpy(&conn->fil, &r_fil, sizeof(r_fil));

	/* set callback for receives on this pcb */
	udp_recv(pcb, (udp_recv_fn) tftp_server_read_req_recv_cb, conn);

	/* initiate the transaction by sending the first block of data
	 * further blocks will be sent when ACKs are received
	 */
	conn->block = 1;

	tftp_send_next_block(pcb, conn, ip, port);

	return 0;
}

/* write callback */
static void tftp_server_write_req_recv_cb(void *_args, struct udp_pcb *upcb,
		struct pbuf *p, ip_addr_t *addr, u16_t port)
{
	ip_addr_t ip = *addr;
	tftp_connection_args *args = (tftp_connection_args *)_args;

	if (p->len != p->tot_len) {
		xil_printf("TFTP_WRQ: Tftp server does not support "
				"chained pbufs\r\n");
		pbuf_free(p);
		return;
	}

	/* make sure data block is what we expect */
	if ((p->len >= TFTP_PACKET_HDR_LEN) &&
		(tftp_get_block_value(p->payload) ==
		 (u16_t) (args->block + 1))) {

		/* write the received data to the file */
		unsigned int n;
		f_write(&args->fil, p->payload + TFTP_PACKET_HDR_LEN,
				p->len - TFTP_PACKET_HDR_LEN, &n);
		if (n != p->len - TFTP_PACKET_HDR_LEN) {
			xil_printf("TFTP_WRQ: Write to file error\r\n");
			tftp_send_error_packet(upcb, &ip, port,
						TFTP_ERR_DISKFULL);
			pbuf_free(p);
			return tftp_cleanup(upcb, args);
		}
		args->block++;
	}

	tftp_send_ack_packet(upcb, &ip, port, args->block);

	/* if the last read returned less than the requested number of bytes,
	 * then we've sent the whole file and so we can quit
	 */
	if (p->len < TFTP_DATA_PACKET_MSG_LEN) {
		xil_printf("TFTP_WRQ: Transfer completed\r\n\r\n");
		return tftp_cleanup(upcb, args);
	}

	pbuf_free(p);
}

/* write the file in fs */
static int tftp_process_write(struct udp_pcb *pcb, ip_addr_t *ip, int port,
		char *fname)
{
	tftp_connection_args *conn;
	FIL w_fil;
	FRESULT Res;

	Res = f_open(&w_fil, fname, FA_CREATE_ALWAYS | FA_WRITE);
	if (Res) {
		xil_printf("Unable to open file %s for writing %d\r\n", fname,
			   Res);
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_DISKFULL);
		udp_remove(pcb);
		return -1;
	}

	conn = mem_malloc(sizeof *conn);
	if (!conn) {
		xil_printf("Unable to allocate memory for tftp conn\r\n");
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_DISKFULL);
		udp_remove(pcb);
		return -1;
	}

	memcpy(&conn->fil, &w_fil, sizeof(w_fil));
	conn->block = 0;

	/* set callback for receives on this pcb */
	udp_recv(pcb, (udp_recv_fn) tftp_server_write_req_recv_cb, conn);

	/* initiate the transaction by sending the first ack */
	tftp_send_ack_packet(pcb, ip, port, conn->block);

	return 0;
}

static void tftp_server_recv_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		ip_addr_t *ip, u16_t port)
{
	tftp_opcode op = tftp_get_opcode(p->payload);
	char fname[512];
	struct udp_pcb *pcb;
	err_t err;

	pcb = udp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\r\n");
		goto cleanup;
	}

	/* bind to port 0 to receive next available free port */
	err = udp_bind(pcb, IP_ADDR_ANY, 0);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d; err %d\r\n", port, err);
		goto cleanup;
	}

	switch (op) {
	case TFTP_RRQ:
		/* get file name from request payload */
		strcpy(fname, p->payload + FIL_NAME_OFFSET);
		printf("TFTP RRQ (read request): %s\r\n", fname);
		tftp_process_read(pcb, ip, port, fname);
		break;
	case TFTP_WRQ:
		/* get file name from request payload */
		strcpy(fname, p->payload + FIL_NAME_OFFSET);
		printf("TFTP WRQ (write request): %s\r\n", fname);
		tftp_process_write(pcb, ip, port, fname);
		break;
	default:
		/* send a generic access violation message */
		tftp_send_error_packet(pcb, ip, port, TFTP_ERR_ILLEGALOP);
		printf("TFTP unknown request op: %d\r\n\r\n", op);
		udp_remove(pcb);
		break;
	}

cleanup:
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

	data_len = strlen(TFTP_TEST_FIL_DATA);
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

void start_application()
{
	struct udp_pcb *pcb;
	err_t err;

	/* create test file for 'tftp get' request from client */
	err = tftp_create_test_file();
	if (err) {
		xil_printf("Unable to create test file\r\n");
		return;
	}

	/* create new UDP PCB structure */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\r\n");
		return;
	}

	/* bind to port */
	err = udp_bind(pcb, IP_ADDR_ANY, TFTP_PORT);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d; err %d\r\n",
				TFTP_PORT, err);
		udp_remove(pcb);
		return;
	}

	udp_recv(pcb, (udp_recv_fn) tftp_server_recv_cb, NULL);
}
