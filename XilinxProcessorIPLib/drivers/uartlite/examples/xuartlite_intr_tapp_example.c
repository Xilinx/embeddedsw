/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_intr_tapp_example.c
*
* @brief Demonstrates interrupt-mode transmission with the UartLite driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b sv   06/08/06 Created for supporting Test App Interrupt examples
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs and minor changes
*		      for coding guidelnes.
* 2.01a ssb  01/11/01 Updated the example to be used with the SCUGIC in
*		      Zynq.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.9   gm   07/08/23 Added SDT support
* 3.9   gm   09/05/23 Corrected SDT support checks.
* 3.12  bkv  07/07/25 Fixed GCC Warnings.
* 3.13  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* 3.14  vn   08/26/26 Fix Doxygen warnings
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup xuartlite_intr_tapp_example
 * @{
 */

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartlite.h"
#include "xil_exception.h"
#include <stdio.h>
#include "xil_printf.h"
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/** @name Device parameters
 *
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here so that all needed parameters
 * can be changed in one place.
 * @{
 */
#ifndef SDT
#ifndef TESTAPP_GEN
/** UartLite device ID. */
#define UARTLITE_DEVICE_ID	  XPAR_UARTLITE_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
/** UartLite interrupt vector ID. */
#define UARTLITE_IRPT_INTR	  XPAR_INTC_0_UARTLITE_0_VEC_ID
#else
/** GIC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
/** UartLite interrupt vector ID. */
#define UARTLITE_IRPT_INTR	  XPAR_FABRIC_UARTLITE_0_VEC_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif /* TESTAPP_GEN */
#else
/** UartLite base address. */
#define XUARTLITE_BASEADDRESS	XPAR_XUARTLITE_0_BASEADDR
#endif
/** @} */

/** @name Buffer size
 * @{
 */
#define TEST_BUFFER_SIZE		100 /**< Length of the send buffer. */
/** @} */

/**************************** Type Definitions *******************************/

/** @name Interrupt controller type definitions
 * @{
 */
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc /**< Interrupt controller type. */
/** Interrupt handler function. */
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic /**< Interrupt controller type. */
/** Interrupt handler function. */
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int UartLiteIntrExample(INTC *IntcInstancePtr,
			XUartLite *UartLiteInstancePtr,
			u16 UartLiteDeviceId,
			u16 UartLiteIntrId);
#else
int UartLiteIntrExample(XUartLite *UartLiteInstancePtr,
			UINTPTR BaseAddress);
#endif

static void UartLiteSendHandler(void *CallBackRef, unsigned int EventData);

static void UartLiteRecvHandler(void *CallBackRef, unsigned int EventData);

#ifndef SDT
static int UartLiteSetupIntrSystem(INTC *IntcInstancePtr,
				   XUartLite *UartLiteInstancePtr,
				   u16 UartLiteIntrId);

static void UartLiteDisableIntrSystem(INTC *IntrInstancePtr,
				      u16 UartLiteIntrId);
#endif


/************************** Variable Definitions *****************************/

/** @name Device driver instances
 *
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs.
 * @{
 */
#ifndef TESTAPP_GEN
#ifndef SDT
static INTC IntcInstance;	/**< Instance of the Interrupt Controller. */
#endif
static XUartLite UartLiteInst;  /**< The instance of the UartLite Device. */
#endif
/** @} */

/** @name Shared example variables
 *
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global. The counter determines
 * when the entire buffer has been sent.
 * @{
 */
u8 SendBuffer[TEST_BUFFER_SIZE];	/**< Buffer for transmitting data. */

static volatile int TotalSentCount;	/**< Counter for total bytes sent. */
/** @} */


/*****************************************************************************/
/**
*
* @brief Calls the UartLite interrupt example.
*
* @return      XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the UartLite Interrupt example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
#ifndef SDT
	Status = UartLiteIntrExample(&IntcInstance,
				     &UartLiteInst,
				     UARTLITE_DEVICE_ID,
				     UARTLITE_IRPT_INTR);
#else
	Status = UartLiteIntrExample(&UartLiteInst, XUARTLITE_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite interrupt tapp Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartlite interrupt tapp Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* @brief Performs a minimal test on the UartLite device and driver.
*
* This function is a design example that illustrates how to use the XUartLite
* component.
*
* This function sends data through the UartLite.
*
* This function uses the interrupt driver mode of the UartLite. The calls to
* the UartLite driver in the interrupt handlers, should only use the
* non-blocking calls.
*
* @param[in]   UartLiteInstPtr Pointer to the instance of UartLite driver.
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   IntcInstancePtr Pointer to the instance of INTC driver.
* @param[in]   UartLiteDeviceId Device ID of the UartLite Device and
*              is the XPAR_<UARTLITE_instance>_DEVICE_ID value from
*              xparameters.h.
* @param[in]   UartLiteIntrId Interrupt ID and is typically
*              XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID value from
*              xparameters.h.
* @endif
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note        - In XSCT/classic flow, DeviceId is used to look up the device
*               configuration.
*              - This function contains an infinite loop such that if
*              interrupts are not working it may never return.
*
******************************************************************************/
#ifndef SDT
int UartLiteIntrExample(INTC *IntcInstancePtr,
			XUartLite *UartLiteInstPtr,
			u16 UartLiteDeviceId,
			u16 UartLiteIntrId)
#else
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			UINTPTR BaseAddress)
#endif

