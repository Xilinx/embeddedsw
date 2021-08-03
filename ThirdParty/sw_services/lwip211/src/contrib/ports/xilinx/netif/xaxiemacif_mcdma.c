/*
 * Copyright (C) 2018 - 2021 Xilinx, Inc.
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
#include "lwip/sys.h"
#endif

#include "lwip/stats.h"
#include "lwip/inet_chksum.h"

#include "netif/xadapter.h"
#include "netif/xaxiemacif.h"

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1
#include "xscugic.h"
#endif

#include "xstatus.h"

#include "xlwipconfig.h"
#include "xparameters.h"

#if defined __aarch64__
#include "xil_mmu.h"
#endif

#if defined ARMR5
#include "xil_mpu.h"
#endif

#define PARTIAL_CSUM_ENABLE	0x00000001 /* Option for partial csum enable */
#define FULL_CSUM_ENABLE	0x00000002 /* Option for full csum enable */

#define BD_USR0_OFFSET          0       /* AXI4-Stream Control Word offset from
					 * the start of app user words in BD.
					 * Offset 0 means, Control Word 0, used
					 * for enabling checksum offloading.
					 */
#define BD_USR1_OFFSET          1       /* AXI4-Stream Control Word offset from
					 * the start of app user words in BD.
					 * Offset means, Control Word 1, used
					 * for mentioning checksum begin and
					 * checksum insert points
					 */
#define BD_USR2_OFFSET          2       /* AXI4-Stream Control Word offset from
					 * the start of app user words in BD.
					 * Offset 2 means, Control Word 2, used
					 * for mentioning checksum seed.
					 */

#define XMCDMA_ALL_BDS          0xFFFF
#define XMCDMA_BD_LENGTH_MASK   0x007FFFFF
#define XMCDMA_COALESCEDELAY    0x1

#define RESET_TIMEOUT_COUNT     10000
#define BLOCK_SIZE_2MB          0x200000
#define BLOCK_SIZE_1MB          0x100000

#if defined (__aarch64__)
#define BD_SIZE                 BLOCK_SIZE_2MB
static u8_t bd_space[BD_SIZE] __attribute__ ((aligned (BLOCK_SIZE_2MB)));
#else
#define BD_SIZE                 BLOCK_SIZE_1MB
static u8_t bd_space[BD_SIZE] __attribute__ ((aligned (BLOCK_SIZE_1MB)));
#endif

static u8_t *bd_mem_ptr = bd_space;

#if !NO_SYS
u32 xInsideISR = 0;
#endif

static inline void bd_csum_enable(XMcdma_Bd *bd)
{
	XMcDma_BdSetAppWord(bd, BD_USR0_OFFSET, PARTIAL_CSUM_ENABLE);
}

static inline void bd_csum_disable(XMcdma_Bd *bd)
{
	XMcDma_BdSetAppWord(bd, BD_USR0_OFFSET, ~PARTIAL_CSUM_ENABLE);
}

static inline void bd_fullcsum_disable(XMcdma_Bd *bd)
{
	XMcDma_BdSetAppWord(bd, BD_USR0_OFFSET, ~FULL_CSUM_ENABLE);
}

static inline void bd_fullcsum_enable(XMcdma_Bd *bd)
{
	XMcDma_BdSetAppWord(bd, BD_USR0_OFFSET, FULL_CSUM_ENABLE);
}

static inline void bd_csum_set(XMcdma_Bd *bd, u16_t tx_csbegin,
		u16_t tx_csinsert, u16_t tx_csinit)
{
	u32_t app1;

	bd_csum_enable(bd);

	/* write start offset and insert offset into BD */
	app1 = ((u32_t)tx_csbegin << 16) | tx_csinsert;
	XMcDma_BdSetAppWord(bd, BD_USR1_OFFSET, app1);

	/* insert init value */
	XMcDma_BdSetAppWord(bd, BD_USR2_OFFSET, tx_csinit);
}

