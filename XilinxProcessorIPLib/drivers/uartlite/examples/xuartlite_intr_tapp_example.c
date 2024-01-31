/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xuartlite_intr_tapp_example.c
*
* This file contains a design example using the UartLite driver and
* hardware device using the interrupt mode for transmission of data.
*
* @note
*
* None.
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
* </pre>
******************************************************************************/

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

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define UARTLITE_DEVICE_ID	  XPAR_UARTLITE_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define UARTLITE_IRPT_INTR	  XPAR_INTC_0_UARTLITE_0_VEC_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define UARTLITE_IRPT_INTR	  XPAR_FABRIC_UARTLITE_0_VEC_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif /* TESTAPP_GEN */
#else
#define XUARTLITE_BASEADDRESS	XPAR_XUARTLITE_0_BASEADDR
#endif

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite device.
 */
#define TEST_BUFFER_SIZE		100


/**************************** Type Definitions *******************************/

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif

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

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
static INTC IntcInstance;	/* The instance of the Interrupt Controller */
#endif
static XUartLite UartLiteInst;  /* The instance of the UartLite Device */
#endif

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffer is used in this example to send data  with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];

/*
 * The following counter is used to determine when the entire buffer has
 * been sent.
 */
static volatile int TotalSentCount;


/******************************************************************************/
/**
*
* Main function to call the UartLite interrupt example.
*
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
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

/****************************************************************************/
/**
*
* This function does a minimal test on the UartLite device and driver as a
* design example. The purpose of this function is to illustrate how to use
* the XUartLite component.
*
* This function sends data through the UartLite.
*
* This function uses the interrupt driver mode of the UartLite.  The calls to
* the  UartLite driver in the interrupt handlers, should only use the
* non-blocking calls.
*
* @param	IntcInstancePtr is a pointer to the instance of INTC driver.
* @param	UartLiteInstPtr is a pointer to the instance of UartLite driver.
* @param	UartLiteDeviceId is the Device ID of the UartLite Device and
*		is the XPAR_<UARTLITE_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	UartLiteIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
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
* This function is the handler which performs processing to send data to the
* UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the UartLite is empty and more data can be sent through the UartLite.
*
* This handler provides an example of how to handle data for the UartLite, but
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void UartLiteSendHandler(void *CallBackRef, unsigned int EventData)
{
	TotalSentCount = EventData;
}

/****************************************************************************/
/**
*
* This function is the handler which performs processing to receive data from
* the UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when any data is
* present in the receive FIFO of the UartLite such that the data can be
* retrieved from the UartLite. The amount of data present in the FIFO is not
* known when this function is called.
*
* This handler provides an example of how to handle data for the UartLite, but
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void UartLiteRecvHandler(void *CallBackRef, unsigned int EventData)
{

}

#ifndef SDT
/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the UartLite. This function is application specific since the actual
* system may or may not have an interrupt controller. The UartLite could be
* directly connected to a processor without an interrupt controller. The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of INTC driver.
* @param	UartLiteInstPtr is a pointer to the instance of UartLite driver.
*		XPAR_<UARTLITE_instance>_DEVICE_ID value from xparameters.h.
* @param	UartLiteIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
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
* This function disables the interrupts that occur for the UartLite.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC driver.
* @param	UartLiteIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
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

