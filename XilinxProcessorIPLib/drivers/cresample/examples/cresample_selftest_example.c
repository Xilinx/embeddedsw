/******************************************************************************
*
* Copyright (C)2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file cresample_selftest_example.c
*
* This file contains an example using the XCRESAMPLE driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 4.0   adk    03/12/14 First release.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcresample.h"
#include "xparameters.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CRESAMPLE_DEVICE_ID	XPAR_CRESAMPLE_0_DEVICE_ID
					/**< CRESAMPLE Device ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/************************** Function Prototypes ******************************/


int XCresampleSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


XCresample Cresample;		/**<Instance of the CRESAMPLE Device */
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
	Status = XCresampleSelfTestExample((u16)(CRESAMPLE_DEVICE_ID));
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		xil_printf("Cresample Selftest Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Cresample Selftest Example\r\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XCRESAMPLE driver.
*
*
* @param	DeviceId is the XPAR_<CRESAMPLE_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XCresampleSelfTestExample(u16 DeviceId)
{
	int Status;
	XCresample_Config *Config;

	/*
	 * Initialize the CRESAMPLE driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCresample_LookupConfig(DeviceId);

	/* Checking Config variable */
	if (NULL == Config) {
		return (XST_FAILURE);
	}

	Status = XCresample_CfgInitialize(&Cresample, Config,
						Config->BaseAddress);
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	/*
	 * Perform a self-test to check hardware build.
	 */
	Status = XCresample_SelfTest(&Cresample);

	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);

}
