/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_sg_poll.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer packets in polling mode when the AXIDMA
 * core is configured in Scatter Gather Mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system. The
 * h/w system built in Area mode has a maximum DDR memory limit of 64MB. In
 * throughput mode, it is 512MB.  These limits are need to ensured for
 * proper operation of this code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   05/17/10 First release
 * 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
 *                     updated tcl file, added xaxidma_porting_guide.h, removed
 *                     workaround for endianness
 * 4.00a rkv  02/22/11 Name of the file has been changed for naming consistency
 *       	       	   Added interrupt support for ARM.
 * 5.00a srt  03/05/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       		   648103, 648701.
 *		       		   Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.2   vak  15/04/16 Fixed compilation warnings in th example
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * 9.10  rsp  09/17/19 Fix cache maintenance ops for source and dest buffer.
 * 9.12  vak  08/21/20 Add support for LIBMETAL APIs.
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xaxidma.h"

#if defined( __LIBMETAL__)
#include <metal/log.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/scatterlist.h>

#if !defined(__BAREMETAL__)
#include "xaxidma_linux.h"
#else
#include "xparameters.h"
#include "xdebug.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

#endif /* __BAREMETAL__ */
#else /* __LIBMETAL__ */
#include "xparameters.h"
#include "xdebug.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

#endif /*__BAREMETAL__*/

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
#if defined(__BAREMETAL__)
#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define TX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00000FFF)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00001000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)
#endif

#define BD_SIZE			0x40
#define MAX_PKT_LEN		0x20
#define MARK_UNCACHEABLE        0x701

#define TEST_START_VALUE	0xC

#define TX_BD_SPACE_SIZE	0x00001000
#define RX_BD_SPACE_SIZE	0x00001000

#if defined(__BAREMETAL__)
#define AXIDMA_BASE_ADDRESS XPAR_AXIDMA_0_BASEADDR
#else
/* FIXME: Get the AXI DMA base address runtime */
#warning CHECK IF THE AXI BASE ADDRESS IS PROPER
#define AXIDMA_BASE_ADDRESS 0xA0000000
#endif

/* The address map size for AxidmaDevice */
#define ADDR_MAP_SIZE			0x1000

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
#define ALIGNPAGESIZE(Len) ((Len % 4096) ? 4096 - ((Len % 4096)) : 0);
#endif

/************************** Function Prototypes ******************************/
#if defined(__BAREMETAL__)
#if defined(XPAR_UARTNS550_0_BASEADDR)
static void Uart550_Setup(void);
#endif
#endif

static int RxSetup(XAxiDma * AxiDmaInstPtr);
static int TxSetup(XAxiDma * AxiDmaInstPtr);
static int SendPacket(XAxiDma * AxiDmaInstPtr);
static int CheckData(void);
static int CheckDmaResult(XAxiDma * AxiDmaInstPtr);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;

static UINTPTR TxBufVirtPtr;
static UINTPTR TxBdVirtPtr;
static UINTPTR TxBufPhysPtr;
static UINTPTR TxBdPhysPtr;
static int TxBufLen;
static int TxBdLen;

static UINTPTR RxBufVirtPtr;
static UINTPTR RxBdVirtPtr;
static UINTPTR RxBufPhysPtr;
static UINTPTR RxBdPhysPtr;
static int RxBufLen;
static int RxBdLen;

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
static struct metal_device AxidmaDevice = {
	/* AXIDMA device */
	.name = "axidma",
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)AXIDMA_BASE_ADDRESS,
			.physmap = (void *)AXIDMA_BASE_ADDRESS,
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
u32 *Packet = (u32 *) TX_BUFFER_BASE;
#endif

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
unsigned int mapped = 0;

struct mem_container {
	int size;
	void *virt;
	unsigned long phys;
	struct metal_generic_shmem *shm;
} mem_cnt[4];

static unsigned long __XAxidma_MetalMap(struct metal_device *DevicePtr, void *vaddr, int size) {
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
		metal_log(METAL_LOG_ERROR,
			  "Failed to dma map addr: 0x%lx size: %d ret: %d\n",
			  vaddr, size, ret);
		return 0;
	}

	metal_device_add_dmamem(DevicePtr, sg_in.io);

	return (unsigned long)*sg_in.io->physmap;
}

static void *XAxidma_MetalMap(struct metal_device *device, int size, unsigned long *phys)
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

		*phys = (unsigned long)__XAxidma_MetalMap(device, va, size);
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

