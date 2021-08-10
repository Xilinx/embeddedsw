/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfxasm_example.c
*
* This file contains an example using the XDfxasm driver to test the registers
* on the device.
*
* @note		None
*
* MODIFICATION HISTORY:
* <pre>
* Ver	 Who   Date	      Changes
* ---- ----- ------------  ----------------------------------------------
* 1.0   dp    07/14/2020     First Release
* 1.1   dp    08/10/2021     Typecast ShutdownMode to XDfxasm_State while
*                            invoking XDfxasm_SetState() in the routine
*                            XDfxasm_TestState() to avoid compilation issue
*                            with cpp compiler.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdfxasm.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XDFX_ASM_DEVICE_ID	XPAR_DFX_ASM_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XDfxasm_Example(u16 DeviceId);
u32 XDfxasm_TestState(void);

/************************** Variable Definitions *****************************/

XDfxasm Dfxasm;	/* Instance of the Dfx Axi shutdown manager */

/*****************************************************************************/
/**
*
* This is the main function to call the example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the selftest example */
	Status = XDfxasm_Example((u16)XDFX_ASM_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Dfx Axi Shutdown manager Example is failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Dfx Axi Shutdown manager Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the DFX Axi shutdown manager driver as a
* design example.
*
* @param	DeviceId is the XPAR_<dfxasm_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.
*
******************************************************************************/
u32 XDfxasm_Example(u16 DeviceId)
{
	u32 Status;
	XDfxasm_Config *XDfxasmCfgPtr;

	/*
	 * Initialize the DFX Axi Shutdown manager driver so that it's ready
	 * to use. Look up the configuration in the config table, then initialize
	 * it.
	 */

	XDfxasmCfgPtr = XDfxasm_LookupConfig(DeviceId);
	if (NULL == XDfxasmCfgPtr) {
		return XST_FAILURE;
	}

	Status = XDfxasm_CfgInitialize(&Dfxasm, XDfxasmCfgPtr, XDfxasmCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDfxasm_TestState();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test the SetState and GetState of Dfxasm driver.
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.
*
******************************************************************************/
u32 XDfxasm_TestState(void)
{
	u16 Index;
	u8 ShutdownMode;
	u32 ShutdownState;

	/*
	 * Loop for 4 times to check atleast two cycles of off/on
	 * shutdown.
	 */
	for (Index = 0; Index < 4; Index++) {
		ShutdownMode = Index %2;
		XDfxasm_SetState(&Dfxasm, (XDfxasm_State)ShutdownMode);
		ShutdownState = XDfxasm_GetState(&Dfxasm) & 0x1;
		if (ShutdownState != ShutdownMode) {
			xil_printf("    ERROR: ShutdownState = %0u,"
				"ShutdownMode = %0u\n", ShutdownState,
				ShutdownMode);
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}
