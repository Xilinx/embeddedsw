/******************************************************************************
*
* (c) Copyright 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsi_example_selftest.c
*
* This file contains an example using the XCsi driver to do self test
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
* 1.1   ms  01/23/17 Modified xil_printf statement in main function to
*                    ensure that "Successfully ran" and "Failed" strings are
*                    available in all examples. This is a fix for CR-965028.
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation of examples.
* </pre>
******************************************************************************/
/***************************** Include Files *********************************/
#include "xcsi.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define CSI2RX_DEV_ID	XPAR_CSI_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 CsiSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XCsi Csi;

/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("\n\r--- Entering main() --- \n\r");

	/* Run the poll example for simple transfer */
	Status = CsiSelfTestExample(CSI2RX_DEV_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("CsiSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CsiSelfTest Example\n\r");

	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Max Lane count from the generated file matches
* the value present in the protocol configuration register
*
* @param	DeviceId is the CSI Controller Device id.
*
* @return
* 		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 CsiSelfTestExample(u32 DeviceId)
{
	XCsi_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XCsi_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XCsi_CfgInitialize(&Csi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XCsi_SelfTest(&Csi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
