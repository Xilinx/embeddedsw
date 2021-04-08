/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_selftest_examplep.c
*
* This file contains a selftest example for using the Channel filter hardware
* and CfFilter driver.
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
*       dc     01/04/21 Set mgt si570 oscillator to 122.88MHz
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/08/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/08/21 Set sequence length only once
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
#include <metal/device.h>
#endif
#include "xdfeccf.h"
#include "xdfeccf_hw.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#define XDFECCF_NODE1_NAME XPAR_XDFECCF_0_DEV_NAME
#define XDFECCF_NODE2_NAME XPAR_XDFECCF_1_DEV_NAME
#else
#define XDFECCF_NODE1_NAME "a7c00000.xdfe_cc_filter"
#define XDFECCF_NODE2_NAME "a7d00000.xdfe_cc_filter"
#endif

#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeCcf_MultiInstancesExample();
static int XDfeCcf_PassThroughTestExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFECCF_MAX_NUM_INSTANCES] = {
	XPAR_XDFECCF_0_BASEADDR,
	XPAR_XDFECCF_1_BASEADDR,
};
struct metal_device CustomDevice[XDFECCF_MAX_NUM_INSTANCES] = {
	{
		.name = XPAR_XDFECCF_0_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XPAR_XDFECCF_0_BASEADDR,
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
	{
		.name = XPAR_XDFECCF_1_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XPAR_XDFECCF_1_BASEADDR,
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
	printf("DFE Channel Filter (CCF) Example Test\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the DFE Channel Filter init/close example, specify the Device
	 * ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeCcf_MultiInstancesExample()) {
		printf("Multi Instances Example Test failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Channel Filter pass through example, specify the Device
	 * ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeCcf_PassThroughTestExample()) {
		printf("Pass through Example Test failed\r\n");
		return XST_FAILURE;
	}

	printf("Successfully ran Multi Instances and Pass Through Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Channel Filter device using the
* driver APIs.
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
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
static int XDfeCcf_MultiInstancesExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr1 = NULL;
	XDfeCcf *InstancePtr2 = NULL;
	XDfeCcf_Init Init;

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr1 = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);
	InstancePtr2 = XDfeCcf_InstanceInit(XDFECCF_NODE2_NAME);
	/* Go through initialization states of the state machine */
	XDfeCcf_Reset(InstancePtr1);
	XDfeCcf_Configure(InstancePtr1, &Cfg);
	XDfeCcf_Initialize(InstancePtr1, &Init);
	XDfeCcf_Activate(InstancePtr1, true);

	XDfeCcf_Reset(InstancePtr2);
	XDfeCcf_Configure(InstancePtr2, &Cfg);
	XDfeCcf_Initialize(InstancePtr2, &Init);
	XDfeCcf_Activate(InstancePtr2, true);

	/* Write and read a dummy Coefficient[0] value */
	XDfeCcf_WriteReg(InstancePtr1, XDFECCF_COEFF_VALUE, 0x1234);
	if (0x1234 != XDfeCcf_ReadReg(InstancePtr1, XDFECCF_COEFF_VALUE)) {
		return XST_FAILURE;
	}

	/* Write and read a dummy Coefficient[0] value */
	XDfeCcf_WriteReg(InstancePtr2, XDFECCF_COEFF_VALUE, 0x4321);
	if (0x4321 != XDfeCcf_ReadReg(InstancePtr2, XDFECCF_COEFF_VALUE)) {
		return XST_FAILURE;
	}

	XDfeCcf_Deactivate(InstancePtr1);
	XDfeCcf_Deactivate(InstancePtr2);
	XDfeCcf_InstanceClose(InstancePtr1);
	XDfeCcf_InstanceClose(InstancePtr2);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Channel Filter device using the
* driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
	- Set the triggers
*	- Activate the device.
*	- Load a channel filter coefficients.
	- Add Component Channel.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
static int XDfeCcf_PassThroughTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_Init Init = { { 4, { 0 } }, 1 };
	XDfeCcf_TriggerCfg TriggerCfg;
	XDfeCcf_Coefficients Coeffs = {
		7, 1, { 0, 0, 0, (2 << 15) - 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};
	XDfeCcf_CarrierCfg CarrierCfg = { 0, 0, 0, 1, 0, 0, 0 };

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);

	/* Go through initialization states of the state machine */
	XDfeCcf_Reset(InstancePtr);
	XDfeCcf_Configure(InstancePtr, &Cfg);
	XDfeCcf_Initialize(InstancePtr, &Init);
	XDfeCcf_SetTriggersCfg(InstancePtr, &TriggerCfg);
	XDfeCcf_Activate(InstancePtr, true);

	/* Set coefficents and add channel */
	XDfeCcf_LoadCoefficients(InstancePtr, 0, &Coeffs);
	XDfeCcf_AddCC(InstancePtr, 0, 0x7, &CarrierCfg);

	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
