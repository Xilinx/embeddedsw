/******************************************************************************
*
* Copyright (C) 2004 - 2018 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xemaclite_selftest_example.c
*
* This file contains a design example using the EmacLite driver.
*
* @note
*
* None.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/25/05 Initial release for TestApp integration.
* 1.00a sv   06/06/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  04/14/09 Removed support for TestApp
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xemaclite.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define EMAC_DEVICE_ID			XPAR_EMACLITE_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EMACLiteSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

/*
 * Instance of the driver
 */
static XEmacLite EmacLite;

/****************************************************************************/
/**
*
* This function is the main function of the EmacLite selftest example.
*
* @param	None
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE.
*
* @note		None
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the EmacLite Self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = EMACLiteSelfTestExample(EMAC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite selftest Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver selftest example.
*
* @param	DeviceId is the XPAR_<xemaclite_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int EMACLiteSelfTestExample(u16 DeviceId)
{
	int Status;
	XEmacLite_Config *ConfigPtr;
	XEmacLite *InstancePtr = &EmacLite;

	/*
	 * Initialize the EmacLite device.
	 */
	ConfigPtr = XEmacLite_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XEmacLite_CfgInitialize(InstancePtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Self Test
	 */
	Status = XEmacLite_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
