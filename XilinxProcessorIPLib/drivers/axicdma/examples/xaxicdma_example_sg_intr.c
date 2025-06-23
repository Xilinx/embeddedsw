/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_sg_intr.c
 *
 * This file demonstrates how to use the xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in scatter gather transfer mode
 * through interrupt.
 *
 * This example assumes that the system has an interrupt controller.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/27/10 First release
 * 2.01a rkv  01/28/11 Changed function prototype of XAxiCdma_SgIntrExample to
 * 		       a function taking arguments interrupt instance,device
 * 		       instance,device id,device interrupt id.
 *		       Added interrupt support for Cortex A9
 * 2.01a srt  03/05/12 Modified interrupt support for Zynq.
 *		       Added V7 DDR Base Address to fix CR 649405.
 * 		       Modified Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 * 2.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 4.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 4.3   ms   01/22/17 Modified xil_printf statement in main function to
 *            ensure that "Successfully ran" and "Failed" strings are
 *            available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 4.4   rsp  02/22/18 Support data buffers above 4GB.Use UINTPTR for storing
 *                     and typecasting buffer address(CR-995116).
 * 4.5   rsp  03/10/18 Fix compilation error in CheckData function when DEBUG
 *                     mode is enabled.
 *                     Reset error and done states before starting the DMA.
 * 4.6   rsp  09/13/19 Fix cache maintenance ops for source and dest buffer.
 * 4.7   rsp  12/06/19 For aarch64 include xil_mmu.h. Fixes gcc warning.
 * 4.8	 sk   09/28/20 Fix the compilation error for xreg_cortexa9.h
 * 		       preprocessor on R5 processor.
 * 4.10  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * 4.11  sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 ****************************************************************************/
#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xil_util.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif

#ifndef __MICROBLAZE__
#include "xpseudo_asm_gcc.h"
#endif

#ifdef __aarch64__
#include "xreg_cortexa53.h"
#include "xil_mmu.h"
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#ifdef XPAR_INTC_0_DEVICE_ID
#define DMA_CTRL_DEVICE_ID	XPAR_AXICDMA_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define DMA_CTRL_IRPT_INTR	XPAR_INTC_0_AXICDMA_0_VEC_ID
#else
#define DMA_CTRL_DEVICE_ID 	XPAR_AXICDMA_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define DMA_CTRL_IRPT_INTR	XPAR_FABRIC_AXICDMA_0_VEC_ID
#endif
#endif


#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define MEMORY_BASE		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define MEMORY_BASE	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define MEMORY_BASE	XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define MEMORY_BASE	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#else

#ifdef XPAR_MEM0_BASEADDRESS
#define MEMORY_BASE		XPAR_MEM0_BASEADDRESS
#endif
#define AXICDMA_BASE_ADDR       XPAR_XAXICDMA_0_BASEADDR
#endif

#ifndef MEMORY_BASE
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEMORY_BASE		0x01000000
#endif

#define BD_SPACE_BASE		(MEMORY_BASE + 0x03000000)
#define BD_SPACE_HIGH		(MEMORY_BASE + 0x03001FFF)
#define TX_BUFFER_BASE		(MEMORY_BASE + 0x00630000)
#define RX_BUFFER_BASE		(MEMORY_BASE + 0x00660000)
#define RX_BUFFER_HIGH		(MEMORY_BASE + 0x0068FFFF)


#define MAX_PKT_LEN			1024
#define MARK_UNCACHEABLE	0x701

/* Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 * The receive side gets one completion per transfer
 */
#define NUMBER_OF_BDS_TO_TRANSFER	30

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1 to 255 for coalescing and 0 to 255 for delay timer
 */
#define COALESCING_COUNT	5
#define DELAY_COUNT		5
#define POLL_TIMEOUT_COUNTER         1000000U
#define NUMBER_OF_EVENTS	1
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if defined(XPAR_UARTNS550_0_BASEADDR)
static void Uart550_Setup(void);
#endif

static void Example_CallBack(void *CallBackRef, u32 IrqMask, int *NumBdPtr);


static int SetupTransfer(XAxiCdma *InstancePtr);
static int DoTransfer(XAxiCdma *InstancePtr);
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length);

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
static int SetupIntrSystem(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId);
static void DisableIntrSystem(XIntc *IntcInstancePtr, u32 IntrId);

int XAxiCdma_SgIntrExample(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u16 DeviceId, u32 IntrId);

#else
static int SetupIntrSystem(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId);
static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId);

int XAxiCdma_SgIntrExample(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u16 DeviceId, u32 IntrId);

#endif
#else

int XAxiCdma_SgIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
#endif


