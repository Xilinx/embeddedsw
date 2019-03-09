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

#include "lwip_example_igmp_app.h"

#include "lwipopts.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "errno.h"
#include "xil_printf.h"

static ip_addr_t multicast_ip;

void print_app_header(void)
{
	xil_printf("\n\r----- IGMP test application ------\n\r");
	xil_printf("IGMP application listening to Multicast IP %s ",
			IGMP_MULTICAST_IP_ADDRESS);
	xil_printf("on port %d\r\n", IGMP_MULTICAST_PORT);
	xil_printf("On Host: Run $ iperf -c %s -t 2 -u -b 1M -B ",
			IGMP_MULTICAST_IP_ADDRESS);
	xil_printf("<Host IP address>\n\r\n");
}

void udp_multicast_recv(void *arg, struct udp_pcb *tpcb,
		struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	err_t err;
	static u32_t rxpkts;

	if (tpcb == NULL) {
		xil_printf("igmp_app: NULL pcb\n\r");
		return;
	}

	if (p == NULL) {
		xil_printf("igmp_app: Received NULL pbuf\n\r");
		return;
	}
	rxpkts++;

	if (rxpkts == IGMP_MULTICAST_RXPKT_COUNT) {

		xil_printf("Received %d Multicast packets\n\r", rxpkts);

		/* send the received Multicast packet back to
		 * network to check transmit path
		 */
		err = udp_sendto(tpcb, p, &multicast_ip, IGMP_MULTICAST_PORT);
		if (err != ERR_OK) {
			xil_printf("Failed to send Multicast packet, ");
			xil_printf("err: %d\n\r", err);
			xil_printf("IGMP application test failed\n\r");
		} else {
			xil_printf("Multicast packet sent\n\r");
			xil_printf("IGMP application test passed\n\r");
		}

		/* We are done with RX multicast packet,
		 * so, we should leave this Multicast group */
		err = igmp_leavegroup(IP_ADDR_ANY,
				(ip_addr_t *) (&multicast_ip));
		if (err != ERR_OK) {
			xil_printf("igmp_leavegroup failed with error: %d\n\r",
					err);
		} else
			xil_printf("IGMP application left group : %s\n\r",
					IGMP_MULTICAST_IP_ADDRESS);

	}
	/* Free received packet here */
	pbuf_free(p);
	return;
}

void start_application(void)
{
	struct udp_pcb *pcb = NULL;
	err_t err;

	err = inet_aton(IGMP_MULTICAST_IP_ADDRESS, &multicast_ip);
	if (!err) {
		xil_printf("igmp_app: Invalid Server IP address: %d\r\n",
				err);
		return;
	}

	/* IGMP join group for Multicast IP */
	err = igmp_joingroup(IP_ADDR_ANY, (ip_addr_t *)(&multicast_ip));
	if (err != ERR_OK) {
		xil_printf("igmp_app: igmp_joingroup failed with  ");
		xil_printf("error: %d\n\r", err);
		return;
	}
	xil_printf("IGMP application joined group : %s\n\r",
			IGMP_MULTICAST_IP_ADDRESS);

	pcb = udp_new();
	if (pcb == NULL) {
		xil_printf("igmp_app: Error creating PCB. Out of Memory\r\n");
		return;
	}

	/* Bind UDP PCB to a particular Multicast IP and port */
	err = udp_bind(pcb, &multicast_ip, IGMP_MULTICAST_PORT);
	if (err != ERR_OK) {
		xil_printf("igmp_app: Unable to bind to multicast port %d, ",
				IGMP_MULTICAST_PORT);
		xil_printf("err = %d\r\n", err);
		udp_remove(pcb);
		return;
	}

	/* register recv callback for Multicast packets */
	udp_recv(pcb, udp_multicast_recv, NULL);

	return;
}
