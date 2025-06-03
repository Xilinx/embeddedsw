/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xiomodule_intr_setup_example.c
*
* This file contains a design example using the IO Module driver (XIOModule)
* and hardware device using interrupt mode.This example tests the internal
* interrupts in the IO Module.
*
* This file can be used as a standalone example or by the TestAppGen utility
* to include a test of IOModule interrupts.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 2.19  ml   05/21/25 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place. This definition is not
 * included if the example is generated from the TestAppGen test tool.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_DEVICE_ID
#else
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_BASEADDR
#endif
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
			       u32 DeviceId);

static void IOModuleSetupIntrSystem(void);

/************************** Variable Definitions *****************************/
#ifndef TESTAPP_GEN
static XIOModule IOModule; /* Instance of the IO Module */
#endif

/*****************************************************************************/
/**
*
* This is the main function for the IO Module example. This function is not
* included if the example is generated from the TestAppGen test tool.
*
* @param    None.
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	XStatus Status;

	/*
	 *  Run the example, specify the Device ID generated in xparameters.h
	 */
	Status = IOModuleInterruptSetup(&IOModule, IOMODULE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Iomodule Interrupt Setup Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Iomodule Interrupt Setup Example\r\n");
	return XST_SUCCESS;
}
#endif


/*****************************************************************************/
/**
*
* This function is used by the TestAppGen generated application to setup
* the IO Module interrupts.
*
* @param    IOModuleInstancePtr is the reference to the IO Module instance.
* @param    DeviceId is device ID of the IO Module Device , typically
*           XPAR_<IOMODULE_instance>_DEVICE_ID value from xparameters.h
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note     None.
*
******************************************************************************/
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
			       u32 DeviceId)
{
	XStatus Status;

	/*
	 * Initialize the IO Module driver so that it is ready to use.
	 */
	Status = XIOModule_Initialize(IOModuleInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIOModule_SelfTest(IOModuleInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize and enable interrupts in the processor.
	 */
	IOModuleSetupIntrSystem();

	/*
	 * Start the IO Module such that interrupts are enabled for all
	 * internal interrupts.
	 */
	Status = XIOModule_Start(IOModuleInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function initializes and enables exception handling for interrupts in
* the processor.
*
* @param	IOModuleInstancePtr is the reference to the IO Module instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void IOModuleSetupIntrSystem(void)
{
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void *) 0);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();
}
