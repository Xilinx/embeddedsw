/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_selftest_example.c
*
* This file contains a selftest example for using the Prach hardware
* and Prach driver.
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
* 1.0   dc     03/08/21 Initial version
*       dc     04/06/21 Register with full node name
*       dc     04/07/21 Fix bare metal initialisation
*       dc     04/10/21 Set sequence length only once
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
#include <metal/device.h>
#endif
#include "xdfeprach.h"
#include "xdfeprach_hw.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#define XDFEPRACH_NODE_NAME XPAR_XDFEPRACH_0_DEV_NAME
#else
#define XDFEPRACH_NODE_NAME "a7d40000.xdfe_nr_prach"
#endif

#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88

/************************** Function Prototypes *****************************/
extern int XDfeSi570_SetMgtOscillator(double CurrentFrequency,
				      double NewFrequency);
static int XDfePrach_SelfTestExample();

/************************** Variable Definitions ****************************/
#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys[XDFEPRACH_MAX_NUM_INSTANCES] = {
	XPAR_XDFEPRACH_0_BASEADDR,
};
struct metal_device CustomDevice[XDFEPRACH_MAX_NUM_INSTANCES] = {
	{
		.name = XPAR_XDFEPRACH_0_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = { {
			.virt = (void *)XPAR_XDFEPRACH_0_BASEADDR,
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
	printf("DFE Prach (PRACH) Selftest Example Test\r\n");

#ifdef __BAREMETAL__
	if (XST_SUCCESS !=
	    XDfeSi570_SetMgtOscillator(XDFESI570_CURRENT_FREQUENCY,
				       XDFESI570_NEW_FREQUENCY)) {
		printf("Setting MGT oscillator failed\r\n");
		return XST_FAILURE;
	}
#endif

	/*
	 * Run the DFE Prach init/close example, specify the Device
	 * ID that is generated in xparameters.h.
	 */
	if (XST_SUCCESS != XDfePrach_SelfTestExample()) {
		printf("Selftest Example Test failed\r\n");
		return XST_FAILURE;
	}

	printf("Successfully run Selftest Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the DFE Prach device using the driver APIs.
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
static int XDfePrach_SelfTestExample()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfePrach_Cfg Cfg;
	XDfePrach *InstancePtr = NULL;
	XDfePrach_Init Init;

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n");
		return XST_FAILURE;
	}

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfePrach_InstanceInit(XDFEPRACH_NODE_NAME);
	/* Go through initialization states of the state machine */
	XDfePrach_Reset(InstancePtr);
	XDfePrach_Configure(InstancePtr, &Cfg);
	XDfePrach_Initialize(InstancePtr, &Init);
	XDfePrach_Activate(InstancePtr, true);

	/* Write and read a dummy frequency configuration */
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_PHASE_PHASE_ACC, 0x12345678);
	if (0x12345678 !=
	    XDfePrach_ReadReg(InstancePtr, XDFEPRACH_PHASE_PHASE_ACC)) {
		return XST_FAILURE;
	}

	XDfePrach_Deactivate(InstancePtr);
	XDfePrach_InstanceClose(InstancePtr);
	return XST_SUCCESS;
}
