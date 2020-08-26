/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmcdma_polled_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in polling mode.
 *
 * This examples shows how to do multiple packets and multiple BD's
 * per packet transfers.
 *
 * H/W Requirements:
 * In order to test this example at the h/w level AXI MCDMA MM2S should
 * be connected to the S2MM channel.
 *
 * System level considerations for Zynq UltraScale+ designs:
 * On ZU+ MPSOC for PL IP's 3 different ports are available HP, HPC and ACP.
 *
 * The explanation below talks about HPC and HP port.
 *
 * HPC design considerations:
 * ZU+ MPSOC has in-built cache coherent interconnect(CCI) to take care of
 * Coherency through HPC port. CCI is only support at EL1 NS level.
 * Following needs to be done by the system components before running the
 * example-
 * 1) Snooping should be enabled in the S3 (0xFD6E4000)
 * 2) Mark the DDR memory being used for buffers as outer-shareable.
 * translation_table.S.
 * .set Memory,	0x405 | (2 << 8) | (0x0).
 *
 * It is recommended to use HPC to make use of H/W coherency feature.
 *
 * HP design considerations:
 * The example uses un-cached memory for buffer descriptors and uses
 * Normal memory for buffers..
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- --------   -------------------------------------------------------
 * 1.0	 adk  18/07/2017 Initial Version.
 * 1.2	 rsp  07/19/2018 Read channel count from IP config.
 *       rsp  08/17/2018 Fix typos and rephrase comments.
 *	 rsp  08/17/2018 Read Length register value from IP config.
 * 1.3   rsp  02/05/2019 Remove snooping enable from application.
 *       rsp  02/06/2019 Programmatically select cache maintenance ops for HPC
 *                       and non-HPC designs. In Rx remove arch64 specific dsb
 *                       instruction by performing cache invalidate operation
 *                       for all supported architectures.
 * 1.5   vak  02/08/2020 Add libmetal support to mcdma.
 *
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xmcdma.h"
#include "xmcdma_hw.h"

#if defined( __LIBMETAL__)
#include <metal/log.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/scatterlist.h>
#if !defined(__BAREMETAL__)
#include "xmcdma_linux.h"
#else
#include "xparameters.h"
#include "xdebug.h"
#ifdef __aarch64__
#include "xil_mmu.h"
#endif
#endif
#else
#include "xparameters.h"
#include "xdebug.h"
#ifdef __aarch64__
#include "xil_mmu.h"
#endif
#endif /* __LIBMETAL__ */


/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#if defined(__BAREMETAL__)
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
#endif

#define NUMBER_OF_BDS_PER_PKT		10
#define NUMBER_OF_PKTS_TO_TRANSFER 	100
#define NUMBER_OF_BDS_TO_TRANSFER	(NUMBER_OF_PKTS_TO_TRANSFER * \
						NUMBER_OF_BDS_PER_PKT)

#define MAX_PKT_LEN		1024
#define BLOCK_SIZE_2MB 0x200000U

#define NUM_MAX_CHANNELS	16

#define TEST_START_VALUE	0xC

#if defined(__BAREMETAL__)
#define MCDMA_BASE_ADDRESS XPAR_MCDMA_0_BASEADDR
#else
/* FIXME: find a way for finding the base address at runtime */
#define MCDMA_BASE_ADDRESS 0xA0000000
#warning CHECK IF MCDMA_BASE_ADDRESS IS PROPER
#endif

static int TxPattern[NUM_MAX_CHANNELS + 1];
static int RxPattern[NUM_MAX_CHANNELS + 1];
static int TestStartValue[] = {0xC, 0xB, 0x3, 0x55, 0x33, 0x20, 0x80, 0x66, 0x88};

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
#define ALIGNPAGESIZE(Len) ((Len % 4096) ? 4096 - ((Len % 4096)) : 0);
#endif

/************************** Function Prototypes ******************************/
static int RxSetup(XMcdma *McDmaInstPtr);
static int TxSetup(XMcdma *McDmaInstPtr);
static int SendPacket(XMcdma *McDmaInstPtr);
static int CheckData(u8 *RxPacket, int ByteCount, u32 ChanId);
static int CheckDmaResult(XMcdma * McDmaInstPtr, u32 Chan_id);
static void Mcdma_Poll(XMcdma * McDmaInstPtr);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XMcdma AxiMcdma;

volatile int TxDone;
volatile int RxDone;
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

#if defined( __BAREMETAL__) && defined( __LIBMETAL__)
static struct metal_device McdmaDevice = {
	/* MCDMA device */
	.name = "mcdma",
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)MCDMA_BASE_ADDRESS,
			.physmap = (void *)MCDMA_BASE_ADDRESS,
			.size = 0x1000,
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
#endif


#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
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
#if defined(__LIBMETAL__)
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

#if defined( __LIBMETAL__)
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

#if defined (__BAREMETAL__) && defined (__LIBMETAL__)
	DevicePtr = &McdmaDevice;
