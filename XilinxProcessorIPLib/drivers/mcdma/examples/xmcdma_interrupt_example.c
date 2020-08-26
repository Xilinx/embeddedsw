/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmcdma_interrupt_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in interrupt mode.
 *
 * This examples shows how to do multiple packets and multiple BD's
 * Per packet transfers.
 *
 * H/W Requirements:
 * In order to test this example at the design level AXI MCDMA MM2S should
 * be connected with the S2MM channel.
 *
 * System level Considerations for Zynq UltraScale+ designs:
 * Please refer xmcdma_polled_example.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date      Changes
 * ----- ---- --------  -------------------------------------------------------
 * 1.0	 adk  18/07/17	Initial Version.
 * 1.2	 rsp  07/19/18  Read channel count from IP config.
 *			Fix gcc 'pointer from integer without a cast' warning.
 *	 rsp  08/17/18	Fix typos and rephrase comments.
 *	 rsp  08/17/18  Read Length register value from IP config.
 * 1.3   rsp  02/05/19  Remove snooping enable from application.
 *       rsp  02/06/19  Programmatically select cache maintenance ops for HPC
 *                      and non-HPC designs. In Rx remove arch64 specific dsb
 *                      instruction by performing cache invalidate operation
 *                      for all supported architectures.
 * 1.5   vak  08/02/20  Add libmetal support to mcdma.
 *
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xmcdma.h"
#include "xmcdma_hw.h"

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
#include "xmcdma_linux.h"
#else
#ifdef __aarch64__
#include "xil_mmu.h"
#endif
#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif
#include "xparameters.h"
#include "xdebug.h"
#endif /* __LIBMETAL__ */

#if defined(__LIBMETAL__)
#include <metal/log.h>
#include <metal/irq.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/scatterlist.h>
#endif
/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#if defined(__BAREMETAL__)
#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_MCDMA_0_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_MCDMA_0_VEC_ID
#else
#define TX_INTR_ID(ChanId) XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH##ChanId##_INTROUT_INTR
#define RX_INTR_ID(ChanId) XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH##ChanId##_INTROUT_INTR
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x10000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x10000000)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x20000000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x50000000)

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif

#endif /* __LIBMETAL__ */

#define NUMBER_OF_BDS_PER_PKT		10
#define NUMBER_OF_PKTS_TO_TRANSFER 	100
#define NUMBER_OF_BDS_TO_TRANSFER	(NUMBER_OF_PKTS_TO_TRANSFER * \
						NUMBER_OF_BDS_PER_PKT)

#define PACKETS_PER_IRQ 50
#define MAX_PKT_LEN		1024
#define BLOCK_SIZE_2MB 0x200000U

#define NUM_MAX_CHANNELS	16

#define TEST_START_VALUE	0xC

#define ADDR_MAP_SIZE           0x00001000

#if defined(__BAREMETAL__)
#define MCDMA_BASE_ADDRESS XPAR_MCDMA_0_BASEADDR
#else
/* FIXME: find a way for finding the base address at runtime */
#define MCDMA_BASE_ADDRESS 0xA0000000
#warning CHECK IF MCDMA_BASE_ADDRESS IS PROPER
#endif

#if defined(__LIBMETAL__)
static int TestStartValue = 0xC;
static int XMcdmaTxIrq[NUM_MAX_CHANNELS];
static int XMcdmaRxIrq[NUM_MAX_CHANNELS];

#if defined( __BAREMETAL__)
static int XMcdmaIrq_Info[32];
#endif

#endif /* __LIBMETAL__ */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#if defined(__BAREMETAL__)
static INTC Intc;
#endif

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
#define ALIGNPAGESIZE(Len) ((Len % 4096) ? 4096 - ((Len % 4096)) : 0);
#endif

/************************** Function Prototypes ******************************/
static int RxSetup(XMcdma *McDmaInstPtr);
static int TxSetup(XMcdma *McDmaInstPtr);
static int SendPacket(XMcdma *McDmaInstPtr);
static int CheckData(u8 *RxPacket, int ByteCount);
static void TxDoneHandler(void *CallBackRef, u32 Chan_id);
static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static void DoneHandler(void *CallBackRef, u32 Chan_id);
static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);

