/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
#include "netif/xaxiemacif.h"

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#include "xscugic.h"
#else
#include "xintc_l.h"
#endif

#include "xstatus.h"

#include "xlwipconfig.h"
#include "xparameters.h"

#ifdef CONFIG_XTRACE
#include "xtrace.h"
#endif

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#define AXIDMA_TX_INTR_PRIORITY_SET_IN_GIC	0xA0
#define AXIDMA_RX_INTR_PRIORITY_SET_IN_GIC	0xA0
#define AXIETH_INTR_PRIORITY_SET_IN_GIC		0xA0
#define TRIG_TYPE_RISING_EDGE_SENSITIVE		0x3


#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_DIST_BASEADDR

#ifndef XCACHE_FLUSH_DCACHE_RANGE
#define XCACHE_FLUSH_DCACHE_RANGE(data, length)	\
		Xil_DCacheFlushRange((u32)data, length)
#endif
#ifndef XCACHE_INVALIDATE_DCACHE_RANGE
#define XCACHE_INVALIDATE_DCACHE_RANGE(data, length)	\
		Xil_DCacheInvalidateRange((u32)data, length)
#endif

#endif

/* Byte alignment of BDs */
#define BD_ALIGNMENT (XAXIDMA_BD_MINIMUM_ALIGNMENT*2)

#ifdef USE_STATIC_BDSPACE
#define RXBD_SPACE_BYTES XAxiDma_BdRingMemCalc(BD_ALIGNMENT, XLWIP_CONFIG_N_RX_DESC)
#define TXBD_SPACE_BYTES XAxiDma_BdRingMemCalc(BD_ALIGNMENT, XLWIP_CONFIG_N_TX_DESC)
char rx_bdspace[RXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char tx_bdspace[TXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
#endif

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

static unsigned sdma_debug = 0;
#undef SDMA_DEBUG

#ifdef SDMA_DEBUG
void print_packet(char *p, int n, int l)
{
	int i, j;

	xil_printf("Prnt size: %d  Packet size: %d\r\n", n, l);
	for (i = 0; i < n; i+=16) {
		for (j = 0; j < 16; j++)
			xil_printf("%02x ", *p++&0xff);
		xil_printf("\r\n");
	}
}
#endif

int is_tx_space_available(xaxiemacif_s *emac)
{
	XAxiDma_BdRing *txring;

	txring = XAxiDma_GetTxRing(&emac->axidma);

	/* tx space is available as long as there are valid BD's */
	return XAxiDma_BdRingGetFreeCnt(txring);
}

int process_sent_bds(XAxiDma_BdRing *txring)
{
	XAxiDma_Bd *txbdset, *txbd;
	int n_bds, i;
	XStatus Status;

	/* obtain a list of processed BD's */
	n_bds = XAxiDma_BdRingFromHw(txring, XAXIDMA_ALL_BDS, &txbdset);
	if (n_bds == 0)
		return -1;

	/* free the pbuf associated with each BD */
	for (i = 0, txbd = txbdset; i < n_bds; i++) {
		struct pbuf *p = (struct pbuf *)XAxiDma_BdGetId(txbd);
		pbuf_free(p);
		txbd = XAxiDma_BdRingNext(txring, txbd);
	}

	/* free the processed BD's */
	Status = XAxiDma_BdRingFree(txring, n_bds, txbdset);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
}

void axidma_send_handler(void *arg)
{
	unsigned irq_status;
	struct xemac_s *xemac;
	xaxiemacif_s   *xaxiemacif;
	XAxiDma_BdRing *TxRingPtr;
	struct xtopology_t *xtopologyp;
	XAxiEthernet *xaxiemac;

	xemac = (struct xemac_s *)(arg);
	xaxiemacif = (xaxiemacif_s *)(xemac->state);
	TxRingPtr = XAxiDma_GetTxRing(&xaxiemacif->axidma);
	xtopologyp = &xtopology[xemac->topology_index];
	xaxiemac = &xaxiemacif->axi_ethernet;

	/* Read pending interrupts */
	irq_status = XAxiDma_BdRingGetIrq(TxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(TxRingPtr, irq_status);

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((irq_status & XAXIDMA_IRQ_ERROR_MASK)) {
		xil_printf("%s: Error: axidma error interrupt is asserted\r\n",
			__FUNCTION__);
		XAxiDma_Reset(&xaxiemacif->axidma);
		return;
	}

	/* If Transmit done interrupt is asserted, process completed BD's */
	if ((irq_status & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		process_sent_bds(TxRingPtr);
	}
}

#if XPAR_AXIETHERNET_0_TXCSUM == 1
#if !CHECKSUM_GEN_TCP
static void bd_csum_enable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 | 1));
}

static void bd_csum_disable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 & ~1));
}

