/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "lwipopts.h"

#if !NO_SYS
#include "xmk.h"
#include "sys/intr.h"
#include "lwip/sys.h"
#endif

#include "lwip/stats.h"
#include "netif/xadapter.h"
#include "netif/xlltemacif.h"

#include "xintc_l.h"
#include "xstatus.h"

#include "xlltemacif_fifo.h"

#include "xlwipconfig.h"

static void
xllfifo_recv_handler(struct xemac_s *xemac)
{
	u32_t frame_length;
	struct pbuf *p;
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
	XLlFifo *llfifo = &xlltemacif->llfifo;

	/* While there is data in the fifo ... */
	while (XLlFifo_RxOccupancy(llfifo)) {
		/* find packet length */
		frame_length = XLlFifo_RxGetLen(llfifo);

		/* allocate a pbuf */
		p = pbuf_alloc(PBUF_RAW, frame_length, PBUF_POOL);
		if (!p) {
                        char tmp_frame[XTE_MAX_FRAME_SIZE];
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
		if (pq_enqueue(xlltemacif->recv_q, (void*)p) < 0) {
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
fifo_error_handler(xlltemacif_s *xlltemacif, u32_t pending_intr)
{
	XLlFifo *llfifo = &xlltemacif->llfifo;

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
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
	XLlFifo *llfifo = &xlltemacif->llfifo;

	u32_t pending_fifo_intr = XLlFifo_IntPending(llfifo);

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
			fifo_error_handler(xlltemacif, pending_fifo_intr);
		}
		pending_fifo_intr = XLlFifo_IntPending(llfifo);
	}
}

XStatus
init_ll_fifo(struct xemac_s *xemac)
{
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
#if NO_SYS
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
#endif

	/* initialize ll fifo */
	XLlFifo_Initialize(&xlltemacif->llfifo,
			XLlTemac_LlDevBaseAddress(&xlltemacif->lltemac));

	/* Clear any pending FIFO interrupts */
	XLlFifo_IntClear(&xlltemacif->llfifo, XLLF_INT_ALL_MASK);

	/* enable fifo interrupts */
	XLlFifo_IntEnable(&xlltemacif->llfifo, XLLF_INT_ALL_MASK);

#if NO_SYS
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xlltemacif->lltemac.Config.TemacIntr,
			(XInterruptHandler)xlltemac_error_handler,
			&xlltemacif->lltemac);

	/* connect & enable FIFO interrupt */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xlltemacif->lltemac.Config.LLFifoIntr,
			(XInterruptHandler)xllfifo_intr_handler,
			xemac);

	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr + XIN_IER_OFFSET);

		/* form new mask enabling SDMA & ll_temac interrupts */
		cur_mask = cur_mask
				| (1 << xlltemacif->lltemac.Config.LLFifoIntr)
				| (1 << xlltemacif->lltemac.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);
#else
	/* connect & enable TEMAC interrupts */
	register_int_handler(xlltemacif->lltemac.Config.TemacIntr,
			(XInterruptHandler)xlltemac_error_handler,
			&xlltemacif->lltemac);
	enable_interrupt(xlltemacif->lltemac.Config.TemacIntr);

	/* connect & enable FIFO interrupts */
	register_int_handler(xlltemacif->lltemac.Config.LLFifoIntr,
			(XInterruptHandler)xllfifo_intr_handler,
			xemac);
	enable_interrupt(xlltemacif->lltemac.Config.LLFifoIntr);
#endif

	return 0;
}

XStatus
xllfifo_send(xlltemacif_s *xlltemacif, struct pbuf *p)
{
	XLlFifo *llfifo = &xlltemacif->llfifo;
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