static inline u32_t extract_packet_len(XMcdma_Bd *rxbd) {
	return XMcDma_BdGetActualLength(rxbd, XMCDMA_BD_LENGTH_MASK);
}

static inline u16_t extract_csum(XMcdma_Bd *rxbd) {
	return XMcdma_BdRead64(rxbd, XMCDMA_BD_USR3_OFFSET) & 0xffff;
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
s32_t is_checksum_valid(XMcdma_Bd *rxbd, struct pbuf *p)
{
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
		tcp_payload_len = htons(IPH_LEN(&ehdr->ip)) -
							IPH_HL(&ehdr->ip) * 4;
		packet_len = extract_packet_len(rxbd);
		padding_len = packet_len - tcp_payload_offset - tcp_payload_len;

		csum_in_rxbd = extract_csum(rxbd);
		pseudo_csum = htons(ip_chksum_pseudo(NULL, proto,
					tcp_payload_len,
					(ip_addr_t *)&ehdr->ip.src,
					(ip_addr_t *)&ehdr->ip.dest));

		/* xps_ll_temac computes the checksum of the packet starting
		 * at byte XAE_HDR_SIZE we need to subtract the values of
		 * the ethernet & IP headers
		 */
		iphdr_csum  = inet_chksum(p->payload + XAE_HDR_SIZE, iphdr_len);

		/* compute csum of padding bytes, if any */
		padding_csum = inet_chksum(p->payload + p->tot_len -
						padding_len, padding_len);

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
			computed_csum = (computed_csum & 0xffff) +
							(computed_csum >> 16);
		}

		/* convert to 16 bits and take 1's complement */
		csum = (u16_t)computed_csum;
		csum = ~csum;

		/* chksum is valid if: computed csum over the packet is 0 */
		return !csum;
	} else {
		/* just say yes to all other packets */
		/* the upper layers in the stack will compute and
		 * verify the checksum */
		return 1;
	}
}

#define XMcdma_BdMemCalc(Alignment, NumBd) \
	(int)((sizeof(XMcdma_Bd)+((Alignment)-1)) & ~((Alignment)-1))*(NumBd)

static inline void *alloc_bdspace(int n_desc, u32 alignment)
{
	int space = XMcdma_BdMemCalc(alignment, n_desc);
	void *unaligned_mem = bd_mem_ptr;
	void *aligned_mem =
	(void *)(((UINTPTR)(unaligned_mem + alignment - 1)) & ~(alignment - 1));

	if (aligned_mem + space > (void *)(bd_space + BD_SIZE)) {
		xil_printf("Unable to allocate BD space\r\n");
		return NULL;
	}

	bd_mem_ptr = aligned_mem + space;

	return aligned_mem;
}

static void axi_mcdma_send_error_handler(void *CallBackRef, u32 ChanId, u32 Mask)
{
	u32 timeOut;
	XMcdma *McDmaInstPtr = (XMcdma *)((void *)CallBackRef);

#if !NO_SYS
	xInsideISR++;
#endif
	xil_printf("%s: Error: aximcdma error interrupt is asserted, Chan_id = "
			"%d, Mask = %d\r\n", __FUNCTION__, ChanId, Mask);

	XMcDma_Reset(McDmaInstPtr);
	timeOut = RESET_TIMEOUT_COUNT;
	while (timeOut) {
		if (XMcdma_ResetIsDone(McDmaInstPtr))
			break;
		timeOut -= 1;
	}

	if (!timeOut) {
		xil_printf("%s: Error: aximcdma reset timed out\r\n", __func__);
	}

#if !NO_SYS
	xInsideISR--;
#endif
}

static void axi_mcdma_send_handler(void *CallBackRef, u32 ChanId)
{
	XMcdma *McDmaInstPtr = (XMcdma *)((void *)CallBackRef);
	XMcdma_ChanCtrl *Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

#if !NO_SYS
	xInsideISR++;
#endif

	process_sent_bds(Tx_Chan);

#if !NO_SYS
	xInsideISR--;
#endif
}

