/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
*
* @file xclk_wiz_versal_example.c
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
* 1.3 sd  4/7/20 Initial version for Clock Wizard Example
* 1.4 sd  5/20/20 Use Macros and use readReg API
* 1.6 sd  7/7/23  Add SDT support.
* 1.8 sd  6/26/24 Update the device name and Baseaddress.
*     sd  9/5/24  Add GetRate and multi clock support.
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

/*
* The following constants are part of clock dynamic reconfiguration
* They are only defined here such that a user can easily change
* needed parameters
*/

#define CLK_LOCK			1

/*
 * Output frequency in MHz. User need to change this value to
 * generate grater/lesser interrupt as per input frequency
 */

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

#ifndef SDT
u32 ClkWiz_Example(u32 DeviceId);
#else
u32 ClkWiz_Example(UINTPTR BaseAddress);
#endif
u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr_Dynamic);

/************************** Variable Definitions *****************************/
XClk_Wiz ClkWiz_Dynamic; /* The instance of the ClkWiz_Dynamic */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the XClk_WaitForLock function, it will wait for lock to settle change
* frequency value
*
* @param	CfgPtr_Dynamic provides pointer to clock wizard dynamic config
*
* @return
*		- XST_SUCCESS if the lock occurs.
*		- XST_FAILURE if timeout.
*
* @note		None
*
******************************************************************************/
u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr_Dynamic)
{
	u32 Count = 0;

	while (!XClk_Wiz_ReadReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK) {
		if (Count == 10000) {
			return XST_FAILURE;
		}
		usleep(100);
		Count++;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the main function for XClk_Wiz rate example. If the
* ClkWiz_Example function which sets up the system succeeds, this function
* will set the desired rate.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
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
	Status = ClkWiz_Example(XCLK_WIZARD_DEVICE_ID);
#else
	Status = ClkWiz_Example(XPAR_XCLK_WIZ_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CLK_WIZARD Monitor interrupt example Failed");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CLK_WIZARD example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the versal example using the
* XClk_Wiz driver. This function will set up the system with interrupts
* handlers.
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
u32 ClkWiz_Example(u32 DeviceId)
#else
u32 ClkWiz_Example(UINTPTR BaseAddress)
#endif
{
	XClk_Wiz_Config *CfgPtr_Dynamic;
	u32 Status = XST_FAILURE;
	u32 Reg;
	u64 Rate;

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

	XClk_Wiz_GetRate(&ClkWiz_Dynamic, 0, &Rate);
	Rate = Rate / XCLK_MHZ;
	xil_printf("\nThe Rate %ld MHz \n",  Rate);
	/* Calling Clock wizard dynamic reconfig */

	XClk_Wiz_WriteReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);
	if (CfgPtr_Dynamic->NumClocks == 1) {
		XClk_Wiz_SetRateHz(&ClkWiz_Dynamic, 200000000);
	} else {
		XClk_Wiz_SetLeafRateHz(&ClkWiz_Dynamic, 0, 200000000);
	}

	XClk_Wiz_WriteReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
			  (XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));
	Status = XClk_WaitForLock(CfgPtr_Dynamic);
	if (Status != XST_SUCCESS) {
		Reg = XClk_Wiz_ReadReg(CfgPtr_Dynamic->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK;
		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
			   ": 0x1\n\r", Reg);
	}

	XClk_Wiz_GetRate(&ClkWiz_Dynamic, 0, &Rate);
	Rate = Rate / XCLK_MHZ;
	xil_printf("\nThe Rate %ld MHz \t Expected 200 MHz \n",  Rate);
	return Status;
}