static void bd_csum_set(XAxiDma_Bd *bd, u16_t tx_csbegin, u16_t tx_csinsert,
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
#endif
#endif

#if XPAR_AXIETHERNET_0_TXCSUM == 2
static void bd_fullcsum_disable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 & ~3));
}

static void bd_fullcsum_enable(XAxiDma_Bd *bd)
{
	XAxiDma_BdWrite((bd), XAXIDMA_BD_USR0_OFFSET,
		(XAxiDma_BdRead((bd), XAXIDMA_BD_USR0_OFFSET)
		 | 2));
}

#endif

XStatus axidma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p)
{
	struct pbuf *q;
	int n_pbufs;
	XAxiDma_Bd *txbdset, *txbd, *last_txbd = NULL;
	XStatus Status;
	XAxiDma_BdRing *txring;
	unsigned max_frame_size;

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
	Status = XAxiDma_BdRingAlloc(txring, n_pbufs, &txbdset);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("sgsend: Error allocating TxBD\r\n"));
		return ERR_IF;
	}

	for(q = p, txbd = txbdset; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		XAxiDma_BdSetBufAddr(txbd, (u32)q->payload);
		if (q->len > max_frame_size) {
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
			XAxiDma_BdSetLength(txbd, max_frame_size);
#else
			XAxiDma_BdSetLength(txbd, max_frame_size,
											txring->MaxTransferLen);
#endif
		}
		else {
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
			XAxiDma_BdSetLength(txbd, q->len);
#else
			XAxiDma_BdSetLength(txbd, q->len, txring->MaxTransferLen);
#endif
		}
		XAxiDma_BdSetId(txbd, (void *)q);
		XAxiDma_BdSetCtrl(txbd, 0);
		XCACHE_FLUSH_DCACHE_RANGE(q->payload, q->len);

#ifdef SDMA_DEBUG
		if (sdma_debug) {
			LWIP_DEBUGF(NETIF_DEBUG, ("sending packet:\r\n"));
			print_packet(q->payload, 60, q->len);
		}
#endif

		pbuf_ref(q);

		last_txbd = txbd;
		txbd = XAxiDma_BdRingNext(txring, txbd);
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

#if XPAR_AXIETHERNET_0_TXCSUM == 1
#if !CHECKSUM_GEN_TCP
	bd_csum_disable(txbdset);

	/* offload TCP checksum calculation to hardware */
	if (XAxiEthernet_IsTxPartialCsum(&xaxiemacif->axi_ethernet)) {
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
	}
#endif
#endif

#if XPAR_AXIETHERNET_0_TXCSUM == 2
	bd_fullcsum_disable(txbdset);

	if (p->len > sizeof(struct ethip_hdr)) {
		bd_fullcsum_enable(txbdset);
	}
#endif
	/* enq to h/w */
	return XAxiDma_BdRingToHw(txring, n_pbufs, txbdset);

}

