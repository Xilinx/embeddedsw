/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 * @file xspdif_selftest_example.c
 *
 * This file contains a example for using the SPDIF hardware device
 * and SPDIF driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	Who	Date		Changes
 * ---- ---	--------	-----------------------------------------------
 * 1.0	kar	01/25/18	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xspdif.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SPDIF_0_DEVICE_ID	XPAR_XSPDIF_0_DEVICE_ID
#define SPDIF_1_DEVICE_ID	XPAR_XSPDIF_1_DEVICE_ID
#endif

/**************************** Type Definitions *****************************i***/

/************************** Function Prototypes *******************************/

int SpdifSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XSpdif Spdif;		/* Instance of the Spdif device */

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

	xil_printf("Spdif Self Test Example \r\n");

	/*
	 * Run the Spdif Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = SpdifSelfTestExample(SPDIF_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Spdif Self Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spdif Self Test Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *
 * This function does a minimal test on the Spdif device and
 * driver as a design example.
 *
 * @param	DeviceId is the Device ID of the Spdif Device and is the
 *		XPAR_<spdif_instance>_DEVICE_ID value from xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *
 *****************************************************************************/
int SpdifSelfTestExample(u16 DeviceId)
{
	int Status;
	XSpdif_Config *Config;
	/*
	 * Initialize the Spdif driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XSpdif_LookupConfig(DeviceId);

	if (Config == NULL)
		return XST_FAILURE;

	Status = XSpdif_CfgInitialize(&Spdif, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return Status;
}