static void setup_rx_bds(XMcdma_ChanCtrl *Rx_Chan, u32_t n_bds)
{
	XMcdma_Bd *rxbd;
	u32_t i = 0;
	XStatus status;
	struct pbuf *p;
	u32 bdsts;
#ifdef USE_JUMBO_FRAMES
	u32 max_frame_size = XAE_MAX_JUMBO_FRAME_SIZE + IEEE_1588_PAD_SIZE;
#else
	u32 max_frame_size = XAE_MAX_FRAME_SIZE + IEEE_1588_PAD_SIZE;
#endif

	for (i = 0; i < n_bds; i++) {
		p = pbuf_alloc(PBUF_RAW, max_frame_size, PBUF_POOL);
		if (!p) {
			xil_printf("unable to alloc pbuf in recv_handler\r\n");
			return;
		}

		rxbd = (XMcdma_Bd *)XMcdma_GetChanCurBd(Rx_Chan);
		status = XMcDma_ChanSubmit(Rx_Chan, (UINTPTR)p->payload,
									p->len);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("setup_rx_bds: Error allocating RxBD\r\n"));
			pbuf_free(p);
			return;
		}
		/* Clear everything but the COMPLETE bit, which is cleared when
		 * committed to hardware.
		 */
		bdsts = XMcDma_BdGetSts(rxbd);
		bdsts &=  XMCDMA_BD_STS_COMPLETE_MASK;
		XMcdma_BdWrite(rxbd, XMCDMA_BD_STS_OFFSET, bdsts);
		XMcDma_BdSetCtrl(rxbd, 0);
		XMcdma_BdSetSwId(rxbd, p);

#if defined(__aarch64__)
		Xil_DCacheInvalidateRange((UINTPTR)p->payload,
						(UINTPTR)max_frame_size);
#else
		Xil_DCacheFlushRange((UINTPTR)p->payload,
						(UINTPTR)max_frame_size);
#endif
	}

#if !defined (__MICROBLAZE__)
		dsb();
#endif

	if (n_bds) {
		/* Enqueue to HW */
		status = XMcDma_ChanToHw(Rx_Chan);
		if (status != XST_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to hardware\n\r"));
		}
	}
}

static void axi_mcdma_recv_error_handler(void *CallBackRef, u32 ChanId)
{
	u32 timeOut;
	XMcdma_ChanCtrl *Rx_Chan;
	struct xemac_s *xemac = (struct xemac_s *)(CallBackRef);
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	XMcdma *McDmaInstPtr = &xaxiemacif->aximcdma;

#if !NO_SYS
	xInsideISR++;
#endif
	xil_printf("%s: Error: aximcdma error interrupt is asserted\r\n",
			__FUNCTION__);
	Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

	setup_rx_bds(Rx_Chan, Rx_Chan->BdCnt);

	XMcDma_Reset(McDmaInstPtr);
	timeOut = RESET_TIMEOUT_COUNT;
	while (timeOut) {
		if (XMcdma_ResetIsDone(McDmaInstPtr))
			break;
		timeOut -= 1;
	}

	if (!timeOut) {
		xil_printf("%s: Error: aximcdma reset timed out\r\n", __func__);
	}

	XMcDma_ChanToHw(Rx_Chan);

#if !NO_SYS
	xInsideISR--;
#endif
	return;
}

