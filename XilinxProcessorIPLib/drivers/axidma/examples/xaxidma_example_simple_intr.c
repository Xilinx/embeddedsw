/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_simple_intr.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer packets.in interrupt mode when the AXIDMA core
 * is configured in simple mode
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
 * 4.00a rkv  02/22/11 New example created for simple DMA, this example is for
 *       	       simple DMA,Added interrupt support for Zynq.
 * 4.00a srt  08/04/11 Changed a typo in the RxIntrHandler, changed
 *		       XAXIDMA_DMA_TO_DEVICE to XAXIDMA_DEVICE_TO_DMA
 * 5.00a srt  03/06/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 *		       Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.2   vak  15/04/16 Fixed compilation warnings in the example
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 * 9.6   rsp  02/14/18 Support data buffers above 4GB.Use UINTPTR for typecasting
 *                     buffer address (CR-992638).
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * 9.10  rsp  09/17/19 Fix cache maintenance ops for source and dest buffer.
 * 9.12  vak  08/21/20 Add support for LIBMETAL APIs.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xaxidma.h"

#if defined(__LIBMETAL__)
#include <metal/log.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/irq.h>
#include <metal/scatterlist.h>

#if !defined(__BAREMETAL__)
#include "xaxidma_linux.h"
#else
#include "xparameters.h"
#include "xil_exception.h"
#include "xdebug.h"

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif
#endif /* __BAREMETAL__ */
#else /* __LIBMETAL__ */
#include "xparameters.h"
#include "xil_exception.h"
#include "xdebug.h"

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

#endif /* __BAREMETAL__ */

/************************** Constant Definitions *****************************/

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

#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_AXIDMA_0_MM2S_INTROUT_VEC_ID
#else
#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID
#endif

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif

#endif

#if defined(__BAREMETAL__)
#define AXIDMA_BASE_ADDRESS XPAR_AXIDMA_0_BASEADDR
#else
/* FIXME: Read the AXI DMA base address runtime */
#warning CHECK IF THE BASE ADDRESS IS PROPER
#define AXIDMA_BASE_ADDRESS 0xA0000000
#endif

#if defined(__LIBMETAL__)
static int TestStartValue = 0xC;
static int XAxidmaTxIrq;
static int XAxidmaRxIrq;

#if defined(__BAREMETAL__)
static int XAxidmaIrq_Info[32];
#endif

#endif /* __LIBMETAL__ */


/* Timeout loop counter for reset
 */
#define RESET_TIMEOUT_COUNTER	10000

#define TEST_START_VALUE	0xC
/*
 * Buffer and Buffer Descriptor related constant definition
 */
#define MAX_PKT_LEN		0x100

#define NUMBER_OF_TRANSFERS	10

/* The address map size for AxidmaDevice */
#define ADDR_MAP_SIZE		0x1000

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1 to 255
 *
 * We set the coalescing threshold to be the total number of packets.
 * The receive side will only get one completion interrupt for this example.
 */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
#define ALIGNPAGESIZE(Len) ((Len % 4096) ? 4096 - ((Len % 4096)) : 0);
#endif

/************************** Function Prototypes ******************************/
#if defined(__BAREMETAL__)
#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif
#endif

static int CheckData(u8 *RxBuffer, int Length, u8 StartValue);
static void TxIntrHandler(void *Callback);
static void RxIntrHandler(void *Callback);

#if defined(__BAREMETAL__)
static int SetupIntrSystem(INTC * IntcInstancePtr,
			   XAxiDma * AxiDmaPtr, u16 TxIntrId, u16 RxIntrId);
static void DisableIntrSystem(INTC * IntcInstancePtr,
					u16 TxIntrId, u16 RxIntrId);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */


static XAxiDma AxiDma;		/* Instance of the XAxiDma */

#if defined(__BAREMETAL__)
static INTC Intc;	/* Instance of the Interrupt Controller */
#endif

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile int TxDone;
volatile int RxDone;
volatile int Error;

