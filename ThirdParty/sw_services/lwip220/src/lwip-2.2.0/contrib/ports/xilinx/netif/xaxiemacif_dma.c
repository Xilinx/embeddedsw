/*
 * Copyright (C) 2010 - 2022 Xilinx, Inc.
 * Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.
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
#include "lwip/inet_chksum.h"

#include "netif/xadapter.h"
#include "netif/xaxiemacif.h"

#ifndef SDT
#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#include "xscugic.h"
#else
#include "xintc_l.h"
#endif
#else
#include "xinterrupt_wrap.h"
#ifndef XCACHE_FLUSH_DCACHE_RANGE
#define XCACHE_FLUSH_DCACHE_RANGE(data, length)	\
		Xil_DCacheFlushRange((UINTPTR)data, length)
#endif
#ifndef XCACHE_INVALIDATE_DCACHE_RANGE
#define XCACHE_INVALIDATE_DCACHE_RANGE(data, length)	\
		Xil_DCacheInvalidateRange((u32)data, length)
#endif
#endif

#include "xstatus.h"

#include "xlwipconfig.h"
#include "xparameters.h"

#ifdef CONFIG_XTRACE
#include "xtrace.h"
#endif

#if defined __aarch64__ || defined (__arm__)
#include "xil_mmu.h"
#elif defined (ARMR5)
#include "xreg_cortexr5.h"
#endif

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#ifndef XCACHE_FLUSH_DCACHE_RANGE
#define XCACHE_FLUSH_DCACHE_RANGE(data, length)	\
		Xil_DCacheFlushRange((UINTPTR)data, length)
#endif
#ifndef XCACHE_INVALIDATE_DCACHE_RANGE
#define XCACHE_INVALIDATE_DCACHE_RANGE(data, length)	\
		Xil_DCacheInvalidateRange((u32)data, length)
#endif

#endif

/* Byte alignment of BDs */
#define BD_ALIGNMENT (XAXIDMA_BD_MINIMUM_ALIGNMENT*2)

#if XPAR_INTC_0_HAS_FAST == 1
/*********** Function Prototypes *********************************************/
/*
 *  Function prototypes of the functions used for registering Fast
 *  Interrupt Handlers
 */
static void axidma_sendfast_handler(void) __attribute__ ((fast_interrupt));
static void axidma_recvfast_handler(void) __attribute__ ((fast_interrupt));
static void xaxiemac_errorfast_handler(void) __attribute__ ((fast_interrupt));

/**************** Variable Declarations **************************************/
/** Variables for Fast Interrupt handlers ***/
struct xemac_s *xemac_fast;
xaxiemacif_s *xaxiemacif_fast;
#endif

#if !NO_SYS
extern u32 xInsideISR;
#endif

#define BD_SIZE_2MB                 0x200000
#define BD_SIZE_1MB                 0x100000
#define BD_SIZE_64KB				0x10000

#if defined (__aarch64__)
u8_t bd_space[BD_SIZE_2MB] __attribute__ ((aligned (BD_SIZE_2MB)));
#elif defined (__arm__) || defined (ARMR5)
u8_t bd_space[BD_SIZE_1MB] __attribute__ ((aligned (BD_SIZE_1MB)));
#endif

#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
volatile u32_t notifyinfo[XLWIP_CONFIG_N_TX_DESC];
#endif

#define XAxiDma_BD_TO_INDEX(ringptr, bdptr)				\
	(((UINTPTR)bdptr - (UINTPTR)(ringptr)->FirstBdAddr) / (ringptr)->Separation)

static inline void bd_csum_enable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 | 1));
}

static inline void bd_csum_disable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 & ~1));
}

static inline void bd_fullcsum_disable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 & ~3));
}

static inline void bd_fullcsum_enable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 | 2));
}

static inline void bd_csum_set(XAxiDma_Bd *bd, u16_t tx_csbegin, u16_t tx_csinsert,
															u16_t tx_csinit)
{
	u32_t app1;

	bd_csum_enable(bd);

	/* write start offset and insert offset into BD */
	app1 = ((u32_t)tx_csbegin << 16) | (u32_t) tx_csinsert;
	XAxiDma_BdWrite(bd, XAXIDMA_BD_USR1_OFFSET, app1);

	/* insert init value */
	XAxiDma_BdWrite(bd, XAXIDMA_BD_USR2_OFFSET, tx_csinit);
}

