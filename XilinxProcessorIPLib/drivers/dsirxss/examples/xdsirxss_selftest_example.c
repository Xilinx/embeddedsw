/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsirxss_selftest_example.c
*
* This file contains a design example using the XDsiRxSs driver. It performs a
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
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsirxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* The unique base address of the MIPI DSI Rx Subsystem instance
 * to be used
 */
#define XDSIRXSS_BASE		XPAR_XDSIRXSS_0_BASEADDR

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DsiRxSs_SelfTestExample(u32 BaseAddr);

/************************** Variable Definitions *****************************/

XDsiRxSs DsiRxSsInst;	/* The DSI RX Subsystem instance.*/

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
s32 main()
{
	u32 Status;

	xil_printf("---------------------------------------\n\r");
	xil_printf("MIPI DSI RX Subsystem self test example\n\r");
	xil_printf("---------------------------------------\n\r\n\r");

	Status = DsiRxSs_SelfTestExample(XDSIRXSS_BASE);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI RX Subsystem self test example failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI DSI RX Subsystem self test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the self test example using the
* XDsiRxSs driver. This function check whether or not its sub-core drivers
* self test functions are in working state.
*
* @param	DeviceId is the unique device ID of the MIPI DSI RX
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
u32 DsiRxSs_SelfTestExample(u32 DeviceId)
{
	u32 Status;
	XDsiRxSs_Config *ConfigPtr;

	/* Obtain the device configuration for the MIPI DSI RX Subsystem */
	ConfigPtr = XDsiRxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	/* Copy the device configuration into the DsiRxSsInst's Config
	 * structure.
	 */
	Status = XDsiRxSs_CfgInitialize(&DsiRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI RX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XDsiRxSs_SelfTest(&DsiRxSsInst);

	return Status;
}
