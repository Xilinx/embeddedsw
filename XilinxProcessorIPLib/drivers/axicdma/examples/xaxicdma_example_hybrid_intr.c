/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_hybrid_intr.c
 * @addtogroup axicdma Overview
 * @{
 *
 * This file demonstrates how to use xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in hybrid transfer mode through
 * interrupt.
 *
 * This example does one simple transfer, then a multiple BD transfer, and
 * finally another simple transfer.
 *
 * Modify NUMBER_OF_BDS_TO_TRANSFER for a different number of BDs to be
 * transferred in the SG transfer.
 *
 * This example assumes that the system has an interrupt controller.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system
 * and the transfer length should be cache-line size aligned.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  02/01/11 Changed function prototype of XAxiCdma_HybridIntrExample
 * 		       to a function taking arguments interrupt instance,device
 * 		       instance,device id,device interrupt id
 *		       Added interrupt support for Cortex A9
 * 2.01a srt  03/06/12 Modified interrupt support for Zynq.
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
 * 4.8   sk   06/15/20 Fix the compilation error for xreg_cortexa9.h
 *		       preprocessor.
 * 4.8	 sk   09/30/20 Modify the buffer length and add cache operations for
 *		       receive and destination buffers to fix the data check
 *		       failure and to make the length cache-line aligned.
 * 4.10  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * 4.11	 sa   09/29/22 Fix infinite loops in the example.
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
#include "xpseudo_asm.h"
#include "xil_mmu.h"
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

#ifndef SDT
#ifndef TESTAPP_GEN
/*
 * Device hardware build related constants.
 */
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


#else

#define AXICDMA_BASE_ADDR	XPAR_XAXICDMA_0_BASEADDR
#endif


#define BUFFER_BYTESIZE 128 	/* Length of the buffers for DMA transfer */
#define MAX_PKT_LEN     1024  /* Length of BD for SG transfer */

#define MARK_UNCACHEABLE        0x701

/* Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 */
#define NUMBER_OF_BDS_TO_TRANSFER	30

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1 to 255 for coalescing and 0 to 255 for delay timer
 */
#define COALESCING_COUNT   5
#define DELAY_COUNT	   5
#define POLL_TIMEOUT_COUNTER   1000000U
#define NUMBER_OF_EVENTS	1
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if defined(XPAR_UARTNS550_0_BASEADDR)
/** @cond INTERNAL */
static void Uart550_Setup(void);
/** @endcond */
#endif

/** @cond INTERNAL */
static int SetupSgTransfer(XAxiCdma *InstancePtr) ;
static void Example_SimpleCallBack(void *CallBackRef, u32 IrqMask,
				   int *IgnorePtr);

static void Example_SgCallBack(void *CallBackRef, u32 IrqMask, int *NumBdPtr);
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length);


static int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries);
static int SubmitSgTransfer(XAxiCdma *InstancePtr);
static int DoSgTransfer(XAxiCdma *InstancePtr);
/** @endcond */

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID

/** @cond INTERNAL */
static int SetupIntrSystem(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId);
static void DisableIntrSystem(XIntc *IntcInstancePtr, u32 IntrId);
/** @endcond */

int XAxiCdma_HybridIntrExample(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			       u16 DeviceId, u32 IntrId);

#else

/** @cond INTERNAL */
static int SetupIntrSystem(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			   u32 IntrId);
static void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId);
/** @endcond */

int XAxiCdma_HybridIntrExample(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			       u16 DeviceId, u32 IntrId);
#endif
#else

int XAxiCdma_HybridIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
#endif



/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
static XAxiCdma Engine;       /* Instance of the XAxiCdma */
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
static XIntc IntcController;	/* Instance of the Interrupt Controller */
#else
static XScuGic IntcController;	/* Instance of the Interrupt Controller */
#endif
#endif

#endif

/**
 * @brief Source and Destination buffer for DMA transfer.
 */
/** @cond INTERNAL */
volatile static u8 SrcBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
volatile static u8 DestBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
/** @endcond */

#define BD_SPACE_SIZE	(64 * sizeof(XAxiCdma_Bd)) /**< BD space size */

/** @cond INTERNAL */
#ifdef ARMR5
#define BD_SPACE_ALIGN 0x100000U
#else
#define BD_SPACE_ALIGN sizeof(XAxiCdma_Bd)
#endif
/** @endcond */

/** @cond INTERNAL */
static u8 BdSpace[BD_SPACE_SIZE] __attribute__ ((aligned (BD_SPACE_ALIGN)));
/** @endcond */