static inline u16_t extract_packet_len(XAxiDma_Bd *rxbd) {
    u16_t packet_len = XAxiDma_BdRead(rxbd, XAXIDMA_BD_USR4_OFFSET) & 0x3fff;
    return packet_len;
}

static inline u16_t extract_csum(XAxiDma_Bd *rxbd) {
    u16_t csum = XAxiDma_BdRead(rxbd, XAXIDMA_BD_USR3_OFFSET) & 0xffff;
    return csum;
}

static inline u32_t csum_sub(u32_t csum, u16_t v)
{
	csum += (u32_t)v;
	return csum + (csum < (u32_t)v);
}

/*
 * compare if the h/w computed checksum (stored in the rxbd)
 * equals the TCP checksum value in the packet
 */
s32_t is_checksum_valid(XAxiDma_Bd *rxbd, struct pbuf *p) {
	struct ethip_hdr *ehdr = p->payload;
	u8_t proto = IPH_PROTO(&ehdr->ip);

	/* check if it is a TCP packet */
	if (htons(ehdr->eth.type) == ETHTYPE_IP && proto == IP_PROTO_TCP) {
		u32_t iphdr_len;
		u16_t csum_in_rxbd, pseudo_csum, iphdr_csum, padding_csum;
		u16_t tcp_payload_offset;
		u32_t computed_csum;
		u16_t padding_len, tcp_payload_len, packet_len;
		u16_t csum;

		/* determine length of IP header */
		iphdr_len = (IPH_HL(&ehdr->ip) * 4);
		tcp_payload_offset = XAE_HDR_SIZE + iphdr_len;
		tcp_payload_len = htons(IPH_LEN(&ehdr->ip)) - IPH_HL(&ehdr->ip) * 4;
                packet_len = extract_packet_len(rxbd);
		padding_len = packet_len - tcp_payload_offset - tcp_payload_len;

		csum_in_rxbd = extract_csum(rxbd);
		pseudo_csum = htons(inet_chksum_pseudo(NULL,
					proto, tcp_payload_len, (ip_addr_t *)&ehdr->ip.src,
					(ip_addr_t *)&ehdr->ip.dest));

		/* xps_ll_temac computes the checksum of the packet starting at byte 14
		 * we need to subtract the values of the ethernet & IP headers
		 */
		iphdr_csum  = inet_chksum(p->payload + 14, tcp_payload_offset - 14);

		/* compute csum of padding bytes, if any */
		padding_csum = inet_chksum(p->payload + p->tot_len - padding_len,
					padding_len);

		/* get the h/w checksum value */
		computed_csum = (u32_t)csum_in_rxbd;

		/* remove the effect of csumming the iphdr */
		computed_csum = csum_sub(computed_csum, ~iphdr_csum);

		/* add in the pseudo csum */
		computed_csum = csum_sub(computed_csum, ~pseudo_csum);

		/* remove any padding effect */
		computed_csum = csum_sub(computed_csum, ~padding_csum);

		/* normalize computed csum */
		while (computed_csum >> 16) {
			computed_csum = (computed_csum & 0xffff) + (computed_csum >> 16);
		}

		/* convert to 16 bits and take 1's complement */
		csum = (u16_t)computed_csum;
		csum = ~csum;

		/* chksum is valid if: computed csum over the packet is 0 */
		return !csum;
	} else {
		/* just say yes to all other packets */
		/* the upper layers in the stack will compute and verify the checksum */
		return 1;
	}
}

static inline void *alloc_bdspace(int n_desc)
{
	int space = XAxiDma_BdRingMemCalc(BD_ALIGNMENT, n_desc);
	int padding = BD_ALIGNMENT*2;
	void *unaligned_mem = mem_malloc(space + padding*4);
	void *aligned_mem =
	(void *)(((UINTPTR)(unaligned_mem + BD_ALIGNMENT)) & ~(BD_ALIGNMENT - 1));

#if DEBUG
	assert(aligned_mem > unaligned_mem);
	assert(aligned_mem + space < unaligned_mem + space + padding);
#endif
	return aligned_mem;
}

