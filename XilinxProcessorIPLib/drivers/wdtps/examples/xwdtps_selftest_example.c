/******************************************************************************
*
* Copyright (C) 2011 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  xwdtps_selftest_example.c
*
* This file contains a design example using the System Watchdog Timer driver
* (XWdtPs) and hardware device.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a sdm    05/27/11 First release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xwdtps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define WDT_DEVICE_ID		XPAR_XWDTPS_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int WdtPsSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XWdtPs Watchdog;		/* Instance of WatchDog Timer  */

/*****************************************************************************/
/**
* Main function to call the self test example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("WDT SelfTest Example Test\r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = WdtPsSelfTestExample(WDT_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("WDT SelfTest Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran WDT SelfTest Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function does a minimal test on the watchdog timer device and driver
* as a design example. The purpose of this function is to illustrate how to use
* the XWdtPs driver.
*
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
* @param	DeviceId is the unique device id of the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int WdtPsSelfTestExample(u16 DeviceId)
{
	int Status;
	XWdtPs_Config *ConfigPtr;

	/*
	 * Initialize the watchdog timer so that it is ready to use
	 */
	ConfigPtr = XWdtPs_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		return XST_FAILURE;
	}
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XWdtPs_CfgInitialize(&Watchdog, ConfigPtr,
				       ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the SelfTest
	 */
	Status = XWdtPs_SelfTest(&Watchdog);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