#if defined(__BAREMETAL__)
static int SetupIntrSystem(INTC * IntcInstancePtr, XMcdma *McDmaInstPtr, u16 IntrId, u8 Direction);
static int ChanIntr_Id(int IsRxChan, int ChanId);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XMcdma AxiMcdma;

volatile int RxChanDone;
volatile int TxChanDone;
volatile int RxDone;
volatile int TxDone;
volatile int Error;
int num_channels;
UINTPTR TxBufVirtPtr;
UINTPTR TxBdVirtPtr;
UINTPTR TxBufPhysPtr;
UINTPTR TxBdPhysPtr;
int TxBufLen;
int TxBdLen;

UINTPTR RxBufVirtPtr;
UINTPTR RxBdVirtPtr;
UINTPTR RxBufPhysPtr;
UINTPTR RxBdPhysPtr;
int RxBufLen;
int RxBdLen;

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
static struct metal_device McdmaDevice = {
	/* MCDMA device */
	.name = "mcdma",
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)MCDMA_BASE_ADDRESS,
			.physmap = (void *)MCDMA_BASE_ADDRESS,
			.size = ADDR_MAP_SIZE,
			.page_shift = (unsigned)(-1),
			.page_mask = (unsigned)(-1),
			.mem_flags = 0x0,
			.ops = {NULL},
		}
	},
	.node = {NULL},
	.irq_num = 0,
	.irq_info = NULL,
};
#endif

#if defined(__BAREMETAL__)
/*
 * Buffer for transmit packet. Must be 32-bit aligned to be used by DMA.
 */
UINTPTR *Packet = (UINTPTR *) TX_BUFFER_BASE;
#endif /* __LIBMETAL__ */

#if defined (__LIBMETAL__) && !defined(__BAREMETAL__)
unsigned int mapped = 0;

struct mem_container {
	int size;
	void *virt;
	unsigned long phys;
	struct metal_generic_shmem *shm;
} mem_cnt[NUMBER_OF_PKTS_TO_TRANSFER * 2];

static unsigned long __XMcdma_MetalMap(struct metal_device *DevicePtr, void *vaddr, int size) {
	struct metal_sg sg_out;
	struct metal_sg sg_in;
	int ret;

	memset(&sg_out, 0, sizeof(struct metal_sg));
	memset(&sg_in, 0, sizeof(struct metal_sg));

	sg_in.virt = vaddr;
	sg_in.len = size;
	sg_in.io = NULL;

	ret = metal_dma_map(DevicePtr, METAL_DMA_DEV_WR, &sg_in, 1, &sg_out);
	if (ret < 0) {
		printf("Failed to dma map addr: 0x%lx size: %d ret: %d\n", vaddr, size, ret);
		return 0;
	}

	metal_device_add_dmamem(DevicePtr, sg_in.io);

	return (unsigned long)*sg_in.io->physmap;
}

static void *XMcdma_MetalMap(struct metal_device *device, int size, unsigned long *phys)
{
	struct metal_generic_shmem *shm;
	struct metal_scatter_list *sg;
	struct metal_io_region *io;
	char shm_name_internal[256];
	int ret;
	void *va;
	unsigned int *a;

	snprintf(shm_name_internal, 256, "ion.reserved/shm%d", mapped);

	ret = metal_shmem_open(shm_name_internal,
			       size, METAL_SHM_NOTCACHED, &shm);
	if (ret) {
		metal_log(METAL_LOG_ERROR,
			  "failed to open shared memory %s.\n",
			  shm_name_internal);
		return NULL;
	}

	sg = metal_shm_attach(shm, device, METAL_SHM_DIR_DEV_RW);
	if (sg == NULL) {
		metal_shmem_close(shm);
		shm = NULL;
		va = aligned_alloc(4096, size);
		if (va == NULL)
			return NULL;

		*phys = (unsigned long)__XMcdma_MetalMap(device, va, size);
		if(*phys == 0)
			return NULL;
	} else {
		io = sg->ios;
		*phys = *(unsigned long *)io->physmap;
		va = metal_io_virt(io, 0);
		metal_device_add_dmamem(device, io);

		if ((va == NULL) || (*phys == 0)) {
			metal_shm_detach(shm, device);
			metal_shmem_close(shm);
			close(shm->id);
			return NULL;
		}
	}

	mem_cnt[mapped].phys = *phys;
	mem_cnt[mapped].virt = va;
	mem_cnt[mapped].size = size;
	mem_cnt[mapped].shm = shm;
	mapped++;

	return va;
}

