/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsi2tx_example_selftest.c
* @addtogroup csi2tx Overview
* @{
*
* This file contains a design example using the XCsi2TxSs driver. It performs a
* self test on the MIPI CSI2 Tx driver that will test its sub-cores self test
* functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 08/02/16 Initial release
*     ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings are
*                  available in all examples. This is a fix for CR-965028.
*     ms  04/05/17 Modified Comment lines in functions to recognize it
*                  as documentation block for doxygen generation of examples.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcsi2tx.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define CSI2TX_DEV_ID	XPAR_CSI2TX_0_DEVICE_ID
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 Csi2TxSelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XCsi2Tx Csi;

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
	Status = Csi2TxSelfTestExample(CSI2TX_DEV_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Csi2TxSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Csi2TxSelfTest Example\n\r");

	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Max Lane count from the generated file matches
* the value present in the protocol configuration register
*
* @param	DeviceId is the CSI2Tx Controller Device id.
*
* @return
* 		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 Csi2TxSelfTestExample(u32 DeviceId)
{
	XCsi2Tx_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XCsi2Tx_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XCsi2Tx_CfgInitialize(&Csi, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XCsi2Tx_SelfTest(&Csi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/** @} */
