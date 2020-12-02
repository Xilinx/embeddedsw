/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_selftest_example.c
*
* This file contains a design example using the Uart 16450/550 driver
* (XUartNs550) and hardware device using polled mode.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   10/07/05 Created for Test App integration
* 2.00a ktn  10/20/09 Updated to use HAL processor APIs and minor modifications
*		      as per coding guidelines.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartns550.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define UART_DEVICE_ID		XPAR_UARTNS550_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartNs550SelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartNs550 UartNs550;	/* Instance of the UART Device */

/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None.
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
	 *  Run the UartNs550 selftest example
	 */
	Status = UartNs550SelfTestExample(UART_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Uartns550 selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartns550 selftest Example\r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the UartNs550 device and driver as a
* design example. The purpose of this function is to illustrate how to use the
* XUartNs550 component.
*
*
* @param	DeviceId is the XPAR_<uartns550_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int UartNs550SelfTestExample(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the UartNs550 driver so that it's ready to use
	 */
	Status = XUartNs550_Initialize(&UartNs550, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly
	 */
	Status = XUartNs550_SelfTest(&UartNs550);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