void XMcdma_MetalUnmap(struct metal_device *DevicePtr, void *virt, int size) {
	struct metal_io_region io;
	struct metal_sg sg;
	struct mem_container *mem_cnt_ptr;

	for (int i = 0; i < 4; i ++) {
		if (mem_cnt[i].virt == virt) {
			mem_cnt_ptr = &mem_cnt[i];
			break;
		}
	}

	if (mem_cnt_ptr->shm != NULL) {
		metal_shm_detach(mem_cnt_ptr->shm, DevicePtr);
		metal_shmem_close(mem_cnt_ptr->shm);
		close(mem_cnt_ptr->shm->id);
		mem_cnt_ptr->shm = NULL;
	} else {
		memset(&io, 0, sizeof(struct metal_io_region));
		memset(&sg, 0, sizeof(struct metal_sg));

		io.physmap = (metal_phys_addr_t)0;
		io.virt = (void *)virt;
		sg.virt = (void *)virt;
		sg.len = size;
		sg.io = &io;

		metal_dma_unmap(DevicePtr, METAL_DMA_DEV_WR, &sg, 1);
		munmap(virt, size);
		free(virt);
	}

	return;
}
#endif

#if defined( __LIBMETAL__)
static int XMcdmaIrq_TxHandler (int vect_id, void *priv)
{
	XMcdma *XMcmdma_ptr = (XMcdma *)priv;

	XMcdma_TxIntrHandler(XMcmdma_ptr);

	return METAL_IRQ_HANDLED;
}

static int XMcdmaIrq_RxHandler (int vect_id, void *priv)
{
	XMcdma *XMcmdma_ptr = (XMcdma *)priv;

	XMcdma_IntrHandler(XMcmdma_ptr);

	return METAL_IRQ_HANDLED;
}
#endif

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA.
*
* @param	None
*
* @return
*		- XST_SUCCESS if test passes
*		- XST_FAILURE if test fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status, i;

	TxDone = 0;
	RxDone = 0;

	XMcdma_Config *Mcdma_Config;

#ifdef __LIBMETAL__
	struct metal_device *DevicePtr = NULL;
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	init_param.log_level = METAL_LOG_ERROR;
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

#if defined(__BAREMETAL__)
#ifdef __aarch64__
#if (TX_BD_SPACE_BASE < 0x100000000UL)
	for (i = 0; i < (RX_BD_SPACE_BASE - TX_BD_SPACE_BASE) / BLOCK_SIZE_2MB; i++) {
		Xil_SetTlbAttributes(TX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
		Xil_SetTlbAttributes(RX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
	}
#else
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, NORM_NONCACHE);
#endif
#endif
#endif

#if defined(__LIBMETAL__)
	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}
#endif

	Mcdma_Config = XMcdma_LookupConfigBaseAddr(MCDMA_BASE_ADDRESS);
	if (!Mcdma_Config) {
		xil_printf("No config found for %d\r\n", MCDMA_BASE_ADDRESS);

		return XST_FAILURE;
	}

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
	/* Add total number of IRQs */
	McdmaDevice.irq_num =
		(Mcdma_Config->TxNumChannels + Mcdma_Config->RxNumChannels);

	/* Populate interrupts */
	for (int i = 0, Chanid = 1; i < Mcdma_Config->TxNumChannels; i++ ) {
		XMcdmaIrq_Info[i * 2] = ChanIntr_Id(0, Chanid);
	}

	for (int i = 0, Chanid = 1; i < Mcdma_Config->RxNumChannels; i++ ) {
		XMcdmaIrq_Info[(i * 2) + 1] = ChanIntr_Id(1, Chanid);
	}

	McdmaDevice.irq_info = XMcdmaIrq_Info;

	DevicePtr = &McdmaDevice;
