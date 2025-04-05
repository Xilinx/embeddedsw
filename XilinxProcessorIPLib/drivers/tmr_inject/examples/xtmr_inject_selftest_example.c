/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
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
* 1.7   adk  04/04/25 Ported example to the SDT flow.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_inject.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_DEVICE_ID
#else
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_BASEADDR
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int TMR_InjectSelfTestExample(u16 DeviceId);
#else
int TMR_InjectSelfTestExample(UINTPTR BaseAddress);
#endif

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
		 xil_printf("TMR_InjectSelfTestExample Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TMR_InjectSelfTestExample\r\n");
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
#ifndef SDT
int TMR_InjectSelfTestExample(u16 DeviceId)
#else
int TMR_InjectSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;

	/*
	 * Initialize the TMR_Inject driver so that it is ready to use.
	 */
#ifndef SDT
	Status = XTMR_Inject_Initialize(&TMR_Inject, DeviceId);
#else
	Status = XTMR_Inject_Initialize(&TMR_Inject, BaseAddress);
#endif
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
