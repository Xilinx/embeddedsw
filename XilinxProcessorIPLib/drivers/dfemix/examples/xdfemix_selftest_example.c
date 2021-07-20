/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_selftest_example.c
*
* This file contains a selftest examples for using the Mixer hardware
* and Mixer driver.
* The examples are:
*     - initialise an instance of Mixer.
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
*       dc     02/15/21 align driver to curent specification
*       dc     02/22/21 include HW in versioning
*       dc     04/06/21 Register with full node name
*       dc     04/08/21 Set sequence length only once
*       dc     04/14/21 Add FIR_ENABLE/MIXER_ENABLE register support
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
#include "xdfemix.h"
#include "xdfemix_hw.h"
#include <math.h>

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#define XDFEMIX_NODE_NAME XPAR_XDFEMIX_0_DEV_NAME
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEMIX_NODE_NAME "a7c40000.xdfe_cc_mixer"
#endif

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeMix_SelfTestExample();
static int XDfeMix_AddCCExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFEMIX_MAX_NUM_INSTANCES] = {
	XPAR_XDFEMIX_0_S_AXI_BASEADDR,
};

struct metal_device CustomDevice[XDFEMIX_MAX_NUM_INSTANCES] = {
	XDFEMIX_CUSTOM_DEV(XPAR_XDFEMIX_0_DEV_NAME,
			   XPAR_XDFEMIX_0_S_AXI_BASEADDR, 0)
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
	printf("DFE Mixer (MIX) Selftest Example\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the DFE Mixer init/close example. For bare metal specify
	 * the Device ID that is generated in xparameters.h.
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

	printf("\r\nSuccessfully run Selftest and Add CC Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs the DFE Mixer device using the driver APIs.
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
static int XDfeMix_SelfTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeMix_Cfg Cfg;
	XDfeMix *InstancePtr = NULL;
	XDfeMix_Version SwVersion;
	XDfeMix_Version HwVersion;
	XDfeMix_Init Init;

	printf("\r\nMixer: One instance initialisation example\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeMix_InstanceInit(XDFEMIX_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeMix_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	XDfeMix_Reset(InstancePtr);
	XDfeMix_Configure(InstancePtr, &Cfg);
	XDfeMix_Initialize(InstancePtr, &Init);
	XDfeMix_Activate(InstancePtr, false);

	XDfeMix_Deactivate(InstancePtr);
	XDfeMix_InstanceClose(InstancePtr);

	printf("Mixer: Multi Instances Example pass\r\n");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs the DFE Mixer device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Read SW and HW version numbers.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Set the triggers
*	- Activate the device.
*	- Add Component Channel.
*	- DeActivate the device.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
static int XDfeMix_AddCCExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeMix_Cfg Cfg;
	XDfeMix *InstancePtr = NULL;
	XDfeMix_Init Init;
	u32 CCID;
	u32 BitSequence = 0xffff;
	u32 AntennaId;
	u32 AntennaGain;
	double FreqMhz;
	double NcoFreqMhz;
	double FrequencyControlWord;
	XDfeMix_TriggerCfg TriggerCfg;
	XDfeMix_CarrierCfg CarrierCfg;
	XDfeMix_Version SwVersion;
	XDfeMix_Version HwVersion;
	XDfeMix_InterruptMask ClearMask;

	printf("\r\nMixer: Pass through example\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\r\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeMix_InstanceInit(XDFEMIX_NODE_NAME);

	/* Get SW and HW version numbers */
	XDfeMix_GetVersions(InstancePtr, &SwVersion, &HwVersion);
	printf("SW Version: Major %d, Minor %d\r\n", SwVersion.Major,
	       SwVersion.Minor);
	printf("HW Version: Major %d, Minor %d, Revision %d, Patch %d\r\n",
	       HwVersion.Major, HwVersion.Minor, HwVersion.Revision,
	       HwVersion.Patch);

	/* Go through initialization states of the state machine */
	/* Reset */
	XDfeMix_Reset(InstancePtr);
	/* Configure */
	XDfeMix_Configure(InstancePtr, &Cfg);
	/* Initialise */
	Init.Sequence.Length = 16;
	XDfeMix_Initialize(InstancePtr, &Init);

	/* Set trigger */
	TriggerCfg.Activate.Mode = 1U;
	TriggerCfg.Activate.TuserEdgeLevel = 0;
	TriggerCfg.Activate.TUSERBit = 0;
	TriggerCfg.CCUpdate.Mode = 1U;
	TriggerCfg.CCUpdate.TuserEdgeLevel = 1U;
	TriggerCfg.CCUpdate.TUSERBit = 0;
	XDfeMix_SetTriggersCfg(InstancePtr, &TriggerCfg);

	/* Activate - disable low power */
	XDfeMix_Activate(InstancePtr, false);

	/* Set antenna gain */

	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Set antenna 0 gain */
	AntennaId = 0;
	AntennaGain = 1U;
	XDfeMix_SetAntennaGain(InstancePtr, AntennaId, AntennaGain);

	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Set antenna 0 gain */
	AntennaId = 1U;
	XDfeMix_SetAntennaGain(InstancePtr, AntennaId, AntennaGain);

	/* Add component carrier */
	/* Clear interrupt status */
	ClearMask.DLCCUpdate = 1U;
	usleep(10000);
	XDfeMix_ClearInterruptStatus(InstancePtr, &ClearMask);
	/* Add CC */
	CCID = 0;
	CarrierCfg.DUCDDCCfg.Rate = 2U;
	CarrierCfg.DUCDDCCfg.NCO = 0;
	CarrierCfg.DUCDDCCfg.CCGain = 3U;
	CarrierCfg.NCO.NCOGain = 0;
	FreqMhz = 40;
	NcoFreqMhz = 491.52;
	FrequencyControlWord = floor((FreqMhz / NcoFreqMhz) * 0x100000000);
	CarrierCfg.NCO.FrequencyCfg.FrequencyControlWord = FrequencyControlWord;
	XDfeMix_AddCC(InstancePtr, CCID, BitSequence, &CarrierCfg);

	/* Close and exit */
	XDfeMix_Deactivate(InstancePtr);
	XDfeMix_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