#endif

#if defined(__LIBMETAL__)
	Status = XMcdma_RegisterMetal(&AxiMcdma, MCDMA_BASE_ADDRESS, &DevicePtr);
	if (Status != XST_SUCCESS) {
		printf("Libmetal register failed %d\r\n", Status);
		return XST_FAILURE;
	}
#endif

	Status = XMcDma_CfgInitialize(&AxiMcdma, Mcdma_Config);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Read numbers of channels from IP config */
	num_channels = Mcdma_Config->RxNumChannels;

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
	Status = metal_xlnx_irq_init();
	if (Status != XST_SUCCESS) {
		xil_printf("\n Failed to initialise interrupt handler \r\n");
		return XST_FAILURE;
	}
#endif

#if defined(__LIBMETAL__)
	/* Populate interrupts */
	if (DevicePtr->irq_num > 1) {
		/* For VFIO the irq_num > 1 */
		for (int i = 0; i < Mcdma_Config->TxNumChannels; i++ ) {
			XMcdmaTxIrq[i] = *(int *)((char *)DevicePtr->irq_info + (sizeof(int) * (i * 2)));
		}

		for (int i = 0; i < Mcdma_Config->RxNumChannels; i++ ) {
			XMcdmaRxIrq[i] = *(int *)((char *)DevicePtr->irq_info + (sizeof(int) * ((i * 2) + 1)));
		}
	} else if (DevicePtr->irq_num == 1) {
		/* For UIO the irq_num == 1 */
		for (int i = 0; i < Mcdma_Config->TxNumChannels; i++ ) {
			XMcdmaTxIrq[i] = (UINTPTR)DevicePtr->irq_info;
		}

		for (int i = 0; i < Mcdma_Config->RxNumChannels; i++ ) {
			XMcdmaRxIrq[i] = (UINTPTR)DevicePtr->irq_info;
		}
	} else {
		/* No irq support, return error */
		return XST_FAILURE;
	}
#endif

	/* Read numbers of channels from IP config */
	num_channels = Mcdma_Config->RxNumChannels;

	Status = TxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = RxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize flags */
	RxChanDone = 0;
	TxChanDone  = 0;
	RxDone = 0;
	TxDone = 0;
	Error = 0;

	Status = SendPacket(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	 while (1) {
	        if ((RxDone >= NUMBER_OF_BDS_TO_TRANSFER * num_channels) && !Error)
	              break;
	 }

	xil_printf("AXI MCDMA SG Interrupt Test %s\r\n",
		(Status == XST_SUCCESS)? "passed":"failed");

#if defined (__LIBMETAL__) && !defined (__BAREMETAL__)
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)TxBufVirtPtr, TxBufLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)TxBdVirtPtr, TxBdLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)RxBufVirtPtr, RxBufLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)RxBdVirtPtr, RxBdLen);
#endif

	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	McDmaInstPtr is the pointer to the instance of the AXI MCDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Rx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR BdVirtPtr;
	UINTPTR BufPhysPtr;
	UINTPTR BdPhysPtr;
	int Status;
	u32 i, j;
	u32 buf_align;
	u64 TempVal;

#if defined(__BAREMETAL__)
	BufPhysPtr = RX_BUFFER_BASE;
	BdPhysPtr = RX_BD_SPACE_BASE;
	BdVirtPtr = BdPhysPtr;