{
	int Status;
	u32 Index;
#ifdef SDT
	XUartLite_Config *CfgPtr;
#endif

	/*
	 * Initialize the UartLite driver so that it's ready to use.
	 */
#ifndef SDT
	Status = XUartLite_Initialize(UartLiteInstPtr, UartLiteDeviceId);
#else
	CfgPtr = XUartLite_LookupConfig(BaseAddress);
	Status = XUartLite_Initialize(UartLiteInstPtr, BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XUartLite_SelfTest(UartLiteInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the UartLite to the interrupt subsystem such that interrupts
	 * can occur. This function is application specific.
	 */
#ifndef SDT
	Status = UartLiteSetupIntrSystem(IntcInstancePtr,
					 UartLiteInstPtr,
					 UartLiteIntrId);
#else
	Status = XSetupInterruptSystem(UartLiteInstPtr, &XUartLite_InterruptHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the UartLite that will be called from the
	 * interrupt context when data has been sent and received,
	 * specify a pointer to the UartLite driver instance as the callback
	 * reference so the handlers are able to access the instance data.
	 */
	XUartLite_SetSendHandler(UartLiteInstPtr, UartLiteSendHandler,
				 UartLiteInstPtr);
	XUartLite_SetRecvHandler(UartLiteInstPtr, UartLiteRecvHandler,
				 UartLiteInstPtr);

	/*
	 * Enable the interrupt of the UartLite so that the interrupts
	 * will occur.
	 */
	XUartLite_EnableInterrupt(UartLiteInstPtr);

	/*
	 * Initialize the send buffer bytes with a pattern to send.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}

	/*
	 * Send the buffer using the UartLite.
	 */
	XUartLite_Send(UartLiteInstPtr, SendBuffer, TEST_BUFFER_SIZE);

	/*
	 * Wait for the entire buffer to be transmitted,  the function may get
	 * locked up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalSentCount != TEST_BUFFER_SIZE)) {
	}

#ifndef SDT
	UartLiteDisableIntrSystem(IntcInstancePtr, UartLiteIntrId);
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Handles the send complete interrupt event.
*
* This function is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the UartLite is empty and more data can be sent through the
* UartLite.
*
* This handler provides an example of how to handle data for the UartLite,
* but is application specific.
*
* @param[in]   CallBackRef Callback reference from the driver.
*              In this case it is the instance pointer for the UartLite driver.
* @param[in]   EventData Number of bytes sent or received for sent
*              and receive events.
*
******************************************************************************/
static void UartLiteSendHandler(void *CallBackRef, unsigned int EventData)
{
	(void)CallBackRef;
	TotalSentCount = EventData;
}

/*****************************************************************************/
/**
*
* @brief Handles the receive data interrupt event.
*
* This function is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when any data is
* present in the receive FIFO of the UartLite such that the data can be
* retrieved from the UartLite. The amount of data present in the FIFO is
* not known when this function is called.
*
* This handler provides an example of how to handle data for the UartLite,
* but is application specific.
*
* @param[in]   CallBackRef Callback reference from the driver,
*              in this case it is the instance pointer for the UartLite driver.
* @param[in]   EventData Number of bytes sent or received for sent
*              and receive events.
*
******************************************************************************/
static void UartLiteRecvHandler(void *CallBackRef, unsigned int EventData)
{
	(void)CallBackRef;
	(void)EventData;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system such that interrupts can occur for the
* UartLite.
*
* This function is application specific since the actual
* system may or may not have an interrupt controller. The UartLite could be
* directly connected to a processor without an interrupt controller.
* Modify this function to fit the application.
*
* @param[in]   IntcInstancePtr Pointer to the instance of INTC driver.
* @param[in]   UartLiteInstPtr Pointer to the instance of UartLite driver.
*              XPAR_<UARTLITE_instance>_DEVICE_ID value from xparameters.h.
* @param[in]   UartLiteIntrId Interrupt ID and is typically
*              XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID
*              value from xparameters.h.
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
int UartLiteSetupIntrSystem(INTC *IntcInstancePtr,
			    XUartLite *UartLiteInstPtr,
			    u16 UartLiteIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it is ready
	 * to use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, UartLiteIntrId,
			       (XInterruptHandler)XUartLite_InterruptHandler,
			       (void *)UartLiteInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the UART can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the UartLite.
	 */
	XIntc_Enable(IntcInstancePtr, UartLiteIntrId);
#else

#ifndef TESTAPP_GEN
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
#endif /* TESTAPP_GEN */

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, UartLiteIntrId,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, UartLiteIntrId,
				 (Xil_ExceptionHandler)XUartLite_InterruptHandler,
				 UartLiteInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Timer device.
	 */
	XScuGic_Enable(IntcInstancePtr, UartLiteIntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */



#ifndef TESTAPP_GEN

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER,
				     IntcInstancePtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* @brief Disables the interrupts for the UartLite.
*
* @param[in]   IntcInstancePtr Pointer to the instance of the INTC driver.
* @param[in]   UartLiteIntrId Interrupt ID and is typically
*              XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID
*              value from xparameters.h.
*
******************************************************************************/
static void UartLiteDisableIntrSystem(INTC *IntcInstancePtr,
				      u16 UartLiteIntrId)
{

	/*
	 * Disconnect and disable the interrupt for the UartLite.
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, UartLiteIntrId);
#else
	XScuGic_Disable(IntcInstancePtr, UartLiteIntrId);
	XScuGic_Disconnect(IntcInstancePtr, UartLiteIntrId);

#endif

}
#endif
/** @} */
