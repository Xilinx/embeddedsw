/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#ifndef SDT
#define XVTC_DEVICE_ID			XPAR_VTC_0_DEVICE_ID
#endif
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int XVtcSelfTestExample(u16 DeviceId);
#else
int XVtcSelfTestExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = XVtcSelfTestExample((XVTC_DEVICE_ID));
#else
	Status = XVtcSelfTestExample((XPAR_XVTC_0_BASEADDR));
#endif

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
#ifndef SDT
int XVtcSelfTestExample(u16 DeviceId)
#else
int XVtcSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XVtc_Config *Config;

	/* Initialize the VTC driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XVtc_LookupConfig(DeviceId);
#else
	Config = XVtc_LookupConfig(BaseAddress);
#endif

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