#else
	u32 MaxLen;
	UINTPTR BufVirtPtr;

	MaxLen = NUMBER_OF_BDS_TO_TRANSFER * MAX_PKT_LEN;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BufVirtPtr = (UINTPTR)XMcdma_MetalMap(McDmaInstPtr->device, MaxLen, &BufPhysPtr);
	if (!BufVirtPtr || !BufPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	BdVirtPtr = (UINTPTR)XMcdma_MetalMap(McDmaInstPtr->device, MaxLen, &BdPhysPtr);
	if (!BdVirtPtr || !BdPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__,  __LINE__);
		return XST_FAILURE;
	}

	RxBufVirtPtr = BufVirtPtr;
	RxBdVirtPtr = BdVirtPtr;
	RxBufPhysPtr = BufPhysPtr;
	RxBdPhysPtr = BdPhysPtr;
	RxBufLen = MaxLen;
	RxBdLen = MaxLen;
#endif

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_InstIntrDisable(McDmaInstPtr, Rx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Rx_Chan, BdVirtPtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				TempVal = (u64)XMcdma_Phys2Virt(McDmaInstPtr, (void *)BufPhysPtr);
				/* Clear the receive buffer, so we can verify data */
				memset((void *)TempVal, 0, MAX_PKT_LEN);

				Status = XMcDma_ChanSubmit(Rx_Chan,
							   (UINTPTR)TempVal,
							   MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				if(!McDmaInstPtr->Config.IsRxCacheCoherent)
					Xil_DCacheInvalidateRange((UINTPTR)BufPhysPtr, MAX_PKT_LEN);

				BufPhysPtr += MAX_PKT_LEN;
				if (!Rx_Chan->Has_Rxdre) {
					buf_align = BufPhysPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
					BufPhysPtr += buf_align;
				}
			}

			XMcdma_SetChanCoalesceDelay(Rx_Chan, PACKETS_PER_IRQ, 255);
		}

		Status = XMcDma_ChanToHw(Rx_Chan);
		if (Status != XST_SUCCESS) {
				xil_printf("XMcDma_ChanToHw failed\n\r");
				return XST_FAILURE;
		}

		BufPhysPtr += MAX_PKT_LEN;
		if (!Rx_Chan->Has_Rxdre) {
			buf_align = BufPhysPtr % 64;
			if (buf_align > 0)
				buf_align = 64 - buf_align;
			BufPhysPtr += buf_align;
		}

		BdPhysPtr += BdCount * Rx_Chan->Separation;

		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_DONE,
		                          (void *)DoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_ERROR,
		                          (void *)ErrorHandler, McDmaInstPtr);

#if defined(__BAREMETAL__)
		Status = SetupIntrSystem(&Intc, McDmaInstPtr, ChanIntr_Id(Rx_Chan->IsRxChan, ChanId),
					 XMCDMA_MEM_TO_DEV);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed RX interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
#endif

#if defined(__LIBMETAL__)
		metal_irq_register(XMcdmaRxIrq[ChanId - 1], XMcdmaIrq_RxHandler, (void *)&AxiMcdma);
		metal_irq_enable(XMcdmaRxIrq[ChanId - 1]);
#endif

		XMcdma_InstIntrEnable(McDmaInstPtr, Rx_Chan, XMCDMA_IRQ_ALL_MASK);
	 }

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the TX channel of a DMA engine to be ready for packet
* transmission
*
* @param	McDmaInstPtr is the instance pointer to the AXI MCDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int TxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR BdVirtPtr;
	UINTPTR BufPhysPtr;
	UINTPTR BdPhysPtr;
	int Status;
	u32 i, j;
	u32 buf_align;
	u64 TempVal;

#if defined(__BAREMETAL__)
	BufPhysPtr = TX_BUFFER_BASE;
	BdPhysPtr = TX_BD_SPACE_BASE;
	BdVirtPtr = BdPhysPtr;
