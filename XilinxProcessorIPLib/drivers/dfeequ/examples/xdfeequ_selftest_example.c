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
* The examples are:
*     - initialise an instance of Equalizer driver.
*     - initialise driver and set coefficients.
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
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEEQU_NODE_NAME "a6080000.xdfe_equalizer"
#endif

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeEqu_SelfExample();
static int XDfeEqu_LoadCoefficientsExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFEEQU_MAX_NUM_INSTANCES] = {
	XPAR_XDFEEQU_0_BASEADDR,
};
struct metal_device CustomDevice[XDFEEQU_MAX_NUM_INSTANCES] = {
	XDFEEQU_CUSTOM_DEV(XPAR_XDFEEQU_0_DEV_NAME, XPAR_XDFEEQU_0_BASEADDR, 0),
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
	printf("DFE Equalizer (EQU) Example\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the DFE Equalizer init/close example. For bare metal specify
	 * the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeEqu_SelfExample()) {
		printf("Selftest Example failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Run the DFE Equalizer load coefficents example. For bare metal
	 * specify the Device ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeEqu_LoadCoefficientsExample()) {
		printf("Pass through Example failed\r\n");
		return XST_FAILURE;
	}

	printf("\r\nSuccessfully run Selftest and Load Coefficients Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs the DFE Equalizer device using the driver APIs.
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
static int XDfeEqu_SelfExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeEqu_Cfg Cfg;
	XDfeEqu *InstancePtr = NULL;
	XDfeEqu_Version SwVersion;
	XDfeEqu_Version HwVersion;
	XDfeEqu_EqConfig Config;

	printf("\r\nEqualizer: One instance initialisation example\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeEqu_InstanceInit(XDFEEQU_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeEqu_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfeEqu_Reset(InstancePtr);
	XDfeEqu_Configure(InstancePtr, &Cfg);
	Config.DatapathMode = 0;
	XDfeEqu_Initialize(InstancePtr, &Config);
	XDfeEqu_Activate(InstancePtr, false);

	XDfeEqu_Deactivate(InstancePtr);

	printf("Equalizer: One Instance Example pass\r\n");

	XDfeEqu_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs the DFE Equalizer device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Load an equalizer coefficients.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
static int XDfeEqu_LoadCoefficientsExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeEqu_Cfg Cfg;
	XDfeEqu *InstancePtr = NULL;
	XDfeEqu_EqConfig Config;
	XDfeEqu_TriggerCfg TriggerCfg;
	XDfeEqu_Coefficients Coeffs = { 1U,
					0,
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
	u32 ChannelField = 0xffU;
	u32 Shift;
	u32 Mode = 1U;
	XDfeEqu_Version SwVersion;
	XDfeEqu_Version HwVersion;

	printf("\r\nEqualizer: Pass through example\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeEqu_InstanceInit(XDFEEQU_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeEqu_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeEqu_Reset(InstancePtr);
	/* Configure */
	XDfeEqu_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Config.DatapathMode = 0;
	XDfeEqu_Initialize(InstancePtr, &Config);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.Update.Mode = 1U;
	TriggerCfg.Update.TuserEdgeLevel = 1U;
	TriggerCfg.Update.TUSERBit = 0;
	XDfeEqu_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeEqu_Activate(InstancePtr, false);

	/* Set coefficents */
	Shift = 5;
	Coeffs.Coefficients[0] = (1 << 15) - 1;
	XDfeEqu_LoadCoefficients(InstancePtr, ChannelField, Mode, Shift,
				 &Coeffs);

	XDfeEqu_Deactivate(InstancePtr);
	XDfeEqu_InstanceClose(InstancePtr);
	printf("Load Coefficients Example pass\r\n");
	return XST_SUCCESS;
}
