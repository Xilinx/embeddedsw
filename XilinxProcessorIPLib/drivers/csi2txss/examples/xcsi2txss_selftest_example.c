/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_selftest_example.c
* @addtogroup csi2txss Overview
* @{
*
* This file contains a design example using the XCsi2TxSs driver. It performs a
* self test on the MIPI CSI2 Tx Subsystem that will test its sub-cores
* self test functions.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
*     ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings are
*                  available in all examples. This is a fix for CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcsi2txss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the MIPI CSI2 Tx Subsystem instance to be used */
#ifndef TESTAPP_GEN
#define XCSI2TXSS_DEVICE_ID	XPAR_CSI2TXSS_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 Csi2TxSs_SelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

XCsi2TxSs Csi2TxSsInst;	/* The MIPI CSI2 Tx Subsystem instance.*/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This is the main function for XCsi2TxSs self test example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the self test example passed.
*		- XST_FAILURE if the self test example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	u32 Status;

	xil_printf("---------------------------------\n\r");
	xil_printf("MIPI CSI2 TX Subsystem self test example\n\r");
	xil_printf("---------------------------------\n\r\n\r");

	Status = Csi2TxSs_SelfTestExample(XCSI2TXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI CSI2 TX Subsystem self test example "
			"failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI CSI2 TX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function is the main entry point for the self test example using the
* XCsi2TxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	DeviceId is the unique device ID of the MIPI CSI2 TX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if any of MIPI CSI2 TX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of MIPI CSI2 TX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 Csi2TxSs_SelfTestExample(u32 DeviceId)
{
	u32 Status;
	XCsi2TxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the MIPI CSI2 TX Subsystem */
	ConfigPtr = XCsi2TxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the Csi2TxSsInst's Config
	 * structure. */
	Status = XCsi2TxSs_CfgInitialize(&Csi2TxSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI CSI2 TX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XCsi2TxSs_SelfTest(&Csi2TxSsInst);

	return Status;
}
/** @} */
