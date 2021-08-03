/*
 * Copyright (C) 2010 - 2021 Xilinx, Inc.
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

#include "lwipopts.h"

#if !NO_SYS
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "lwip/sys.h"
#endif

#include "lwip/stats.h"
#include "netif/xadapter.h"
#include "netif/xaxiemacif.h"

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#include "xscugic.h"
#else
#include "xintc_l.h"
#endif

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#define AXIFIFO_INTR_PRIORITY_SET_IN_GIC	0xA0
#define AXIETH_INTR_PRIORITY_SET_IN_GIC		0xA0
#define TRIG_TYPE_RISING_EDGE_SENSITIVE		0x3
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_DIST_BASEADDR
#endif

#include "xstatus.h"

#include "xaxiemacif_fifo.h"

#include "xlwipconfig.h"

#if XPAR_INTC_0_HAS_FAST == 1
/*********** Function Prototypes *********************************************/

/*
 *  Function prototypes of the functions used for registering Fast
 *  Interrupt Handlers
 */
static void xllfifo_fastintr_handler(void) __attribute__ ((fast_interrupt));
static void xaxiemac_fasterror_handler(void) __attribute__ ((fast_interrupt));

/**************** Variable Declarations **************************************/

/** Variables for Fast Interrupt handlers ***/
struct xemac_s *xemac_fast;
xaxiemacif_s *xaxiemacif_fast;
#endif

#if !NO_SYS
unsigned int xInsideISR = 0;
#endif

int is_tx_space_available(xaxiemacif_s *emac)
{
	return ((XLlFifo_TxVacancy(&emac->axififo) * 4) > XAE_MAX_FRAME_SIZE);
}

static void
xllfifo_recv_handler(struct xemac_s *xemac)
{
	u32_t frame_length;
	struct pbuf *p;
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	XLlFifo *llfifo = &xaxiemacif->axififo;

	/* While there is data in the fifo ... */
	while (XLlFifo_RxOccupancy(llfifo)) {
		/* find packet length */
		frame_length = XLlFifo_RxGetLen(llfifo);

		/* allocate a pbuf */
		p = pbuf_alloc(PBUF_RAW, frame_length, PBUF_POOL);
		if (!p) {
                        char tmp_frame[XAE_MAX_FRAME_SIZE];
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			/* receive and drop packet to keep data & len registers in sync */
		        XLlFifo_Read(llfifo, tmp_frame, frame_length);

			continue;
                }

		/* receive packet */
		XLlFifo_Read(llfifo, p->payload, frame_length);

#if ETH_PAD_SIZE
		len += ETH_PAD_SIZE;		/* allow room for Ethernet padding */
#endif

		/* store it in the receive queue, where it'll be processed by xemacif input thread */
		if (pq_enqueue(xaxiemacif->recv_q, (void*)p) < 0) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			pbuf_free(p);
			continue;
		}

#if !NO_SYS
		sys_sem_signal(&xemac->sem_rx_data_available);
#endif

#if LINK_STATS
		lwip_stats.link.recv++;
#endif
	}
}

static void
fifo_error_handler(xaxiemacif_s *xaxiemacif, u32_t pending_intr)
{
	XLlFifo *llfifo = &xaxiemacif->axififo;

	if (pending_intr & XLLF_INT_RPURE_MASK) {
		print("llfifo: Rx under-read error");
	}
	if (pending_intr & XLLF_INT_RPORE_MASK) {
		print("llfifo: Rx over-read error");
	}
	if (pending_intr & XLLF_INT_RPUE_MASK) {
		print("llfifo: Rx fifo empty");
	}
	if (pending_intr & XLLF_INT_TPOE_MASK) {
		print("llfifo: Tx fifo overrun");
	}
	if (pending_intr & XLLF_INT_TSE_MASK) {
		print("llfifo: Tx length mismatch");
	}

	/* Reset the tx or rx side of the fifo as needed */
	if (pending_intr & XLLF_INT_RXERROR_MASK) {
		XLlFifo_IntClear(llfifo, XLLF_INT_RRC_MASK);
		XLlFifo_RxReset(llfifo);
	}

	if (pending_intr & XLLF_INT_TXERROR_MASK) {
		XLlFifo_IntClear(llfifo, XLLF_INT_TRC_MASK);
		XLlFifo_TxReset(llfifo);
	}
}