static void axi_mcdma_recv_handler(void *CallBackRef, u32 ChanId)
{
	struct pbuf *p;
	u32 i, rx_bytes, ProcessedBdCnt;
	XMcdma_Bd *rxbd, *rxbdset;
	struct xemac_s *xemac = (struct xemac_s *)(CallBackRef);
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	XMcdma *McDmaInstPtr = &xaxiemacif->aximcdma;
	XMcdma_ChanCtrl *Rx_Chan;

#if !NO_SYS
	xInsideISR++;
#endif

	Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

	ProcessedBdCnt = XMcdma_BdChainFromHW(Rx_Chan, XMCDMA_ALL_BDS, &rxbdset);

	for (i = 0, rxbd = rxbdset; i < ProcessedBdCnt; i++) {

		p = (struct pbuf *)(UINTPTR)XMcdma_BdGetSwId(rxbd);

		/* Adjust the buffer size to actual number of bytes received.*/
		rx_bytes = extract_packet_len(rxbd);
#ifndef __aarch64__
		Xil_DCacheInvalidateRange((UINTPTR)p->payload,
							(UINTPTR)rx_bytes);
#endif
		pbuf_realloc(p, rx_bytes);

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
		rxbd = (XMcdma_Bd *)XMcdma_BdChainNextBd(Rx_Chan, rxbd);
	}

	/* free up the BD's */
	XMcdma_BdChainFree(Rx_Chan, ProcessedBdCnt, rxbdset);

	/* return all the processed bd's back to the stack */
	setup_rx_bds(Rx_Chan, Rx_Chan->BdCnt);
#if !NO_SYS
	sys_sem_signal(&xemac->sem_rx_data_available);
	xInsideISR--;
#endif
}

s32_t is_tx_space_available(xaxiemacif_s *xaxiemacif)
{
	XMcdma_ChanCtrl *Tx_Chan;
	u8_t ChanId;
	for (ChanId = 1;
		ChanId <= xaxiemacif->axi_ethernet.Config.AxiMcDmaChan_Cnt;
								ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(&xaxiemacif->aximcdma, ChanId);

		if (Tx_Chan->BdCnt) {
			return Tx_Chan->BdCnt;
		}
	}
	return 0;
}

s32_t process_sent_bds(XMcdma_ChanCtrl *Tx_Chan)
{
	int ProcessedBdCnt, i;
	XStatus status;
	XMcdma_Bd *txbdset, *txbd;

	ProcessedBdCnt = XMcdma_BdChainFromHW(Tx_Chan, XMCDMA_ALL_BDS,
								&txbdset);
	if (ProcessedBdCnt == 0) {
		return XST_FAILURE;
	}

	/* free the pbuf associated with each BD */
	for (i = 0, txbd = txbdset; i < ProcessedBdCnt; i++) {
		struct pbuf *p = (struct pbuf *)(UINTPTR)XMcdma_BdGetSwId(txbd);
		pbuf_free(p);
		txbd = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, txbd);
	}

	/* free the processed BD's */
	status =  XMcdma_BdChainFree(Tx_Chan, ProcessedBdCnt, txbdset);
	if (status != XST_SUCCESS) {
		xil_printf("Error freeing up TxBDs");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

#if LWIP_PARTIAL_CSUM_OFFLOAD_TX==1
static void update_partial_cksum_offload(XMcdma_Bd *txbdset, struct pbuf *p)
{
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
			csum_init = ip_chksum_pseudo(NULL, proto, tcp_len,
					(ip_addr_t *)&ehdr->ip.src,
					(ip_addr_t *)&ehdr->ip.dest);
			/* init buffer descriptor */
			bd_csum_set(txbdset, tcp_payload_offset,
					csum_insert_offset, htons(~csum_init));
		}
	}
}
#endif