#else
	u32 MaxLen;
	UINTPTR BufVirtPtr;

	MaxLen = NUMBER_OF_BDS_TO_TRANSFER * MAX_PKT_LEN;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BufVirtPtr = (UINTPTR)XMcdma_MetalMap(McDmaInstPtr->device, MaxLen, &BufPhysPtr);
	if (!BufVirtPtr || !BufPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	BdVirtPtr = (UINTPTR)XMcdma_MetalMap(McDmaInstPtr->device, MaxLen, &BdPhysPtr);
	if (!BdVirtPtr || !BdPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	TxBufVirtPtr = BufVirtPtr;
	TxBdVirtPtr = BdVirtPtr;
	TxBufPhysPtr = BufPhysPtr;
	TxBdPhysPtr = BdPhysPtr;

	TxBufLen = MaxLen;
	TxBdLen = MaxLen;
#endif
	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_InstIntrDisable(McDmaInstPtr, Tx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Tx_Chan, BdVirtPtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				TempVal = (u64)XMcdma_Phys2Virt(McDmaInstPtr,
							   (void *)BufPhysPtr);

				/* Clear the Transmit buffer */
				memset((void *)TempVal, 0, MAX_PKT_LEN);

				Status = XMcDma_ChanSubmit(Tx_Chan,
							   (UINTPTR)TempVal,
							   MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				BufPhysPtr += MAX_PKT_LEN;
				if (!Tx_Chan->Has_Txdre) {
					buf_align = BufPhysPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
				    BufPhysPtr += buf_align;
				}

			}
		}

		BufPhysPtr += MAX_PKT_LEN;
		if (!Tx_Chan->Has_Txdre) {
			buf_align = BufPhysPtr % 64;
			if (buf_align > 0)
				buf_align = 64 - buf_align;
		    BufPhysPtr += buf_align;
		}

		BdVirtPtr += BdCount * Tx_Chan->Separation;
		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_DONE,
                            (void *)TxDoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_ERROR,
                             (void *)TxErrorHandler, McDmaInstPtr);

#if defined(__BAREMETAL__)
		Status = SetupIntrSystem(&Intc, McDmaInstPtr, ChanIntr_Id(Tx_Chan->IsRxChan, ChanId),
					XMCDMA_DEV_TO_MEM);

		if (Status != XST_SUCCESS) {
		      xil_printf("Failed Tx interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
#endif

#if defined(__LIBMETAL__)
		metal_irq_register(XMcdmaTxIrq[ChanId - 1], XMcdmaIrq_TxHandler, (void *)&AxiMcdma);
		metal_irq_enable(XMcdmaTxIrq[ChanId - 1]);
#endif

		XMcdma_InstIntrEnable(McDmaInstPtr, Tx_Chan, XMCDMA_IRQ_ALL_MASK);
	 }

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	RxPacket is the pointer to Rx packet.
* @param	ByteCount is the length of Rx packet.
* @param	ChanId is the MCDMA channel id to be worked on.
*
* @return	- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *RxPacket, int ByteCount)
{
	u32 Index;
	u8 Value;

	Value = TEST_START_VALUE;

	for(Index = 0; Index < ByteCount; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error : %x/%x\r\n",
						(unsigned int)RxPacket[Index],
						(unsigned int)Value);
			return XST_FAILURE;
			break;
		}
		Value = (Value + 1) & 0xFF;
	}


	return XST_SUCCESS;
}

static int SendPacket(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan = NULL;
	u32 Index, Pkts, Index1;
	XMcdma_Bd *BdCurPtr;
	u32 Status;
	u8 *TxPacket;
	u8 Value;
	u32 ChanId;
	u64 TempVal;

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		BdCurPtr = XMcdma_GetChanCurBd(Tx_Chan);
		for(Index = 0; Index < NUMBER_OF_PKTS_TO_TRANSFER; Index++) {
			for(Pkts = 0; Pkts < NUMBER_OF_BDS_PER_PKT; Pkts++) {
				u32 CrBits = 0;

				Value = TEST_START_VALUE;
				TempVal = (u64)XMcdma_BdRead64(BdCurPtr, XMCDMA_BD_BUFA_OFFSET);
				TxPacket = (u8 *)XMcdma_Phys2Virt(McDmaInstPtr, (void *)TempVal);
				for(Index1 = 0; Index1 < MAX_PKT_LEN; Index1++) {
					TxPacket[Index1] = Value;

					Value = (Value + 1) & 0xFF;
				}

				if (!McDmaInstPtr->Config.IsTxCacheCoherent)
					Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN);

				if (Pkts == 0) {
					CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
				}

				if (Pkts == (NUMBER_OF_BDS_PER_PKT - 1)) {
					CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
				}
				XMcDma_BdSetCtrl(BdCurPtr, CrBits);
				XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
				BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
			}
		}

		Status = XMcDma_ChanToHw(Tx_Chan);
		if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed for Channel %d\n\r", ChanId);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

