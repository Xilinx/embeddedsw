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
#include "lwip/inet_chksum.h"

#include "netif/xadapter.h"
#include "netif/xlltemacif.h"

#include "xintc_l.h"
#include "xstatus.h"

#include "xlltemacif_fifo.h"

#include "xlwipconfig.h"
#include "xparameters.h"

#ifdef CONFIG_XTRACE
#include "xtrace.h"
#endif

/* Byte alignment of BDs */
#define BD_ALIGNMENT (XLLDMA_BD_MINIMUM_ALIGNMENT*2)

//#define USE_STATIC_BDSPACE
#ifdef USE_STATIC_BDSPACE
#define RXBD_SPACE_BYTES XLlDma_BdRingMemCalc(BD_ALIGNMENT, XLWIP_CONFIG_N_RX_DESC)
#define TXBD_SPACE_BYTES XLlDma_BdRingMemCalc(BD_ALIGNMENT, XLWIP_CONFIG_N_TX_DESC)
char rx_bdspace[RXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char tx_bdspace[TXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
#endif

static unsigned sdma_debug = 0;
//#define SDMA_DEBUG

#ifdef SDMA_DEBUG
void print_packet(char *p, int n)
{
	int i, j;

	for (i = 0; i < n; i+=16) {
		for (j = 0; j < 16; j++)
			xil_printf("%02x ", *p++&0xff);
		xil_printf("\r\n");
	}
}
#endif

int
process_sent_bds(XLlDma_BdRing *txring)
{
	XLlDma_Bd *txbdset, *txbd;
	int n_bds, i;
	XStatus Status;

	/* obtain a list of processed BD's */
	n_bds = XLlDma_BdRingFromHw(txring, XLLDMA_ALL_BDS, &txbdset);
	if (n_bds == 0)
		return -1;

	/* free the pbuf associated with each BD */
	for (i = 0, txbd = txbdset; i < n_bds; i++) {
		struct pbuf *p = (struct pbuf *)XLlDma_BdGetId(txbd);
		pbuf_free(p);
		txbd = XLlDma_BdRingNext(txring, txbd);
	}

	/* free the processed BD's */
	Status = XLlDma_BdRingFree(txring, n_bds, txbdset);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
}

void
lldma_send_handler(void *arg)
{
	unsigned irq_status;
	struct xemac_s *xemac = (struct xemac_s *)(arg);
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
	XLlDma_BdRing *TxRingPtr = &XLlDma_GetTxRing(&xlltemacif->lldma);
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
	XLlTemac *xlltemac = &xlltemacif->lltemac;

	/* Read pending interrupts */
	irq_status = XLlDma_BdRingGetIrq(TxRingPtr);

	/* Acknowledge pending interrupts */
	XLlDma_BdRingAckIrq(TxRingPtr, irq_status);
	XIntc_AckIntr(xtopologyp->intc_baseaddr, 1 << xlltemac->Config.LLDmaTxIntr);

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((irq_status & XLLDMA_IRQ_ALL_ERR_MASK)) {
		LWIP_DEBUGF(NETIF_DEBUG, ("%s: Error: irq_status & XLLDMA_IRQ_ALL_ERR_MASK", __FUNCTION__));
		LWIP_DEBUGF(NETIF_DEBUG, ("lldma error interrupt is asserted\r\n"));
		XLlDma_Reset(&xlltemacif->lldma);
		return;
	}

	/* If Transmit done interrupt is asserted, process completed BD's */
	if ((irq_status & (XLLDMA_IRQ_DELAY_MASK | XLLDMA_IRQ_COALESCE_MASK))) {
		process_sent_bds(TxRingPtr);
	}
}

#if !CHECKSUM_GEN_TCP
static void
_bd_csum_enable(XLlDma_Bd *bd)
{
	XLlDma_BdWrite((bd), XLLDMA_BD_STSCTRL_USR0_OFFSET,
		(XLlDma_BdRead((bd), XLLDMA_BD_STSCTRL_USR0_OFFSET)
		 | 1));
}

static void
_bd_csum_disable(XLlDma_Bd *bd)
{
	XLlDma_BdWrite((bd), XLLDMA_BD_STSCTRL_USR0_OFFSET,
		(XLlDma_BdRead((bd), XLLDMA_BD_STSCTRL_USR0_OFFSET)
		 & ~1));
}

static void
_bd_csum_set(XLlDma_Bd *bd, u16_t tx_csbegin, u16_t tx_csinsert, u16_t tx_csinit)
{
	u32_t app1;

	_bd_csum_enable(bd);

	/* write start offset and insert offset into BD */
	app1 = ((u32_t)tx_csbegin << 16) | (u32_t) tx_csinsert;
	XLlDma_BdWrite(bd, XLLDMA_BD_USR1_OFFSET, app1);

	/* insert init value */
	XLlDma_BdWrite(bd, XLLDMA_BD_USR2_OFFSET, tx_csinit);
}
#endif

XStatus
lldma_sgsend(xlltemacif_s *xlltemacif, struct pbuf *p)
{
	struct pbuf *q;
	int n_pbufs;
	XLlDma_Bd *txbdset, *txbd, *last_txbd = NULL;
	XStatus Status;
	XLlDma_BdRing *txring;
        unsigned max_frame_size;

#ifdef USE_JUMBO_FRAMES
        max_frame_size = XTE_MAX_JUMBO_FRAME_SIZE - 18;
#else
        max_frame_size = XTE_MAX_FRAME_SIZE - 18;
#endif

	txring = &XLlDma_GetTxRing(&xlltemacif->lldma);

	/* first count the number of pbufs */
	for (q = p, n_pbufs = 0; q != NULL; q = q->next)
		n_pbufs++;

	/* obtain as many BD's */
	Status = XLlDma_BdRingAlloc(txring, n_pbufs, &txbdset);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error allocating RxBD"));
		return ERR_IF;
	}

	for(q = p, txbd = txbdset; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		XLlDma_BdSetBufAddr(txbd, q->payload);
		if (q->len > max_frame_size)
			XLlDma_BdSetLength(txbd, max_frame_size);
		else
			XLlDma_BdSetLength(txbd, q->len);
		XLlDma_BdSetId(txbd, (void *)q);
		XLlDma_BdSetStsCtrl(txbd, 0);
		XCACHE_FLUSH_DCACHE_RANGE(q->payload, q->len);

#ifdef SDMA_DEBUG
		if (sdma_debug) {
			LWIP_DEBUGF(NETIF_DEBUG, ("sending packet:\r\n"));
			print_packet(q->payload, 60);
		}
#endif

		pbuf_ref(q);

		last_txbd = txbd;
		txbd = XLlDma_BdRingNext(txring, txbd);
	}

	if (n_pbufs == 1) {
		XLlDma_BdSetStsCtrl(txbdset, XLLDMA_BD_STSCTRL_SOP_MASK
				| XLLDMA_BD_STSCTRL_EOP_MASK);
	} else {
		/* in the first packet, set the SOP */
		XLlDma_BdSetStsCtrl(txbdset, XLLDMA_BD_STSCTRL_SOP_MASK);
		/* in the last packet, set the EOP */
		XLlDma_BdSetStsCtrl(last_txbd, XLLDMA_BD_STSCTRL_EOP_MASK);
	}

#if !CHECKSUM_GEN_TCP
	_bd_csum_disable(txbdset);

	/* offload TCP checksum calculation to hardware */
	if (XLlTemac_IsTxCsum(&xlltemacif->lltemac)) {
		if (p->len > sizeof(struct ethip_hdr)) {
			struct ethip_hdr *ehdr = p->payload;
			u8_t proto = IPH_PROTO(&ehdr->ip);

			/* check if it is a TCP packet */
			if (ehdr->eth.type == ETHTYPE_IP && proto == IP_PROTO_TCP) {
				u32_t iphdr_len, csum_insert_offset;
				u16_t tcp_len;	/* TCP header length + data length in bytes */
				u16_t csum_init = 0;
				u16_t tcp_payload_offset;

				/* determine length of IP header */
				iphdr_len = (IPH_HL(&ehdr->ip) * 4);

				tcp_payload_offset = XTE_HDR_SIZE + iphdr_len;
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
						(ip_addr_t *)&ehdr->ip.src, (ip_addr_t *)&ehdr->ip.dest,
						proto, tcp_len);

				/* init buffer descriptor */
				_bd_csum_set(txbdset, tcp_payload_offset, csum_insert_offset, ~csum_init);
			}
		}
	}
#endif

	/* enq to h/w */
	return XLlDma_BdRingToHw(txring, n_pbufs, txbdset);
}