static void axidma_send_handler(void *arg)
{
	unsigned irq_status;
	struct xemac_s *xemac;
	xaxiemacif_s   *xaxiemacif;
	XAxiDma_BdRing *txringptr;

#if !NO_SYS
	xInsideISR++;
#endif
	xemac = (struct xemac_s *)(arg);
	xaxiemacif = (xaxiemacif_s *)(xemac->state);
	txringptr = XAxiDma_GetTxRing(&xaxiemacif->axidma);

	XAxiDma_BdRingIntDisable(txringptr, XAXIDMA_IRQ_ALL_MASK);

	/* Read pending interrupts */
	irq_status = XAxiDma_BdRingGetIrq(txringptr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(txringptr, irq_status);

	/* If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if (irq_status & XAXIDMA_IRQ_ERROR_MASK) {
		LWIP_DEBUGF(NETIF_DEBUG, ("%s: Error: axidma error interrupt is asserted\r\n",
			__FUNCTION__));
		XAxiDma_Reset(&xaxiemacif->axidma);
#if !NO_SYS
		xInsideISR--;
#endif
		return;
	}
	/* If Transmit done interrupt is asserted, process completed BD's */
	if (irq_status & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK)) {
		process_sent_bds(txringptr);
	}

	XAxiDma_BdRingIntEnable(txringptr, XAXIDMA_IRQ_ALL_MASK);

#if !NO_SYS
	xInsideISR--;
#endif
}

static void setup_rx_bds(XAxiDma_BdRing *rxring)
{
	XAxiDma_Bd *rxbd;
	s32_t n_bds;
	XStatus status;
	struct pbuf *p;
	u32 bdsts;

	n_bds = XAxiDma_BdRingGetFreeCnt(rxring);
	while (n_bds > 0) {
		n_bds--;
#ifdef USE_JUMBO_FRAMES
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_JUMBO_FRAME_SIZE, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			xil_printf("unable to alloc pbuf in recv_handler\r\n");
			return;
		}
		status = XAxiDma_BdRingAlloc(rxring, 1, &rxbd);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("setup_rx_bds: Error allocating RxBD\r\n"));
			pbuf_free(p);
			return;
		}
		 /* Setup the BD. */
		XAxiDma_BdSetBufAddr(rxbd, (UINTPTR)p->payload);
		/* Clear everything but the COMPLETE bit, which is cleared when
		 * committed to hardware.
		 */
		bdsts = XAxiDma_BdGetSts(rxbd);
		bdsts &=  XAXIDMA_BD_STS_COMPLETE_MASK;
		XAxiDma_BdWrite(rxbd, XAXIDMA_BD_STS_OFFSET, bdsts);
		XAxiDma_BdSetLength(rxbd, p->len, rxring->MaxTransferLen);
		XAxiDma_BdSetCtrl(rxbd, 0);
		XAxiDma_BdSetId(rxbd, p);
#if !defined (__MICROBLAZE__)  && !defined (__riscv)
		dsb();
#endif
#ifdef USE_JUMBO_FRAMES
		XCACHE_FLUSH_DCACHE_RANGE((UINTPTR)p->payload, (UINTPTR)XAE_MAX_JUMBO_FRAME_SIZE);
#else
		XCACHE_FLUSH_DCACHE_RANGE((UINTPTR)p->payload, (UINTPTR)XAE_MAX_FRAME_SIZE);
#endif
#if !defined(__aarch64__)
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);
#endif

		/* Enqueue to HW */
		status = XAxiDma_BdRingToHw(rxring, 1, rxbd);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to hardware: "));
			if (status == XST_DMA_SG_LIST_ERROR) {
				LWIP_DEBUGF(NETIF_DEBUG, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with XAxiDma_BdRingAlloc()\r\n"));
			}
			else {
				LWIP_DEBUGF(NETIF_DEBUG, ("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
			}
			pbuf_free(p);
			XAxiDma_BdRingUnAlloc(rxring, 1, rxbd);
			return;
		}
	}
}

