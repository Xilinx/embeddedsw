/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_intr_example.c
*
* @brief Demonstrates the UartLite driver in interrupt mode.
*
* @note
* Provide a physical loopback so transmitted data is received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  02/13/02 First release
* 1.00b rpm  10/01/03 Made XIntc declaration global
* 1.00b sv   06/09/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs and minor changes
*		      for coding guidelnes.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.9   gm   07/08/23 Added SDT support
* 3.12  bkv  07/07/25 Fixed GCC Warnings.
* 3.13  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* 3.14  vn   08/26/26 Fix Doxygen warnings
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup xuartlite_intr_example
 * @{
 */

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartlite.h"
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#endif
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
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
/** UartLite device ID. */
#define UARTLITE_DEVICE_ID      XPAR_UARTLITE_0_DEVICE_ID
/** UartLite interrupt vector ID. */
#define UARTLITE_IRPT_INTR	  XPAR_FABRIC_UARTLITE_0_VEC_ID
#ifdef XPAR_INTC_0_DEVICE_ID
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
/** UartLite IRQ ID. */
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID
#else
/** GIC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#else
/** UartLite base address. */
#define XUARTLITE_BASEADDRESS	XPAR_XUARTLITE_0_BASEADDR
#endif
/** @} */

/** @name Buffer size
 * @{
 */
/** Length of the send and receive buffers. */
#define TEST_BUFFER_SIZE        100
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
int UartLiteIntrExample(INTC *IntcInstancePtr, XUartLite *UartLiteInstancePtr,
			u16 UartLiteDeviceId, u16 UartLiteIntrId);
#else
int UartLiteIntrExample(XUartLite *UartLiteInstancePtr,
			UINTPTR BaseAddress);
#endif

#ifndef SDT
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XUartLite *UartLiteInstancePtr, u16 UartLiteIntrId);
#endif

void SendHandler(void *CallBackRef, unsigned int EventData);

void RecvHandler(void *CallBackRef, unsigned int EventData);

/************************** Variable Definitions *****************************/

XUartLite UartLite;		/**< The instance of the UartLite Device. */
#ifndef SDT
static INTC IntcInstance;	/**< Instance of the Interrupt Controller. */
#endif

/** @name Shared example variables
 *
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global. The counters are used
 * to determine when the entire buffer has been sent and received.
 * @{
 */
u8 SendBuffer[TEST_BUFFER_SIZE];	/**< Buffer for transmitting data. */
u8 ReceiveBuffer[TEST_BUFFER_SIZE];	/**< Buffer for receiving data. */

static volatile int TotalReceivedCount;	/**< Count of total bytes received. */
static volatile int TotalSentCount;	/**< Counter for total bytes sent. */
/** @} */

/*****************************************************************************/
/**
 *
 * @brief Calls the UartLite interrupt example.
 *
 * @return      XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
 *
 ******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the UartLite Interrupt example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
#ifndef SDT
	Status = UartLiteIntrExample(&IntcInstance, &UartLite,
				     UARTLITE_DEVICE_ID,
				     UARTLITE_IRPT_INTR);
#else
	Status = UartLiteIntrExample(&UartLite, XUARTLITE_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartlite interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Performs a minimal test on the UartLite device and driver.
*
* This function is a design example that illustrates how to use the XUartLite
* component.
*
* This function sends data and expects to receive the same data through the
* UartLite. A physical loopback must be provided such that data which is
* transmitted is received.
*
* This function uses interrupt driver mode of the UartLite device. The calls
* to the UartLite driver in the handlers should only use the non-blocking
* calls.
*
* @param[in]   UartLiteInstPtr Pointer to the instance of UartLite driver.
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   IntcInstancePtr Pointer to the instance of the interrupt
*              controller.
* @param[in]   DeviceId Device ID of the UartLite Device and is the
*              XPAR_<uartlite_instance>_DEVICE_ID value from xparameters.h.
* @param[in]   UartLiteIntrId Interrupt Id and is typically
*              XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID value from
*              xparameters.h.
* @endif
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note        - This function contains an infinite loop such that if interrupts
*               are not working it may never return.
*              - In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
int UartLiteIntrExample(INTC *IntcInstancePtr, XUartLite *UartLiteInstPtr,
			u16 DeviceId, u16 UartLiteIntrId)
#else
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			UINTPTR BaseAddress)
#endif
{
	int Status;
	int Index;
#ifdef SDT
	XUartLite_Config *CfgPtr;
	(void)UartLiteInstPtr;
#endif

	/*
	 * Initialize the UartLite driver so that it's ready to use.
	 */
