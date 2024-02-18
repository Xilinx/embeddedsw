/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_rx_phy_example_selftest.c
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.1    pg    16/02/24 Initial release - Application initializes mipi_rx_phy
*                        and ensures register reads and writes are good.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xmipi_rx_phy.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef TESTAPP_GEN
#define MIPI_RX_PHY_BASEADDR XPAR_XMIPI_RX_PHY_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 Mipi_Rx_PhySelfTestExample(UINTPTR BaseAddress);

/************************** Variable Definitions *****************************/

/*
 * Device instance definitions
 */
XMipi_Rx_Phy	Mipirxphy;

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
	Status = Mipi_Rx_PhySelfTestExample((UINTPTR)MIPI_RX_PHY_BASEADDR);
	if (Status != XST_SUCCESS) {

		xil_printf("Mipi_Rx_PhySelfTestExample Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Mipi_Rx_PhySelfTestExample Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This function checks if the HS_TIMEOUT from the generated file matches
* the value present in the corresponding configuration register
*
* @param 	BaseAddress is the Mipi_Rx_Phy BaseAddress.
*
* @return
* 		- XST_SUCCESS if values match
*		- XST_FAILURE if values differ.
*
* @note		None.
*
******************************************************************************/
u32 Mipi_Rx_PhySelfTestExample(UINTPTR BaseAddress)
{
	XMipi_Rx_Phy_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = (XMipi_Rx_Phy_Config *) XMipi_Rx_Phy_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XMipi_Rx_Phy_CfgInitialize(&Mipirxphy, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XMipi_Rx_Phy_GetRegIntfcPresent(&Mipirxphy);
	if (Status == 0) {
		return XST_FAILURE;
	}

	Status = XMipi_Rx_Phy_SelfTest(&Mipirxphy);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
