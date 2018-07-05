/*******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 * @file xsdiaud_selftest_example.c
 *
 * This file contains a example for using the SdiAud hardware device
 * and SdiAud driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	 Who	Date		Changes
 * ----  ---	--------	-----------------------------------------------
 * 1.0	 kar	02/16/18	First release
 * 1.1   kar    04/25/18        Changed default value of the clk phase bit to 1.
 *                              Removed version register macro.
 *                              Removed get version API call from the self test.
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xsdiaud.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SDIAUD_0_DEVICE_ID	XPAR_XSDIAUD_0_DEVICE_ID
#define SDIAUD_1_DEVICE_ID	XPAR_XSDIAUD_1_DEVICE_ID
#endif

#define XSDIAUD_NUM_REG 4 /* Number of registers to be read after reset */
#define XSDIAUD_ACR 3 /*loop count value to read the audio control register */

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int SdiAud_SelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XSdiAud SdiAud0;		/* Instance0 of the SdiAud device */

/******************************************************************************/
/**
 *
 * Main function to call the Self Test example.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
 *
 * @note		None.
 *
 *****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SDI Audio Self Test Example \r\n");

	/*
	 * Run the SdiAud Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = SdiAud_SelfTestExample(SDIAUD_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SDI Audio Self Test Failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SDI Audio Self Test Example \r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
 *
 * This function does a minimal test on the SdiAud device and
 * driver as a design example.
 *
 * @param	DeviceId is the Device ID of the SdiAud Device and is the
 *		XPAR_<sdiaud_instance>_DEVICE_ID value from xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *
 *****************************************************************************/
int SdiAud_SelfTestExample(u16 DeviceId)
{
	int Status;
	/*
	 * Initialize the SdiAud driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Status = XSdiAud_Initialize(&SdiAud0, DeviceId);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;


	return XST_SUCCESS;
}
/** @} */