static void axidma_recv_handler(void *arg)
{
	struct pbuf *p;
	u32 irq_status, i, timeOut;
	XAxiDma_Bd *rxbd, *rxbdset;
	struct xemac_s *xemac;
	xaxiemacif_s *xaxiemacif;
	XAxiDma_BdRing *rxring;

#if !NO_SYS
	xInsideISR++;
#endif

	xemac = (struct xemac_s *)(arg);
	xaxiemacif = (xaxiemacif_s *)(xemac->state);
	rxring = XAxiDma_GetRxRing(&xaxiemacif->axidma);

	XAxiDma_BdRingIntDisable(rxring, XAXIDMA_IRQ_ALL_MASK);

	/* Read pending interrupts */
	irq_status = XAxiDma_BdRingGetIrq(rxring);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(rxring, irq_status);

	/* If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((irq_status & XAXIDMA_IRQ_ERROR_MASK)) {
		setup_rx_bds(rxring);
		LWIP_DEBUGF(NETIF_DEBUG, ("%s: Error: axidma error interrupt is asserted\r\n",
			__FUNCTION__));
		XAxiDma_Reset(&xaxiemacif->axidma);
		timeOut = 10000;
		while (timeOut) {
			if (XAxiDma_ResetIsDone(&xaxiemacif->axidma)) {
				break;
			}
			timeOut -= 1;
		}
		XAxiDma_BdRingIntEnable(rxring, XAXIDMA_IRQ_ALL_MASK);
		XAxiDma_Resume(&xaxiemacif->axidma);
#if !NO_SYS
		xInsideISR--;
#endif
		return;
	}
	/* If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if (irq_status & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK)) {
		u32 bd_processed;
		u32 rx_bytes;

		bd_processed = XAxiDma_BdRingFromHw(rxring, XAXIDMA_ALL_BDS, &rxbdset);

		for (i = 0, rxbd = rxbdset; i < bd_processed; i++) {
			p = (struct pbuf *)(UINTPTR)XAxiDma_BdGetId(rxbd);
			/* Adjust the buffer size to the actual number of bytes received.*/
			rx_bytes = extract_packet_len(rxbd);
			pbuf_realloc(p, rx_bytes);

#if defined(__aarch64__)
#ifdef USE_JUMBO_FRAMES
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload,
							XAE_MAX_JUMBO_FRAME_SIZE);
#else
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload, XAE_MAX_FRAME_SIZE);
#endif
#endif

#if LWIP_PARTIAL_CSUM_OFFLOAD_RX==1
			/* Verify for partial checksum offload case */
			if (!is_checksum_valid(rxbd, p)) {
				LWIP_DEBUGF(NETIF_DEBUG, ("Incorrect csum as calculated by the hw\r\n"));
			}
#endif
			/* store it in the receive queue,
			 * where it'll be processed by a different handler
			 */
			if (pq_enqueue(xaxiemacif->recv_q, (void*)p) < 0) {
#if LINK_STATS
				lwip_stats.link.memerr++;
				lwip_stats.link.drop++;
#endif
				pbuf_free(p);
			}
			rxbd = (XAxiDma_Bd *)XAxiDma_BdRingNext(rxring, rxbd);
		}
		/* free up the BD's */
		XAxiDma_BdRingFree(rxring, bd_processed, rxbdset);
		/* return all the processed bd's back to the stack */
		/* setup_rx_bds -> use XAxiDma_BdRingGetFreeCnt */
		setup_rx_bds(rxring);
	}
	XAxiDma_BdRingIntEnable(rxring, XAXIDMA_IRQ_ALL_MASK);
#if !NO_SYS
	sys_sem_signal(&xemac->sem_rx_data_available);
	xInsideISR--;
#endif

}

s32_t xaxiemac_is_tx_space_available(xaxiemacif_s *emac)
{
	XAxiDma_BdRing *txring;

	txring = XAxiDma_GetTxRing(&emac->axidma);
	/* tx space is available as long as there are valid BD's */
	return XAxiDma_BdRingGetFreeCnt(txring);
}

