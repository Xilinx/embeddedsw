/******************************************************************************
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_selftest_example.c
*
* This file contains a selftest examples for using the Channel Filter hardware
* and Channel Filter driver.
* The examples are:
*     - initialise two instances of Channel Filter driver.
*     - add CCs to a sequence.
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
*       dc     02/08/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/08/21 Set sequence length only once
* 1.1   dc     07/13/21 Update to common latency requirements
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
#include <math.h>

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#define XDFECCF_NODE1_NAME XPAR_XDFECCF_0_DEV_NAME
#define XDFECCF_NODE2_NAME XPAR_XDFECCF_1_DEV_NAME
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFECCF_NODE1_NAME "a7c00000.xdfe_cc_filter"
#define XDFECCF_NODE2_NAME "a7d00000.xdfe_cc_filter"
#endif

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeCcf_MultiInstancesExample();
static int XDfeCcf_PassThroughExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFECCF_MAX_NUM_INSTANCES] = {
	XPAR_XDFECCF_0_BASEADDR,
	XPAR_XDFECCF_1_BASEADDR,
};
struct metal_device CustomDevice[XDFECCF_MAX_NUM_INSTANCES] = {
	XDFECCF_CUSTOM_DEV(XPAR_XDFECCF_0_DEV_NAME, XPAR_XDFECCF_0_BASEADDR, 0),
	XDFECCF_CUSTOM_DEV(XPAR_XDFECCF_1_DEV_NAME, XPAR_XDFECCF_1_BASEADDR, 1),
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
	printf("DFE Channel Filter (CCF) Example\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the DFE Channel Filter init/close example. For bare metal
	 * specify the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeCcf_MultiInstancesExample()) {
		printf("Multi Instances Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Channel Filter pass through example. For bare metal
	 * specify the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeCcf_PassThroughExample()) {
		printf("Pass through Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run Multi Instances and Pass Through Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs two instances of the DFE Channel Filter using the driver
* APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Activate the device.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
static int XDfeCcf_MultiInstancesExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr1 = NULL;
	XDfeCcf *InstancePtr2 = NULL;
	XDfeCcf_Version SwVersion;
	XDfeCcf_Version HwVersion;
	XDfeCcf_Init Init;

	printf("\r\nChannel Filter: Multi instances initialisation example\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr1 = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);
	InstancePtr2 = XDfeCcf_InstanceInit(XDFECCF_NODE2_NAME);

	/* Get SW and HW version numbers */
	XDfeCcf_GetVersions(InstancePtr1, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfeCcf_Reset(InstancePtr1);
	XDfeCcf_Configure(InstancePtr1, &Cfg);
	XDfeCcf_Initialize(InstancePtr1, &Init);
	XDfeCcf_Activate(InstancePtr1, false);

	XDfeCcf_Reset(InstancePtr2);
	XDfeCcf_Configure(InstancePtr2, &Cfg);
	XDfeCcf_Initialize(InstancePtr2, &Init);
	XDfeCcf_Activate(InstancePtr2, false);

	XDfeCcf_Deactivate(InstancePtr1);
	XDfeCcf_Deactivate(InstancePtr2);
	XDfeCcf_InstanceClose(InstancePtr1);
	XDfeCcf_InstanceClose(InstancePtr2);

	printf("Channel Filter: Multi Instances Example pass\r\n");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs the DFE Channel Filter device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Load a channel filter coefficients.
*	- Add Component Channel.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
static int XDfeCcf_PassThroughExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_Init Init;
	u32 Shift;
	u32 CCID;
	u32 BitSequence;
	XDfeCcf_TriggerCfg TriggerCfg;
	XDfeCcf_Coefficients Coeffs = {
		0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};
	XDfeCcf_CarrierCfg CarrierCfg = { 0, 0, 0, 1, 0, 0, 0 };
	XDfeCcf_Version SwVersion;
	XDfeCcf_Version HwVersion;

	printf("\r\nChannel Filter: Pass through example\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);

	/* Get SW and HW version numbers */
	XDfeCcf_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeCcf_Reset(InstancePtr);
	/* Configure */
	XDfeCcf_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Init.GainStage = 1;
	Init.Sequence.Length = 16;
	XDfeCcf_Initialize(InstancePtr, &Init);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.CCUpdate.Mode = 1U;
	TriggerCfg.CCUpdate.TuserEdgeLevel = 1U;
	TriggerCfg.CCUpdate.TUSERBit = 0;
	XDfeCcf_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeCcf_Activate(InstancePtr, false);

	/* Set coefficents */
	Shift = 5;
	Coeffs.Num = 7U;
	Coeffs.Symmetric = 1U;
	Coeffs.Value[1] = 1 << 15;
	Coeffs.Value[2] = 1 << 13;
	XDfeCcf_LoadCoefficients(InstancePtr, 1, Shift, &Coeffs);

	/* Clear events */
	XDfeCcf_ClearEventStatus(InstancePtr);
	usleep(10000U); /* Give trigger time to finish */

	/* Add component carrier */
	CCID = 0;
	BitSequence = 0xffff;
	CarrierCfg.Rate = 16U;
	CarrierCfg.Gain = round(1024*8);
	CarrierCfg.ImagCoeffSet = 0;
	CarrierCfg.RealCoeffSet = 0;
	XDfeCcf_AddCC(InstancePtr, CCID, BitSequence, &CarrierCfg);

	/* Close and exit */
	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);

	printf("Pass Through Example pass\r\n");
	return XST_SUCCESS;
}