void
_setup_rx_bds(XLlDma_BdRing *rxring)
{
	XLlDma_Bd *rxbd;
	int n_bds, i;
	XStatus Status;
	struct pbuf *p;

	n_bds = XLlDma_BdRingGetFreeCnt(rxring);
	if (n_bds == 0)
		return;

	for (i = 0; i < n_bds; i++) {
		Status = XLlDma_BdRingAlloc(rxring, 1, &rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error allocating RxBD"));
			return;
		}

#ifdef USE_JUMBO_FRAMES
		p = pbuf_alloc(PBUF_RAW, XTE_MAX_JUMBO_FRAME_SIZE, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, XTE_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			LWIP_DEBUGF(NETIF_DEBUG, ("unable to alloc pbuf in recv_handler\r\n"));
#ifdef LWIP_STATS
			if (sdma_debug) {
				stats_display();
				while (1)
					;
			}
#endif
			return;
		}

		/*
		 * Setup the BD. The BD template used in the call to XLlTemac_SgSetSpace()
		 * set the "last" field of all RxBDs. Therefore we are not required to
		 * issue a XLlDma_Bd_SetLast(rxbd) here.
		 */
		XLlDma_BdSetBufAddr(rxbd, p->payload);
		XLlDma_BdSetLength(rxbd, p->len);
		XLlDma_BdSetStsCtrl(rxbd, XLLDMA_BD_STSCTRL_SOP_MASK | XLLDMA_BD_STSCTRL_EOP_MASK);
		XLlDma_BdSetId(rxbd, p);
		XCACHE_FLUSH_DCACHE_RANGE(p, sizeof *p);
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);

		/* Enqueue to HW */
		Status = XLlDma_BdRingToHw(rxring, 1, rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to hardware: "));
			if (Status == XST_DMA_SG_LIST_ERROR)
				LWIP_DEBUGF(NETIF_DEBUG, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with XLlDma_BdRingAlloc()\r\n"));
			else
				LWIP_DEBUGF(NETIF_DEBUG, ("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
			return;
		}
	}
}

/*
 * zero out the checksum field of this packet
 */
void
zero_checksum_field(struct pbuf *p)
{
	/* the way checksum works in this implementation of lwIP is as follows:
		- if checksum offload is disabled, then lwIP stack performs all checksum calculations
		- if checksum is enabled,
			- if computed csum says packet is valid, then we zero out the checksum field
			- if computed csum says packet is invalid, we leave checksum as is
			- upper layer recomputes checksum if it finds a non-zero checksum value
	 */
	struct ethip_hdr *ehdr = p->payload;
	u32_t iphdr_len;
	u16_t tcp_payload_offset;
	u8_t proto = IPH_PROTO(&ehdr->ip);

	iphdr_len = (IPH_HL(&ehdr->ip) * 4);
	tcp_payload_offset = XTE_HDR_SIZE + iphdr_len;

	if (ehdr->eth.type == ETHTYPE_IP && proto == IP_PROTO_TCP) {
		/* set checksum = 0 */
		*(u16_t*)(p->payload + tcp_payload_offset + 16) = 0;
	}
}

u32_t
csum_sub(u32_t csum, u16_t v)
{
	csum += (u32_t)v;
	return csum + (csum < (u32_t)v);
}

static u16_t
extract_packet_len(XLlDma_Bd *rxbd) {
    u16_t packet_len = XLlDma_BdRead(rxbd, XLLDMA_BD_USR4_OFFSET) & 0x3fff;
    return packet_len;
}

static u16_t
extract_csum(XLlDma_Bd *rxbd) {
    u16_t csum = XLlDma_BdRead(rxbd, XLLDMA_BD_USR3_OFFSET) & 0xffff;
    return csum;
}

/*
 * compare if the h/w computed checksum (stored in the rxbd)
 * equals the TCP checksum value in the packet
 */
int
is_checksum_valid(XLlDma_Bd *rxbd, struct pbuf *p) {
	struct ethip_hdr *ehdr = p->payload;
	u8_t proto = IPH_PROTO(&ehdr->ip);

	/* check if it is a TCP packet */
	if (ehdr->eth.type == ETHTYPE_IP && proto == IP_PROTO_TCP) {
		u32_t iphdr_len;
		u16_t csum_in_packet, csum_in_rxbd, pseudo_csum, iphdr_csum, padding_csum;
		u16_t tcp_payload_offset;
		u32_t computed_csum;
		u16_t padding_len, tcp_payload_len, packet_len;
		u16_t csum;

		/* determine length of IP header */
		iphdr_len = (IPH_HL(&ehdr->ip) * 4);
		tcp_payload_offset = XTE_HDR_SIZE + iphdr_len;
		tcp_payload_len = IPH_LEN(&ehdr->ip) - IPH_HL(&ehdr->ip) * 4;
                packet_len = extract_packet_len(rxbd);
		padding_len = packet_len - tcp_payload_offset - tcp_payload_len;

		csum_in_packet = *(u16_t*)(p->payload + tcp_payload_offset + 16);
		csum_in_rxbd = extract_csum(rxbd);
		pseudo_csum = inet_chksum_pseudo(NULL,
					(ip_addr_t *)&ehdr->ip.src, (ip_addr_t *)&ehdr->ip.dest,
					proto, tcp_payload_len);

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

void
lldma_recv_handler(void *arg)
{
	struct pbuf *p;
	unsigned irq_status, i;
	XLlDma_Bd *rxbd, *rxbdset;
	struct xemac_s *xemac = (struct xemac_s *)(arg);
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
	XLlDma_BdRing *rxring = &XLlDma_GetRxRing(&xlltemacif->lldma);
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
	XLlTemac *xlltemac = &xlltemacif->lltemac;

	XIntc_AckIntr(xtopologyp->intc_baseaddr, 1 << xlltemac->Config.LLDmaRxIntr);
	XLlDma_BdRingIntDisable(rxring, XLLDMA_CR_IRQ_ALL_EN_MASK);

	/* Read pending interrupts */
	irq_status = XLlDma_BdRingGetIrq(rxring);

	/* Acknowledge pending interrupts */
	XLlDma_BdRingAckIrq(rxring, irq_status);

	/* If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((irq_status & XLLDMA_IRQ_ALL_ERR_MASK)) {
		LWIP_DEBUGF(NETIF_DEBUG, ("LlDma: Error: irq_status & XLLDMA_IRQ_ALL_ERR_MASK"));
		XLlDma_Reset(&xlltemacif->lldma);
		return;
	}

	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((irq_status & (XLLDMA_IRQ_DELAY_MASK | XLLDMA_IRQ_COALESCE_MASK))) {
		int bd_processed;
		bd_processed = XLlDma_BdRingFromHw(rxring, XLLDMA_ALL_BDS, &rxbdset);

		for (i = 0, rxbd = rxbdset; i < bd_processed; i++) {
			p = (struct pbuf *)XLlDma_BdGetId(rxbd);
#ifdef USE_JUMBO_FRAMES
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload, XTE_MAX_JUMBO_FRAME_SIZE);
#else
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload, XTE_MAX_FRAME_SIZE);
#endif

#if !CHECKSUM_CHECK_TCP
			/* compare RX checksum to TCP checksum value */
			if (is_checksum_valid(rxbd, p)) {
				/* if checksum is correct, then we re-write
				 * checksum field with 0's so the upper layer doesn't recompute
				 */
				zero_checksum_field(p);
			}
#endif

#ifdef SDMA_DEBUG
			if (sdma_debug)
				print_packet(p->payload, 60);
#endif

			/* store it in the receive queue,
			 * where it'll be processed by a different handler
			 */
			if (pq_enqueue(xlltemacif->recv_q, (void*)p) < 0) {
#if LINK_STATS
				lwip_stats.link.memerr++;
				lwip_stats.link.drop++;
#endif
				pbuf_free(p);
			} else {
#if !NO_SYS
				sys_sem_signal(&xemac->sem_rx_data_available);
#endif
			}

			rxbd = XLlDma_BdRingNext(rxring, rxbd);
		}

		/* free up the BD's */
		XLlDma_BdRingFree(rxring, bd_processed, rxbdset);

		/* return all the processed bd's back to the stack */
		/* _setup_rx_bds -> use XLlDma_BdRingGetFreeCnt */
		_setup_rx_bds(rxring);
	}

	XLlDma_BdRingIntEnable(rxring, XLLDMA_CR_IRQ_ALL_EN_MASK);
}

void *
alloc_bdspace(int n_desc)
{
	int space = XLlDma_BdRingMemCalc(BD_ALIGNMENT, n_desc);
	int padding = BD_ALIGNMENT*2;
	void *unaligned_mem = mem_malloc(space + padding*4);
	void *aligned_mem = (void *)(((unsigned)(unaligned_mem + BD_ALIGNMENT)) & ~(BD_ALIGNMENT - 1));

#ifdef SDMA_DEBUG
	xil_printf("unaligned_mem start: %8x, end: %8x\r\n", unaligned_mem, unaligned_mem + space + padding * 4);
	xil_printf("  aligned_mem start: %8x, end: %8x\r\n",   aligned_mem,   aligned_mem + space);
#endif
#if DEBUG
	assert(aligned_mem > unaligned_mem);
	assert(aligned_mem + space < unaligned_mem + space + padding);
#endif
	return aligned_mem;
}

XStatus
init_sdma(struct xemac_s *xemac)
{
	XLlDma_Bd BdTemplate;
	XLlDma_BdRing *RxRingPtr, *TxRingPtr;
	XLlDma_Bd *rxbd;
	struct pbuf *p;
	XStatus Status;
	int i;
	xlltemacif_s *xlltemacif = (xlltemacif_s *)(xemac->state);
#if NO_SYS
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
#endif

	/* initialize DMA */
	XLlDma_Initialize(&xlltemacif->lldma, xlltemacif->lltemac.Config.LLDevBaseAddress);

	RxRingPtr = &XLlDma_GetRxRing(&xlltemacif->lldma);
	TxRingPtr = &XLlDma_GetTxRing(&xlltemacif->lldma);

	xlltemacif->rx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_RX_DESC);
	xlltemacif->tx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_TX_DESC);

	if (!xlltemacif->rx_bdspace || !xlltemacif->tx_bdspace) {
		xil_printf("%s@%d: Error: Unable to allocate memory for RX buffer descriptors",
				__FILE__, __LINE__);
		return XST_FAILURE;
	}

	/*
	 * Setup RxBD space.
	 *
	 * Setup a BD template for the Rx channel. This template will be copied to
	 * every RxBD. We will not have to explicitly set these again.
	 */
	XLlDma_BdClear(&BdTemplate);

	/*
	 * Create the RxBD ring
	 */
#ifdef USE_STATIC_BDSPACE
	Status = XLlDma_BdRingCreate(RxRingPtr, (u32) &rx_bdspace,
				     (u32) &rx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_RX_DESC);
#else
	Status = XLlDma_BdRingCreate(RxRingPtr, (u32) xlltemacif->rx_bdspace,
				     (u32) xlltemacif->rx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_RX_DESC);
#endif
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting up RxBD space"));
		return XST_FAILURE;
	}
	Status = XLlDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error initializing RxBD space"));
		return XST_FAILURE;
	}

	/*
	 * Create the TxBD ring
	 */