/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
static XAxiCdma Engine;		/* Instance of the XAxiCdma */
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
static XIntc IntcController;	/* Instance of the Interrupt Controller */
#else
static XScuGic IntcController;	/* Instance of the Interrupt Controller */
#endif
#endif
#endif

/* Transmit buffer for DMA transfer.
 */
static u32 *TransmitBufferPtr = (u32 *) TX_BUFFER_BASE;
static u32 *ReceiveBufferPtr = (u32 *) RX_BUFFER_BASE;

/* Shared variables used to test the callbacks.
 */
 static volatile u32 Done = 0;	/* Dma transfer is done */
 static volatile u32 Error = 0;	/* Dma Bus Error occurs */


/*****************************************************************************/
/**
* The entry point for this example. It sets up uart16550 if one is available,
* invokes the example function, and reports the execution status.
*
* @param	None.
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{

	int Status;

#ifdef XPAR_UARTNS550_0_BASEADDR
	Uart550_Setup();
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the interrupt example for simple transfer
	 */
#ifndef SDT
	Status = XAxiCdma_SgIntrExample(&IntcController, &Engine,
					DMA_CTRL_DEVICE_ID, DMA_CTRL_IRPT_INTR);
#else
	Status = XAxiCdma_SgIntrExample(&Engine, AXICDMA_BASE_ADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiCdma_SgIntr Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiCdma_SgIntr Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return 0;

}
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
* This function setup the baudrate to 9600 and data bits to 8 in Uart16550
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

/*****************************************************************************/
/*
* Callback function for the scatter gather transfer.
* It is called by the driver's interrupt handler.
*
* @param	CallBackRef is the reference pointer registered through
* 		transfer submission. In this case, it is the pointer to the
* 		driver instance
* @param	IrqMask is the interrupt mask the driver interrupt handler
* 		passes to the callback function.
* @param 	NumBdPtr is the pointer to number of BDs this handler handles
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Example_CallBack(void *CallBackRef, u32 IrqMask, int *NumBdPtr)
{

	XAxiCdma *InstancePtr;
	int BdCount;
	XAxiCdma_Bd *BdPtr;
	int Status;
	int Tmp;

	InstancePtr = (XAxiCdma *)CallBackRef;
	Tmp = *NumBdPtr;

	/* If error interrupt happened, the driver interrupt handler
	 * has already reset the hardware
	 */
	if (IrqMask & XAXICDMA_XR_IRQ_ERROR_MASK) {
		Error = 1;
	}

	if (IrqMask & XAXICDMA_XR_IRQ_IOC_MASK) {

		/* Get all processed BDs from hardware
		 */
		BdCount = XAxiCdma_BdRingFromHw(InstancePtr, Tmp, &BdPtr);

		/* Release finished BDs
		 *
		 * It is ok if BdCount is zero as a previous callback may
		 * have ripen all finished BDs
		 */
		if (BdCount > 0) {

			Status = XAxiCdma_BdRingFree(InstancePtr,
						     BdCount, BdPtr);

			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_ERROR,
					    "Error free BD %x\r\n", Status);

				Error = 1;
				return;
			}

			Done += BdCount;
			*NumBdPtr = Tmp - BdCount;
		}
	}

	return;
}
#ifndef SDT
/******************************************************************************/
/*
* Setup the interrupt system, including:
*	- Initialize the interrupt controller,
*	- Register the XAxiCdma interrupt handler to the interrupt controller
*	- Enable interrupt
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC
* @param	InstancePtr is a pointer to the instance of the XAxiCdma
* @param	IntrId is the interrupt Id for XAxiCdma
*
* @return
* 		- XST_SUCCESS if interrupt system setup successfully
* 		- XST_FAILURE if error occurs
*
* @note		None.
*
*******************************************************************************/
#ifdef XPAR_INTC_0_DEVICE_ID
static int SetupIntrSystem(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Interrupt controller initialization failed %d\r\n", Status);

		return XST_FAILURE;
	}
#endif

	/*
	 * Connect the driver interrupt handler to intc.
	 * It will call the example callback upon transfer completion
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
			       (XInterruptHandler)XAxiCdma_IntrHandler, (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Interrupt handler registration failed %d\r\n", Status);

		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts. Specify real mode so that the DMA
	 * engine can generate interrupts through the interrupt controller
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the DMA engine
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

#ifndef TESTAPP_GEN

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIntc_InterruptHandler,
				     (void *)IntcInstancePtr);

	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}
#else

static int SetupIntrSystem(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver
	 */
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
#endif

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_InterruptHandler)XAxiCdma_IntrHandler,
				 InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the DMA device.
	 */
	XScuGic_Enable(IntcInstancePtr, IntrId);



