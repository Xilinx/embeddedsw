/******************************************************************************
* Copyright (C) 2017 - 2023 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditxss_selftest_example.c
* @addtogroup v_sditxss Overview
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
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the SDI Tx Subsystem instance to be used */
#ifndef SDT
#ifndef TESTAPP_GEN
#define XV_SDITXSS_DEVICE_ID	XPAR_XV_SDITXSS_0_DEVICE_ID
#endif
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/
#ifndef SDT
u32 SdiTxSs_SelfTestExample(u32 DeviceId);
#else
u32 SdiTxSs_SelfTestExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = SdiTxSs_SelfTestExample(XV_SDITXSS_DEVICE_ID);
#else
	Status = SdiTxSs_SelfTestExample(XPAR_XV_SDITXSS_0_BASEADDR);
#endif
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
#ifndef SDT
u32 SdiTxSs_SelfTestExample(u32 DeviceId)
#else
u32 SdiTxSs_SelfTestExample(UINTPTR BaseAddress)
#endif
{
	u32 Status;
	XV_SdiTxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the SDI TX Subsystem */
#ifndef SDT
	ConfigPtr = XV_SdiTxSs_LookupConfig(DeviceId);
#else
	ConfigPtr = XV_SdiTxSs_LookupConfig(BaseAddress);
#endif
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