#endif

#if defined (__LIBMETAL__)
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
	Status = TxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = RxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SendPacket(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	/* Check DMA transfer result */
	while (1) {
		Mcdma_Poll(&AxiMcdma);
		if (RxDone >= NUMBER_OF_BDS_TO_TRANSFER * num_channels)
			break;
	}

	xil_printf("AXI MCDMA SG Polling Test %s\r\n",
		(Status == XST_SUCCESS)? "passed":"failed");

	xil_printf("--- Exiting main() --- \r\n");

#if defined(__LIBMETAL__) && !defined( __BAREMETAL__)
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)TxBufPhysPtr, TxBufLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)TxBdPhysPtr, TxBdLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)RxBufPhysPtr, RxBufLen);
	XMcdma_MetalUnmap(AxiMcdma.device, (void *)RxBdPhysPtr, RxBdLen);
#endif

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
					Xil_DCacheInvalidateRange(BufPhysPtr, MAX_PKT_LEN);

				BufPhysPtr += MAX_PKT_LEN;
				if (!Rx_Chan->Has_Rxdre) {
					buf_align = BufPhysPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
					BufPhysPtr += buf_align;
				}
			}
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
		BdVirtPtr += BdCount * Rx_Chan->Separation;
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
static int CheckData(u8 *RxPacket, int ByteCount, u32 ChanId)
{
	u32 Index;
	u8 Value;


	Value = TestStartValue[ChanId] + RxPattern[ChanId]++;

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

/*****************************************************************************/
/**
*
* This function waits until the DMA transaction is finished, checks data,
* and cleans up.
*
* @param	McDmaInstPtr is the instance pointer to the AXI MCDMA engine.
* @Chan_id	ChanId is the MCDMA channel id to be worked on.
*
* @return	- XST_SUCCESS if DMA transfer is successful and data is correct,
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
static int CheckDmaResult(XMcdma *McDmaInstPtr, u32 Chan_id)
{
        XMcdma_ChanCtrl *Rx_Chan = 0, *Tx_Chan = 0;
        XMcdma_Bd *BdPtr1;
        u8 *RxPacket;
	u64 TempVal;
        int ProcessedBdCount, i;
        int MaxTransferBytes;
        int RxPacketLength;

        Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Tx_Chan,
                                                0xFFFF,
                                                &BdPtr1);
        TxDone += ProcessedBdCount;

        Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Rx_Chan,
                                                        0xFFFF,
                                                        &BdPtr1);
        RxDone += ProcessedBdCount;
        MaxTransferBytes = MAX_TRANSFER_LEN(McDmaInstPtr->Config.MaxTransferlen - 1);

        /* Check received data */
        for (i = 0; i < ProcessedBdCount; i++) {
		TempVal = (u64)XMcdma_BdRead64(BdPtr1, XMCDMA_BD_BUFA_OFFSET);
		RxPacket = XMcdma_Phys2Virt(McDmaInstPtr, (void *)TempVal);
		RxPacketLength = XMcDma_BdGetActualLength(BdPtr1, MaxTransferBytes);

		/* Invalidate the DestBuffer before receiving the data,
		 * in case the data cache is enabled
		 */
		if (!McDmaInstPtr->Config.IsRxCacheCoherent)
			Xil_DCacheInvalidateRange((UINTPTR)RxPacket, RxPacketLength);

		if (CheckData((u8 *) RxPacket, RxPacketLength, Chan_id) != XST_SUCCESS) {
                        xil_printf("Data check failed for the Chan %x\n\r", Chan_id);
                        return XST_FAILURE;
                }

		TempVal = (u64)XMcdma_BdRead64(BdPtr1, XMCDMA_BD_NDESC_OFFSET);
		BdPtr1 = (XMcdma_Bd *)XMcdma_Phys2Virt(McDmaInstPtr, (void *)TempVal);
        }

        return XST_SUCCESS;
}


static void Mcdma_Poll(XMcdma * McDmaInstPtr) {
        u16 Chan_id = 1;
        u32 i;
        u32 Chan_SerMask;

        Chan_SerMask = XMcdma_InstReadReg(McDmaInstPtr, McDmaInstPtr->Config.BaseAddress,
                                      XMCDMA_RX_OFFSET + XMCDMA_RXCH_SER_OFFSET);

        for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask; i <<= 1, Chan_id++)
             if (Chan_SerMask & i)
                 CheckDmaResult(&AxiMcdma, Chan_id);
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

				Value = TestStartValue[ChanId] + TxPattern[ChanId]++;
				TempVal = (u64)XMcdma_BdRead64(BdCurPtr, XMCDMA_BD_BUFA_OFFSET);
				TxPacket = (u8 *)XMcdma_Phys2Virt(McDmaInstPtr, (void *)TempVal);
				for(Index1 = 0; Index1 < MAX_PKT_LEN; Index1++) {
					TxPacket[Index1] = Value;

					Value = (Value + 1) & 0xFF;
				}
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
