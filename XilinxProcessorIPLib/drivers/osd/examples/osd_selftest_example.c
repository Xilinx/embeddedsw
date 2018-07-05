/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file osd_selftest_example.c
*
* This file contains an example using the OSD driver to do self test
* on the core.
*
* @note		None.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 4.0   adk    02/18/14 First Release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xosd.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/**
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define XOSD_DEVICE_ID			XPAR_OSD_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XOsdSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XOsd OsdInst;		/**< Instance of the OSD Device */

/*****************************************************************************/
/**
*
* Main/Entry function for self test example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*  Run selftest example */
	Status = XOsdSelfTestExample(XOSD_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("OSD driver Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran OSD driver Selftest Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the OSD driver.
*
* @param	DeviceId is an ID of OSD core or device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XOsdSelfTestExample(u16 DeviceId)
{
	int Status;
	XOsd_Config *Config;

	/* Initialize the OSD driver so that it's ready to use look up the
	 * configuration in the config table, then initialize it.
	 */
	Config = XOsd_LookupConfig(DeviceId);

	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XOsd_CfgInitialize(&OsdInst, Config, Config->BaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*  Perform a self-test to check hardware build. */
	Status = XOsd_SelfTest(&OsdInst);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
