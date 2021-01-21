/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_selftest_example.c
*
* This file contains a selftest example for using the Mixer hardware
* and Mixer driver.
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
* 1.1   dc     04/01/21 Set mgt si570 oscillator to 122.88MHz
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

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef __BAREMETAL__
#define XDFEMIX_DEVICE_ID XPAR_XDFEMIX_0_DEVICE_ID
#define XDFEMIX_BASE_ADDR XPAR_XDFEMIX_0_S_AXI_BASEADDR
#else
#define XDFEMIX_DEVICE_ID 0
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif

#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfeMix_SelfTestExample(u16 DeviceId);

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[1] = {
	XDFEMIX_BASE_ADDR,
};
struct metal_device CustomDevice[1] = {
	{
		.name = XPAR_XDFEMIX_0_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XDFEMIX_BASE_ADDR,
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
	printf("Mixer Selftest Example Test\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the Mixer fabric rate example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeMix_SelfTestExample(XDFEMIX_DEVICE_ID)) {
		printf(" Selftest Example Test failed\r\n");
		return XST_FAILURE;
	}

	printf("Successfully ran Selftest Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Mixer device using the driver APIs.
* This function does the following tasks:
*	- Create and system initialize the device driver instance.
*	- Reset the device.
*	- Configure the device.
*	- Initialize the device.
*	- Activate the device.
*	- Write and read coefficient.
*	- DeActivate the device.
*
* @param	DeviceId is the instances device Id.
*
* @return
*		- XDFECCF_SUCCESS if the example has completed successfully.
*		- XDFECCF_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
static int XDfeMix_SelfTestExample(u16 DeviceId)
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeMix_Cfg Cfg;
	XDfeMix *InstancePtr = NULL;

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeMix_InstanceInit(DeviceId);
	/* Go through initialization states of the state machine */
	XDfeMix_Reset(InstancePtr);
	XDfeMix_Configure(InstancePtr, &Cfg);
	XDfeMix_Initialize(InstancePtr);
	XDfeMix_Activate(InstancePtr, true);

	/* Write and read a dummy frequency configuration */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_FREQ_CONTROL_WORD,
			 0x12345678);
	if (0x12345678 !=
	    XDfeMix_ReadReg(InstancePtr, XDFEMIX_FREQ_CONTROL_WORD)) {
		return XST_FAILURE;
	}

	XDfeMix_Deactivate(InstancePtr);
	XDfeMix_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
