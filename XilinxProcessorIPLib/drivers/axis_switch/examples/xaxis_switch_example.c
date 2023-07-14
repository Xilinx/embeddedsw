/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch_example.c
*
* This file demonstrates the use AXI4-Stream Switch Control Router driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.6   sd  07/14/23 Added SDT support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxis_switch.h"

/************************** Constant Definitions *****************************/

/**
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#ifndef SDT
#define XAXIS_SWITCH_DEVICE_ID		XPAR_AXIS_SWITCH_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int AxisSwitch_Example(u16 DeviceId);
#else
int AxisSwitch_Example(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XAxis_Switch AxisSwitch;	/**< Instance of the AXI4-Stream Switch core */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the main function for AXI4-Stream Switch example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if AXI4-Stream Switch example ran successfully.
*		- XST_FAILURE if AXI4-Stream Switch example failed.
*
* @note		Axis_Switch core connects up to 16 masters to 16 slaves.
*
******************************************************************************/
int main()
{
	int Status;

	/* Call the AXI4-Stream Switch example */
#ifndef SDT
	Status = AxisSwitch_Example(XAXIS_SWITCH_DEVICE_ID);
#else
	Status = AxisSwitch_Example(XPAR_AXIS_SWITCH_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("AXI4-Stream Switch driver example failed.\r\n");
		return XST_FAILURE;
	}

	xil_printf("AXI4-Stream Switch driver example ran successfully.\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the use of AXI-Stream Switch driver functions.
*
* @param	DeviceId is the unique device id of the AXI4-Stream Switch
*		core.
*
* @return
*		- XST_SUCCESS if AXI4-Stream Switch example ran successfully.
*		- XST_FAILURE if AXI4-Stream Switch example failed.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int AxisSwitch_Example(u16 DeviceId)
#else
int AxisSwitch_Example(UINTPTR BaseAddress)
#endif
{
	XAxis_Switch_Config *Config;
	int Status;
	u8 MiIndex;
	u8 SiIndex;

	/* Initialize the AXI4-Stream Switch driver so that it's ready to
	 * use look up configuration in the config table, then
	 * initialize it.
	 */
#ifndef SDT
	Config = XAxisScr_LookupConfig(DeviceId);
#else
	Config = XAxisScr_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XAxisScr_CfgInitialize(&AxisSwitch, Config,
						Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("AXI4-Stream initialization failed.\r\n");
		return XST_FAILURE;
	}

	/* Disable register update */
	XAxisScr_RegUpdateDisable(&AxisSwitch);

	/* Disable all MI ports */
	XAxisScr_MiPortDisableAll(&AxisSwitch);

	/* Source SI[1] to MI[0] */
	MiIndex = 0;
	SiIndex = 1;
	XAxisScr_MiPortEnable(&AxisSwitch, MiIndex, SiIndex);

	/* Enable register update */
	XAxisScr_RegUpdateEnable(&AxisSwitch);

	/* Check for MI port enable */
	Status = XAxisScr_IsMiPortEnabled(&AxisSwitch, MiIndex, SiIndex);
	if (Status) {
		xil_printf("MI[%d] is sourced from SI[%d].\r\n", MiIndex,
				SiIndex);
	}

	/* Disable MI[MiIndex] */
	XAxisScr_MiPortDisable(&AxisSwitch, MiIndex);

	/* Check for MI port enable */
	Status = XAxisScr_IsMiPortEnabled(&AxisSwitch, MiIndex, SiIndex);
	if (!Status) {
		xil_printf("MI[%d] is not sourced from SI[%d].\r\n", MiIndex,
				SiIndex);
	}

	return XST_SUCCESS;
}