s32_t process_sent_bds(XAxiDma_BdRing *txring)
{
	XAxiDma_Bd *txbdset, *txbd;
	int n_bds, i;
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
	u32_t bdindex;
#endif

	/* obtain a list of processed BD's */
	n_bds = XAxiDma_BdRingFromHw(txring, XAXIDMA_ALL_BDS, &txbdset);
	if (n_bds == 0) {
		return XST_FAILURE;
	}
	/* free the pbuf associated with each BD */
	for (i = 0, txbd = txbdset; i < n_bds; i++) {
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
		bdindex = XAxiDma_BD_TO_INDEX(txring, txbd);
#endif
		struct pbuf *p = (struct pbuf *)(UINTPTR)XAxiDma_BdGetId(txbd);
		pbuf_free(p);
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
    notifyinfo[bdindex] = 0;
#endif
		txbd = (XAxiDma_Bd *)XAxiDma_BdRingNext(txring, txbd);
	}
	/* free the processed BD's */
	return (XAxiDma_BdRingFree(txring, n_bds, txbdset));
}

#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
XStatus axidma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p,
        u32_t block_till_tx_complete, u32_t *to_block_index)
#else
XStatus axidma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p)
#endif
{
	struct pbuf *q;
	s32_t n_pbufs;
	XAxiDma_Bd *txbdset, *txbd, *last_txbd = NULL;
	XStatus status;
	XAxiDma_BdRing *txring;
	u32_t max_frame_size;
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
	u32_t bdindex = 0;
#endif

#ifdef USE_JUMBO_FRAMES
	max_frame_size = XAE_MAX_JUMBO_FRAME_SIZE - 18;
#else
	max_frame_size = XAE_MAX_FRAME_SIZE - 18;
#endif
	txring = XAxiDma_GetTxRing(&xaxiemacif->axidma);

	/* first count the number of pbufs */
	for (q = p, n_pbufs = 0; q != NULL; q = q->next)
		n_pbufs++;

	/* obtain as many BD's */
	status = XAxiDma_BdRingAlloc(txring, n_pbufs, &txbdset);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("sgsend: Error allocating TxBD\r\n"));
		return ERR_IF;
	}

	for(q = p, txbd = txbdset; q != NULL; q = q->next) {
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
		bdindex = XAxiDma_BD_TO_INDEX(txring, txbd);
#endif
		/* Send the data from the pbuf to the interface, one pbuf at a
		 * time. The size of the data in each pbuf is kept in the ->len
		 * variable.
		 */
		XAxiDma_BdSetBufAddr(txbd, (UINTPTR)q->payload);
		if (q->len > max_frame_size) {
			XAxiDma_BdSetLength(txbd, max_frame_size,
											txring->MaxTransferLen);
		}
		else {
			XAxiDma_BdSetLength(txbd, q->len, txring->MaxTransferLen);
		}
		XAxiDma_BdSetId(txbd, (void *)q);
		XAxiDma_BdSetCtrl(txbd, 0);
		XCACHE_FLUSH_DCACHE_RANGE(q->payload, q->len);

		pbuf_ref(q);

		last_txbd = txbd;
		txbd = (XAxiDma_Bd *)XAxiDma_BdRingNext(txring, txbd);
	}

	if (n_pbufs == 1) {
		XAxiDma_BdSetCtrl(txbdset, XAXIDMA_BD_CTRL_TXSOF_MASK
				| XAXIDMA_BD_CTRL_TXEOF_MASK);
	} else {
		/* in the first packet, set the SOP */
		XAxiDma_BdSetCtrl(txbdset, XAXIDMA_BD_CTRL_TXSOF_MASK);
		/* in the last packet, set the EOP */
		XAxiDma_BdSetCtrl(last_txbd, XAXIDMA_BD_CTRL_TXEOF_MASK);
	}

#if LWIP_FULL_CSUM_OFFLOAD_TX==1
	bd_fullcsum_disable(txbdset);
	if (p->len > sizeof(struct ethip_hdr)) {
		bd_fullcsum_enable(txbdset);
	}
