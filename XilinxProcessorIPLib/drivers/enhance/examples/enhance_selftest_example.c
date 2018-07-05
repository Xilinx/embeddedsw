/******************************************************************************
 *
 * (c) Copyright 2010-14 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file enhance_selftest_example.c
*
* This file contains an example using the XEnhance driver to do self test
* on the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a drg/jz 01/13/10 First Release
* 1.03a  sg    08/14/12 Updated the example for CR 666306. Modified
*		     	the device ID to use the first Device Id
*			Removed the printf at the start of the main
* 7.0   adk    02/19/14 Modified function names as per guidelines
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"
#include "xparameters.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 XEnhanceSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XEnhance InstancePtr;		/**< Instance of the Enhance Device */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/*
	 * Run the selftest example
	 */
	Status = XEnhanceSelfTestExample((u16)XPAR_ENHANCE_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ENHANCE Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ENHANCE Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XEnhance driver.
*
*
* @param	DeviceId is the XPAR_<ENHANCE_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
u32 XEnhanceSelfTestExample(u16 DeviceId)
{
	u32 Status;
	XEnhance_Config *Config;

	/*
	 * Initialize the ENHANCE driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XEnhance_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XEnhance_CfgInitialize(&InstancePtr, Config,
			Config->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XEnhance_SelfTest(&InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	return XST_SUCCESS;
}