static void
xllfifo_intr_handler(struct xemac_s *xemac)
{
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	XLlFifo *llfifo = &xaxiemacif->axififo;

	u32_t pending_fifo_intr = XLlFifo_IntPending(llfifo);

#if !NO_SYS
	xInsideISR++;
#endif

	while (pending_fifo_intr) {
		if (pending_fifo_intr & XLLF_INT_RC_MASK) {
			/* receive interrupt */
			XLlFifo_IntClear(llfifo, XLLF_INT_RC_MASK);
			xllfifo_recv_handler(xemac);
		} else if (pending_fifo_intr & XLLF_INT_TC_MASK) {
			/* tx intr */
			XLlFifo_IntClear(llfifo, XLLF_INT_TC_MASK);
		} else {
			XLlFifo_IntClear(llfifo, XLLF_INT_ALL_MASK &
					 ~(XLLF_INT_RC_MASK |
					   XLLF_INT_TC_MASK));
			fifo_error_handler(xaxiemacif, pending_fifo_intr);
		}
		pending_fifo_intr = XLlFifo_IntPending(llfifo);
	}

#if !NO_SYS
	xInsideISR--;
#endif

}

XStatus init_axi_fifo(struct xemac_s *xemac)
{
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
#if XPAR_INTC_0_HAS_FAST == 1
	xaxiemacif_fast = xaxiemacif;
	xemac_fast = xemac;
#endif
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];

	/* initialize ll fifo */
	XLlFifo_Initialize(&xaxiemacif->axififo,
			XAxiEthernet_AxiDevBaseAddress(&xaxiemacif->axi_ethernet));

	/* Clear any pending FIFO interrupts */
	XLlFifo_IntClear(&xaxiemacif->axififo, XLLF_INT_ALL_MASK);

	/* enable fifo interrupts */
	XLlFifo_IntEnable(&xaxiemacif->axififo, XLLF_INT_ALL_MASK);

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
				xaxiemacif->axi_ethernet.Config.TemacIntr,
				(XInterruptHandler)xaxiemac_error_handler,
				&xaxiemacif->axi_ethernet);
	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
				xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
				(XInterruptHandler)xllfifo_intr_handler,
				xemac);
	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			AXIETH_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);
	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
			AXIFIFO_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);

	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
				xaxiemacif->axi_ethernet.Config.TemacIntr);
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
				xaxiemacif->axi_ethernet.Config.AxiFifoIntr);
#else
#if NO_SYS
#if XPAR_INTC_0_HAS_FAST == 1
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XFastInterruptHandler)xaxiemac_fasterror_handler);

	/* connect & enable FIFO interrupt */
	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
			(XFastInterruptHandler)xllfifo_fastintr_handler);
#else
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XInterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);

	/* connect & enable FIFO interrupt */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
			(XInterruptHandler)xllfifo_intr_handler,
			xemac);

#endif
	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr + XIN_IER_OFFSET);

		/* form new mask enabling SDMA & ll_temac interrupts */
		cur_mask = cur_mask
				| (1 << xaxiemacif->axi_ethernet.Config.AxiFifoIntr)
				| (1 << xaxiemacif->axi_ethernet.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);
#else
#if XPAR_INTC_0_HAS_FAST == 1
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XFastInterruptHandler)xaxiemac_fasterror_handler);

	/* connect & enable FIFO interrupt */
	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
			(XFastInterruptHandler)xllfifo_fastintr_handler);
#else
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XInterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);

	/* connect & enable FIFO interrupt */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiFifoIntr,
			(XInterruptHandler)xllfifo_intr_handler,
			xemac);

#endif
	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr + XIN_IER_OFFSET);

		/* form new mask enabling SDMA & ll_temac interrupts */
		cur_mask = cur_mask
				| (1 << xaxiemacif->axi_ethernet.Config.AxiFifoIntr)
				| (1 << xaxiemacif->axi_ethernet.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);
#endif
#endif

	return 0;
}

XStatus axififo_send(xaxiemacif_s *xaxiemacif, struct pbuf *p)
{
	XLlFifo *llfifo = &xaxiemacif->axififo;
	u32_t l = 0;
	struct pbuf *q;

	for(q = p; q != NULL; q = q->next) {
		/* write frame data to FIFO */
		XLlFifo_Write(llfifo, q->payload, q->len);
		l += q->len;
	}

	/* initiate transmit */
	XLlFifo_TxSetLen(llfifo, l);

	return 0;
}

#if XPAR_INTC_0_HAS_FAST == 1
/*********** Fast Error Handler ********************************************/
void xaxiemac_fasterror_handler(void)
{
	xaxiemac_error_handler(&xaxiemacif_fast->axi_ethernet);
}

/********** Fast Interrupt handler *****************************************/
void xllfifo_fastintr_handler(void)
{
	xllfifo_intr_handler(xemac_fast);
}
#endif
