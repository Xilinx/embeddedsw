/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xuartpsv_hello_world_example.c
*
* This file contains a design example using the XUartPsv driver in polled
* mode.
* The example uses the default setting in the XUartPsv driver:
*	. baud rate 9600
*	. 8 bit data
*	. 1 stop bit
*	. no parity
*
* @note
* This example requires an external SchmartModule connected to the pins for
* the device to display the 'Hello World' message onto a hyper-terminal.
*
* MODIFICATION HISTORY:
* <pre>
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTPSV_DEVICE_ID		XPAR_XUARTPSV_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int UartPsvHelloWorldExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartPsv Uart_Psv;		/* The instance of the UARTPSV Driver */

/*****************************************************************************/
/**
*
* Main function to call the Hello World example.
*
* @param	None
*
* @return
*		- XST_FAILURE if the Test Failed .
*		- A non-negative number indicating the number of characters
*		  sent.
*
* @note 	None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Hello World example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = UartPsvHelloWorldExample(UARTPSV_DEVICE_ID);

	if (Status == XST_FAILURE) {
		xil_printf("UartPsv Hello World Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Hello World Example\r\n");

	return Status;
}

/*****************************************************************************/
/**
*
* This function sends 'Hello World' to an external terminal in polled mode.
* The purpose of this function is to illustrate how to use the XUartPsv
* driver.
*
*
* @param	DeviceId is the unique ID for the device from hardware build.
*
* @return
*		- XST_FAILURE if the UART driver could not be initialized
*		  successfully.
*		- A non-negative number indicating the number of characters
*		  sent.
*
* @note 	None.
*
******************************************************************************/
int UartPsvHelloWorldExample(u16 DeviceId)
{
	u8 HelloWorld[] = "Hello World";
	int SentCount = 0;
	int Status;
	XUartPsv_Config *Config;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then
	 * initialize it.
	 */
	Config = XUartPsv_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPsv_CfgInitialize(&Uart_Psv, Config,
					Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (SentCount < (sizeof(HelloWorld) - 1)) {
		/* Transmit the data */
		SentCount += XUartPsv_Send(&Uart_Psv,
					&HelloWorld[SentCount], 1);
	}

	xil_printf("Hello Test Passed\r\n");

	return SentCount;
}