#ifdef USE_STATIC_BDSPACE
	Status = XLlDma_BdRingCreate(TxRingPtr, (u32) &tx_bdspace,
				     (u32) &tx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_TX_DESC);
#else
	Status = XLlDma_BdRingCreate(TxRingPtr, (u32) xlltemacif->tx_bdspace,
				     (u32) xlltemacif->tx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_TX_DESC);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* We reuse the bd template, as the same one will work for both rx and tx. */
	Status = XLlDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		return ERR_IF;
	}

	/* enable DMA interrupts */
	XLlDma_BdRingIntEnable(TxRingPtr, XLLDMA_CR_IRQ_ALL_EN_MASK);
	XLlDma_BdRingIntEnable(RxRingPtr, XLLDMA_CR_IRQ_ALL_EN_MASK);

	/*
	 * Allocate 1 RxBD. Note that TEMAC utilizes an in-place allocation
	 * scheme. The returned rxbd will point to a free BD in the memory
	 * segment setup with the call to XLlTemac_SgSetSpace()
	 */
	for (i = 0; i < XLWIP_CONFIG_N_RX_DESC - 1; i++) {
		Status = XLlDma_BdRingAlloc(RxRingPtr, 1, &rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error allocating RxBD"));
			return ERR_IF;
		}

#ifdef USE_JUMBO_FRAMES
		p = pbuf_alloc(PBUF_RAW, XTE_MAX_JUMBO_FRAME_SIZE, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, XTE_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			LWIP_DEBUGF(NETIF_DEBUG, ("unable to alloc pbuf in recv_handler\r\n"));
			return -1;
		}

		/*
		 * Setup the BD. The BD template used in the call to XLlTemac_SgSetSpace()
		 * set the "last" field of all RxBDs. Therefore we are not required to
		 * issue a XLlDma_Bd_SetLast(rxbd) here.
		 */
		XLlDma_BdSetBufAddr(rxbd, p->payload);
		XLlDma_BdSetLength(rxbd, p->len);
		XLlDma_BdSetStsCtrl(rxbd, XLLDMA_BD_STSCTRL_SOP_MASK | XLLDMA_BD_STSCTRL_EOP_MASK);
		XLlDma_BdSetId(rxbd, p);
		XCACHE_FLUSH_DCACHE_RANGE(p, sizeof *p);
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);

		/* Enqueue to HW */
		Status = XLlDma_BdRingToHw(RxRingPtr, 1, rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to HW"));
			return XST_FAILURE;
		}
	}

	/* start DMA */
	Status = XLlDma_BdRingStart(TxRingPtr);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error failed to start TX BD ring"));
		return XST_FAILURE;
	}

	Status = XLlDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error failed to start RX BD ring"));
		return XST_FAILURE;
	}

	Status = XLlDma_BdRingSetCoalesce(TxRingPtr, XLWIP_CONFIG_N_TX_COALESCE, 1);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return XST_FAILURE;
	}

	Status = XLlDma_BdRingSetCoalesce(RxRingPtr, XLWIP_CONFIG_N_RX_COALESCE, 1);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return XST_FAILURE;
	}

