/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_selftest_example.c
*
* This file contains a design example using the UartLite driver (XUartLite) and
* hardware device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/25/04 First Release.
* 1.00a sv   06/13/05 Minor changes to comply to Doxygen and Coding guidelines
* 2.00a ktn  10/20/09 Minor changes as per coding guidelines.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.9   gm   07/08/23 Added SDT support
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartlite.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define UARTLITE_DEVICE_ID		XPAR_UARTLITE_0_DEVICE_ID
#else
#define XUARTLITE_BASEADDRESS		XPAR_XUARTLITE_0_BASEADDR
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int UartLiteSelfTestExample(u16 DeviceId);
#else
int UartLiteSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XUartLite UartLite;		 /* Instance of the UartLite device */

/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the UartLite self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = UartLiteSelfTestExample(UARTLITE_DEVICE_ID);
#else
	Status = UartLiteSelfTestExample(XUARTLITE_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartlite selftest Example\r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the UartLite device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XUartLite component.
*
*
* @param	DeviceId is the XPAR_<uartlite_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int UartLiteSelfTestExample(u16 DeviceId)
#else
int UartLiteSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;

	/*
	 * Initialize the UartLite driver so that it is ready to use.
	 */
#ifndef SDT
	Status = XUartLite_Initialize(&UartLite, DeviceId);
#else
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

	return XST_SUCCESS;
}