#endif
#if LWIP_PARTIAL_CSUM_OFFLOAD_TX==1
	bd_csum_disable(txbdset);
	if (p->len > sizeof(struct ethip_hdr)) {
		struct ethip_hdr *ehdr = p->payload;
		u8_t proto = IPH_PROTO(&ehdr->ip);

		/* check if it is a TCP packet */
		if (htons(ehdr->eth.type) == ETHTYPE_IP && proto ==
											IP_PROTO_TCP) {
			u32_t iphdr_len, csum_insert_offset;
			u16_t tcp_len;	/* TCP header length + data length in bytes */
			u16_t csum_init = 0;
			u16_t tcp_payload_offset;

			/* determine length of IP header */
			iphdr_len = (IPH_HL(&ehdr->ip) * 4);

			tcp_payload_offset = XAE_HDR_SIZE + iphdr_len;
			tcp_len = p->tot_len - tcp_payload_offset;

			/* insert checksum at offset 16 for TCP, 6 for UDP */
			if (proto == IP_PROTO_TCP)
				csum_insert_offset = tcp_payload_offset + 16;
			else if (proto == IP_PROTO_UDP)
				csum_insert_offset = tcp_payload_offset + 6;
			else
				csum_insert_offset = 0;
			/* compute pseudo header checksum value */
			csum_init = inet_chksum_pseudo(NULL,
					(ip_addr_t *)&ehdr->ip.src,
					(ip_addr_t *)&ehdr->ip.dest, proto, tcp_len);
			/* init buffer descriptor */
			bd_csum_set(txbdset, tcp_payload_offset,
			csum_insert_offset, htons(~csum_init));
		}
	}
#endif

#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
    if (block_till_tx_complete == 1) {
        notifyinfo[bdindex] = 1;
        *to_block_index = bdindex;
    }
#endif

	/* enq to h/w */
	return XAxiDma_BdRingToHw(txring, n_pbufs, txbdset);
}

XStatus init_axi_dma(struct xemac_s *xemac)
{
	XAxiDma_Config *dmaconfig;
	XAxiDma_Bd bdtemplate;
	XAxiDma_BdRing *rxringptr, *txringptr;
	XAxiDma_Bd *rxbd;
	struct pbuf *p;
	XStatus status;
	u32_t i;
#if !defined(__MICROBLAZE__) && !defined(__riscv__)
	u32_t bd_space_index = 0;
#endif
	UINTPTR baseaddr;

	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);

#if XPAR_INTC_0_HAS_FAST == 1
	xaxiemacif_fast = xaxiemacif;
	xemac_fast = xemac;
#endif
#if NO_SYS
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
#endif
#if !NO_SYS
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
#endif

	/* FIXME: On ZyqnMP Multiple Axi Ethernet are not supported */
#if defined (__aarch64__)
	Xil_SetTlbAttributes((u64)bd_space, NORM_NONCACHE | INNER_SHAREABLE);
#elif defined (ARMR5)
	Xil_SetTlbAttributes((s32_t)bd_space, STRONG_ORDERD_SHARED | PRIV_RW_USER_RW);
#elif defined (__arm__)
	Xil_SetTlbAttributes((s32_t)bd_space, DEVICE_MEMORY);
#endif

#if defined (__MICROBLAZE__) || defined (__riscv)
	xaxiemacif->rx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_RX_DESC);
	xaxiemacif->tx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_TX_DESC);
#else
	xaxiemacif->rx_bdspace = (void *)(UINTPTR)&(bd_space[bd_space_index]);
	bd_space_index += BD_SIZE_64KB;
	xaxiemacif->tx_bdspace = (void *)(UINTPTR)&(bd_space[bd_space_index]);
#endif

	LWIP_DEBUGF(NETIF_DEBUG, ("rx_bdspace: 0x%08x\r\n",
												xaxiemacif->rx_bdspace));
	LWIP_DEBUGF(NETIF_DEBUG, ("tx_bdspace: 0x%08x\r\n",
												xaxiemacif->tx_bdspace));

	if (!xaxiemacif->rx_bdspace || !xaxiemacif->tx_bdspace) {
		xil_printf("%s@%d: Error: Unable to allocate memory for RX buffer descriptors",
				__FILE__, __LINE__);
		return ERR_IF;
	}
	/* initialize DMA */
