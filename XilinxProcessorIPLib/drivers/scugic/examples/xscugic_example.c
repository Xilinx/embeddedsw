/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xscugic_example.c
*
* This file contains a design example using the Interrupt Controller driver
* (XScuGic) and hardware device. Please reference other device driver examples
* to see more examples of how the intc and interrupts can be used by a software
* application.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------
* 1.00a drg  01/18/10 First release
* 5.0   mus  04/04/22 Updated example to support it on any CPU
*                     instance, for which example is compiled.
*                     It fixes CR#1126331.
* 5.0   adk  04/18/22 Replace infinite while loop with
* 		      Xil_WaitForEventSet() API.
*       adk  30/05/22 Fix typecast of the variable InterruptProcessed.
*       adk  20/07/22 Update the Xil_WaitForEventSet() API arguments as
*      		      per latest API.
* 5.1   mus  02/13/23 Support example for each core of APU/RPU clusters in
*                     VERSAL_NET SoC.
* 5.2   mus  07/27/23 Removed dependency on XPAR_CPU_ID.
* 5.2   ml   02/21/24 Fix compilation error reported by C++ compiler.
* 5.2   mus  04/33/34 Use interrupt wrapper APIs in case of SDT flow.
* 5.5   ml   02/05/25 Fixed compilation warnings and errors
* 5.6   ml   07/21/25 Fixed GCC warnings.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include "xil_io.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xscugic.h"
#include "xil_util.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#else
#define XSCUGiC_DIST_BASEADDR 	XPAR_XSCUGIC_0_BASEADDR
#endif
#define INTC_DEVICE_INT_ID	0x0E

#define XSCUGIC_SPI_CPU_MASK	(XSCUGIC_SPI_CPU0_MASK << XPAR_CPU_ID)
#define XSCUGIC_SW_TIMEOUT_VAL	10000000U /* Wait for 10 sec */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int ScuGicExample(u16 DeviceId);
#else
int ScuGicExample(void);
#endif
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);
void DeviceDriverHandler(void *CallbackRef);

/************************** Variable Definitions *****************************/

XScuGic InterruptController; 	     /* Instance of the Interrupt Controller */
#ifndef SDT
static XScuGic_Config *GicConfig;    /* The configuration parameters of the
                                       controller */
#endif
/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing
 */
static volatile u32 InterruptProcessed = FALSE;

static void AssertPrint(const char8 *FilenamePtr, s32 LineNumber)
{
	xil_printf("ASSERT: File Name: %s ", FilenamePtr);
	xil_printf("Line Number: %d\r\n", LineNumber);
}

/*****************************************************************************/
/**
*
* This is the main function for the Interrupt Controller example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Setup an assert call back to get some info if we assert.
	 */
	Xil_AssertSetCallback(AssertPrint);

	xil_printf("GIC Example Test\r\n");

	/*
	 *  Run the Gic example , specify the Device ID generated in xparameters.h
	 */
#ifndef SDT
	Status = ScuGicExample(INTC_DEVICE_ID);
#else
	Status = ScuGicExample();
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran GIC Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is an example of how to use the interrupt controller driver
* (XScuGic) and the hardware device.  This function is designed to
* work without any hardware devices to cause interrupts. It may not return
* if the interrupt controller is not properly connected to the processor in
* either software or hardware.
*
* This function relies on the fact that the interrupt controller hardware
* has come out of the reset state such that it will allow interrupts to be
* simulated by the software.
*
* @param	DeviceId is Device ID of the Interrupt Controller Device,
*		typically XPAR_<INTC_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int ScuGicExample(u16 DeviceId)
#else
int ScuGicExample(void)
#endif
{
	int Status;
	u32 CoreId, CpuId;
#if defined (VERSAL_NET)
	u32 ClusterId;
#endif

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
#ifndef SDT
	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
				       GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XScuGic_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System
	 */
	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
				 (Xil_ExceptionHandler)DeviceDriverHandler,
				 (void *)&InterruptController);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the device and then cause (simulate) an
	 * interrupt so the handlers will be called
	 */
	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);
#else
	u32 IntrId;
	UINTPTR IntcBaseAddr;

	Status = XGetEncodedIntrId(INTC_DEVICE_INT_ID,XINTR_IS_EDGE_TRIGGERED,XINTR_IS_SGI,XINTC_TYPE_IS_SCUGIC, &IntrId);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
        }

	IntcBaseAddr = XGetEncodedIntcBaseAddr(XPAR_XSCUGIC_0_BASEADDR, XINTC_TYPE_IS_SCUGIC);

	Status =  XSetupInterruptSystem(&InterruptController,(void *)DeviceDriverHandler, IntrId, IntcBaseAddr, XINTERRUPT_DEFAULT_PRIORITY);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
        }
#endif
	/*
	 *  Simulate the Interrupt
	 */
	CoreId = XGetCoreId();
#if defined (VERSAL_NET)
	ClusterId = XGetClusterId();

#if defined (ARMR52)
	CoreId = (1 << CoreId);
#endif

	CpuId = ((ClusterId << XSCUGIC_CLUSTERID_SHIFT ) | CoreId);
#else
	CpuId = (XSCUGIC_SPI_CPU0_MASK << CoreId);
#endif

#ifndef SDT
        Status = XScuGic_SoftwareIntr(&InterruptController,
                                      INTC_DEVICE_INT_ID,
                                      CpuId);
#else
        Status = XTriggerSoftwareIntr(IntrId, IntcBaseAddr,
                                      CpuId);
#endif

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the interrupt to be processed, if the interrupt does not
	 * occur return failure after timeout.
	 */
	Status = Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1,
				     &InterruptProcessed);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the interrupt controller to
* the processor.  This function is separate to allow it to be customized for
* each application.  Each processor or RTOS may require unique processing to
* connect the interrupt handler.
*
* @param	XScuGicInstancePtr is the instance of the interrupt controller
*		that needs to be worked on.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XScuGic_InterruptHandler,
				     XScuGicInstancePtr);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* interrupt controller interrupt handler.  This handler would typically
* perform device specific processing such as reading and writing the registers
* of the device to clear the interrupt condition and pass any data to an
* application using the device driver.  Many drivers already provide this
* handler and the user is not required to create it.
*
* @param	CallbackRef is passed back to the device driver's interrupt
*		handler by the XScuGic driver.  It was given to the XScuGic
*		driver in the XScuGic_Connect() function call.  It is typically
*		a pointer to the device driver instance variable.
*		In this example, we do not care about the callback
*		reference, so we passed it a 0 when connecting the handler to
*		the XScuGic driver and we make no use of it here.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
	(void)CallbackRef;
	/*
	 * Indicate the interrupt has been processed using a shared variable
	 */
	InterruptProcessed = TRUE;
}