XStatus axi_mcdma_sgsend(xaxiemacif_s *xaxiemacif, struct pbuf *p)
{
	struct pbuf *q;
	u32_t n_pbufs = 0;
	XMcdma_Bd *txbdset, *txbd, *last_txbd = NULL;
	XMcdma_ChanCtrl *Tx_Chan;
	XStatus status;
	static u8_t ChanId = 1;
	u8_t next_ChanId = ChanId;

	/* first count the number of pbufs */
	for (q = p; q != NULL; q = q->next)
		n_pbufs++;

	/* Transfer packets to TX DMA Channels in round-robin manner */
	do {
		Tx_Chan = XMcdma_GetMcdmaTxChan(&xaxiemacif->aximcdma, ChanId);

		if (++ChanId > xaxiemacif->axi_ethernet.Config.AxiMcDmaChan_Cnt)
			ChanId = 1;

		if ((next_ChanId == ChanId) && (n_pbufs > Tx_Chan->BdCnt)) {
			LWIP_DEBUGF(NETIF_DEBUG, ("sgsend: Error, not enough BD space in All Chans\r\n"));
			return ERR_IF;
		}

	} while (n_pbufs > Tx_Chan->BdCnt);

	txbdset = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);

	for (q = p, txbd = txbdset; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		 * time. The size of the data in each pbuf is kept in the ->len
		 * variable.
		 */
		XMcDma_BdSetCtrl(txbd, 0);
		XMcdma_BdSetSwId(txbd, (void *)q);

		Xil_DCacheFlushRange((UINTPTR)q->payload, q->len);

		status = XMcDma_ChanSubmit(Tx_Chan, (UINTPTR)q->payload,
				q->len);
		if (status != XST_SUCCESS) {
			xil_printf("ChanSubmit failed\n\r");
			return XST_FAILURE;
		}

		pbuf_ref(q);

		last_txbd = txbd;
		txbd = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, txbd);
	}

	if (n_pbufs == 1) {
		XMcDma_BdSetCtrl(txbdset, XMCDMA_BD_CTRL_SOF_MASK
				| XMCDMA_BD_CTRL_EOF_MASK);
	} else {
		/* in the first packet, set the SOP */
		XMcDma_BdSetCtrl(txbdset, XMCDMA_BD_CTRL_SOF_MASK);
		/* in the last packet, set the EOP */
		XMcDma_BdSetCtrl(last_txbd, XMCDMA_BD_CTRL_EOF_MASK);
	}

#if LWIP_FULL_CSUM_OFFLOAD_TX==1
	bd_fullcsum_disable(txbdset);
	if (p->len > sizeof(struct ethip_hdr)) {
		bd_fullcsum_enable(txbdset);
	}
#endif

#if LWIP_PARTIAL_CSUM_OFFLOAD_TX==1
	bd_csum_disable(txbdset);
	update_partial_cksum_offload(txbdset, p);
#endif
	DATA_SYNC;
	/* enq to h/w */
	return XMcDma_ChanToHw(Tx_Chan);
}

void axi_mcdma_register_handlers(struct xemac_s *xemac, u8 ChanId)
{
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	XMcdma *McDmaInstPtr = &xaxiemacif->aximcdma;

	struct xtopology_t *xtopologyp = &xtopology[xemac->topology_index];

#if XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ == 1

	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			(Xil_InterruptHandler)xaxiemac_error_handler,
			&xaxiemacif->axi_ethernet);

	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaRxIntr[ChanId - 1],
			(Xil_InterruptHandler)XMcdma_IntrHandler,
			McDmaInstPtr);

	XScuGic_RegisterHandler(xtopologyp->scugic_baseaddr,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaTxIntr[ChanId - 1],
			(Xil_InterruptHandler)XMcdma_TxIntrHandler,
			McDmaInstPtr);

	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.TemacIntr,
			AXIETH_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);

	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaTxIntr[ChanId - 1],
			AXIDMA_TX_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);

	XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaRxIntr[ChanId - 1],
			AXIDMA_RX_INTR_PRIORITY_SET_IN_GIC,
			TRIG_TYPE_RISING_EDGE_SENSITIVE);

	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.TemacIntr);

	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaTxIntr[ChanId - 1]);

	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR,
			xaxiemacif->axi_ethernet.Config.AxiMcDmaRxIntr[ChanId - 1]);

#endif /* XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ */
}

