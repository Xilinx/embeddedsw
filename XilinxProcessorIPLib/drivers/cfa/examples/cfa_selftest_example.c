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
*
* @file cfa_selftest_example.c
*
* This file contains an example using the XCfa driver to do self test
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver	Who	Date	Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a drg/jz	01/13/10 First Release
* 1.03a sg 	08/14/12 Updated the example for CR 666306. Modified
*			 the device ID to use the first Device Id
*			 Removed the printf at the start of the main
* 7.0	adk	01/15/14 Implimented main and XCfaSelfTestExample
*			 functions.
*			 Adherence to MISRA C 2012
*			 standard guidelines.
* 7.1   ms  01/23/17 Added xil_printf statement in main function to
*                    ensure that "Successfully ran" and "Failed" strings are
*                    available in all examples. This is a fix for CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"
#include "xparameters.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CFA_DEVICE_ID		XPAR_CFA_0_DEVICE_ID	/**< CFA Device ID */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


u32 XCfaSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XCfa Cfa;		/**<Instance of the CFA Device */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE.
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
	Status = XCfaSelfTestExample((u16)CFA_DEVICE_ID);
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("CFA Selftest Example Failed\r\n");
		return (int)XST_FAILURE;
	}

	xil_printf("Successfully ran CFA Selftest Example\r\n");
	return (int)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the XCfa driver.
*
*
* @param	DeviceId is the XPAR_<CFA_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
u32 XCfaSelfTestExample(u16 DeviceId)
{
	u32 Status;
	XCfa_Config *Config;
	u32 BeforeData = (u32)0x00; /**< Default value of the Control registers
					* Status Register and Error Register */
	u32 AfterData;

	/*
	 * Initialize the CFA driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCfa_LookupConfig(DeviceId);
	if (NULL == Config) {
		return (u32)XST_FAILURE;
	}

	Status = XCfa_CfgInitialize(&Cfa, Config, Config->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build.
	 */

	Status = XCfa_SelfTest(&Cfa);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test with Control register default values
	 */
	AfterData = XCfa_ReadReg(Config->BaseAddress, XCFA_CONTROL_OFFSET);
	if(BeforeData != AfterData){
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test with Status register default values
	 */
	AfterData = XCfa_ReadReg(Config->BaseAddress, XCFA_STATUS_OFFSET);
	if(BeforeData != AfterData){
		return (u32)XST_FAILURE;
	}

	/*
	 * Perform a self-test with Error register default values
	 */

	AfterData = XCfa_ReadReg(Config->BaseAddress, XCFA_ERROR_OFFSET);
	if(BeforeData != AfterData){
		return (u32)XST_FAILURE;
	}
	return (u32)XST_SUCCESS;
}