void XAxidma_MetalUnmap(struct metal_device *DevicePtr, void *virt, int size) {
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

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA loopback
* widget.
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
	int Status;
	XAxiDma_Config *Config;

#if defined(__LIBMETAL__)
	struct metal_device *DevicePtr = NULL;

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	init_param.log_level = METAL_LOG_CRITICAL;

	if (metal_init(&init_param)) {
		xil_printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}
#endif

#if defined(__BAREMETAL__)
#if defined(XPAR_UARTNS550_0_BASEADDR)

	Uart550_Setup();

#endif
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

#if defined(__BAREMETAL__)
#ifdef __aarch64__
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, MARK_UNCACHEABLE);
	Xil_SetTlbAttributes(RX_BD_SPACE_BASE, MARK_UNCACHEABLE);
#endif
#endif

	Config = XAxiDma_LookupConfigBaseAddr(AXIDMA_BASE_ADDRESS);
	if (!Config) {
		xil_printf("No config found for 0x%lx\r\n", AXIDMA_BASE_ADDRESS);

		return XST_FAILURE;
	}

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
	DevicePtr = &AxidmaDevice;
#endif

#if defined (__LIBMETAL__)
	Status = XAxiDma_RegisterMetal(&AxiDma, AXIDMA_BASE_ADDRESS, &DevicePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}
#endif

#if !defined(__BAREMETAL__) && defined(__LIBMETAL__)
	/* Set DMA as 32 bit addressing capable */
	metal_device_set_dmacap(DevicePtr, 32);
