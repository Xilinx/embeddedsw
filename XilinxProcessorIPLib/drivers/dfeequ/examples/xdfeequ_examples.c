/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_examples.c
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
* 1.0   dc     12/06/20 Initial version
*       dc     04/01/21 Set mgt si570 oscillator to 122.88MHz
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/22/21 align driver to current specification
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
* 1.1   dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
* 1.3   dc     03/21/22 Add prefix to global variables
* 1.5   cog    07/18/23 Modify example for SDT flow
* 1.6   cog    04/20/24 Configure si570 in Linux examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfeequ_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
extern int XDfeEqu_SelfExample();
extern int XDfeEqu_PassThroughExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t XDfeEqu_metal_phys[XDFEEQU_MAX_NUM_INSTANCES] = {
	XDFEEQU_NODE_BASE,
};

struct metal_device XDfeEqu_CustomDevice[XDFEEQU_MAX_NUM_INSTANCES] = {
	XDFEEQU_CUSTOM_DEV(XDFEEQU_NODE_NAME, XDFEEQU_NODE_BASE, 0),
};
#endif

/****************************************************************************/
/**
*
* Main function that initialise si570 and runs examples.
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
	printf("\r\n\nDFE Equalizer (EQU) Examples: Start\r\n");

	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Equalizer init/close example. Specify the Device ID
	 * that is generated in xparameters.h for bare metal.
	 */
	if (XST_SUCCESS != XDfeEqu_SelfExample()) {
		printf("Selftest Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Equalizer pass through example. For bare metal specify
	 * the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeEqu_PassThroughExample()) {
		printf("Pass through Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run DFE Equalizer (EQU) Examples\r\n");

	return XST_SUCCESS;
}
