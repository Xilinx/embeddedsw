/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss_selftest_example.c
*
* This file contains a design example using the XDsi2RxSs driver. It performs a
* self test on the MIPI DSI Rx Subsystem that will test its sub-cores
* self test functions.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 Kunal 12/2/24 Initial Release for MIPI DSI RX subsystem
* 1.1 Kunal 18/4/24 Driver name changed to DSI2RXSS.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsi2rxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique base address of the MIPI DSI2 Rx Subsystem instance
 * to be used
 */
#define XDSI2RXSS_BASE		XPAR_XDSIRXSS_0_BASEADDR

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 Dsi2RxSs_SelfTestExample(u32 BaseAddr);

/************************** Variable Definitions *****************************/

XDsi2RxSs Dsi2RxSsInst;	/* The DSI RX Subsystem instance.*/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the main function for XDsiRxSs self test example.
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

	xil_printf("---------------------------------------\n\r");
	xil_printf("MIPI DSI2 RX Subsystem self test example\n\r");
	xil_printf("---------------------------------------\n\r\n\r");

	Status = Dsi2RxSs_SelfTestExample(XDSI2RXSS_BASE);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI2 RX Subsystem self test example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI DSI2 RX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the self test example using the
* XDsi2RxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	BaseAddress is the unique Address of the MIPI DSI2 RX
*.		Subsystem core.
*
* @return
*		- XST_FAILURE if any of MIPI DSI RX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of MIPI DSI RX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 Dsi2RxSs_SelfTestExample(u32 BaseAddr)
{
	u32 Status;
	XDsi2RxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the MIPI DSI RX Subsystem */
	ConfigPtr = XDsi2RxSs_LookupConfig(BaseAddr);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	/* Copy the device configuration into the DsiRxSsInst's Config
	 * structure.
	 */
	Status = XDsi2RxSs_CfgInitialize(&Dsi2RxSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI2 RX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XDsi2RxSs_SelfTest(&Dsi2RxSsInst);

	return Status;
}