#if defined(__BAREMETAL__) && defined( __LIBMETAL__)
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

#if defined(__LIBMETAL__)
static int XAxidmaIrq_TxHandler (int vect_id, void *priv)
{
	XAxiDma *XAxidma_ptr = (XAxiDma *)priv;

	TxIntrHandler(XAxidma_ptr);

	return METAL_IRQ_HANDLED;
}

static int XAxidmaIrq_RxHandler (int vect_id, void *priv)
{
	XAxiDma *XAxidma_ptr = (XAxiDma *)priv;

	RxIntrHandler(XAxidma_ptr);

	return METAL_IRQ_HANDLED;
}
#endif

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the interrupt test. It does the following:
*	Set up the output terminal if UART16550 is in the hardware build
*	Initialize the DMA engine
*	Set up Tx and Rx channels
*	Set up the interrupt system for the Tx and Rx interrupts
*	Submit a transfer
*	Wait for the transfer to finish
*	Check transfer status
*	Disable Tx and Rx interrupts
*	Print test status and exit
*
* @param	None
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	XAxiDma_Config *Config;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	UINTPTR TxBufferPhysPtr;
	UINTPTR TxBufferVirtPtr;
	UINTPTR RxBufferPhysPtr;
	UINTPTR RxBufferVirtPtr;
	u8 *TmpBufferPtr;
	u8 Value;

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
	/* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Initialize the XAxiDma device.
	 */
	Config = XAxiDma_LookupConfigBaseAddr(AXIDMA_BASE_ADDRESS);
	if (!Config) {
		xil_printf("No config found for 0x%lx\r\n", AXIDMA_BASE_ADDRESS);
		return XST_FAILURE;
	}

#if defined(__BAREMETAL__) && defined(__LIBMETAL__)
	/* One Tx IRQ and RX IRQ */
	AxidmaDevice.irq_num = 2;

	/* Populate interrupts */
	XAxidmaIrq_Info[0] = TX_INTR_ID;

	XAxidmaIrq_Info[1] = RX_INTR_ID;

	AxidmaDevice.irq_info = XAxidmaIrq_Info;

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

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

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
		XAxidmaTxIrq = *(int *)((char *)DevicePtr->irq_info);

		XAxidmaRxIrq = *(int *)((char *)DevicePtr->irq_info + sizeof(int));
	} else if (DevicePtr->irq_num == 1) {
		/* For UIO the irq_num == 1 */
		XAxidmaTxIrq = (UINTPTR)DevicePtr->irq_info;

		XAxidmaRxIrq = (UINTPTR)DevicePtr->irq_info;
	} else {
		/* No irq support, return error */
		return XST_FAILURE;
	}
#endif


#if defined(__BAREMETAL__)
	/* Set up Interrupt system  */
	Status = SetupIntrSystem(&Intc, &AxiDma, TX_INTR_ID, RX_INTR_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed intr setup\r\n");
		return XST_FAILURE;
	}
#endif

#if defined(__BAREMETAL__)
	TxBufferPhysPtr = TX_BUFFER_BASE;
	TxBufferVirtPtr = TX_BUFFER_BASE;
	RxBufferPhysPtr = RX_BUFFER_BASE;
	RxBufferVirtPtr = RX_BUFFER_BASE;
