/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_sdirxss_selftest_example.c
* @addtogroup xv_sdirxss_v1_0
* @{
*
* This file contains a design example using the XV_SdiRxSs driver. It performs a
* self test on the SDI Rx Subsystem that will test its sub-cores
* self test functions.
*
* @note		None.
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
#include "xv_sdirxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the SDI Rx Subsystem instance to be used */
#ifndef TESTAPP_GEN
#define XV_SDIRXSS_DEVICE_ID	XPAR_XV_SDIRXSS_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 SdiRxSs_SelfTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

XV_SdiRxSs SdiRxSsInst;	/* The SDI Rx Subsystem instance.*/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This is the main function for XV_SdiRxSs self test example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the self test example passed.
*		- XST_FAILURE if the self test example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	u32 Status;

	xil_printf("---------------------------------\n\r");
	xil_printf("SDI RX Subsystem self test example\n\r");
	xil_printf("---------------------------------\n\r\n\r");

	Status = SdiRxSs_SelfTestExample(XV_SDIRXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SDI RX Subsystem self test example "
			"failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SDI RX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function is the main entry point for the self test example using the
* XV_SdiRxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param         DeviceId is the unique device ID of the SDI RX Subsystem core.
*
* @return
*		- XST_FAILURE if any of SDI RX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of SDI RX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 SdiRxSs_SelfTestExample(u32 DeviceId)
{
	u32 Status;
	XV_SdiRxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the SDI RX Subsystem */
	ConfigPtr = XV_SdiRxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the SdiRxSsInst's Config
	 * structure. */
	Status = XV_SdiRxSs_CfgInitialize(&SdiRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("SDI RX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XV_SdiRxSs_SelfTest(&SdiRxSsInst);

	return Status;
}
/** @} */