#ifndef SDT
	baseaddr = xaxiemacif->axi_ethernet.Config.AxiDevBaseAddress;
	dmaconfig = XAxiDma_LookupConfigBaseAddr(baseaddr);
#else
	baseaddr = xaxiemacif->axi_ethernet.AxiDevBaseAddress;
	dmaconfig = XAxiDma_LookupConfig(baseaddr);
#endif
	XAxiDma_CfgInitialize(&xaxiemacif->axidma, dmaconfig);

	rxringptr = XAxiDma_GetRxRing(&xaxiemacif->axidma);
	txringptr = XAxiDma_GetTxRing(&xaxiemacif->axidma);
	LWIP_DEBUGF(NETIF_DEBUG, ("rxringptr: 0x%08x\r\n", rxringptr));
	LWIP_DEBUGF(NETIF_DEBUG, ("txringptr: 0x%08x\r\n", txringptr));

	/* Setup RxBD space.
	 * Setup a BD template for the Rx channel. This template will be copied to
	 * every RxBD. We will not have to explicitly set these again.
	 */
	XAxiDma_BdClear(&bdtemplate);
	/* Create the RxBD ring */
	status = XAxiDma_BdRingCreate(rxringptr, (UINTPTR) xaxiemacif->rx_bdspace,
					(UINTPTR) xaxiemacif->rx_bdspace, BD_ALIGNMENT,
					XLWIP_CONFIG_N_RX_DESC);

	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting up RxBD space\r\n"));
		return ERR_IF;
	}
	XAxiDma_BdClear(&bdtemplate);
	status = XAxiDma_BdRingClone(rxringptr, &bdtemplate);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error initializing RxBD space\r\n"));
		return ERR_IF;
	}
	/* Create the TxBD ring */
	status = XAxiDma_BdRingCreate(txringptr, (UINTPTR) xaxiemacif->tx_bdspace,
				     (UINTPTR) xaxiemacif->tx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_TX_DESC);
	if (status != XST_SUCCESS) {
		return ERR_IF;
	}
	/* We reuse the bd template, as the same one will work for both rx and tx. */
	status = XAxiDma_BdRingClone(txringptr, &bdtemplate);
	if (status != XST_SUCCESS) {
		return ERR_IF;
	}
	/* Allocate RX descriptors, 1 RxBD at a time.*/
	for (i = 0; i < XLWIP_CONFIG_N_RX_DESC; i++) {
		status = XAxiDma_BdRingAlloc(rxringptr, 1, &rxbd);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("init_axi_dma: Error allocating RxBD\r\n"));
			return ERR_IF;
		}
#ifdef USE_JUMBO_FRAMES
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_JUMBO_FRAME_SIZE, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			LWIP_DEBUGF(NETIF_DEBUG, ("unable to alloc pbuf in recv_handler\r\n"));
			return ERR_IF;
		}
		/* Setup the BD. The BD template used in the call to
		 * XAxiEthernet_SgSetSpace() set the "last" field of all RxBDs.
		 * Therefore we are not required to issue a XAxiDma_Bd_SetLast(rxbd)
		 * here.
		 */
		XAxiDma_BdSetBufAddr(rxbd, (UINTPTR)p->payload);
		XAxiDma_BdSetLength(rxbd, p->len, rxringptr->MaxTransferLen);
		XAxiDma_BdSetCtrl(rxbd, 0);
		XAxiDma_BdSetId(rxbd, p);
#ifdef USE_JUMBO_FRAMES
		XCACHE_FLUSH_DCACHE_RANGE((UINTPTR)p->payload, (UINTPTR)XAE_MAX_JUMBO_FRAME_SIZE);
#else
		XCACHE_FLUSH_DCACHE_RANGE((UINTPTR)p->payload, (UINTPTR)XAE_MAX_FRAME_SIZE);
