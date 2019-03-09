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

/* webserver.c: An example Webserver application using the RAW API
 *	This program serves web pages resident on FAT File System using
 * lwIP's RAW API. Use of RAW API implies that the webserver is blazingly
 * fast, but the design is not obvious since a lot of the work happens in
 * asynchronous callback functions.
 *
 * The webserver works as follows:
 *	- on every accepted connection, only 1 read is performed to
 * identify the file requested. Further reads are avoided by sending
 * a "Connection: close" in the HTTP response header
 *	- the read determines what file needs to be set (by parsing
 * "GET / HTTP/1.1" request
 *	- once the file to be sent is determined, tcp_write is called
 * in chunks of size tcp_sndbuf() until the whole file is sent
 *
 */

#include "lwip/err.h"
#include "lwip/tcp.h"

#include "lwip_example_webserver.h"
#include "xil_printf.h"

extern void print_ip(char *msg, ip_addr_t *ip);
extern struct netif server_netif;

/* static variables controlling debug printf's in this file */
static int g_webserver_debug;

err_t http_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	int BUFSIZE = 1400, sndbuf, n;
	char buf[BUFSIZE];
	http_arg *a = (http_arg *)arg;

	if (g_webserver_debug)
		xil_printf("%d (%d): S%d..\r\n", a?a->count:0, tpcb->state, len);

	if (tpcb->state > ESTABLISHED) {
		if (a) {
			f_close(&a->fil);
			pfree_arg(a);
			a = NULL;
		}

		http_close(tpcb);
		return ERR_OK;
	}

	 /* no more data to be sent */
	if (a->fsize <= 0) {
		f_close(&a->fil);
		return ERR_OK;
	}

	/* read more data out of the file and send it */
	do {
		err_t err;
		sndbuf = tcp_sndbuf(tpcb);
		if (sndbuf < BUFSIZE)
			return ERR_OK;

		xil_printf("attempting to read %d bytes, left = %d bytes\r\n",
			   BUFSIZE, a->fsize);

		f_read(&a->fil, buf, BUFSIZE, (unsigned int *)&n);
		err = tcp_write(tpcb, buf, n, TCP_WRITE_FLAG_COPY);
		if (err != ERR_OK) {
			xil_printf("Failed to write data; aborting\r\n");
			http_close(tpcb);
			break;
		}

		a->fsize -= n;
	} while (a->fsize > 0);

	f_close(&a->fil);
	return ERR_OK;
}

err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	http_arg *a = (http_arg *)arg;

	if ((err != ERR_OK) || (p == NULL)) {
		http_close(tpcb);
		return ERR_OK;
	}

	if (g_webserver_debug)
		xil_printf("%d (%d): R%d %d..\r\n", a?a->count:0, tpcb->state, p->len, p->tot_len);

	/* do not read the packet if we are not in ESTABLISHED state */
	if (tpcb->state >= FIN_WAIT_1) {
		pbuf_free(p);
		return -1;
	}

	/* acknowledge that we've read the payload */
	tcp_recved(tpcb, p->len);

	/* read and decipher the request */
	/* this function takes care of generating a request, sending it,
	 * and closing the connection if all data can been sent. If
	 * not, then it sets up the appropriate arguments to the sent
	 * callback handler.
	 */
	generate_response(tpcb, p->payload, p->len);

	/* free received packet */
	pbuf_free(p);

	return ERR_OK;
}

static err_t http_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	/* keep a count of connection # */
	tcp_arg(newpcb, (void *)palloc_arg());

	tcp_recv(newpcb, http_recv_callback);
	tcp_sent(newpcb, http_sent_callback);

	return ERR_OK;
}

void start_application()
{
	struct tcp_pcb *pcb;
	err_t err;

	/* initialize file system layer */
	if (platform_init_fs()) {
		xil_printf("Can't run webserver as FS init failed\r\n");
		return;
	}

	/* create new TCP PCB structure */
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\r\n");
		return;
	}

	/* bind to http port 80 */
	err = tcp_bind(pcb, IP_ADDR_ANY, HTTP_PORT);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port 80: err = %d\r\n", err);
		return;
	}

	/* we do not need any arguments to the first callback */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\r\n");
		return;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, http_accept_callback);
}

void print_app_header()
{
	xil_printf("http server is running on port %d\r\n", HTTP_PORT);
	print_ip("Please point your web browser to http://", &server_netif.ip_addr);
	xil_printf("\r\n");
}
