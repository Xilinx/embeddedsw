/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_examples.c
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
*       dc     01/04/21 Set mgt si570 oscillator to 122.88MHz
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     04/06/21 Register with full node name
*       dc     04/08/21 Set sequence length only once
*       dc     04/14/21 Add FIR_ENABLE/MIXER_ENABLE register support
* 1.1   dc     07/13/21 Update to common latency requirements
*       dc     07/21/21 Add and reorganise examples
* 1.2   dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
* 1.3   dc     03/21/22 Add prefix to global variables
* 1.6   cog    07/18/23 Modify example for SDT flow
* 1.7   cog    04/20/24 Configure si570 in Linux examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfemix_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
extern int XDfeMix_SelfTestExample();
extern int XDfeMix_AddCCExample();
extern int XDfeMix_MultiAddCCExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t XDfeMix_metal_phys[XDFEMIX_MAX_NUM_INSTANCES] = {
	XDFEMIX_NODE_BASE,
};

struct metal_device XDfeMix_CustomDevice[XDFEMIX_MAX_NUM_INSTANCES] = {
	XDFEMIX_CUSTOM_DEV(XDFEMIX_NODE_NAME, XDFEMIX_NODE_BASE, 0)
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
	printf("\r\n\nDFE Mixer (MIX) Examples: Start\r\n");

	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Mixer init/close example. Specify the Device ID
	 * that is generated in xparameters.h for bare metal.
	 */
	if (XST_SUCCESS != XDfeMix_SelfTestExample()) {
		printf("Selftest Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Mixer pass through example. For bare metal specify
	 * the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeMix_AddCCExample()) {
		printf("Pass through Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Mixer pass through example with multiAddCC API.
	 * For bare metal specify the Device ID that is generated in
	 * xparameters.h.
	 */
	if (XST_SUCCESS != XDfeMix_MultiAddCCExample()) {
		printf("Pass through Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run DFE Mixer (MIX) Examples\r\n");

	return XST_SUCCESS;
}