#endif

	/* Initialize DMA engine */
	Status = XAxiDma_CfgInitialize(&AxiDma, Config);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(!XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as Simple mode \r\n");

		return XST_FAILURE;
	}

	Status = TxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = RxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Send a packet */
	Status = SendPacket(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check DMA transfer result */
	Status = CheckDmaResult(&AxiDma);

	if (Status != XST_SUCCESS) {
		xil_printf("AXI DMA SG Polling Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran AXI DMA SG Polling Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

#if defined (__LIBMETAL__) && !defined (__BAREMETAL__)
	XAxidma_MetalUnmap(DevicePtr, (void *)TxBufVirtPtr, TxBufLen);
	XAxidma_MetalUnmap(DevicePtr, (void *)TxBdVirtPtr, TxBdLen);
	XAxidma_MetalUnmap(DevicePtr, (void *)RxBufVirtPtr, RxBufLen);
	XAxidma_MetalUnmap(DevicePtr, (void *)RxBdVirtPtr, RxBdLen);
#endif

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#if defined(__BAREMETAL__) && defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600, and data bits to 8
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Uart550_Setup(void)
{

	/* Set the baudrate to be predictable
	 */
	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);

}
#endif

/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	UINTPTR BufVirtPtr;
	UINTPTR BdVirtPtr;
	UINTPTR BufPhysPtr;
	UINTPTR BdPhysPtr;
	u32 BdCount;
	u32 FreeBdCount;
	int Index;

	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	/* Disable all RX interrupts before RxBD space setup */

	XAxiDma_InstBdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set delay and coalescing */
	XAxiDma_BdRingSetCoalesce(RxRingPtr, Coalesce, Delay);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
									RX_BD_SPACE_SIZE);

#if defined(__BAREMETAL__)
	BufPhysPtr = RX_BUFFER_BASE;
	BdPhysPtr = RX_BD_SPACE_BASE;
	BufVirtPtr = BufPhysPtr;
	BdVirtPtr = BdPhysPtr;

	RxBufVirtPtr = BufVirtPtr;
	RxBdVirtPtr = BdVirtPtr;
	RxBufPhysPtr = BufPhysPtr;
	RxBdPhysPtr = BdPhysPtr;

	(void)RxBufLen;
	(void)RxBdLen;
#else
	u32 MaxLen;

	MaxLen = BdCount * MAX_PKT_LEN;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BufVirtPtr = (UINTPTR)XAxidma_MetalMap(AxiDmaInstPtr->device, MaxLen, &BufPhysPtr);
	if (!BufVirtPtr || !BufPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	RxBufLen = MaxLen;

	MaxLen = BdCount * BD_SIZE;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BdVirtPtr = (UINTPTR)XAxidma_MetalMap(AxiDmaInstPtr->device, MaxLen, &BdPhysPtr);
	if (!BdVirtPtr || !BdPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	RxBdLen = MaxLen;

	RxBufVirtPtr = BufVirtPtr;
	RxBdVirtPtr = BdVirtPtr;
	RxBufPhysPtr = BufPhysPtr;
	RxBdPhysPtr = BdPhysPtr;
#endif

	Status = XAxiDma_BdRingCreate(RxRingPtr, BdPhysPtr,
								  BdVirtPtr,
								  XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);

	if (Status != XST_SUCCESS) {
		xil_printf("RX create BD ring failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/*
	 * Setup an all-zero BD as the template for the Rx channel.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("RX clone BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Attach buffers to RxBD ring so we are ready to receive packets */

	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);

	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX alloc BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	for (Index = 0; Index < FreeBdCount; Index++) {
		Status = XAxiDma_BdSetBufAddr(BdCurPtr, BufPhysPtr);

		if (Status != XST_SUCCESS) {
			xil_printf("Set buffer addr %x on BD %x failed %d\r\n",
			    (unsigned int)BufPhysPtr,
			    (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
				RxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		/* Receive BDs do not need to set anything for the control
		 * The hardware will set the SOF/EOF bits per stream status
		 */
		XAxiDma_BdSetCtrl(BdCurPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, BufVirtPtr);

		BufPhysPtr += MAX_PKT_LEN;
		BufVirtPtr += MAX_PKT_LEN;
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	/* Clear the receive buffer, so we can verify data
	 */
	memset((void *)BufVirtPtr, 0, MAX_PKT_LEN);

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount,
						BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX submit hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX start hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the TX channel of a DMA engine to be ready for packet
* transmission
*
* @param	AxiDmaInstPtr is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int TxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	XAxiDma_Bd BdTemplate;
	UINTPTR BufVirtPtr;
	UINTPTR BdVirtPtr;
	UINTPTR BufPhysPtr;
	UINTPTR BdPhysPtr;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	u32 BdCount;

	TxRingPtr = XAxiDma_GetTxRing(&AxiDma);

	/* Disable all TX interrupts before TxBD space setup */

	XAxiDma_InstBdRingIntDisable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set TX delay and coalesce */
	XAxiDma_BdRingSetCoalesce(TxRingPtr, Coalesce, Delay);

	/* Setup TxBD space  */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
									TX_BD_SPACE_SIZE);

#if defined(__BAREMETAL__)
	BufPhysPtr = TX_BUFFER_BASE;
	BdPhysPtr = TX_BD_SPACE_BASE;
	BufVirtPtr = BufPhysPtr;
	BdVirtPtr = BdPhysPtr;

	TxBufVirtPtr = BufVirtPtr;
	TxBdVirtPtr = BdVirtPtr;
	TxBufPhysPtr = BufPhysPtr;
	TxBdPhysPtr = BdPhysPtr;

	(void)TxBufLen;
	(void)TxBdLen;
#else
	u32 MaxLen;

	MaxLen = BdCount * MAX_PKT_LEN;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BufVirtPtr = (UINTPTR)XAxidma_MetalMap(AxiDmaInstPtr->device, MaxLen, &BufPhysPtr);
	if (!BufVirtPtr || !BufPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	TxBufLen = MaxLen;

	MaxLen = BdCount * BD_SIZE;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	BdVirtPtr = (UINTPTR)XAxidma_MetalMap(AxiDmaInstPtr->device, MaxLen, &BdPhysPtr);
	if (!BdVirtPtr || !BdPhysPtr) {
		xil_printf("%s: %d: Failed to XMcdma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	TxBdLen = MaxLen;

	TxBufVirtPtr = BufVirtPtr;
	TxBdVirtPtr = BdVirtPtr;
	TxBufPhysPtr = BufPhysPtr;
	TxBdPhysPtr = BdPhysPtr;
#endif

	Status = XAxiDma_BdRingCreate(TxRingPtr, BdPhysPtr,
								  BdVirtPtr,
								  XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("failed create BD ring in txsetup\r\n");

		return XST_FAILURE;
	}

	/*
	 * We create an all-zero BD as the template.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("failed bdring clone in txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start the TX channel */
	Status = XAxiDma_BdRingStart(TxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("failed start bdring txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function transmits one packet non-blockingly through the DMA engine.
*
* @param	AxiDmaInstPtr points to the DMA engine instance
*
* @return	- XST_SUCCESS if the DMA accepts the packet successfully,
*		- XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
static int SendPacket(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	u8 *TxPacket;
	u8 Value;
	XAxiDma_Bd *BdPtr;
	int Status;
	int Index;

	TxRingPtr = XAxiDma_GetTxRing(AxiDmaInstPtr);

	/* Create pattern in the packet to transmit */
	TxPacket = (u8 *) TxBufVirtPtr;

	Value = TEST_START_VALUE;

	for(Index = 0; Index < MAX_PKT_LEN; Index ++) {
		TxPacket[Index] = Value;

		Value = (Value + 1) & 0xFF;
	}

	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN);
	Xil_DCacheFlushRange((UINTPTR)RxBufVirtPtr, MAX_PKT_LEN);

	/* Allocate a BD */
	Status = XAxiDma_BdRingAlloc(TxRingPtr, 1, &BdPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the BD using the information of the packet to transmit */
	Status = XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR) TxBufPhysPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Tx set buffer addr %x on BD %x failed %d\r\n",
		    (UINTPTR)TxBufPhysPtr, (UINTPTR)BdPtr, Status);

		return XST_FAILURE;
	}

	Status = XAxiDma_BdSetLength(BdPtr, MAX_PKT_LEN,
				TxRingPtr->MaxTransferLen);
	if (Status != XST_SUCCESS) {
		xil_printf("Tx set length %d on BD %x failed %d\r\n",
		    MAX_PKT_LEN, (UINTPTR)BdPtr, Status);

		return XST_FAILURE;
	}

#if (XPAR_AXIDMA_0_SG_INCLUDE_STSCNTRL_STRM == 1)
	Status = XAxiDma_BdSetAppWord(BdPtr,
	    XAXIDMA_LAST_APPWORD, MAX_PKT_LEN);

	/* If Set app length failed, it is not fatal
	 */
	if (Status != XST_SUCCESS) {
		xil_printf("Set app word failed with %d\r\n", Status);
	}
#endif

	/* For single packet, both SOF and EOF are to be set
	 */
	XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_TXEOF_MASK |
						XAXIDMA_BD_CTRL_TXSOF_MASK);

	XAxiDma_BdSetId(BdPtr, (UINTPTR)TxBufVirtPtr);

	/* Give the BD to DMA to kick off the transmission. */
	Status = XAxiDma_BdRingToHw(TxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("to hw failed %d\r\n", Status);
		return XST_FAILURE;
	}



	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return	- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(void)
{
	u8 *RxPacket;
	int Index = 0;
	u8 Value;


	RxPacket = (u8 *) RxBufVirtPtr;
	Value = TEST_START_VALUE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, MAX_PKT_LEN);

	for(Index = 0; Index < MAX_PKT_LEN; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error %d: %x/%x\r\n",
			    Index, (unsigned int)RxPacket[Index],
			    (unsigned int)Value);

			return XST_FAILURE;
		}
		Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits until the DMA transaction is finished, checks data,
* and cleans up.
*
* @param	None
*
* @return	- XST_SUCCESS if DMA transfer is successful and data is correct,
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
static int CheckDmaResult(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	XAxiDma_BdRing *RxRingPtr;
	XAxiDma_Bd *BdPtr;
	int ProcessedBdCount;
	int FreeBdCount;
	int Status;

	TxRingPtr = XAxiDma_GetTxRing(AxiDmaInstPtr);
	RxRingPtr = XAxiDma_GetRxRing(AxiDmaInstPtr);

	/* Wait until the one BD TX transaction is done */
	while ((ProcessedBdCount = XAxiDma_BdRingFromHw(TxRingPtr,
						       XAXIDMA_ALL_BDS,
						       &BdPtr)) == 0) {
	}

	/* Free all processed TX BDs for future transmission */
	Status = XAxiDma_BdRingFree(TxRingPtr, ProcessedBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to free %d tx BDs %d\r\n",
		    ProcessedBdCount, Status);
		return XST_FAILURE;
	}

	/* Wait until the data has been received by the Rx channel */
	while ((ProcessedBdCount = XAxiDma_BdRingFromHw(RxRingPtr,
						       XAXIDMA_ALL_BDS,
						       &BdPtr)) == 0) {
	}

	/* Check received data */
	if (CheckData() != XST_SUCCESS) {

		return XST_FAILURE;
	}

	/* Free all processed RX BDs for future transmission */
	Status = XAxiDma_BdRingFree(RxRingPtr, ProcessedBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to free %d rx BDs %d\r\n",
		    ProcessedBdCount, Status);
		return XST_FAILURE;
	}

	/* Return processed BDs to RX channel so we are ready to receive new
	 * packets:
	 *    - Allocate all free RX BDs
	 *    - Pass the BDs to RX channel
	 */
	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("bd alloc failed\r\n");
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Submit %d rx BDs failed %d\r\n", FreeBdCount, Status);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