#ifndef SDT
	Status = XUartLite_Initialize(&UartLite, DeviceId);
#else
	CfgPtr = XUartLite_LookupConfig(BaseAddress);
	Status = XUartLite_Initialize(&UartLite, BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XUartLite_SelfTest(&UartLite);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the UartLite to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(IntcInstancePtr, UartLiteInstPtr,
				      UartLiteIntrId);
#else
	Status = XSetupInterruptSystem(&UartLite, &XUartLite_InterruptHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the UartLite that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the UartLite driver instance as the callback reference so
	 * that the handlers are able to access the instance data.
	 */
	XUartLite_SetSendHandler(&UartLite, SendHandler, &UartLite);
	XUartLite_SetRecvHandler(&UartLite, RecvHandler, &UartLite);

	/*
	 * Enable the interrupt of the UartLite so that interrupts will occur.
	 */
	XUartLite_EnableInterrupt(&UartLite);

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
		ReceiveBuffer[Index] = 0;
	}

	/*
	 * Start receiving data before sending it since there is a loopback.
	 */
	XUartLite_Recv(&UartLite, ReceiveBuffer, TEST_BUFFER_SIZE);

	/*
	 * Send the buffer using the UartLite.
	 */
	XUartLite_Send(&UartLite, SendBuffer, TEST_BUFFER_SIZE);

	/*
	 * Wait for the entire buffer to be received, letting the interrupt
	 * processing work in the background, this function may get locked
	 * up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalReceivedCount != TEST_BUFFER_SIZE)
	       || (TotalSentCount != TEST_BUFFER_SIZE)) {
	}

	/*
	 * Verify the entire receive buffer was successfully received.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (ReceiveBuffer[Index] != SendBuffer[Index]) {
			return XST_FAILURE;
		}
	}

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
void SendHandler(void *CallBackRef, unsigned int EventData)
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
* processing performed should be minimized. It is called when data is
* present in the receive FIFO of the UartLite such that the data can be
* retrieved from the UartLite. The size of the data present in the FIFO is
* not known when this function is called.
*
* This handler provides an example of how to handle data for the UartLite,
* but is application specific.
*
* @param[in]   CallBackRef Callback reference from the driver, in
*              this case it is the instance pointer for the UartLite driver.
* @param[in]   EventData Number of bytes sent or received for sent
*              and receive events.
*
******************************************************************************/
void RecvHandler(void *CallBackRef, unsigned int EventData)
{
	(void)CallBackRef;
	TotalReceivedCount = EventData;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system such that interrupts can occur for the
* UartLite device.
*
* This function is application specific since the actual system may or may
* not have an interrupt controller. The UartLite could be directly connected
* to a processor without an interrupt controller. Modify this function
* to fit the application.
*
* @param[in]   IntcInstancePtr Pointer to the instance of the interrupt
*              controller.
* @param[in]   UartLiteInstancePtr Pointer to the instance of the
*              UartLite component which is going to be connected to the
*              interrupt controller.
* @param[in]   UartLiteIntrId Interrupt Id and is typically
*              XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID value from
*              xparameters.h.
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XUartLite *UartLiteInstancePtr, u16 UartLiteIntrId)
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
			       (void *)UartLiteInstancePtr);
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

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, UartLiteIntrId, 0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, UartLiteIntrId,
				 (Xil_ExceptionHandler) XUartLite_InterruptHandler,
				 UartLiteInstancePtr);
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
				     (Xil_ExceptionHandler) INTC_HANDLER, IntcInstancePtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable()
	;

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}
#endif
/** @} */
