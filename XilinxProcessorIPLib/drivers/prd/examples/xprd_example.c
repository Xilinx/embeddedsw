/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
*
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
#define XPRD_DEVICE_ID	XPAR_PR_DECOUPLER_0_DEVICE_ID
#define XGPIO_DEVICE_ID	XPAR_GPIO_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XPrd_Example(u16 DeviceId);
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
	Status = XPrd_Example((u16)XPRD_DEVICE_ID);
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
u32 XPrd_Example(u16 DeviceId)
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

	XPrdCfgPtr = XPrd_LookupConfig(DeviceId);
	if (NULL == XPrdCfgPtr) {
		return XST_FAILURE;
	}

	Status = XPrd_CfgInitialize(&Prd, XPrdCfgPtr, XPrdCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the Decoupler GPIO */
	XDecouplerGpioCfgPtr = XGpio_LookupConfig((u16)XGPIO_DEVICE_ID);
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