XStatus axi_mcdma_setup_rx_chan(struct xemac_s *xemac, u32_t ChanId)
{
	XMcdma_ChanCtrl *Rx_Chan;
	XStatus status;

	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);

	/* RX chan configurations */
	Rx_Chan = XMcdma_GetMcdmaRxChan(&xaxiemacif->aximcdma, ChanId);

	/* Disable all interrupts */
	XMcdma_IntrDisable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	status = XMcDma_ChanBdCreate(Rx_Chan, (UINTPTR) xaxiemacif->rx_bdspace,
			XLWIP_CONFIG_N_RX_DESC);
	if (status != XST_SUCCESS) {
		xil_printf("Rx bd create failed with %d\r\n", status);
		return XST_FAILURE;
	}

	xaxiemacif->rx_bdspace += (XLWIP_CONFIG_N_RX_DESC * sizeof(XMcdma_Bd));

	/* Setup Interrupt System and register callbacks */
	XMcdma_SetCallBack(&xaxiemacif->aximcdma, XMCDMA_HANDLER_DONE,
			(void *)axi_mcdma_recv_handler, xemac);
	XMcdma_SetCallBack(&xaxiemacif->aximcdma, XMCDMA_HANDLER_ERROR,
			(void *)axi_mcdma_recv_error_handler, xemac);

	status = XMcdma_SetChanCoalesceDelay(Rx_Chan,
					     XLWIP_CONFIG_N_RX_COALESCE,
					     XMCDMA_COALESCEDELAY);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return ERR_IF;
	}

	setup_rx_bds(Rx_Chan, XLWIP_CONFIG_N_RX_DESC);

	/* enable DMA interrupts */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	return XST_SUCCESS;
}

XStatus axi_mcdma_setup_tx_chan(struct xemac_s *xemac, u8 ChanId)
{
	XStatus status;
	XMcdma_ChanCtrl *Tx_Chan;
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);

	/* TX chan configurations */
	Tx_Chan = XMcdma_GetMcdmaTxChan(&xaxiemacif->aximcdma, ChanId);

	XMcdma_IntrDisable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	status = XMcDma_ChanBdCreate(Tx_Chan, (UINTPTR) xaxiemacif->tx_bdspace,
			XLWIP_CONFIG_N_TX_DESC);
	if (status != XST_SUCCESS) {
		xil_printf("TX bd create failed with %d\r\n", status);
		return XST_FAILURE;
	}

	xaxiemacif->tx_bdspace += (XLWIP_CONFIG_N_TX_DESC * sizeof(XMcdma_Bd));

	/* Setup Interrupt System and register callbacks */
	XMcdma_SetCallBack(&xaxiemacif->aximcdma, XMCDMA_TX_HANDLER_DONE,
			(void *)axi_mcdma_send_handler, &xaxiemacif->aximcdma);
	XMcdma_SetCallBack(&xaxiemacif->aximcdma, XMCDMA_TX_HANDLER_ERROR,
			(void *)axi_mcdma_send_error_handler,
			&xaxiemacif->aximcdma);

	status = XMcdma_SetChanCoalesceDelay(Tx_Chan,
					     XLWIP_CONFIG_N_TX_COALESCE,
					     XMCDMA_COALESCEDELAY);
	if (status != XST_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting coalescing settings\r\n"));
		return ERR_IF;
	}

	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	return XST_SUCCESS;
}

