/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ycrcb2rgb_selftest_example.c
*
* This file contains an example using the XYCrCb2RGB driver to do self test
* on the device.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:

* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 7.0   adk    02/10/14 First Release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xycrcb2rgb.h"
#include "xparameters.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XYCC_DEVICE_ID			XPAR_YCRCB2RGB_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XYCrCb2RgbSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XYCrCb2Rgb YccInst;		/**< Instance of the YCrCb2RGB Device */

/*****************************************************************************/
/**
*
* Main/Entry function for self test example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if it is unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run selftest example */
	Status = XYCrCb2RgbSelfTestExample(XYCC_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		xil_printf("YCrCb2RGB Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran YCrCb2RGB driver Selftest example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the YCrCb2RGB driver.
*
* @param	DeviceId is an ID of the YCrCb2RGB core or device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if it is unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XYCrCb2RgbSelfTestExample(u16 DeviceId)
{
	int Status;
	XYCrCb2Rgb_Config *Config;

	/* Initialize the YCrCb2RGB driver so that it's ready to use look up
	 * the configuration in the config table, then initialize it.
	 */
	Config = XYCrCb2Rgb_LookupConfig(DeviceId);
	if(NULL == Config){
		return XST_FAILURE;
	}

	Status = XYCrCb2Rgb_CfgInitialize(&YccInst, Config,
						Config->BaseAddress);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build. */
	Status = XYCrCb2Rgb_SelfTest(&YccInst);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