void setup_rx_bds(XAxiDma_BdRing *rxring)
{
	XAxiDma_Bd *rxbd;
	int n_bds, i;
	XStatus Status;
	struct pbuf *p;
	u32 BdSts;

	n_bds = XAxiDma_BdRingGetFreeCnt(rxring);
	if (n_bds == 0)
		return;

	for (i = 0; i < n_bds; i++) {
		Status = XAxiDma_BdRingAlloc(rxring, 1, &rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("setup_rx_bds: Error allocating RxBD\r\n"));
			return;
		}

#ifdef USE_JUMBO_FRAMES
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_JUMBO_FRAME_SIZE, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, XAE_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
			XAxiDma_BdRingUnAlloc(rxring, 1, rxbd);
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
		 * Setup the BD.
		 */
		XAxiDma_BdSetBufAddr(rxbd, (u32)p->payload);
		/* Clear everything but the COMPLETE bit, which is cleared when
		 * committed to hardware.
		 */
		BdSts = XAxiDma_BdGetSts(rxbd);
		BdSts &=  XAXIDMA_BD_STS_COMPLETE_MASK;
		XAxiDma_BdWrite(rxbd, XAXIDMA_BD_STS_OFFSET, BdSts);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
		XAxiDma_BdSetLength(rxbd, p->len);
#else
		XAxiDma_BdSetLength(rxbd, p->len, rxring->MaxTransferLen);
#endif
		XAxiDma_BdSetCtrl(rxbd, 0);
		XAxiDma_BdSetId(rxbd, p);
		XCACHE_FLUSH_DCACHE_RANGE(p, sizeof *p);
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);

		/* Enqueue to HW */
		Status = XAxiDma_BdRingToHw(rxring, 1, rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to hardware: "));
			if (Status == XST_DMA_SG_LIST_ERROR)
				LWIP_DEBUGF(NETIF_DEBUG, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with XAxiDma_BdRingAlloc()\r\n"));
			else
				LWIP_DEBUGF(NETIF_DEBUG, ("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
			return;
		}
	}
}

/*
 * zero out the checksum field of this packet
 */
void zero_checksum_field(struct pbuf *p)
{
	/* the way checksum works in this implementation of lwIP is as follows:
	 - if checksum offload is disabled, then lwIP stack performs all checksum
	   calculations
	 - if checksum is enabled,
	 - if computed csum says packet is valid, then we zero out the checksum
	   field
	 - if computed csum says packet is invalid, we leave checksum as is
	 - upper layer recomputes checksum if it finds a non-zero checksum value
	 */
	struct ethip_hdr *ehdr = p->payload;
	u32_t iphdr_len;
	u16_t tcp_payload_offset;
	u8_t proto = IPH_PROTO(&ehdr->ip);

	iphdr_len = (IPH_HL(&ehdr->ip) * 4);
	tcp_payload_offset = XAE_HDR_SIZE + iphdr_len;

	if (htons(ehdr->eth.type) == ETHTYPE_IP && proto == IP_PROTO_TCP) {
		/* set checksum = 0 */
		*(u16_t*)(p->payload + tcp_payload_offset + 16) = 0;
	}
}

void zero_tcp_ip_checksum_fields(struct pbuf *p)
{
	/* the way checksum works in this implementation of lwIP is as follows:
	 - if checksum offload is disabled, then lwIP stack performs all checksum
	   calculations
	 - if checksum is enabled,
	 - if computed csum says packet is valid, then we zero out the checksum
	   field
	 - if computed csum says packet is invalid, we leave checksum as is
	 - upper layer recomputes checksum if it finds a non-zero checksum value
	 */
	struct ethip_hdr *ehdr = p->payload;
	u32_t iphdr_len;
	u8_t proto = IPH_PROTO(&ehdr->ip);

	iphdr_len = (IPH_HL(&ehdr->ip) * 4);

	if (htons(ehdr->eth.type) == ETHTYPE_IP)  {
		*(u16_t*)(p->payload + XAE_HDR_SIZE + 10) = 0;
		if (proto == IP_PROTO_TCP) {
			/* set checksum = 0 */
			*(u16_t*)(p->payload + XAE_HDR_SIZE + iphdr_len + 16) = 0;
		}
		if (proto == IP_PROTO_UDP) {
			/* set checksum = 0 */
			*(u16_t*)(p->payload + XAE_HDR_SIZE + iphdr_len + 6) = 0;
		}
	}
}


u32_t csum_sub(u32_t csum, u16_t v)
{
	csum += (u32_t)v;
	return csum + (csum < (u32_t)v);
}

static u16_t extract_packet_len(XAxiDma_Bd *rxbd) {
    u16_t packet_len = XAxiDma_BdRead(rxbd, XAXIDMA_BD_USR4_OFFSET) & 0x3fff;
    return packet_len;
}

static u16_t extract_csum(XAxiDma_Bd *rxbd) {
    u16_t csum = XAxiDma_BdRead(rxbd, XAXIDMA_BD_USR3_OFFSET) & 0xffff;
    return csum;
}

static u32_t extract_csum_valid_status(XAxiDma_Bd *rxbd) {
	u32_t status = (XAxiDma_BdRead(rxbd, XAXIDMA_BD_USR2_OFFSET) & 0x00000038);
	status = status  >> 3;
	return status;
}

int is_tcp_ip_checksum_valid(XAxiDma_Bd *rxbd) {
	u32_t csum_status = extract_csum_valid_status(rxbd);
	if (csum_status == 0x00000002)
		return 1;
	else
		return 0;
}


/*
 * compare if the h/w computed checksum (stored in the rxbd)
 * equals the TCP checksum value in the packet
 */
int is_checksum_valid(XAxiDma_Bd *rxbd, struct pbuf *p) {
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
					(ip_addr_t *)&ehdr->ip.src, (ip_addr_t *)&ehdr->ip.dest,
					proto, tcp_payload_len));

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

