/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_simple_poll.c
* @addtogroup AXIDMA Overview
* @{
* @brief Demonstrates AXI DMA simple mode transfers in polling mode.
 * DMA core (AXIDMA) to transfer packets in polling mode when the AXI DMA core
 * is configured in simple mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.00a rkv  02/22/11 New example created for simple DMA, this example is for
 *       	       simple DMA
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
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 9.9   rsp  01/21/19 Fix use of else-if preprocessor check in deriving DDR_BASE_ADDR.
 * 9.10  rsp  09/17/19 Fix cache maintenance ops for source and dest buffer.
 * 9.14  sk   03/08/22 Delete DDR memory limits comments as they are not
 * 		       relevant to this driver version.
 * 9.15  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * 9.16  sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 * ***************************************************************************

 */
/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "sleep.h"

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef SDT
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID	/**< Device ID of the AXI DMA under test. */
#endif

#define MAX_PKT_LEN		0x20	/**< Maximum packet length in bytes for each transfer. */

#define TEST_START_VALUE	0xC	/**< Starting value used to fill and verify test packet data. */

#define NUMBER_OF_TRANSFERS	10	/**< Number of simple transfers to run in this test. */
#define POLL_TIMEOUT_COUNTER    1000000U	/**< Maximum poll iterations before a transfer times out. */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#if (!defined(DEBUG))
/**
 * @brief Prints a formatted message when DEBUG is not defined.
 *
 * @param[in] format is the printf-style format string.
 */
extern void xil_printf(const char *format, ...);
#endif

#ifndef SDT
int XAxiDma_SimplePollExample(u16 DeviceId);
#else
int XAxiDma_SimplePollExample(UINTPTR BaseAddress);
#endif
/** @cond INTERNAL */
static int CheckData(void);
/** @endcond */


/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;	/**< Driver instance for the AXI DMA under test. */

/** @cond INTERNAL */
static u8 TxBuffer[MAX_PKT_LEN] __attribute__ ((aligned(64)));
static u8 RxBuffer[MAX_PKT_LEN] __attribute__ ((aligned(64)));
/** @endcond */


/*****************************************************************************/
/**
* @brief Invokes the example function and reports the execution status.
*
* @return XST_SUCCESS if Example finished successfully, or XST_FAILURE if Example failed.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
#ifndef SDT
	Status = XAxiDma_SimplePollExample(DMA_DEV_ID);
#else
	Status = XAxiDma_SimplePollExample(XPAR_XAXIDMA_0_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiDma_SimplePoll Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiDma_SimplePoll Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}

#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* @brief Sets up UART16550 with a baud rate of 9600 and 8 data bits.
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

#ifndef SDT
/*****************************************************************************/
/**
* @brief Runs a simple polled transfer example.
*
* NUMBER_OF_TRANSFERS defines how many times the transfer repeats.
*
* @param[in] DeviceId is the device ID of the XAxiDma instance.
*
* @return XST_SUCCESS if the example finishes successfully, XST_FAILURE otherwise.
*
******************************************************************************/
int XAxiDma_SimplePollExample(u16 DeviceId)
#else
/*****************************************************************************/
/**
* @brief Runs a simple polled transfer example.
*
* NUMBER_OF_TRANSFERS defines how many times the transfer repeats.
*
* @param[in] BaseAddress contains the base address of the device.
*
* @return XST_SUCCESS if the example finishes successfully, XST_FAILURE otherwise.
*
******************************************************************************/
int XAxiDma_SimplePollExample(UINTPTR BaseAddress)
#endif
{
	XAxiDma_Config *CfgPtr;
	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	u8 *TxBufferPtr;
	u8 *RxBufferPtr;
	u8 Value;
	int TimeOut = POLL_TIMEOUT_COUNTER;

	TxBufferPtr = (u8 *)TxBuffer;
	RxBufferPtr = (u8 *)RxBuffer;

	/* Initialize the XAxiDma device.
	 */
#ifndef SDT
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}
#else
	CfgPtr = XAxiDma_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", BaseAddress);
		return XST_FAILURE;
	}
#endif

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if (XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DMA_TO_DEVICE);

	Value = TEST_START_VALUE;

	for (Index = 0; Index < MAX_PKT_LEN; Index ++) {
		TxBufferPtr[Index] = Value;

		Value = (Value + 1) & 0xFF;
	}
	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, MAX_PKT_LEN);
	Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN);

	for (Index = 0; Index < Tries; Index ++) {


		Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) RxBufferPtr,
						MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) TxBufferPtr,
						MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*Wait till transfer is done or 1usec * 10^6 iterations of timeout occurs*/
		while (TimeOut) {
			if (!(XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) &&
			    !(XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE))) {
				break;
			}
			TimeOut--;
			usleep(1U);
		}

		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* @brief Checks the data buffer after the DMA transfer completes.
*
* @return XST_SUCCESS if Validation is successful, or XST_FAILURE if Validation failed.
*
******************************************************************************/
static int CheckData(void)
{
	u8 *RxPacket;
	int Index = 0;
	u8 Value;

	RxPacket = (u8 *) RxBuffer;
	Value = TEST_START_VALUE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, MAX_PKT_LEN);

	for (Index = 0; Index < MAX_PKT_LEN; Index++) {
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

/** @} */
