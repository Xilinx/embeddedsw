/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_tx_phy_example_selftest.c
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.1    pg    16/02/24 Initial release - Application initializes mipi_tx_phy
*                        and ensures register reads and writes are good.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xmipi_tx_phy.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define MIPI_TX_PHY_BASEADDR XPAR_XMIPI_TX_PHY_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 Mipi_Tx_PhySelfTestExample(UINTPTR BaseAddress);

/************************** Variable Definitions *****************************/

/*
 * Device instance definitions
 */
XMipi_Tx_Phy	Mipitxphy;

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

	/* Run the poll example for simple transfer */
	Status = Mipi_Tx_PhySelfTestExample((UINTPTR)MIPI_TX_PHY_BASEADDR);
	if (Status != XST_SUCCESS) {

		xil_printf("Mipi_Tx_PhySelfTestExample Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Mipi_Tx_PhySelfTestExample Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the HS_TIMEOUT from the generated file matches
* the value present in the corresponding configuration register
*
* @param 	BaseAddress is the Mipi_Tx_Phy BaseAddress.
*
* @return
* 		- XST_SUCCESS if values match
*		- XST_FAILURE if values differ.
*
* @note		None.
*
******************************************************************************/
u32 Mipi_Tx_PhySelfTestExample(UINTPTR BaseAddress)
{
	XMipi_Tx_Phy_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = (XMipi_Tx_Phy_Config *) XMipi_Tx_Phy_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XMipi_Tx_Phy_CfgInitialize(&Mipitxphy, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XMipi_Tx_Phy_GetRegIntfcPresent(&Mipitxphy);
	if (Status == 0) {
		return XST_FAILURE;
	}

	Status = XMipi_Tx_Phy_SelfTest(&Mipitxphy);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
