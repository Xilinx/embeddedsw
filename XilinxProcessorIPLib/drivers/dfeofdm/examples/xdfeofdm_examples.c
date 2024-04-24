/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm_examples.c
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
* 1.0   dc     11/21/22 Initial version
* 1.1   cog    07/18/23 Modify example for SDT flow
* 1.2   cog    04/20/24 Configure si570 in Linux examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfeofdm_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
extern int XDfeOfdm_AddCCExample();
extern int XDfeOfdm_MultiAddCCExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t XDfeOfdm_metal_phys[XDFEOFDM_MAX_NUM_INSTANCES] = {
	XDFEOFDM_NODE1_BASE,
};

struct metal_device XDfeOfdm_CustomDevice[XDFEOFDM_MAX_NUM_INSTANCES] = {
	XDFEOFDM_CUSTOM_DEV(XDFEOFDM_NODE1_NAME, XDFEOFDM_NODE1_BASE,
			    0),
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
	printf("\r\n\nDFE Orthogonal Frequency Division Multiplexing (OFDM) Examples: Start\r\n");

	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE OFDM AddCC example. For bare metal
	 * specify the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeOfdm_AddCCExample()) {
		printf("AddCC Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE OFDM multiAddCC example. For bare metal
	 * specify the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeOfdm_MultiAddCCExample()) {
		printf("MultiAddCC Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run DFE Orthogonal Frequency Division Multiplexing (OFDM) Examples\r\n");

	return XST_SUCCESS;
}