/* Transmit and Receive buffers for scatter gather transfer.
 */
#define SG_BUFFER_SIZE	(MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER) /**< SG buffer size */
/** @cond INTERNAL */
static u8 SgTxBuffer[SG_BUFFER_SIZE] __attribute__ ((aligned (64)));
static u8 SgRxBuffer[SG_BUFFER_SIZE] __attribute__ ((aligned (64)));
/** @endcond */

u32 *TransmitBufferPtr; /**< Pointer to transmit buffer */
u32 *ReceiveBufferPtr; /**< Pointer to receive buffer */

/* Shared variables used to test the callbacks.
 */
volatile static u32 Done = 0;    /* Dma transfer is done */
volatile static u32 Error = 0;   /* Dma Bus Error occurs */


/*****************************************************************************/
/**
* @brief Runs the example entry point, sets up UART16550 if available, invokes
* the example function, and reports the execution status.
*
* @return
*		 - XST_SUCCESS if example finishes successfully
*		 - XST_FAILURE if example fails.
*
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{

	int Status;

#ifdef XPAR_UARTNS550_0_BASEADDR
	Uart550_Setup();
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");
	xil_printf("Using global buffer allocation\r\n");

	/* Initialize SG buffer pointers */
	TransmitBufferPtr = (u32 *)SgTxBuffer;
	ReceiveBufferPtr = (u32 *)SgRxBuffer;

	/* Run the interrupt example for simple transfer
	 */
#ifndef SDT
	Status = XAxiCdma_HybridIntrExample(&IntcController, &Engine,
					    DMA_CTRL_DEVICE_ID, DMA_CTRL_IRPT_INTR);
#else
	Status = XAxiCdma_HybridIntrExample(&Engine, AXICDMA_BASE_ADDR);
