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
* @file xuartpsv_selftest_example.c
*
* This file contains an example using the XUartPsv driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartpsv.h"
#include "xil_printf.h"

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

int UartPsvSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartPsv Uart_Psv;		/* Instance of the UARTPSV Device */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE
*
* @note 	None
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
	Status = UartPsvSelfTestExample(UARTPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("UartPsv Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XUartPsv driver.
*
*
* @param	DeviceId is the XPAR_<UARTPSV_instance>_DEVICE_ID value from
*			xparameters.h
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE
*
* @note 	None
*
****************************************************************************/
int UartPsvSelfTestExample(u16 DeviceId)
{
	int Status;
	XUartPsv_Config *Config;

	/*
	 * Initialize the UARTPSV driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
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

	/* Perform a self-test to check hardware build. */
	Status = XUartPsv_SelfTest(&Uart_Psv);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