#if NO_SYS
	/* Register temac interrupt with interrupt controller */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xlltemacif->lltemac.Config.TemacIntr,
			(XInterruptHandler)xlltemac_error_handler,
			&xlltemacif->lltemac);

	/* connect & enable DMA interrupts */
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xlltemacif->lltemac.Config.LLDmaTxIntr,
			(XInterruptHandler)lldma_send_handler,
			xemac);
	XIntc_RegisterHandler(xtopologyp->intc_baseaddr,
			xlltemacif->lltemac.Config.LLDmaRxIntr,
			(XInterruptHandler)lldma_recv_handler,
			xemac);

	/* Enable EMAC interrupts in the interrupt controller */
	do {
		/* read current interrupt enable mask */
		unsigned int cur_mask = XIntc_In32(xtopologyp->intc_baseaddr + XIN_IER_OFFSET);

		/* form new mask enabling SDMA & ll_temac interrupts */
		cur_mask = cur_mask
				| (1 << xlltemacif->lltemac.Config.LLDmaTxIntr)
				| (1 << xlltemacif->lltemac.Config.LLDmaRxIntr)
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

	/* connect & enable DMA interrupts */
	register_int_handler(xlltemacif->lltemac.Config.LLDmaTxIntr,
			(XInterruptHandler)lldma_send_handler,
			xemac);
	enable_interrupt(xlltemacif->lltemac.Config.LLDmaTxIntr);

	register_int_handler(xlltemacif->lltemac.Config.LLDmaRxIntr,
			(XInterruptHandler)lldma_recv_handler,
			xemac);
	enable_interrupt(xlltemacif->lltemac.Config.LLDmaRxIntr);
#endif

	return 0;
}