#else
	u32 MaxLen;

	MaxLen = MAX_PKT_LEN;
	MaxLen += ALIGNPAGESIZE(MaxLen);

	TxBufferVirtPtr = (UINTPTR)XAxidma_MetalMap(DevicePtr, MaxLen, (unsigned long *)&TxBufferPhysPtr);
	if (!TxBufferVirtPtr || !TxBufferPhysPtr) {
		xil_printf("%s: %d: Failed to XAxidma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}

	RxBufferVirtPtr = (UINTPTR)XAxidma_MetalMap(DevicePtr, MaxLen, (unsigned long *)&RxBufferPhysPtr);
	if (!RxBufferVirtPtr || !RxBufferPhysPtr) {
		xil_printf("%s: %d: Failed to XAxidma_MetalMap\n", __func__, __LINE__);
		return XST_FAILURE;
	}
#endif

	/* Disable all interrupts before setup */

	XAxiDma_InstIntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	XAxiDma_InstIntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
				XAXIDMA_DEVICE_TO_DMA);

#if defined(__LIBMETAL__)
	metal_irq_register(XAxidmaTxIrq, XAxidmaIrq_TxHandler, (void *)&AxiDma);
	metal_irq_enable(XAxidmaTxIrq);

	metal_irq_register(XAxidmaRxIrq, XAxidmaIrq_RxHandler, (void *)&AxiDma);
	metal_irq_enable(XAxidmaRxIrq);
#endif

	/* Enable all interrupts */
	XAxiDma_InstIntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
							XAXIDMA_DMA_TO_DEVICE);


	XAxiDma_InstIntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
							XAXIDMA_DEVICE_TO_DMA);

	/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error = 0;

	Value = TEST_START_VALUE;

	TmpBufferPtr = (u8 *)TxBufferVirtPtr;
	for(Index = 0; Index < MAX_PKT_LEN; Index ++) {
			TmpBufferPtr[Index] = Value;

			Value = (Value + 1) & 0xFF;
	}

	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxBufferPhysPtr, MAX_PKT_LEN);
	Xil_DCacheFlushRange((UINTPTR)RxBufferPhysPtr, MAX_PKT_LEN);

	/* Send a packet */
	for(Index = 0; Index < Tries; Index ++) {

		Status = XAxiDma_SimpleTransfer(&AxiDma,(UINTPTR) RxBufferPhysPtr,
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma,(UINTPTR) TxBufferPhysPtr,
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}


		/*
		 * Wait TX done and RX done
		 */
		while (!TxDone && !RxDone && !Error) {
				/* NOP */
		}

		if (Error) {
			xil_printf("Failed test transmit%s done, "
			"receive%s done\r\n", TxDone? "":" not",
							RxDone? "":" not");

			goto Done;

		}

		/*
		 * Test finished, check data
		 */
		Status = CheckData((u8 *)RxBufferVirtPtr, MAX_PKT_LEN, 0xC);
		if (Status != XST_SUCCESS) {
			xil_printf("Data check failed\r\n");
			goto Done;
		}
	}


	xil_printf("Successfully ran AXI DMA interrupt Example\r\n");


#if defined(__BAREMETAL__)
	/* Disable TX and RX Ring interrupts and return success */

	DisableIntrSystem(&Intc, TX_INTR_ID, RX_INTR_ID);
#endif

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)
	XAxidma_MetalUnmap(DevicePtr, (void *)TxBufferVirtPtr, MaxLen);
	XAxidma_MetalUnmap(DevicePtr, (void *)RxBufferVirtPtr, MaxLen);
#endif

#if defined(__LIBMETAL__)
	metal_device_close(DevicePtr);
#endif

Done:
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}

#if defined(__BAREMETAL__)
#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
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

	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);
}
#endif
#endif

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* We use the static tx/rx buffers.
*
* @param	Length is the length to check
* @param	StartValue is the starting value of the first byte
*
* @return
*		- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *RxBuffer, int Length, u8 StartValue)
{
	u8 *RxPacket;
	int Index = 0;
	u8 Value;

	RxPacket = (u8 *)RxBuffer;
	Value = StartValue;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, Length);

	for(Index = 0; Index < Length; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error %d: %x/%x\r\n",
			    Index, RxPacket[Index], Value);

			return XST_FAILURE;
		}
		Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This is the DMA TX Interrupt handler function.
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then sets the TxDone.flag
*
* @param	Callback is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxIntrHandler(void *Callback)
{

	u32 IrqStatus;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_InstIntrGetIrq(AxiDmaInst, XAXIDMA_DMA_TO_DEVICE);

	/* Acknowledge pending interrupts */


	XAxiDma_InstIntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DMA_TO_DEVICE);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {

		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/*
		 * Reset should never fail for transmit channel
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If Completion interrupt is asserted, then set the TxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

		TxDone = 1;
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then it sets the RxDone flag.
*
* @param	Callback is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(void *Callback)
{
	u32 IrqStatus;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_InstIntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

	/* Acknowledge pending interrupts */
	XAxiDma_InstIntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if(XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If completion interrupt is asserted, then set RxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

		RxDone = 1;
	}
}

