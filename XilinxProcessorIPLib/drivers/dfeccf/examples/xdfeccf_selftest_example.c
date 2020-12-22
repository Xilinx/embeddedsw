/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef __BAREMETAL__
#define XDFECCF_DEVICE_ID XPAR_XDFECCF_0_DEVICE_ID
#define XDFECCF_BASE_ADDR XPAR_XDFECCF_0_BASEADDR
#else
#define XDFECCF_DEVICE_ID 0
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/
static int XDfeCcf_SelfTestExample(u16 DeviceId);

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[1] = {
	XDFECCF_BASE_ADDR,
};
struct metal_device CustomDevice[1] = {
	{
		.name = XPAR_XDFECCF_0_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XDFECCF_BASE_ADDR,
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
	printf("DFE Channel Filter (CCF) Selftest Example Test\r\n");
	/*
	 * Run the DFE Channel Filter fabric rate example, specify the Device
	 * ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfeCcf_SelfTestExample(XDFECCF_DEVICE_ID)) {
		printf(" Selftest Example Test failed\r\n");
		return XST_FAILURE;
	}

	printf("Successfully ran Selftest Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Channel Filter device using the
* driver APIs.
* This function does the following tasks:
*	- Create and system initialise the device driver instance.
*	- Reset the device.
*	- Configure the device.
*	- Initialise the device.
*	- Activate the device.
*	- Write and read coefficient.
*	- DeActivate the device.
*
* @param	DeviceId is the instances device Id.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
static int XDfeCcf_SelfTestExample(u16 DeviceId)
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_Init Init;

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialise the instance of channel filter driver */
	InstancePtr = XDfeCcf_InstanceInit(DeviceId);
	/* Go through initialisation states of the state machine */
	XDfeCcf_Reset(InstancePtr);
	XDfeCcf_Configure(InstancePtr, &Cfg);
	XDfeCcf_Initialize(InstancePtr, &Init);
	XDfeCcf_Activate(InstancePtr, true);

	/* Write and read a dummy Coefficient[0] value */
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_COEFF_VALUE, 0x1234);
	if (0x1234 != XDfeCcf_ReadReg(InstancePtr, XDFECCF_COEFF_VALUE)) {
		return XST_FAILURE;
	}

	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
