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

#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "freertos_lwip_example_webserver.h"
#include "xil_printf.h"

extern void print_ip(char *msg, ip_addr_t *ip);

extern struct netif server_netif;

void process_http_request(int sd)
{
	int read_len;
	int RECV_BUF_SIZE = 1400;  /* http request size can be a max of RECV_BUF_SIZE */
	char recv_buf[RECV_BUF_SIZE];

	/* read in the request */
	if ((read_len = read(sd, recv_buf, RECV_BUF_SIZE)) <= 0) {
		close(sd);
		vTaskDelete(NULL);
		return;
	}

	/* respond to request */
	generate_response(sd, recv_buf, read_len);

	/* close connection */
	close(sd);
}

void start_application()
{
	int sock, new_sd;
	int size = sizeof(struct sockaddr_in);
	struct sockaddr_in address, remote;

	/* initialize FS */
	if (platform_init_fs()) {
		xil_printf("Can't run webserver as FS init failed\r\n");
		return;
	}

	/* create a TCP socket */
	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("Failed to create socket\r\n");
		return;
	}

	/* bind to port 80 at any interface */
	address.sin_family = AF_INET;
	address.sin_port = htons(HTTP_PORT);
	address.sin_addr.s_addr = INADDR_ANY;
	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		xil_printf("Failed to bind at port '%d'\r\n", HTTP_PORT);
		return;
	}

	/* listen for incoming connections */
	if (lwip_listen(sock, 0)) {
		xil_printf("Failed to listen\r\n");
		return;
	}

	while (1) {
		new_sd = lwip_accept(sock, (struct sockaddr *)&remote,
					(socklen_t *)&size);
		process_http_request(new_sd);
	}
}

void print_app_header()
{
	xil_printf("http server is running on port %d\r\n", HTTP_PORT);
	print_ip("Please point your web browser to http://",
		&server_netif.ip_addr);
	xil_printf("\r\n");
}