XStatus init_axi_mcdma(struct xemac_s *xemac)
{
	XMcdma_Config *dmaconfig;
	XStatus status;
	u32_t ChanId;
	xaxiemacif_s *xaxiemacif = (xaxiemacif_s *)(xemac->state);
	UINTPTR baseaddr;

	/*
	 * Disable L1 prefetch if the processor type is Cortex A53. It is
	 * observed that the L1 prefetching for ARMv8 can cause issues while
	 * dealing with cache memory on Rx path. On Rx path, the lwIP adapter
	 * does a clean and invalidation of buffers (pbuf payload) before
	 * allocating them to Rx BDs. However, there are chances that the
	 * the same cache line may get prefetched by the time Rx data is
	 * DMAed to the same buffer. In such cases, CPU fetches stale data from
	 * cache memory instead of getting them from memory. To avoid such
	 * scenarios L1 prefetch is being disabled for ARMv8. That can cause
	 * a performance degradation in the range of 3-5%. In tests, it is
	 * generally observed that this performance degradation is quite
	 * insignificant to be really visible.
	 */
#if defined __aarch64__
	Xil_ConfigureL1Prefetch(0);
#endif

	xaxiemacif->rx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_RX_DESC *
					       (XMCDMA_MAX_CHAN_PER_DEVICE / 2),
					       XMCDMA_BD_MINIMUM_ALIGNMENT);
	if (!xaxiemacif->rx_bdspace) {
		xil_printf("%s@%d: Error: Unable to allocate memory for "
				"RX buffer descriptors", __FILE__, __LINE__);
		return ERR_IF;
	}

	xaxiemacif->tx_bdspace = alloc_bdspace(XLWIP_CONFIG_N_TX_DESC *
					       (XMCDMA_MAX_CHAN_PER_DEVICE / 2),
					       XMCDMA_BD_MINIMUM_ALIGNMENT);
	if (!xaxiemacif->tx_bdspace) {
		xil_printf("%s@%d: Error: Unable to allocate memory for "
				"TX buffer descriptors", __FILE__, __LINE__);
		return ERR_IF;
	}

	/* Mark the BD Region as uncacheable */
#if defined(__aarch64__)
	Xil_SetTlbAttributes((UINTPTR)bd_space,
					NORM_NONCACHE | INNER_SHAREABLE);
#elif defined (ARMR5)
	Xil_SetTlbAttributes((INTPTR)bd_space,
					DEVICE_SHARED | PRIV_RW_USER_RW);
#else
	Xil_SetTlbAttributes((INTPTR)bd_space, DEVICE_MEMORY);
#endif

	LWIP_DEBUGF(NETIF_DEBUG, ("rx_bdspace: 0x%08x\r\n",
				xaxiemacif->rx_bdspace));
	LWIP_DEBUGF(NETIF_DEBUG, ("tx_bdspace: 0x%08x\r\n",
				xaxiemacif->tx_bdspace));

	/* Initialize MCDMA */
	baseaddr = xaxiemacif->axi_ethernet.Config.AxiDevBaseAddress;
	dmaconfig = XMcdma_LookupConfigBaseAddr(baseaddr);
	if (!baseaddr) {
		xil_printf("%s@%d: Error: Lookup Config failed\r\n", __FILE__,
				__LINE__);
	}
	status = XMcDma_CfgInitialize(&xaxiemacif->aximcdma, dmaconfig);
	if (status != XST_SUCCESS) {
		xil_printf("%s@%d: Error: MCDMA config initialization failed\r\n",
				__FILE__, __LINE__);
		return XST_FAILURE;
	}

	/* Setup Rx/Tx chan and Interrupts */
	for (ChanId = 1;
		ChanId <= xaxiemacif->axi_ethernet.Config.AxiMcDmaChan_Cnt;
								ChanId++) {

		status = axi_mcdma_setup_rx_chan(xemac, ChanId);
		if (status != XST_SUCCESS) {
			xil_printf("%s@%d: Error: MCDMA Rx chan setup failed\r\n",
					__FILE__, __LINE__);
			return XST_FAILURE;
		}

		status = axi_mcdma_setup_tx_chan(xemac, ChanId);
		if (status != XST_SUCCESS) {
			xil_printf("%s@%d: Error: MCDMA Tx chan setup failed\r\n",
					__FILE__, __LINE__);
			return XST_FAILURE;
		}

		axi_mcdma_register_handlers(xemac, ChanId);

	}
	return XST_SUCCESS;
}