static void DoneHandler(void *CallBackRef, u32 Chan_id)
{
        XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
        XMcdma_ChanCtrl *Rx_Chan = 0;
        XMcdma_Bd *BdPtr1, *FreeBdPtr;
        u8 *RxPacket;
        int ProcessedBdCount, i;
        int MaxTransferBytes;
        int RxPacketLength;
        u64 TempVal;

        Rx_Chan = (XMcdma_ChanCtrl *)XMcdma_GetMcdmaRxChan(InstancePtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Rx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);

        FreeBdPtr = BdPtr1;
        MaxTransferBytes = MAX_TRANSFER_LEN(InstancePtr->Config.MaxTransferlen - 1);

        for (i = 0; i < ProcessedBdCount; i++) {
		TempVal = (u64)XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_BUFA_OFFSET);
		RxPacket = (u8 *)XMcdma_Phys2Virt(InstancePtr, (void *)TempVal);
		RxPacketLength = XMcDma_BdGetActualLength(FreeBdPtr, MaxTransferBytes);
		/* Invalidate the DestBuffer before receiving the data, in case
		 * the data cache is enabled
		 */
		if (!InstancePtr->Config.IsRxCacheCoherent)
			Xil_DCacheInvalidateRange((UINTPTR)RxPacket, RxPacketLength);

                if (CheckData((void *)RxPacket, RxPacketLength) != XST_SUCCESS) {
                        xil_printf("Data check failed for the Chan %x\n\r", Chan_id);
                }
		TempVal = (u64)XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_NDESC_OFFSET);
		FreeBdPtr = (XMcdma_Bd *)XMcdma_Phys2Virt(InstancePtr, (void *)TempVal);
        }

        RxDone += ProcessedBdCount;
        RxChanDone += 1;
}

static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside error Handler Chan_id is %d\n\r", Chan_id);
	Error = 1;
}

static void TxDoneHandler(void *CallBackRef, u32 Chan_id)
{
	XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
	XMcdma_ChanCtrl *Tx_Chan = 0;
	XMcdma_Bd *BdPtr1;
	int ProcessedBdCount;

	Tx_Chan = XMcdma_GetMcdmaTxChan(InstancePtr, Chan_id);
	ProcessedBdCount = XMcdma_BdChainFromHW(Tx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);

	TxDone += ProcessedBdCount;
	TxChanDone += 1;
}

static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside Tx error Handler Chan_id is %d and Mask %x\n\r", Chan_id, Mask);
	Error = 1;
}

#if defined(__BAREMETAL__)
/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	McDmaInstPtr is a pointer to the instance of the MCDMA.
* @param	InrId is the MCDMA Channel Interrupt Id.
* @param	Direction is the MCDMA Channel S2MM or MM2S path.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE.if not successful
*
* @note		None.
*
******************************************************************************/

static int SetupIntrSystem(INTC * IntcInstancePtr, XMcdma *McDmaInstPtr, u16 IntrId, u8 Direction)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

	if (Direction == XMCDMA_DEV_TO_MEM)
#if defined(__LIBMETAL__)
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) metal_xlnx_irq_isr, (void *)(unsigned long)IntrId);
#else
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) XMcdma_TxIntrHandler, McDmaInstPtr);
#endif
	else
#if defined(__LIBMETAL__)
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) metal_xlnx_irq_isr, (void *)(unsigned long)IntrId);
#else
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) XMcdma_IntrHandler, McDmaInstPtr);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("Failed tx connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, IntrId);
#else

	XScuGic_Config *IntcConfig;
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	if (Direction == XMCDMA_DEV_TO_MEM)
#if defined(__LIBMETAL__)
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)metal_xlnx_irq_isr, (void *)(unsigned long)IntrId);
#else
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)XMcdma_TxIntrHandler, McDmaInstPtr);
#endif
	else