#ifndef TESTAPP_GEN

	Xil_ExceptionInit();

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

#endif

/*****************************************************************************/
/*
*
* This function disables the interrupt for the XAxiCdma device
*
* @param	IntcInstancePtr is the pointer to the instance of the INTC
* @param	IntrId is the interrupt Id for the XAxiCdma instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifdef XPAR_INTC_0_DEVICE_ID
static void DisableIntrSystem(XIntc *IntcInstancePtr, u32 IntrId)
{

	/* Disconnect the interrupt
	 */
	XIntc_Disconnect(IntcInstancePtr, IntrId);

}
#else
static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId)
{

	/* Disconnect the interrupt
	 */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);


}

#endif
#endif
/*****************************************************************************/
/**
*
* This function sets up the DMA engine to be ready for scatter gather transfer
*
* @param    InstancePtr is the pointer to the instance of the DMA engine.
*
* @return
* 		- XST_SUCCESS if the setup is successful
* 		- XST_FAILURE if following error occurs:
* 			BD buffer address is not aligned
* 			BD ring creation failed
* 			BD ring clone failed
* 			Set coalescing failed
*
* @note		None.
*
******************************************************************************/
static int SetupTransfer(XAxiCdma *InstancePtr)
{
	int Status;
	XAxiCdma_Bd BdTemplate;
	int BdCount;
	u8 *SrcBufferPtr;
	int Index;

	/* Disable all interrupts
	 */
	XAxiCdma_IntrDisable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

	/* Setup BD ring */
	BdCount = XAxiCdma_BdRingCntCalc(XAXICDMA_BD_MINIMUM_ALIGNMENT,
					 BD_SPACE_HIGH - BD_SPACE_BASE + 1,
					 (UINTPTR)BD_SPACE_BASE);

	if (BdCount < 1) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid buffer %x\r\n",
			    (unsigned int)BD_SPACE_BASE);
		return XST_FAILURE;
	}

	Status = XAxiCdma_BdRingCreate(InstancePtr, BD_SPACE_BASE,
				       BD_SPACE_BASE, XAXICDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Create BD ring failed %d\r\n",
			    Status);

		return XST_FAILURE;
	}

	/*
	 * Setup a BD template to copy to every BD.
	 */
	XAxiCdma_BdClear(&BdTemplate);
	Status = XAxiCdma_BdRingClone(InstancePtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Clone BD ring failed %d\r\n",
			    Status);

		return XST_FAILURE;
	}

	/* Initialize receive buffer to 0's and transmit buffer with pattern
	 */
	memset((void *)ReceiveBufferPtr, 0,
	       MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	SrcBufferPtr = (u8 *)TransmitBufferPtr;
	for (Index = 0; Index < MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER;
	     Index++) {
		SrcBufferPtr[Index] = Index & 0xFF;
	}

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TransmitBufferPtr,
			     MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	Xil_DCacheFlushRange((UINTPTR)ReceiveBufferPtr,
			     MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	Status = XAxiCdma_SetCoalesce(InstancePtr, COALESCING_COUNT,
				      DELAY_COUNT);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Set coalescing failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function non-blockingly transmits all packets through the DMA engine.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return
* 		- XST_SUCCESS if the DMA accepts all the packets successfully,
* 		- XST_FAILURE if following error occurs
* 			BD ring allocation failed
* 			One of the buffer transfer length is invalid
* 			Submission to hardware failed
*
* @note		None.
*
******************************************************************************/
static int DoTransfer(XAxiCdma *InstancePtr)
{
	XAxiCdma_Bd *BdPtr;
	XAxiCdma_Bd *BdCurPtr;
	int Status;
	int Index;
	UINTPTR SrcBufferAddr;
	UINTPTR DstBufferAddr;
	static int Counter = 0;

	Status = XAxiCdma_BdRingAlloc(InstancePtr,
				      NUMBER_OF_BDS_TO_TRANSFER, &BdPtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed bd alloc\r\n");

		return XST_FAILURE;
	}

	SrcBufferAddr = (UINTPTR)TransmitBufferPtr;
	DstBufferAddr = (UINTPTR)ReceiveBufferPtr;
	BdCurPtr = BdPtr;

	/* Set up the BDs
	 */
	for (Index = 0; Index < NUMBER_OF_BDS_TO_TRANSFER; Index++) {
		Counter += 1;

		Status = XAxiCdma_BdSetSrcBufAddr(BdCurPtr, SrcBufferAddr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Set src addr failed %d, %x/%x\r\n",
				    Status, (unsigned int)BdCurPtr,
				    (unsigned int)SrcBufferAddr);

			return XST_FAILURE;
		}

		Status = XAxiCdma_BdSetDstBufAddr(BdCurPtr, DstBufferAddr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Set dst addr failed %d, %x/%x\r\n",
				    Status, (unsigned int)BdCurPtr,
				    (unsigned int)DstBufferAddr);

			return XST_FAILURE;
		}

		Status = XAxiCdma_BdSetLength(BdCurPtr, MAX_PKT_LEN);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Set BD length failed %d\r\n", Status);

			return XST_FAILURE;
		}

		SrcBufferAddr += MAX_PKT_LEN;
		DstBufferAddr += MAX_PKT_LEN;
		BdCurPtr = XAxiCdma_BdRingNext(InstancePtr, BdCurPtr);
	}

	/* Give the BDs to hardware */
	Status = XAxiCdma_BdRingToHw(InstancePtr,
				     NUMBER_OF_BDS_TO_TRANSFER, BdPtr, Example_CallBack,
				     (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed to hw %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function checks that two buffers have the same data
*
* @param	SrcPtr is the source buffer
* @param 	DestPtr is the destination buffer
* @param	Length is the length of the buffer to check
*
* @return
* 		- XST_SUCCESS if the two buffer matches
* 		- XST_FAILURE otherwise
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length)
{
	int Index;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)DestPtr, Length);

	for (Index = 0; Index < Length; Index++) {
		if ( DestPtr[Index] != SrcPtr[Index]) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				    "Data check failure %d: %x/%x\r\n",
				    Index, (unsigned int)DestPtr[Index],
				    (unsigned int)SrcPtr[Index]);

			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The example to do the scatter gather transfer through interrupt.
