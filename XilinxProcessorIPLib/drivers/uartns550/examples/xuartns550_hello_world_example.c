/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file     xuartns550_hello_world_example.c
*
* This file contains a design example using the Uart 16450/550 driver
* (XUartNs550) and hardware device using polled mode.
*
* @note     None.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a jhl  05/13/02 First release
* 1.00a sv   06/08/05 Minor changes to comply to Doxygen and coding guidelines
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
#define UART_DEVICE_ID		XPAR_UARTNS550_0_DEVICE_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartNs550HelloWorldExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartNs550 UartNs550;		/* The instance of the UART Driver */

/*****************************************************************************/
/**
* Main function to call the example.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the Test Failed.
*		- A non-negative number indicating the number of
*		characters sent.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the UartNs550 example, specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = UartNs550HelloWorldExample(UART_DEVICE_ID);

	if (Status == XST_FAILURE) {
		xil_printf("Uartns550 hello world Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartns550 hello world Example\r\n");
	return Status;
}


/******************************************************************************/
/**
*
* This function sends Hello World with the UART 16450/550 device and driver as
* a design example. The purpose of this function is to illustrate how to use
* the XUartNs550 driver.
*
* This function polls the UART and does not require the use of interrupts.
*
* @param	DeviceId is the XPAR_<UARTNS550_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return
*		- XST_FAILURE if the UART driver could not be initialized
*		successfully.
*		- A non-negative number indicating the number of characters
*		sent.
*
* @note		None.
*
****************************************************************************/
int UartNs550HelloWorldExample(u16 DeviceId)
{
	u8 HelloWorld[] = "Hello World";
	int SentCount = 0;
	int Status;

	/*
	 * Initialize the UartNs550 device so that it is ready to use
	 */
	Status = XUartNs550_Initialize(&UartNs550, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (SentCount < sizeof(HelloWorld)) {
		/*
		 * Transmit the data
		 */
		SentCount += XUartNs550_Send(&UartNs550,
				&HelloWorld[SentCount], 1);
	}

	return SentCount;
}