#if defined(__LIBMETAL__)
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)metal_xlnx_irq_isr, (void *)(unsigned long)IntrId);
#else
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)XMcdma_IntrHandler, McDmaInstPtr);
#endif


	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, IntrId);

#endif
	/* Enable interrupts from the hardware */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER,
				(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * This function maps the mcdma channel with the Corresponding interrupt id
 * Generated in the xparameters.h file.
 *
 * @param	Chan is the MCDMA Channel instance to be worked on.
 * @param:	ChanId is the MCDMA channel id to be worked on
 *
 * @return:
 *		- Corresponding interrupt ID on success.
 *		- XST_FAILURE if unable to find a valid interrupt id
 * 		  For a given MCDMA channel.
 *
 * @note:	Make sure the XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH1_INTROUT_INTR
 *		Checks in the API below are properly mapped for your design.
 */
/*****************************************************************************/
static int ChanIntr_Id(int IsRxChan, int ChanId)
{


	switch(ChanId) {
	case 1:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH1_INTROUT_INTR
			return TX_INTR_ID(1);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH1_INTROUT_INTR
			return RX_INTR_ID(1);
#endif
		}
	case 2:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH2_INTROUT_INTR
			return TX_INTR_ID(2);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH2_INTROUT_INTR
			return RX_INTR_ID(2);
#endif
		}
	case 3:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH3_INTROUT_INTR
			return TX_INTR_ID(3);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH3_INTROUT_INTR
			return RX_INTR_ID(3);
#endif
		}
	case 4:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH4_INTROUT_INTR
			return TX_INTR_ID(4);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH4_INTROUT_INTR
			return RX_INTR_ID(4);
#endif
		}
	case 5:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH5_INTROUT_INTR
			return TX_INTR_ID(5);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH5_INTROUT_INTR
			return RX_INTR_ID(5);
#endif
		}
	case 6:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH6_INTROUT_INTR
			return TX_INTR_ID(6);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH6_INTROUT_INTR
			return RX_INTR_ID(6);
#endif
		}
	case 7:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH7_INTROUT_INTR
			return TX_INTR_ID(7);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH7_INTROUT_INTR
			return RX_INTR_ID(7);
#endif
		}
	case 8:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH8_INTROUT_INTR
			return TX_INTR_ID(8);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH8_INTROUT_INTR
			return RX_INTR_ID(8);
#endif
		}
	case 9:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH9_INTROUT_INTR
			return TX_INTR_ID(9);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH9_INTROUT_INTR
			return RX_INTR_ID(9);
#endif
		}
	case 10:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH10_INTROUT_INTR
			return TX_INTR_ID(10);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH10_INTROUT_INTR
			return RX_INTR_ID(10);
#endif
		}
	case 11:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH11_INTROUT_INTR
			return TX_INTR_ID(11);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH11_INTROUT_INTR
			return RX_INTR_ID(11);
#endif
		}
	case 12:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH12_INTROUT_INTR
			return TX_INTR_ID(12);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH12_INTROUT_INTR
			return RX_INTR_ID(12);
#endif
		}
	case 13:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH13_INTROUT_INTR
			return TX_INTR_ID(13);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH13_INTROUT_INTR
			return RX_INTR_ID(13);
#endif
		}
	case 14:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH14_INTROUT_INTR
			return TX_INTR_ID(14);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH14_INTROUT_INTR
			return RX_INTR_ID(14);
#endif
		}
	case 15:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH15_INTROUT_INTR
			return TX_INTR_ID(15);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH15_INTROUT_INTR
			return RX_INTR_ID(15);
#endif
		}
	case 16:
		if(!(IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH16_INTROUT_INTR
			return TX_INTR_ID(16);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH16_INTROUT_INTR
			return RX_INTR_ID(16);
#endif
		}
	default:
		break;
	}

	return XST_FAILURE;
}
#endif /* __LIBMETAL__ */