void axidma_recv_handler(void *arg)
{
	struct pbuf *p;
	unsigned irq_status, i;
	XAxiDma_Bd *rxbd, *rxbdset;
	struct xemac_s *xemac;
	xaxiemacif_s *xaxiemacif;
	XAxiDma_BdRing *rxring;
	struct xtopology_t *xtopologyp;
	XAxiEthernet *xaxiemac;

	xemac = (struct xemac_s *)(arg);
	xaxiemacif = (xaxiemacif_s *)(xemac->state);
	rxring = XAxiDma_GetRxRing(&xaxiemacif->axidma);
	xtopologyp = &xtopology[xemac->topology_index];
	xaxiemac = &xaxiemacif->axi_ethernet;

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
		xil_printf("%s: Error: axidma error interrupt is asserted\r\n",
			__FUNCTION__);
		XAxiDma_Reset(&xaxiemacif->axidma);
		return;
	}

	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((irq_status & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		int bd_processed;
		int rx_bytes;

		bd_processed = XAxiDma_BdRingFromHw(rxring, XAXIDMA_ALL_BDS, &rxbdset);

		for (i = 0, rxbd = rxbdset; i < bd_processed; i++) {
			p = (struct pbuf *)XAxiDma_BdGetId(rxbd);

			/*
			 * Adjust the buffer size to the actual number of bytes received.
			 */
			rx_bytes = extract_packet_len(rxbd);
			pbuf_realloc(p, rx_bytes);

#ifdef USE_JUMBO_FRAMES
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload,
							XAE_MAX_JUMBO_FRAME_SIZE);
#else
			XCACHE_INVALIDATE_DCACHE_RANGE(p->payload, XAE_MAX_FRAME_SIZE);
#endif

#if XPAR_AXIETHERNET_0_RXCSUM == 1
#if !CHECKSUM_CHECK_TCP
			/* compare RX checksum to TCP checksum value */
			if (is_checksum_valid(rxbd, p)) {
				/* if checksum is correct, then we re-write
				 * checksum field with 0's so the upper layer doesn't recompute
				 */
				zero_checksum_field(p);
#ifdef SDMA_DEBUG
			} else {
				LWIP_DEBUGF(NETIF_DEBUG, ("RX CHECKSUM INVALID for offload\r\n"));
#endif
			}

#endif
#endif

#if XPAR_AXIETHERNET_0_RXCSUM == 2
#if !CHECKSUM_CHECK_TCP
#if !CHECKSUM_CHECK_IP
	if (is_tcp_ip_checksum_valid(rxbd)) {
		zero_tcp_ip_checksum_fields(p);
#ifdef SDMA_DEBUG
	} else {
		LWIP_DEBUGF(NETIF_DEBUG, ("TCP/IP CHECKSUM INVALID for offload\r\n"));
#endif
	}
#endif
#endif
#endif

#ifdef SDMA_DEBUG
			if (sdma_debug) {
			  LWIP_DEBUGF(NETIF_DEBUG, ("receiving packet:\r\n"));
				print_packet(p->payload, 60, rx_bytes);
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
			} else {
#if !NO_SYS
				sys_sem_signal(&xemac->sem_rx_data_available);
#endif
			}

			rxbd = XAxiDma_BdRingNext(rxring, rxbd);
		}

		/* free up the BD's */
		XAxiDma_BdRingFree(rxring, bd_processed, rxbdset);

		/* return all the processed bd's back to the stack */
		/* setup_rx_bds -> use XAxiDma_BdRingGetFreeCnt */
		setup_rx_bds(rxring);
	}

	XAxiDma_BdRingIntEnable(rxring, XAXIDMA_IRQ_ALL_MASK);
}

