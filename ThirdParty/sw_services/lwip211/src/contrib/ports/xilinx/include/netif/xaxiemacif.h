/*
 * Copyright (C) 2010 - 2020 Xilinx, Inc.
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
 */

#ifndef __NETIF_XAXIEMACIF_H__
#define __NETIF_XAXIEMACIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xlwipconfig.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "netif/xadapter.h"

#include "xparameters.h"
#include "xstatus.h"

#include "xaxiethernet.h"
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
#include "xllfifo.h"
#elif XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_MCDMA
#include "xmcdma.h"
#else
#include "xaxidma.h"
#include "xaxidma_hw.h"
#endif

#include "netif/xpqueue.h"
#include "xlwipconfig.h"

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#define AXIDMA_TX_INTR_PRIORITY_SET_IN_GIC      0xA0
#define AXIDMA_RX_INTR_PRIORITY_SET_IN_GIC      0xA0
#define AXIETH_INTR_PRIORITY_SET_IN_GIC         0xA0
#define TRIG_TYPE_RISING_EDGE_SENSITIVE         0x3

#define INTC_DIST_BASE_ADDR     XPAR_SCUGIC_0_DIST_BASEADDR
#endif

void 	xaxiemacif_setmac(u32_t index, u8_t *addr);
u8_t*	xaxiemacif_getmac(u32_t index);
err_t 	xaxiemacif_init(struct netif *netif);
int 	xaxiemacif_input(struct netif *netif);

unsigned get_IEEE_phy_speed(XAxiEthernet *xaxiemacp);
void enable_sgmii_clock(XAxiEthernet *xaxiemacp);
unsigned configure_IEEE_phy_speed(XAxiEthernet *xaxiemacp, unsigned speed);
unsigned phy_setup_axiemac (XAxiEthernet *xaxiemacp);

/* xaxiemacif_hw.c */
void 	xaxiemac_error_handler(XAxiEthernet * Temac);

/* structure within each netif, encapsulating all information required for
 * using a particular temac instance
 */
typedef struct {
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
	XLlFifo      axififo;
#elif defined(XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_MCDMA)
	XMcdma       aximcdma;
#else
	XAxiDma      axidma;
#endif
	XAxiEthernet axi_ethernet;

	/* queue to store overflow packets */
	pq_queue_t *recv_q;
	pq_queue_t *send_q;

	/* pointers to memory holding buffer descriptors (used only with SDMA) */
	void *rx_bdspace;
	void *tx_bdspace;
} xaxiemacif_s;

extern xaxiemacif_s xaxiemacif;

#ifndef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
s32_t is_tx_space_available(xaxiemacif_s *emac);
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_MCDMA
s32_t process_sent_bds(XMcdma_ChanCtrl *Tx_Chan);
#else
s32_t process_sent_bds(XAxiDma_BdRing *txring);
#endif
#endif

/* xaxiemacif_dma.c/xaxiemacif_mcdma.c */
#ifndef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO
#ifdef XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_MCDMA
XStatus init_axi_mcdma(struct xemac_s *xemac);
XStatus axi_mcdma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p);
#else
XStatus init_axi_dma(struct xemac_s *xemac);
XStatus axidma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_XAXIEMACIF_H__ */
