/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file rgb2ycrcb_selftest_example.c
*
* This file contains an example using the XRgb2YCrCb driver to do self test
* on the device.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 7.0   adk    01/20/14 First Release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xrgb2ycrcb.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XRGB_DEVICE_ID		XPAR_RGB2YCRCB_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XRgb2YCrCbSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRgb2YCrCb Rgb2YCrCbInst;	/**< Instance of the RGB2YCRCB core */

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
	Status = XRgb2YCrCbSelfTestExample(XRGB_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		xil_printf("RGB Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RGB2YCrCb driver Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the RGB driver.
*
* @param	DeviceId is an ID of RGB core or device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if it is unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XRgb2YCrCbSelfTestExample(u16 DeviceId)
{
	int Status;
	XRgb2YCrCb_Config *Config;

	/* Initialize the RGB2YCrCb driver so that it's ready to use look up
	 * the configuration in the config table, then initialize it.
	 */
	Config = XRgb2YCrCb_LookupConfig(DeviceId);
	if(NULL == Config){
		return XST_FAILURE;
	}

	Status = XRgb2YCrCb_CfgInitialize(&Rgb2YCrCbInst, Config,
						Config->BaseAddress);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build. */
	Status = XRgb2YCrCb_SelfTest(&Rgb2YCrCbInst);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
