/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject_selftest_example.c
*
* This file contains a design example using the TMR_Inject driver (XTMR_Inject)
* and hardware device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_inject.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_DEVICE_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMR_InjectSelfTestExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XTMR_Inject TMR_Inject;	/* Instance of the TMR_Inject device */

/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the TMR_Inject self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = TMR_InjectSelfTestExample(TMR_INJECT_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the TMR_Inject device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Inject component.
*
*
* @param	DeviceId is the XPAR_<tmr_inject_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int TMR_InjectSelfTestExample(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the TMR_Inject driver so that it is ready to use.
	 */
	Status = XTMR_Inject_Initialize(&TMR_Inject, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMR_Inject_SelfTest(&TMR_Inject);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
