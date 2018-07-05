/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* @file vtc_selftest_example.c
*
* This file contains an example using the VTC driver to do self test
* on the core.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 6.1   adk    08/23/14  First Release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xvtc.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/** The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define XVTC_DEVICE_ID			XPAR_VTC_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XVtcSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XVtc	VtcInst;		/**< Instance of the VTC core. */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Main/Entry function for self test example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run selftest example */
	Status = XVtcSelfTestExample((XVTC_DEVICE_ID));

	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		xil_printf("VTC Selftest Example Failed.\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran VTC driver Selftest Example.\r\n");

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the VTC driver.
*
* @param	DeviceId is an ID of VTC core or device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XVtcSelfTestExample(u16 DeviceId)
{
	int Status;
	XVtc_Config *Config;

	/* Initialize the VTC driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	Config = XVtc_LookupConfig(DeviceId);

	/* Checking Config variable */
	if (NULL == Config) {
		return (XST_FAILURE);
	}

	Status = XVtc_CfgInitialize(&VtcInst, Config, Config->BaseAddress);

	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	/* Perform a self-test  */
	Status = XVtc_SelfTest(&VtcInst);

	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);
}
