/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_selftest_example.c
*
* This file contains a selftest example for using the Equalizer hardware
* and Equalizer driver.
* This example does some writes to the hardware to do some sanity checks.
*
* Note: MGT si570 oscillator is set to 152.25MHz by default. The DFE IP wrapper
*       requires MGT clock to be set to 122.88MHz (some IP use 61.44MHz).
*       Prerequisite is to set the MGT si570 oscillator to the required IP
*       before running the example code. This is for the ZCU208 prodaction
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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
#include <metal/device.h>
#endif
#include "xdfeequ.h"
#include "xdfeequ_hw.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#define XDFEEQU_NODE_NAME XPAR_XDFEEQU_0_DEV_NAME
#else
#define XDFEEQU_NODE_NAME "a6080000.xdfe_equalizer"
#endif

#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeEqu_SelfTestExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFEEQU_MAX_NUM_INSTANCES] = {
	XPAR_XDFEEQU_0_BASEADDR,
};
struct metal_device CustomDevice[XDFEEQU_MAX_NUM_INSTANCES] = {
	{
		.name = XPAR_XDFEEQU_0_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XPAR_XDFEEQU_0_BASEADDR,
			.physmap = &metal_phys[0],
			.size = 0x10000,
			.page_shift = (u32)(-1),
			.page_mask = (u32)(-1),
			.mem_flags = 0x0,
			.ops = { NULL },
		} },
		.node = { NULL },
		.irq_num = 0,
		.irq_info = NULL,
	},
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
* @note		None.
*
*****************************************************************************/
int main(void)
{
	printf("DFE Equalizer (EQU) Example Test\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the Equalizer fabric rate example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeEqu_SelfTestExample()) {
		printf("Selftest Example Test failed\r\n");
		return XST_FAILURE;
	}

	printf("Successfully ran Selftest Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Equalizer device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Activate the device.
*	- Write and read coefficient.
*	- DeActivate the device.
*
* @return
*		- XDFEEQU_SUCCESS if the example has completed successfully.
*		- XDFEEQU_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
static int XDfeEqu_SelfTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeEqu_Cfg Cfg;
	XDfeEqu *InstancePtr = NULL;
	XDfeEqu_EqConfig Config;

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeEqu_InstanceInit(XDFEEQU_NODE_NAME);
	Config.DatapathMode = 0;

	/* Go through initialization states of the state machine */
	XDfeEqu_Reset(InstancePtr);
	XDfeEqu_Configure(InstancePtr, &Cfg);
	XDfeEqu_Initialize(InstancePtr, &Config);
	XDfeEqu_Activate(InstancePtr, true);

	/* Write and read a dummy Coefficient[0] value */
	XDfeEqu_WriteReg(InstancePtr, XDFEEQU_COEFFICIENT_SET, 0x1234);
	if (0x1234 != XDfeEqu_ReadReg(InstancePtr, XDFEEQU_COEFFICIENT_SET)) {
		return XST_FAILURE;
	}

	XDfeEqu_Deactivate(InstancePtr);
	XDfeEqu_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