#endif
#if !defined(__aarch64__)
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);
#endif

		/* Enqueue to HW */
		status = XAxiDma_BdRingToHw(rxringptr, 1, rxbd);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error: committing RxBD to HW\r\n"));
			return ERR_IF;
		}
	}

	status = XAxiDma_BdRingSetCoalesce(txringptr, XLWIP_CONFIG_N_TX_COALESCE,
							0x1);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return ERR_IF;
	}
	status = XAxiDma_BdRingSetCoalesce(rxringptr, XLWIP_CONFIG_N_RX_COALESCE,
						0x1);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return ERR_IF;
	}
	/* start DMA */
	status = XAxiDma_BdRingStart(txringptr);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error: failed to start TX BD ring\r\n"));
		return ERR_IF;
	}
	status = XAxiDma_BdRingStart(rxringptr);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error: failed to start RX BD ring\r\n"));
		return ERR_IF;
	}
	/* enable DMA interrupts */
	XAxiDma_BdRingIntEnable(txringptr, XAXIDMA_IRQ_ALL_MASK);
	XAxiDma_BdRingIntEnable(rxringptr, XAXIDMA_IRQ_ALL_MASK);

#ifndef SDT
#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(Xil_ExceptionHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);
	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
				xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
				(Xil_ExceptionHandler)axidma_send_handler,
				xemac);
	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
				xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
				(Xil_ExceptionHandler)axidma_recv_handler,
				xemac);

	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			AXIETH_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);
	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			AXIDMA_TX_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);
        XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			AXIDMA_RX_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);

	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
				xaxiemacif->axi_ethernet.Config.TemacIntr);
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
				xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr);
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
				xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr);
#else
#if NO_SYS
#if XPAR_INTC_0_HAS_FAST == 1

	/* Register axiethernet interrupt with interrupt controller as Fast
							Interrupts */

	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XFastInterruptHandler)xaxiemac_errorfast_handler);

	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			(XFastInterruptHandler)axidma_sendfast_handler);

	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			(XFastInterruptHandler)axidma_recvfast_handler);
#else
	/* Register axiethernet interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XInterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);
	/* connect & enable DMA interrupts */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			(XInterruptHandler)axidma_send_handler,
				xemac);
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			(XInterruptHandler)axidma_recv_handler,
				xemac);
#endif
	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr +
							XIN_IER_OFFSET);

		/* form new mask enabling AXIDMA & axiethernet interrupts */
		cur_mask = cur_mask
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);
#else
#if XPAR_INTC_0_HAS_FAST == 1

	/* Register axiethernet interrupt with interrupt controller as Fast
							Interrupts */
	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XFastInterruptHandler)xaxiemac_errorfast_handler);

	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			(XFastInterruptHandler)axidma_sendfast_handler);

	XIntc_RegisterFastHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			(XFastInterruptHandler)axidma_recvfast_handler);
#else
	/* Register axiethernet interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XInterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);
	/* connect & enable DMA interrupts */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			(XInterruptHandler)axidma_send_handler,
				xemac);
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			(XInterruptHandler)axidma_recv_handler,
				xemac);
#endif
	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr +
							XIN_IER_OFFSET);

		/* form new mask enabling AXIDMA & axiethernet interrupts */
		cur_mask = cur_mask
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);
#endif
#endif
#else
	XSetupInterruptSystem(&xaxiemacif->axi_ethernet, &xaxiemac_error_handler,
			      xaxiemacif->axi_ethernet.Config.IntrId,
			      xaxiemacif->axi_ethernet.Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
	XSetupInterruptSystem(xemac, &axidma_send_handler,
			      dmaconfig->IntrId[0],
			      dmaconfig->IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
	XSetupInterruptSystem(xemac, &axidma_recv_handler,
			      dmaconfig->IntrId[1],
			      dmaconfig->IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);

#endif
	return 0;
}

#ifndef SDT
#if XPAR_INTC_0_HAS_FAST == 1
/****************************** Fast receive Handler *************************/
static void axidma_recvfast_handler(void)
{
	axidma_recv_handler((void *)xemac_fast);
}

/****************************** Fast Send Handler ****************************/
static void axidma_sendfast_handler(void)
{
	axidma_send_handler((void *)xemac_fast);
}

/****************************** Fast Error Handler ***************************/
static void xaxiemac_errorfast_handler(void)
{
	xaxiemac_error_handler(&xaxiemacif_fast->axi_ethernet);
}
#endif
#endif
