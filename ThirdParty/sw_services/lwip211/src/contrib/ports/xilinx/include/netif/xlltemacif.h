/*
 * Copyright (C) 2007 - 2019 Xilinx, Inc.
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

#ifndef __NETIF_XLLTEMACIF_H__
#define __NETIF_XLLTEMACIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "netif/etharp.h"
#include "netif/xadapter.h"

#include "xparameters.h"
#include "xstatus.h"
#include "xlltemac.h"
#include "xlldma.h"
#include "xllfifo.h"
#include "xlldma_bdring.h"

#include "netif/xpqueue.h"
#include "xlwipconfig.h"

void 	xlltemacif_setmac(u32_t index, u8_t *addr);
u8_t*	xlltemacif_getmac(u32_t index);
err_t 	xlltemacif_init(struct netif *netif);
int 	xlltemacif_input(struct netif *netif);
unsigned get_IEEE_phy_speed(XLlTemac *xlltemacp);
unsigned Phy_Setup (XLlTemac *xlltemacp);
unsigned configure_IEEE_phy_speed(XLlTemac *xlltemacp, unsigned speed);

/* xlltemacif_hw.c */
void 	xlltemac_error_handler(XLlTemac * Temac);

/* structure within each netif, encapsulating all information required for
 * using a particular temac instance
 */
typedef struct {
	XLlDma lldma;
	XLlFifo llfifo;
	XLlTemac lltemac;

	/* queue to store overflow packets */
	pq_queue_t *recv_q;
	pq_queue_t *send_q;

	/* pointers to memory holding buffer descriptors (used only with SDMA) */
	void *rx_bdspace;
	void *tx_bdspace;
} xlltemacif_s;

extern xlltemacif_s xlltemacif;

/* xlltemacif_sdma.c */
XStatus init_sdma(struct xemac_s *xemac);
int  process_sent_bds(XLlDma_BdRing *txring);
void lldma_send_handler(void *arg);
XStatus lldma_sgsend(xlltemacif_s *xlltemacif, struct pbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_XLLTEMACIF_H__ */
