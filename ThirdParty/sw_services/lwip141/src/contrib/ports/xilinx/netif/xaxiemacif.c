/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2013 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <string.h>

#include <xparameters.h>

#include "xlwipconfig.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/igmp.h"

#include "netif/etharp.h"
#include "netif/xaxiemacif.h"
#include "netif/xadapter.h"
#include "netif/xpqueue.h"

#include "xaxiemacif_fifo.h"
#include "xaxiemacif_hw.h"

#include "xparameters.h"
#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#include "xscugic.h"
#else
#include "xintc.h"
#endif


/* Define those to better describe your network interface. */
#define IFNAME0 't'
#define IFNAME1 'e'

#if LWIP_IGMP
static err_t xaxiemacif_mac_filter_update (struct netif *netif,
								struct ip_addr *group, u8_t action);

static u8_t xaxiemac_mcast_entry_mask = 0;
#endif
/*
 * this function is always called with interrupts off
 * this function also assumes that there are available BD's
 */
static err_t _unbuffered_low_level_output(xaxiemacif_s *xaxiemacif,
														struct pbuf *p)
{
	XStatus status = 0;

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif
	if (XAxiEthernet_IsDma(&xaxiemacif->axi_ethernet)) {
#ifndef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
		status = axidma_sgsend(xaxiemacif, p);
#endif
	} else {
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
		status = axififo_send(xaxiemacif, p);
#endif
	}

	if (status != XST_SUCCESS) {
#if LINK_STATS
		lwip_stats.link.drop++;
#endif
	}

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE);	/* reclaim the padding word */
#endif

#if LINK_STATS
	lwip_stats.link.xmit++;
#endif /* LINK_STATS */

	return ERR_OK;

}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
        SYS_ARCH_DECL_PROTECT(lev);
        err_t err;
        struct xemac_s *xemac = (struct xemac_s *)(netif->state);
        xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);

#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_DMA
	/*
	 * With AXI Ethernet on Zynq, we observed unexplained delays for
	 * BD Status update. As a result, we are hitting a condition where
	 * there are no BDs free to transmit packets. So, we have added
	 * this logic where we look for the status update in a definite
	 * loop.
	 */
	XAxiDma_BdRing *txring = XAxiDma_GetTxRing(&xaxiemacif->axidma);
#endif
        int count = 100;

        SYS_ARCH_PROTECT(lev);

        while (count) {

		/* check if space is available to send */
		if (is_tx_space_available(xaxiemacif)) {
			_unbuffered_low_level_output(xaxiemacif, p);
			err = ERR_OK;
			break;
		} else {
#if LINK_STATS
			lwip_stats.link.drop++;
#endif
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_DMA
			process_sent_bds(txring);
#endif
			count--;
		}
        }

	if (count == 0) {
		print("pack dropped, no space\r\n");
		err = ERR_MEM;
	}

        SYS_ARCH_UNPROTECT(lev);
        return err;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
static struct pbuf *low_level_input(struct netif *netif)
{
	struct xemac_s *xemac = (struct xemac_s *)(netif->state);
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	struct pbuf *p;

	/* see if there is data to process */
	if (pq_qlength(xaxiemacif->recv_q) == 0)
		return NULL;

	/* return one packet from receive q */
	p = (struct pbuf *)pq_dequeue(xaxiemacif->recv_q);
	return p;
}

/*
 * xaxiemacif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t xaxiemacif_output(struct netif *netif, struct pbuf *p,
		struct ip_addr *ipaddr)
{
	/* resolve hardware address, then send (or queue) packet */
	return etharp_output(netif, p, ipaddr);
}

/*
 * xaxiemacif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 * Returns the number of packets read (max 1 packet on success,
 * 0 if there are no packets)
 *
 */

int xaxiemacif_input(struct netif *netif)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;
	SYS_ARCH_DECL_PROTECT(lev);

#if !NO_SYS
	while (1)
#endif
	{
		/* move received packet into a new pbuf */
		SYS_ARCH_PROTECT(lev);
		p = low_level_input(netif);
		SYS_ARCH_UNPROTECT(lev);

		/* no packet could be read, silently ignore this */
		if (p == NULL)
			return 0;

		/* points to packet payload, which starts with an Ethernet header */
		ethhdr = p->payload;

#if LINK_STATS
		lwip_stats.link.recv++;
#endif /* LINK_STATS */

		switch (htons(ethhdr->type)) {
			/* IP or ARP packet? */
			case ETHTYPE_IP:
			case ETHTYPE_ARP:
#if PPPOE_SUPPORT
				/* PPPoE packet? */
			case ETHTYPE_PPPOEDISC:
			case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
				/* full packet send to tcpip_thread to process */
				if (netif->input(p, netif) != ERR_OK) {
					LWIP_DEBUGF(NETIF_DEBUG, ("xaxiemacif_input: IP input error\r\n"));
					pbuf_free(p);
					p = NULL;
				}
				break;

			default:
				pbuf_free(p);
				p = NULL;
				break;
		}
	}
	return 1;
}

