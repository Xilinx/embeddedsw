/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_example.c
*
* This file contains an example using the XPrd driver to test the registers
* on the device.
*
* @note		None
*
* MODIFICATION HISTORY:
* <pre>
* Ver	 Who   Date	      Changes
* ---- ----- ------------  ----------------------------------------------
* 1.0   ms    07/14/2016     First Release
*       ms    04/05/2017     Modified comment lines notation in functions to
*                            avoid unnecessary description displayed
*                            while generating doxygen.
* 2.2   Nava  06/22/2023     Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"
#include "xgpio.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define XPRD_DEVICE_ID	XPAR_PR_DECOUPLER_0_DEVICE_ID
#define XGPIO_DEVICE_ID	XPAR_GPIO_0_DEVICE_ID
#else
#define PRD_BASEADDR    XPAR_XPRD_0_BASEADDR
#define XGPIO_BASEADDR	XPAR_XGPIO_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
u32 XPrd_Example(u16 DeviceId);
#else
u32 XPrd_Example(UINTPTR BaseAddress);
#endif
u32 XPrd_TestDecouplerState(XGpio DecouplerGpio);

/************************** Variable Definitions *****************************/

XPrd Prd;	/* Instance of the PR Decoupler */

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
#ifndef SDT
	Status = XPrd_Example((u16)XPRD_DEVICE_ID);
#else
	Status = XPrd_Example(PRD_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("PR Decoupler Example is failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PR Decoupler Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the PR Decoupler device and driver as a
* design example.
*
* @param	DeviceId is the XPAR_<prd_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
u32 XPrd_Example(u16 DeviceId)
#else
u32 XPrd_Example(UINTPTR BaseAddress)
#endif
{
	u32 Status;
	XPrd_Config *XPrdCfgPtr;
	XGpio_Config *XDecouplerGpioCfgPtr;
	XGpio DecouplerGpio;
		/* Instance of the GPIO that's connected to the USR_ACCESS primitive */

	/*
	 * Initialize the PR Decoupler driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	XPrdCfgPtr = XPrd_LookupConfig(DeviceId);
#else
	XPrdCfgPtr = XPrd_LookupConfig(BaseAddress);
#endif
	if (NULL == XPrdCfgPtr) {
		return XST_FAILURE;
	}

	Status = XPrd_CfgInitialize(&Prd, XPrdCfgPtr, XPrdCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the Decoupler GPIO */
#ifndef SDT
	XDecouplerGpioCfgPtr = XGpio_LookupConfig((u16)XGPIO_DEVICE_ID);
#else
	XDecouplerGpioCfgPtr = XGpio_LookupConfig(XGPIO_BASEADDR);
#endif
	if (NULL == XDecouplerGpioCfgPtr) {
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&DecouplerGpio, XDecouplerGpioCfgPtr,
				XDecouplerGpioCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XPrd_SelfTest(&Prd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XPrd_TestDecouplerState(DecouplerGpio);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test the SetDecouplerState and GetDecouplerState.
*
* @param	DecouplerGpio is the Instance of the GPIO that's connected to
*		the USR_ACCESS primitive.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.
*
******************************************************************************/
u32 XPrd_TestDecouplerState(XGpio DecouplerGpio)
{
	u16 Index;
	u8 DecouplingOn;
	u32 Value_in;
	u32 Value_out;
	u32 Expected_Value_out;
	u32 DecouplerState;

	/*
	 * Decoupling OFF
	 * REPEAT: Write to GPIO 2 and make sure GPIO 1 takes the same value
	 * Decoupling ON
	 * REPEAT: Write to GPIO 2 and make sure GPIO 1 takes the decoupled
	 * value
	 */
	for (Index = 0; Index < 4; Index++) {
		DecouplingOn = Index %2;
		XPrd_SetDecouplerState(&Prd, DecouplingOn);
		DecouplerState = XPrd_GetDecouplerState(&Prd);
		if (DecouplerState != DecouplingOn) {
			xil_printf("    ERROR: DecouplerState = %0u,"
				"DecouplingOn = %0u\n", DecouplerState,
				DecouplingOn);
			return XST_FAILURE;
		}

		for (Value_in = 0; Value_in < 10; Value_in++) {

			XGpio_DiscreteWrite(&DecouplerGpio, 2, Value_in);
			Value_out = XGpio_DiscreteRead(&DecouplerGpio, 1);

			if (DecouplingOn) {
				Expected_Value_out = 0;
			} else {
				Expected_Value_out = Value_in;
			}

			if (Expected_Value_out != Value_out) {
				xil_printf("    ERROR: DecouplingOn = %0d."
					"In = %0u, out = %0u, expected = %0u"
					"\n", DecouplingOn, Value_in,
					Value_out, Expected_Value_out);
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}