#endif

	if (Status != (XST_SUCCESS)) {
		xil_printf("Axicdma Hybrid interrupt Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Axicdma Hybrid interrupt Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif


#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/**
* @brief Sets up the baud rate to 9600 and 8 data bits in UART16550.
*
*
*
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
/**
* @brief Sets up the driver for scatter gather transfer.
*
* @param[in] InstancePtr is a pointer to the XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if setup successful
* 		- XST_FAILURE if setup failed with error
*
*
******************************************************************************/
static int SetupSgTransfer(XAxiCdma *InstancePtr)
{
	int Status;
	XAxiCdma_Bd BdTemplate;
	int BdCount;
	u8 *SrcBufferPtr;
	int Index;

	BdCount = BD_SPACE_SIZE / sizeof(XAxiCdma_Bd);

	Status = XAxiCdma_BdRingCreate(InstancePtr, (UINTPTR)BdSpace,
				       (UINTPTR)BdSpace, XAXICDMA_BD_MINIMUM_ALIGNMENT, BdCount);
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

	/* Flush the TransmitBuffer and ReceiveBuffer before the DMA transfer,
	 * in case the Data Cache is enabled.
	 */
	Xil_DCacheFlushRange((UINTPTR)TransmitBufferPtr,
			     MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	Xil_DCacheFlushRange((UINTPTR)ReceiveBufferPtr,
			     MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	/* Setup interrupt coalescing and delay timer
	 */
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
/**
* @brief Handles the simple transfer callback from the driver interrupt handler.
*
* @param[in] CallBackRef is the reference pointer registered through
*		transfer submission. In this case, it is the pointer to the
*		driver instance
* @param[in] IrqMask is the interrupt mask the driver interrupt handler
* 		passes to the callback function.
* @param[out] IgnorePtr Pointer unused by the simple transfer callback.
*
*
*
******************************************************************************/
static void Example_SimpleCallBack(void *CallBackRef, u32 IrqMask,
				   int *IgnorePtr)
{
	if (IrqMask & XAXICDMA_XR_IRQ_ERROR_MASK) {
		Error = TRUE;
	}

	if (IrqMask & XAXICDMA_XR_IRQ_IOC_MASK) {
		Done = TRUE;
	}

	return;
}

/*****************************************************************************/
/**
* Callback function for the scatter gather transfer.
* It is called by the driver's interrupt handler.
*
* @param[in] CallBackRef is the reference pointer registered through
*		transfer submission. In this case, it is the pointer to the
*		driver instance
* @param[in] IrqMask is the interrupt mask the driver interrupt handler
*		passes to the callback function.
* @param[out] NumBdPtr is the pointer to number of BDs this handler handles
*
*
*
******************************************************************************/
static void Example_SgCallBack(void *CallBackRef, u32 IrqMask, int *NumBdPtr)
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

/*****************************************************************************/
/**
* @brief Checks that two buffers have the same data.
*
* @param[in] SrcPtr is the source buffer
* @param[in] DestPtr is the destination buffer
* @param[in] Length is the length of the buffer to check
*
* @return
* 		- XST_SUCCESS if the two buffer matches
* 		- XST_FAILURE otherwise
*
*
******************************************************************************/
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length)
{
	int Index;

	/* Invalidate the DestBuffer before receiving the data,
	 * in case the data cache is enabled
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

#ifndef SDT
/******************************************************************************/
/**
* @brief Sets up the interrupt system, including:
*	- Initialize the interrupt controller,
*	- Register the XAxiCdma interrupt handler to the interrupt controller
*	- Enable interrupt
*
* @param[in] IntcInstancePtr is a pointer to the instance of the INTC
* @param[in] InstancePtr is a pointer to the instance of the XAxiCdma
* @param[in] IntrId is the interrupt Id for XAxiCdma
*
* @return
* 		- XST_SUCCESS if interrupt system setup successfully
* 		- XST_FAILURE if error occurs
*
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
			    "Interrupt controller initialization failed %d\r\n",
			    Status);

		return XST_FAILURE;
	}
#endif

	/*
	 * Connect the driver interrupt handler
	 * It will call the example callback upon transfer completion
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
			       (XInterruptHandler)XAxiCdma_IntrHandler,
			       (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Interrupt handler registration failed %d\r\n",
			    Status);

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
/**
*
* @brief Disables the interrupt for the XAxiCdma device.
*
* @param[in] IntcInstancePtr is the pointer to the instance of the INTC
* @param[in] IntrId is the interrupt Id for the XAxiCdma instance
*
*
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
* @brief Performs one simple transfer.
*
* @param[in] InstancePtr is a pointer to the XAxiCdma instance
* @param[in] Length is the transfer length
* @param[in] Retries is how many times to retry on submission
*
* @return
* 		- XST_SUCCESS if transfer is successful
* 		- XST_FAILURE if either the transfer fails or the data has
*			error.
*
*
******************************************************************************/
static int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries)
{
	u32 Index;
	u8  *SrcPtr;
	u8  *DestPtr;
	int Status;

	Done = 0;
	Error = 0;

	/* Initialize the source buffer bytes with a pattern and the
	 * the destination buffer bytes to zero
	 */
	SrcPtr = (u8 *)SrcBuffer;
	DestPtr = (u8 *)DestBuffer;
	for (Index = 0; Index < Length; Index++) {
		SrcPtr[Index] = Index & 0xFF;
		DestPtr[Index] = 0;
	}

	/* Flush the SrcBuffer and DestBuffer before the DMA transfer,
	 * in case the Data Cache is enabled.
	 */
	Xil_DCacheFlushRange((UINTPTR)&SrcBuffer, Length);
	Xil_DCacheFlushRange((UINTPTR)&DestBuffer, Length);

	/* Try to start the DMA transfer
	 */
	while (Retries) {
		Retries -= 1;

		Status = XAxiCdma_SimpleTransfer(InstancePtr, (UINTPTR)SrcBuffer,
						 (UINTPTR)DestBuffer, Length, Example_SimpleCallBack,
						 InstancePtr);

		if (Status == XST_SUCCESS) {
			break;
		}
	}

	if (!Retries) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Submit transfer failed\r\n");
		return XST_FAILURE;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Error);
	if (Status == XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Simple transfer has error %x\r\n",
			    (unsigned int)XAxiCdma_GetError(InstancePtr));

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or timeout */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Done);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Simple transfer failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Transfer completes successfully, check data
	 */
	Status = CheckData(SrcPtr, DestPtr, Length);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Check data failed for simple "
			    "transfer\r\n");

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Performs a non-blocking transmission of all packets through the DMA engine.
*
* @param[in] InstancePtr Points to the DMA engine instance.
*
* @return
*		- XST_SUCCESS if the DMA accepts all the packets successfully,
*		- XST_FAILURE if following error occurs
*			BD ring allocation failed
*			One of the buffer transfer length is invalid
*			Submission to hardware failed
*
*
******************************************************************************/
static int SubmitSgTransfer(XAxiCdma *InstancePtr)
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
				     NUMBER_OF_BDS_TO_TRANSFER, BdPtr,
				     (XAxiCdma_CallBackFn)Example_SgCallBack, (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed to hw %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Does a multi-BD scatter gather transfer.
*
* @param[in] InstancePtr Points to the DMA engine instance.
*
* @return
* 		- XST_SUCCESS if transfer finishes successfully
* 		- XST_FAILURE if transfer has errors
*
*
******************************************************************************/
static int DoSgTransfer(XAxiCdma *InstancePtr)
{
	int Status;
	u8 *SrcPtr;
	u8 *DstPtr;

	SrcPtr = (u8 *)TransmitBufferPtr;
	DstPtr = (u8 *)ReceiveBufferPtr;

#ifdef __aarch64__
	Xil_SetTlbAttributes((UINTPTR)BdSpace, MARK_UNCACHEABLE);
#elif defined(ARMR5)
	Xil_SetTlbAttributes((UINTPTR)BdSpace, STRONG_ORDERD_SHARED | PRIV_RW_USER_RW);
#elif !defined(__MICROBLAZE__)
	Xil_SetTlbAttributes((UINTPTR)BdSpace, DEVICE_MEMORY);
#endif
	/* Setup the BD ring
	 */
	Status = SetupSgTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Setup BD ring failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Submit BDs to the hardware
	 */
	Status = SubmitSgTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Submit transfer failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Error);
	if (Status == XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "SG transfer has error %x\r\n",
			    (unsigned int)XAxiCdma_GetError(InstancePtr));

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or timeout */
	Status = Xil_WaitForEvent((UINTPTR)&Done, NUMBER_OF_BDS_TO_TRANSFER, NUMBER_OF_BDS_TO_TRANSFER, POLL_TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "SG transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Transfer completes successfully, check data
	 */
	Status = CheckData(SrcPtr, DstPtr,
			   MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Check data failed for sg "
			    "transfer\r\n");

		return XST_FAILURE;
	}

	/* Transfer finishes successfully
	 */
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Demonstrates hybrid interrupt transfers with the following transfers:
*	a simple transfer
*	a multiple BD scatter gather transfer
*	another simple transfer
*
* @if SDT
* @param[in] InstancePtr is a pointer to the XAxiCdma instance
* @param[in] BaseAddress contains the base address of the device
* @else
* @param[in] IntcInstancePtr is a pointer to the INTC instance
* @param[in] InstancePtr is a pointer to the XAxiCdma instance
* @param[in] DeviceId is the Device Id of the XAxiCdma instance
* @param[in] IntrId is the interrupt Id for the XAxiCdma instance in build
* @endif
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if error occurs
*
* @note		If the hardware build has problems with interrupt, then this
*		function hangs
*
*
******************************************************************************/
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
int XAxiCdma_HybridIntrExample(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
			       u16 DeviceId, u32 IntrId)
#else
int XAxiCdma_HybridIntrExample(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
			       u16 DeviceId, u32 IntrId)
#endif
#else
int XAxiCdma_HybridIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	int SubmitTries = 10;		/* Retry to submit */

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

	/* Enable all (completion/error/delay) interrupts
	 */
	XAxiCdma_IntrEnable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

	/* First simple transfer
	 */
	Done = 0;
	Error = 0;

	Status = DoSimpleTransfer(InstancePtr,
				  BUFFER_BYTESIZE, SubmitTries);

	if (Status != XST_SUCCESS) {
#ifndef SDT
		DisableIntrSystem(IntcInstancePtr, IntrId);
#else
		XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif
		return XST_FAILURE;
	}

	xil_printf("First simple transfer successful\r\n");

	/* The scatter gather transfer
	 */
	Done = 0;
	Error = 0;

	Status = DoSgTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
#ifndef SDT
		DisableIntrSystem(IntcInstancePtr, IntrId);
#else
		XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif

		return XST_FAILURE;
	}

	xil_printf("Scatter gather transfer successful\r\n");

	/* Second simple transfer
	 */
	Done = 0;
	Error = 0;

	Status = DoSimpleTransfer(InstancePtr,
				  BUFFER_BYTESIZE, SubmitTries);

	if (Status != XST_SUCCESS) {
#ifndef SDT
		DisableIntrSystem(IntcInstancePtr, IntrId);
#else
		XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif

		return XST_FAILURE;
	}

	xil_printf("Second simple transfer successful\r\n");

	/* Test finishes successfully, clean up and return
	 */
#ifndef SDT
	DisableIntrSystem(IntcInstancePtr, IntrId);
#else
	XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
#endif

	return XST_SUCCESS;
}

/** @} */