#if defined(__BAREMETAL__)
/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	TxIntrId is the TX channel Interrupt ID.
* @param	RxIntrId is the RX channel Interrupt ID.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE.if not successful
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(INTC * IntcInstancePtr,
			   XAxiDma * AxiDmaPtr, u16 TxIntrId, u16 RxIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

#if defined(__LIBMETAL__)
	Status = XIntc_Connect(IntcInstancePtr, TxIntrId,
				(XInterruptHandler) metal_xlnx_irq_isr,
				(void *)(unsigned long)TxIntrId);
#else
	Status = XIntc_Connect(IntcInstancePtr, TxIntrId,
				(XInterruptHandler) TxIntrHandler, AxiDmaPtr);
#endif
	if (Status != XST_SUCCESS) {

		xil_printf("Failed tx connect intc\r\n");
		return XST_FAILURE;
	}

#if defined(__LIBMETAL__)
	Status = XIntc_Connect(IntcInstancePtr, RxIntrId,
				(XInterruptHandler) metal_xlnx_irq_isr,
				(void *)(unsigned long)RxIntrId);
#else
	Status = XIntc_Connect(IntcInstancePtr, RxIntrId,
				(XInterruptHandler) RxIntrHandler, AxiDmaPtr);
#endif
	if (Status != XST_SUCCESS) {

		xil_printf("Failed rx connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, TxIntrId);
	XIntc_Enable(IntcInstancePtr, RxIntrId);

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


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, TxIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, RxIntrId, 0xA0, 0x3);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
#if defined(__LIBMETAL__)
	Status = XScuGic_Connect(IntcInstancePtr, TxIntrId,
				(Xil_InterruptHandler)metal_xlnx_irq_isr,
				(void *)(unsigned long)TxIntrId);
#else
	Status = XScuGic_Connect(IntcInstancePtr, TxIntrId,
				(Xil_InterruptHandler)TxIntrHandler,
				AxiDmaPtr);
#endif
	if (Status != XST_SUCCESS) {
		return Status;
	}

#if defined(__LIBMETAL__)
	Status = XScuGic_Connect(IntcInstancePtr, RxIntrId,
				(Xil_InterruptHandler)metal_xlnx_irq_isr,
				(void *)(unsigned long)RxIntrId);
#else
	Status = XScuGic_Connect(IntcInstancePtr, RxIntrId,
				(Xil_InterruptHandler)RxIntrHandler,
				AxiDmaPtr);
#endif
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, TxIntrId);
	XScuGic_Enable(IntcInstancePtr, RxIntrId);


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
/**
*
* This function disables the interrupts for DMA engine.
*
* @param	IntcInstancePtr is the pointer to the INTC component instance
* @param	TxIntrId is interrupt ID associated w/ DMA TX channel
* @param	RxIntrId is interrupt ID associated w/ DMA RX channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(INTC * IntcInstancePtr,
					u16 TxIntrId, u16 RxIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(IntcInstancePtr, TxIntrId);
	XIntc_Disconnect(IntcInstancePtr, RxIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, TxIntrId);
	XScuGic_Disconnect(IntcInstancePtr, RxIntrId);
#endif
}
#endif