static err_t low_level_init(struct netif *netif)
{
	unsigned mac_address = (unsigned)(netif->state);
	struct xemac_s *xemac;
	xaxiemacif_s *xaxiemacif;
	XAxiEthernet_Config *mac_config;

	xaxiemacif = mem_malloc(sizeof *xaxiemacif);
	if (xaxiemacif == NULL) {
		LWIP_DEBUGF(NETIF_DEBUG, ("xaxiemacif_init: out of memory\r\n"));
		return ERR_MEM;
	}

	xemac = mem_malloc(sizeof *xemac);
	if (xemac == NULL) {
		LWIP_DEBUGF(NETIF_DEBUG, ("xaxiemacif_init: out of memory\r\n"));
		return ERR_MEM;
	}

	xemac->state = (void *)xaxiemacif;
	xemac->topology_index = xtopology_find_index(mac_address);
	xemac->type = xemac_type_axi_ethernet;

	xaxiemacif->send_q = NULL;
	xaxiemacif->recv_q = pq_create_queue();
	if (!xaxiemacif->recv_q)
		return ERR_MEM;

	/* maximum transfer unit */
#ifdef USE_JUMBO_FRAMES
	netif->mtu = XAE_JUMBO_MTU - XAE_HDR_SIZE;
#else
	netif->mtu = XAE_MTU - XAE_HDR_SIZE;
#endif

#if LWIP_IGMP
	netif->igmp_mac_filter = xaxiemacif_mac_filter_update;
#endif

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
				NETIF_FLAG_LINK_UP;

#if LWIP_IGMP
	netif->flags |= NETIF_FLAG_IGMP;
#endif

#if !NO_SYS
	sys_sem_new(&xemac->sem_rx_data_available, 0);
#endif

	/* obtain config of this emac */
	mac_config = xaxiemac_lookup_config((unsigned)netif->state);

	XAxiEthernet_CfgInitialize(&xaxiemacif->axi_ethernet,
		mac_config, mac_config->BaseAddress);

	/* figure out if the system has DMA */
	if (XAxiEthernet_IsDma(&xaxiemacif->axi_ethernet)) {
#ifndef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
		/* initialize the DMA engine */
		init_axi_dma(xemac);
#endif
	} else if (XAxiEthernet_IsFifo(&xaxiemacif->axi_ethernet)) {
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
		/* initialize the locallink FIFOs */
		init_axi_fifo(xemac);
#endif
	} else {
		/* should not occur */
		LWIP_DEBUGF(NETIF_DEBUG, ("xaxiemacif_init: mac is not configured with DMA or FIFO\r\n"));
		return ERR_IF;
	}

	/* initialize the mac */
	init_axiemac(xaxiemacif, netif);

	/* replace the state in netif (currently the emac baseaddress)
	 * with the mac instance pointer.
	 */
	netif->state = (void *)xemac;

	return ERR_OK;
}

