/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_selftest_example.c
*
* This file contains an example using the XZDma driver to do self test
* on the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
* 1.5   adk    11/22/17  Added peripheral test app support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"

/************************** Function Prototypes ******************************/

#ifndef SDT
int XZDma_SelfTestExample(u16 DeviceId);
#else
int XZDma_SelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define ZDMA_DEVICE_ID 	XPAR_XZDMA_0_DEVICE_ID /* ZDMA device Id */
#endif

/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */

#ifndef TESTAPP_GEN
/*****************************************************************************/
/**
*
* Main function to call the example.
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
	int Status;

	/* Run the selftest example */
#ifndef SDT
	Status = XZDma_SelfTestExample((u16)ZDMA_DEVICE_ID);
#else
	Status = XZDma_SelfTestExample(XPAR_XZDMA_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ZDMA Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the ZDMA driver.
*
* @param	DeviceId is the XPAR_<ZDMA Instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int XZDma_SelfTestExample(u16 DeviceId)
#else
int XZDma_SelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XZDma_Config *Config;

	/*
	 * Initialize the ZDMA driver so that it's ready to use.
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XZDma_LookupConfig(DeviceId);
#else
	Config = XZDma_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XZDma_CfgInitialize(&ZDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */

	Status = XZDma_SelfTest(&ZDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
