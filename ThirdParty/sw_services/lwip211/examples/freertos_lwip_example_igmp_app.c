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

#include "freertos_lwip_example_igmp_app.h"
#include "lwipopts.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "errno.h"
#include "xil_printf.h"

void print_app_header()
{
	xil_printf("\n\r----- IGMP test application ------\n\r");
	xil_printf("IGMP application listening to Multicast IP %s ",
			IGMP_MULTICAST_IP_ADDRESS);
	xil_printf(" on port %d\r\n", IGMP_MULTICAST_PORT);
	xil_printf("On Host: Run $ iperf -c %s -i 1 -t 2 -u -b 1M -B"
			" <Host IP address>\n\r\n", IGMP_MULTICAST_IP_ADDRESS);
}

void igmp_app_thread()
{
	err_t err;
	int sock, n;
	static int rxpkt = 0;
	struct sockaddr_in local_addr, send_addr;
	struct ip_mreq mreq;
	char recv_buf[BUF_SIZE];

	if ((sock = lwip_socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		xil_printf("igmp_app: Error creating Socket\r\n");
		vTaskDelete(NULL);
		return;
	}

	memset(&local_addr, 0, sizeof(struct sockaddr_in));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(IGMP_MULTICAST_PORT);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Bind socket to a particular Multicast IP and port */
	err = bind(sock, (struct sockaddr *)&local_addr, sizeof (local_addr));
	if (err != ERR_OK) {
		xil_printf("igmp_app: Unable to bind to multicast port %d:",
				IGMP_MULTICAST_PORT);
		xil_printf(" err %d\r\n", err);
		goto cleanup;
	}

	/* IGMP join group for Multicast IP */
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(IGMP_MULTICAST_IP_ADDRESS);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
		sizeof(mreq)) < 0) {
		xil_printf("igmp_app: setsockopt to add multicast \
				group failed\r\n");
		goto cleanup;
	}
	xil_printf("IGMP application joined group : %s\n\r",
			IGMP_MULTICAST_IP_ADDRESS);

	while (rxpkt != IGMP_MULTICAST_RXPKT_COUNT) {
		if((n = recv(sock, recv_buf, BUF_SIZE - 1, 0)) <= 0) {
			xil_printf("igmp_app: Invalid packet received\r\n");
			goto leave_group_cleanup;
		}

		rxpkt++;
	}
	xil_printf("Received %d Multicast packets\n\r",
			IGMP_MULTICAST_RXPKT_COUNT);

	/* send the received Multicast packet back to
	 * network to check transmit path
	 */
	memset((void*) &send_addr, 0, sizeof(send_addr));
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(IGMP_MULTICAST_PORT);
	send_addr.sin_addr.s_addr = inet_addr(IGMP_MULTICAST_IP_ADDRESS);

	if ((n = sendto(sock, recv_buf, n, 0, (struct sockaddr *)&send_addr,
		sizeof (send_addr))) < 0) {
		xil_printf("Failed to send Multicast packet, err: %d\n\r",err);
		xil_printf("IGMP application test failed\n\r");
	} else {
		xil_printf("Multicast packet sent\n\r");
		xil_printf("IGMP application test passed\n\r");
	}

leave_group_cleanup:
	/* We are done with RX multicast packet or haven't received proper pkt
	 * so, we should leave this Multicast group
	 */
	err = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq,
			sizeof(mreq));
	if (err < 0)
		xil_printf("setsockopt IP_DROP_MEMBERSHIP failed with error:"
			"%d\n\r", err);
	else
		xil_printf("IGMP application left group : %s\n\r",
			IGMP_MULTICAST_IP_ADDRESS);

cleanup:
	close(sock);
	vTaskDelete(NULL);

	return;
}

void start_application()
{
	sys_thread_new("igmp_app_thread", igmp_app_thread, 0,
			IGMP_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
	return;
}