void *alloc_bdspace(int n_desc)
{
	int space = XAxiDma_BdRingMemCalc(BD_ALIGNMENT, n_desc);
	int padding = BD_ALIGNMENT*2;
	void *unaligned_mem = mem_malloc(space + padding*4);
	void *aligned_mem =
	(void *)(((unsigned)(unaligned_mem + BD_ALIGNMENT)) & ~(BD_ALIGNMENT - 1));

#ifdef SDMA_DEBUG
	xil_printf("unaligned_mem start: %8x, end: %8x\r\n",
						unaligned_mem, unaligned_mem + space + padding * 4);
	xil_printf("  aligned_mem start: %8x, end: %8x\r\n",
						aligned_mem, aligned_mem + space);
#endif
#if DEBUG
	assert(aligned_mem > unaligned_mem);
	assert(aligned_mem + space < unaligned_mem + space + padding);
#endif

	return aligned_mem;
}

XStatus init_axi_dma(struct xemac_s *xemac)
{
	XAxiDma_Config *DMAConfig;
	XAxiDma_Bd BdTemplate;
	XAxiDma_BdRing *RxRingPtr, *TxRingPtr;
	XAxiDma_Bd *rxbd;
	struct pbuf *p;
	XStatus Status;
	int i;
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
#if XPAR_INTC_0_HAS_FAST == 1
	xaxiemacif_fast = xaxiemacif;
	xemac_fast = xemac;
#endif
#if NO_SYS
	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];
#endif

	/* initialize DMA */
	DMAConfig = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
	XAxiDma_CfgInitialize(&xaxiemacif->axidma, DMAConfig);

	RxRingPtr = XAxiDma_GetRxRing(&xaxiemacif->axidma);
	TxRingPtr = XAxiDma_GetTxRing(&xaxiemacif->axidma);
	LWIP_DEBUGF(NETIF_DEBUG, ("RxRingPtr: 0x%08x\r\n", RxRingPtr));
	LWIP_DEBUGF(NETIF_DEBUG, ("TxRingPtr: 0x%08x\r\n", TxRingPtr));

	xaxiemacif->rx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_RX_DESC);
	xaxiemacif->tx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_TX_DESC);
	LWIP_DEBUGF(NETIF_DEBUG, ("rx_bdspace: 0x%08x\r\n",
												xaxiemacif->rx_bdspace));
	LWIP_DEBUGF(NETIF_DEBUG, ("tx_bdspace: 0x%08x\r\n",
												xaxiemacif->tx_bdspace));

	if (!xaxiemacif->rx_bdspace || !xaxiemacif->tx_bdspace) {
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
	XAxiDma_BdClear(&BdTemplate);

	/*
	 * Create the RxBD ring
	 */
#ifdef USE_STATIC_BDSPACE
	Status = XAxiDma_BdRingCreate(RxRingPtr, (u32) &rx_bdspace,
					(u32) &rx_bdspace, BD_ALIGNMENT,
					XLWIP_CONFIG_N_RX_DESC);
#else
	Status = XAxiDma_BdRingCreate(RxRingPtr, (u32) xaxiemacif->rx_bdspace,
					(u32) xaxiemacif->rx_bdspace, BD_ALIGNMENT,
					XLWIP_CONFIG_N_RX_DESC);
#endif
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting up RxBD space\r\n"));
		return XST_FAILURE;
	}
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error initializing RxBD space\r\n"));
		return XST_FAILURE;
	}

	/*
	 * Create the TxBD ring
	 */
