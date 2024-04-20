/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_examples.c
*
* This file contains the examples main function.
*
* Note: MGT si570 oscillator is set to 152.25MHz by default. The DFE IP wrapper
*       requires MGT clock to be set to 122.88MHz (some IP use 61.44MHz).
*       Prerequisite is to set the MGT si570 oscillator to the required IP
*       before running the example code. This is for the ZCU670 production
*       platform.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   dc     03/08/21 Initial version
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/10/21 Set sequence length only once
* 1.1   dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
* 1.3   dc     02/07/22 Configure 2 CC and 3 RC examples
*       dc     03/21/22 Add prefix to global variables
* 1.4   dc     04/26/22 Add dynamic config example
* 1.7   cog    19/02/24 SDT Support
*       cog    04/20/24 Configure si570 in Linux examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfeprach_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
extern int XDfePrach_SelfTestExample();
extern int XDfePrach_2CC3RCTestExample();
extern int XDfePrach_2CC3RCReconfigureTestExample();
extern int XDfePrach_2CC3RCDynamicTestExample();
/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t XDfePrach_metal_phys[XDFEPRACH_MAX_NUM_INSTANCES] = {
	XDFEPRACH_NODE_BASE,
};

struct metal_device XDfePrach_CustomDevice[XDFEPRACH_MAX_NUM_INSTANCES] = {
	XDFEPRACH_CUSTOM_DEV(XDFEPRACH_NODE_NAME,
			     XDFEPRACH_NODE_BASE, 0),
};
#endif

/****************************************************************************/
/**
*
* Main function that invokes the polled example in this file.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
*****************************************************************************/
int main(void)
{
	printf("\r\n\nDFE Prach (PRACH) Selftest Examples: Start\r\n");

	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Prach init/close example. Specify the Device ID
	 * that is generated in xparameters.h for bare metal.
	 */
	if (XST_SUCCESS != XDfePrach_SelfTestExample()) {
		printf("Selftest Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Prach example setting 2 CC and 3 RC.
	 */
	if (XST_SUCCESS != XDfePrach_2CC3RCTestExample()) {
		printf("Setting 2 CC and 3 RC Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Prach example setting 2 CC and 3 RC than reconfigure
	 * one RC.
	 */
	if (XST_SUCCESS != XDfePrach_2CC3RCReconfigureTestExample()) {
		printf("Setting 2 CC and 3 RC Reconfigure Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Prach example setting dynamically 2 CC and 3 RC.
	 */
	if (XST_SUCCESS != XDfePrach_2CC3RCDynamicTestExample()) {
		printf("Setting 2 CC and 3 RC Dynamic Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run DFE Prach (PRACH) Examples\r\n");

	return XST_SUCCESS;
}
