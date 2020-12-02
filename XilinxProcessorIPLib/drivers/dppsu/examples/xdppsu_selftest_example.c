/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_selftest_example.c
 *
 * Contains a design example using the XDpPsu driver. It performs a self test on
 * the DisplayPort TX core that will compare many of the DisplayPort TX core's
 * registers against their default reset values.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  10/17/17 Initial creation.
 * </pre>
 *
*******************************************************************************/
/**************************** Function Prototypes *****************************/

u32 DpPsu_SelfTestExample(XDpPsu *InstancePtr, u16 DeviceId);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDpPsu selftest example.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the self test example passed.
 *		- XST_FAILURE if the self test example was unsuccessful - the
 *		  DisplayPort TX's registers do not match their default values
 *		  or no DisplayPort TX instance was found.
 *
 * @note	None.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	Status = DpPsu_SelfTestExample(&DpPsuInstance, DPPSU_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("XDpPsu_SelfTest failed, check register values.\n");
		return XST_FAILURE;
	}
	xil_printf("XDpPsu_SelfTest passed.\n");
	return Status;
}

/******************************************************************************/
/**
 * The main entry point for the selftest example using the XDpPsu driver. This
 * function will check whether or not the DisplayPort TX's registers are at
 * their default reset values to ensure that the core is in a known and working
 * state.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if the DisplayPort TX's registers are at their
 *		  default reset values.
 *		- XST_FAILURE if the DisplayPort TX's registers do not match
 *		  their default values or no DisplayPort TX instance was found.
 *
 * @note	None.
 *
*******************************************************************************/
u32 DpPsu_SelfTestExample(XDpPsu *InstancePtr, u16 DeviceId)
{
	u32 Status;
	XDpPsu_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort TX core. */
	ConfigPtr = XDpPsu_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDpPsu_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	/* Run the self test. */
	Status = XDpPsu_SelfTest(InstancePtr);
	return Status;
}