*
* @param	IntcInstancePtr is a pointer to the INTC instance
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	DeviceId is the Device Id of the XAxiCdma instance
* @param	IntrId is the interrupt Id for the XAxiCdma instance in build
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if error occurs during transfer setup or transfer
*		 has errors
*
* @note		If the hardware build has problems with interrupt,
*		then this function hangs
*
******************************************************************************/
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
int XAxiCdma_SgIntrExample(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u16 DeviceId, u32 IntrId)
#else
int XAxiCdma_SgIntrExample(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u16 DeviceId, u32 IntrId)
#endif
#else
int XAxiCdma_SgIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	u8 *SrcPtr;
	u8 *DstPtr;

	SrcPtr = (u8 *)TransmitBufferPtr;
	DstPtr = (u8 *)ReceiveBufferPtr;

#ifdef __aarch64__
	Xil_SetTlbAttributes(BD_SPACE_BASE, MARK_UNCACHEABLE);
#endif

	/* Initialize the XAxiCdma device.
	 */
#ifndef SDT
	CfgPtr = XAxiCdma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Cannot find config structure for device %d\r\n",
			    XPAR_AXICDMA_0_DEVICE_ID);

		return XST_FAILURE;
	}
#else
	CfgPtr = XAxiCdma_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Cannot find config structure for device %llx\r\n",
			    BaseAddress);

		return XST_FAILURE;
	}
#endif

	Status = XAxiCdma_CfgInitialize(InstancePtr, CfgPtr,
					CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Initialization failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Setup the BD ring
	 */
	Status = SetupTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Setup BD ring failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Setup the interrupt system
	 */
#ifndef SDT
	Status = SetupIntrSystem(IntcInstancePtr, InstancePtr, IntrId);
#else
	Status = XSetupInterruptSystem(InstancePtr, &XAxiCdma_IntrHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Setup Intr system failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Enable completion and error interrupts
	 */
	XAxiCdma_IntrEnable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

	Done = 0;
	Error = 0;

	/* Start the DMA transfer
	 */
	Status = DoTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Do transfer failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Error);
	if (Status == XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Transfer has error %x\r\n",
			    (unsigned int)XAxiCdma_GetError(InstancePtr));
#ifndef SDT
		DisableIntrSystem(IntcInstancePtr, IntrId);
#else
		XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or timeout */
	Status = Xil_WaitForEvent((UINTPTR)&Done, NUMBER_OF_BDS_TO_TRANSFER, NUMBER_OF_BDS_TO_TRANSFER, POLL_TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Transfer completes successfully, check data
	 */
	Status = CheckData(SrcPtr, DstPtr,
			   MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Check data failed for sg "
			    "transfer\r\n");
#ifndef SDT
		DisableIntrSystem(IntcInstancePtr, IntrId);
#else
		XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif
		return XST_FAILURE;
	}

	/* Test finishes successfully, clean up and return
	 */
#ifndef SDT
	DisableIntrSystem(IntcInstancePtr, IntrId);
#else
	XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif

	return XST_SUCCESS;
}


