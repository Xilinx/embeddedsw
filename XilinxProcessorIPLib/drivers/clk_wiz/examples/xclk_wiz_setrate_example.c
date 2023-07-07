/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
*
* @file xclk_wiz_setrate_example.c
*
* This file contains a design example using the XClk_Wiz driver with different rates.
* assigned to clock wizard 1. Modify this value as per your dynamic clock
* reconfiguration Clocking wizard
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.4 sd  5/21/20 Initial version for Clock Wizard Example
* 1.6 sd 7/7/23   Add SDT support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xclk_wiz.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
*/
#ifndef SDT
#define XCLK_WIZARD_DEVICE_ID		XPAR_CLK_WIZ_0_DEVICE_ID
#endif
#define XCLK_US_WIZ_RECONFIG_OFFSET	0x0000025C  /**< Reconfig Register */

/*
* The following constants are part of clock dynamic reconfiguration
* They are only defined here such that a user can easily change
* needed parameters
*/

#define CLK_LOCK			1

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

#ifndef SDT
u32 ClkWiz_Example(XClk_Wiz *IntcInstancePtr, u32 DeviceId);
#else
u32 ClkWiz_Example(XClk_Wiz *IntcInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/
XClk_Wiz ClkWiz_Dynamic; /* The instance of the ClkWiz_Dynamic */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the main function for XClk_Wiz rate example. If the
* ClkWiz_Example function which sets up the system succeeds, this function
* will set the desired rate.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("CLK_WIZARD example\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

#ifndef SDT
	Status = ClkWiz_Example(&ClkWiz_Dynamic, XCLK_WIZARD_DEVICE_ID);
#else
	Status = ClkWiz_Example(&ClkWiz_Dynamic, XPAR_CLK_WIZ_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CLK_WIZARD example Failed");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CLK_WIZARD example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the example using the
* XClk_Wiz driver.
*
* @param	DeviceId is the unique device ID of the CLK_WIZ
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS if successful.
*
******************************************************************************/
#ifndef SDT
u32 ClkWiz_Example(XClk_Wiz *IntcInstancePtr, u32 DeviceId)
#else
u32 ClkWiz_Example(XClk_Wiz *IntcInstancePtr, UINTPTR BaseAddress)
#endif
{
	XClk_Wiz_Config *CfgPtr_Dynamic;
	u32 Status = XST_FAILURE;
	u32 Reg;

	/*
	 * Get the CLK_WIZ Dynamic reconfiguration driver instance
	 */
#ifndef SDT
	CfgPtr_Dynamic = XClk_Wiz_LookupConfig(DeviceId);
#else
	CfgPtr_Dynamic = XClk_Wiz_LookupConfig(BaseAddress);
#endif
	if (!CfgPtr_Dynamic) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ Dynamic reconfiguration driver
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
		 CfgPtr_Dynamic->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Calling Clock wizard dynamic reconfig */

	XClk_Wiz_WriteReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);
	XClk_Wiz_SetRate(&ClkWiz_Dynamic, 200);

	XClk_Wiz_WriteReg(CfgPtr_Dynamic->BaseAddr, XCLK_US_WIZ_RECONFIG_OFFSET, (XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));
	Status = XClk_Wiz_WaitForLock(&ClkWiz_Dynamic);
	if (Status != XST_SUCCESS) {
		Reg = XClk_Wiz_ReadReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_STATUS_OFFSET) & CLK_LOCK;
		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
		": 0x1\n\r", Reg);
	}

	return Status;
}
