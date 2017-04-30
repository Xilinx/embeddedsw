/******************************************************************************
*
* (c) Copyright 2016 Xilinx, Inc. All rights reserved.
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
* @file xdsi_example_selftest.c
*
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.1  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
*      ms  04/05/17 Modified Comment lines in functions to
*                   recognize it as documentation block for doxygen
*                   generation of examples.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xdsi.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/* Device hardware build related constants. */
#ifndef TESTAPP_GEN
#define DSI_DEV_ID	XPAR_DSI_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 DsiSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

/* Device instance definitions */
XDsi Dsi;

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

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the self test example */
	Status = DsiSelfTestExample(DSI_DEV_ID);

	if (Status != XST_SUCCESS) {

		xil_printf("DsiSelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DsiSelfTest Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function will perform DSI self test prints GUI parameters
*
* @param	DeviceId is the DSI Controller Device id.
*
* @return
*		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 DsiSelfTestExample(u32 DeviceId)
{
	XDsi_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XDsi_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XDsi_CfgInitialize(&Dsi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDsi_SelfTest(&Dsi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
