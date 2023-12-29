/******************************************************************************
*
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirx_example_selftest.c
* @addtogroup v_sdirx Overview
* @{
*
* This file contains a design example using the XV_SdiRx driver. It performs a
* self test on the SDI Rx driver that will test its sub-cores self test
* functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_sdirx.h"
#include "xparameters.h"
#ifdef SDT
#include "xv_sdirxss.h"
#endif
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define XV_SDIRX_DEV_ID XPAR_XV_SDIRX_0_DEVICE_ID
#endif
#endif
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
u32 XV_SdiRxSelfTestExample(u32 DeviceId);
#else
u32 XV_SdiRxSelfTestExample(UINTPTR BaseAddress);
#endif
/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XV_SdiRx Sdi;

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
int main(void)
{
	int Status;

	xil_printf("\n\r--- Entering main() --- \n\r");

	/* Run the poll example for simple transfer */
#ifndef SDT
	Status = XV_SdiRxSelfTestExample(XV_SDIRX_DEV_ID);
#else
	Status = XV_SdiRxSelfTestExample(XPAR_XV_SDIRX_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {

		xil_printf("XV_SdiRxSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XV_SdiRxSelfTest Example\n\r");

	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Rx core initialization is successful and
* the default register values are matching
*
* @param	DeviceId is the XV_SdiRx Controller Device id.
*
* @return
*		- XST_SUCCESS if Rx initialixation and self test are successsful
*		- XST_FAILURE if either Rx initialixation and self test fails
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
u32 XV_SdiRxSelfTestExample(u32 DeviceId)
#else
u32 XV_SdiRxSelfTestExample(UINTPTR BaseAddress)
#endif
{
	XV_SdiRx_Config *CfgPtr;
	u32 Status = XST_SUCCESS;
#ifndef SDT
	CfgPtr = XV_SdiRx_LookupConfig(DeviceId);
#else
	XV_SdiRxSs_Config *SdiRxSsCfg = NULL;

	SdiRxSsCfg = XV_SdiRxSs_LookupConfig(XPAR_XV_SDIRXSS_0_BASEADDR);
	CfgPtr = XV_SdiRx_LookupConfig(BaseAddress);
#endif
	if (!CfgPtr) {
		return XST_FAILURE;
	}
#ifdef SDT
	CfgPtr->BaseAddress += SdiRxSsCfg->BaseAddress;
#endif
	Status = XV_SdiRx_CfgInitialize(&Sdi, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XV_SdiRx_SelfTest(&Sdi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/** @} */