#if LWIP_IGMP
static err_t
xaxiemacif_mac_filter_update (struct netif *netif, struct ip_addr *group,
								u8_t action)
{
	err_t return_val = ERR_OK;
	u8_t multicast_mac_addr[6];
	u8_t multicast_mac_addr_to_clr[6];
	u8_t temp_mask;
	int entry;
	int i;
	u8_t * ip_addr_temp  = (u8_t *)group;
	struct xemac_s *xemac = (struct xemac_s *)(netif->state);
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);

	if (action == IGMP_ADD_MAC_FILTER) {
		if ((ip_addr_temp[0] >= 224) && (ip_addr_temp[0] <= 239)) {
			if (xaxiemac_mcast_entry_mask >= 0x0F) {
				LWIP_DEBUGF(NETIF_DEBUG,
				("xaxiemacif_mac_filter_update: No multicast address registers left.\r\n"));
				LWIP_DEBUGF(NETIF_DEBUG,
				("				 Multicast MAC address add operation failure !!\r\n"));

				return_val = ERR_MEM;
			} else {
				for (i = 0; i < 4; i++) {
					temp_mask = (0x01) << i;
					if ((xaxiemac_mcast_entry_mask &
						temp_mask) == temp_mask) {

						continue;

					} else {
						entry = i;
						xaxiemac_mcast_entry_mask
								|= temp_mask;
						multicast_mac_addr[0] = 0x01;
						multicast_mac_addr[1] = 0x00;
						multicast_mac_addr[2] = 0x5E;

						multicast_mac_addr[3] =
							ip_addr_temp[1] & 0x7F;
						multicast_mac_addr[4] =
							ip_addr_temp[2];
						multicast_mac_addr[5] =
							ip_addr_temp[3];

						XAxiEthernet_Stop
						(&xaxiemacif->axi_ethernet);

						XAxiEthernet_MulticastAdd
						(&xaxiemacif->axi_ethernet,
						multicast_mac_addr,entry);

						XAxiEthernet_Start
						(&xaxiemacif->axi_ethernet);

						LWIP_DEBUGF(NETIF_DEBUG,
						("xaxiemacif_mac_filter_update: Muticast MAC address successfully added.\r\n"));

						return_val = ERR_OK;
						break;
					}
				}
				if (i == 4) {
					LWIP_DEBUGF(NETIF_DEBUG,
					("xaxiemacif_mac_filter_update: No multicast address registers left.\r\n"));
					LWIP_DEBUGF(NETIF_DEBUG,
					("				Multicast MAC address add operation failure !!\r\n"));

					return_val = ERR_MEM;
				}
			}
		} else {
			LWIP_DEBUGF(NETIF_DEBUG,
			("xaxiemacif_mac_filter_update: The requested MAC address is not a multicast address.\r\n"));
			LWIP_DEBUGF(NETIF_DEBUG,
			("				 Multicast address add operation failure !!\r\n"));

			return_val = ERR_ARG;
		}
	} else if (action == IGMP_DEL_MAC_FILTER) {
		if ((ip_addr_temp[0] < 224) || (ip_addr_temp[0] > 239)) {

			LWIP_DEBUGF(NETIF_DEBUG,
			("xaxiemacif_mac_filter_update: The requested MAC address is not a multicast address.\r\n"));
			LWIP_DEBUGF(NETIF_DEBUG,
			("				 Multicast address add operation failure !!\r\n"));

			return_val = ERR_ARG;
		} else {
			for (i = 0; i < 4; i++) {
				temp_mask = (0x01) << i;
				if ((xaxiemac_mcast_entry_mask & temp_mask)
								== temp_mask) {
					XAxiEthernet_MulticastGet
					(&xaxiemacif->axi_ethernet,
					multicast_mac_addr_to_clr, i);

					if ((ip_addr_temp[3] ==
					multicast_mac_addr_to_clr[5]) &&
					(ip_addr_temp[2] ==
					multicast_mac_addr_to_clr[4]) &&
					((ip_addr_temp[1] & 0x7f) ==
					multicast_mac_addr_to_clr[3])) {

						XAxiEthernet_Stop
						(&xaxiemacif->axi_ethernet);

						XAxiEthernet_MulticastClear
						(&xaxiemacif->axi_ethernet, i);

						XAxiEthernet_Start
						(&xaxiemacif->axi_ethernet);

						LWIP_DEBUGF(NETIF_DEBUG,
						("xaxiemacif_mac_filter_update: Muticast MAC address successfully removed.\r\n"));

						return_val = ERR_OK;
						xaxiemac_mcast_entry_mask &=
								(~temp_mask);
						break;
					} else {
						continue;
					}
				} else {
					continue;
				}
			}
			if (i == 4) {
				LWIP_DEBUGF(NETIF_DEBUG,
				("xaxiemacif_mac_filter_update: No multicast address registers present with\r\n"));
				LWIP_DEBUGF(NETIF_DEBUG,
				("				 the requested Multicast MAC address.\r\n"));
				LWIP_DEBUGF(NETIF_DEBUG,
				("				 Multicast MAC address removal failure!!.\r\n"));

				return_val = ERR_MEM;
			}
		}
	}
	return return_val;
}
#endif

/*
 * xaxiemacif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t
xaxiemacif_init(struct netif *netif)
{
#if LWIP_SNMP
	/* ifType ethernetCsmacd(6) @see RFC1213 */
	netif->link_type = 6;
	/* your link speed here */
	netif->link_speed = ;
	netif->ts = 0;
	netif->ifinoctets = 0;
	netif->ifinucastpkts = 0;
	netif->ifinnucastpkts = 0;
	netif->ifindiscards = 0;
	netif->ifoutoctets = 0;
	netif->ifoutucastpkts = 0;
	netif->ifoutnucastpkts = 0;
	netif->ifoutdiscards = 0;
#endif

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = xaxiemacif_output;
	netif->linkoutput = low_level_output;

	low_level_init(netif);

	return ERR_OK;
}