#ifdef USE_STATIC_BDSPACE
	Status = XAxiDma_BdRingCreate(TxRingPtr, (u32) &tx_bdspace,
				     (u32) &tx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_TX_DESC);
#else
	Status = XAxiDma_BdRingCreate(TxRingPtr, (u32) xaxiemacif->tx_bdspace,
				     (u32) xaxiemacif->tx_bdspace, BD_ALIGNMENT,
				     XLWIP_CONFIG_N_TX_DESC);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* We reuse the bd template, as the same one will work for both rx and tx. */
	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		return ERR_IF;
	}

	/* enable DMA interrupts */
	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);
	XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Allocate RX descriptors, 1 RxBD at a time.
	 */
	for (i = 0; i < XLWIP_CONFIG_N_RX_DESC; i++) {
		Status = XAxiDma_BdRingAlloc(RxRingPtr, 1, &rxbd);
		if (Status != XST_SUCCESS) {
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
			return -1;
		}

		/*
		 * Setup the BD. The BD template used in the call to
		 * XAxiEthernet_SgSetSpace() set the "last" field of all RxBDs.
		 * Therefore we are not required to issue a XAxiDma_Bd_SetLast(rxbd)
		 * here.
		 */
		XAxiDma_BdSetBufAddr(rxbd, (u32)p->payload);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
		XAxiDma_BdSetLength(rxbd, p->len);
#else
		XAxiDma_BdSetLength(rxbd, p->len, RxRingPtr->MaxTransferLen);
#endif
		XAxiDma_BdSetCtrl(rxbd, 0);
		XAxiDma_BdSetId(rxbd, p);
		XCACHE_FLUSH_DCACHE_RANGE(p, sizeof *p);
		XCACHE_FLUSH_DCACHE_RANGE(rxbd, sizeof *rxbd);

		/* Enqueue to HW */
		Status = XAxiDma_BdRingToHw(RxRingPtr, 1, rxbd);
		if (Status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error: committing RxBD to HW\r\n"));
			return XST_FAILURE;
		}
	}

	/* start DMA */
	Status = XAxiDma_BdRingStart(TxRingPtr);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error: failed to start TX BD ring\r\n"));
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error: failed to start RX BD ring\r\n"));
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingSetCoalesce(TxRingPtr, XLWIP_CONFIG_N_TX_COALESCE,
						0x1);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingSetCoalesce(RxRingPtr, XLWIP_CONFIG_N_RX_COALESCE,
						0x1);
	if (Status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return XST_FAILURE;
	}

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

	/* Register temac interrupt with interrupt controller as Fast
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

	/* Register temac interrupt with interrupt controller */


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

		/* form new mask enabling SDMA & temac interrupts */
		cur_mask = cur_mask
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr)
			| (1 << xaxiemacif->axi_ethernet.Config.TemacIntr);

		/* set new mask */
		XIntc_EnableIntr(xtopologyp->intc_baseaddr, cur_mask);
	} while (0);

#else
	/* connect & enable TEMAC interrupts */
	register_int_handler(xaxiemacif->axi_ethernet.Config.TemacIntr,
			(XInterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);
	enable_interrupt(xaxiemacif->axi_ethernet.Config.TemacIntr);

	/* connect & enable DMA interrupts */
	register_int_handler(xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr,
			(XInterruptHandler)axidma_send_handler,
			xemac);
	enable_interrupt(xaxiemacif->axi_ethernet.Config.AxiDmaTxIntr);

	register_int_handler(xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr,
			(XInterruptHandler)axidma_recv_handler,
			xemac);
	enable_interrupt(xaxiemacif->axi_ethernet.Config.AxiDmaRxIntr);
#endif
#endif
	return 0;
}

#if XPAR_INTC_0_HAS_FAST == 1


/****************************** Fast receive Handler *************************/

void axidma_recvfast_handler(void)
{
	axidma_recv_handler((void *)xemac_fast);
}

/****************************** Fast Send Handler ****************************/
void axidma_sendfast_handler(void)
{
	axidma_send_handler((void *)xemac_fast);
}

/****************************** Fast Error Handler ***************************/
void xaxiemac_errorfast_handler(void)
{
	xaxiemac_error_handler(&xaxiemacif_fast->axi_ethernet);
}

#endif
