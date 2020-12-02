/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditxss_selftest_example.c
* @addtogroup xv_sditxss_v1_0
* @{
*
* This file contains a design example using the XV_SdiTxSs driver. It performs a
* self test on the SDI Tx Subsystem that will test its sub-cores
* self test functions.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_sditxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the SDI Tx Subsystem instance to be used */
#ifndef TESTAPP_GEN
#define XV_SDITXSS_DEVICE_ID	XPAR_XV_SDITXSS_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 SdiTxSs_SelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

XV_SdiTxSs SdiTxSsInst;	/* The SDI Tx Subsystem instance.*/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This is the main function for XV_SdiTxSs self test example.
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
int main(void)
{
	u32 Status;

	xil_printf("---------------------------------\n\r");
	xil_printf("SDI TX Subsystem self test example\n\r");
	xil_printf("---------------------------------\n\r\n\r");

	Status = SdiTxSs_SelfTestExample(XV_SDITXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SDI TX Subsystem self test example "
			"failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SDI TX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function is the main entry point for the self test example using the
* XV_SdiTxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	DeviceId is the unique device ID of the SDI TX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if any of SDI TX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of SDI TX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 SdiTxSs_SelfTestExample(u32 DeviceId)
{
	u32 Status;
	XV_SdiTxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the SDI TX Subsystem */
	ConfigPtr = XV_SdiTxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the SdiTxSsInst's Config
	 * structure. */
	Status = XV_SdiTxSs_CfgInitialize(&SdiTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("SDI TX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XV_SdiTxSs_SelfTest(&SdiTxSsInst);

	return Status;
}
/** @} */
