/******************************************************************************
*
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditx_example_selftest.c
* @addtogroup v_sditx Overview
* @{
*
* This file contains a design example using the XV_SdiTx driver. It performs a
* self test on the SDI Tx driver that will test its sub-cores self test
* functions.
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
#include "xv_sditx.h"
#include "xv_sditxss.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
u32 XV_SdiTxSelfTestExample(UINTPTR BaseAddress);
/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XV_SdiTx Sdi;

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
	Status = XV_SdiTxSelfTestExample(XPAR_XV_SDITX_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("XV_SdiTxSelfTest Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XV_SdiTxSelfTest Example\n\r");
	xil_printf("--- Exiting main() --- \n\r");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the Tx core initialization is successful and
* the default register values are matching
*
* @param	DeviceId is the XV_SdiTx Controller Device id.
*
* @return
*		- XST_SUCCESS if Tx initialixation and self test are successsful
*		- XST_FAILURE if either Tx initialixation and self test fails
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiTxSelfTestExample(UINTPTR BaseAddress)
{
	XV_SdiTx_Config *CfgPtr;
	u32 Status = XST_SUCCESS;
	XV_SdiTxSs_Config *SdiTxSsCfg = NULL;

	SdiTxSsCfg = XV_SdiTxSs_LookupConfig(XPAR_XV_SDITXSS_0_BASEADDR);
	CfgPtr = XV_SdiTx_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		return XST_FAILURE;
	}
	CfgPtr->BaseAddress += SdiTxSsCfg->BaseAddress;
	Status = XV_SdiTx_CfgInitialize(&Sdi, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XV_SdiTx_SelfTest(&Sdi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/** @} */
