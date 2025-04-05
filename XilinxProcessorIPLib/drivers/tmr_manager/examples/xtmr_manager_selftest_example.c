/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_selftest_example.c
*
* This file contains a design example using the TMR_Manager driver
* (XTMR_Manager) and hardware device.
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
#include "xtmr_manager.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMR_MANAGER_DEVICE_ID		XPAR_TMR_MANAGER_0_DEVICE_ID
#else
#define TMR_MANAGER_DEVICE_ID		XPAR_TMR_MANAGER_0_BASEADDR
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int TMR_ManagerSelfTestExample(u16 DeviceId);
#else
int TMR_ManagerSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XTMR_Manager TMR_Manager;	/* Instance of the TMR_Manager device */

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
	 * Run the TMR_Manager self test example, specify the Device ID that
	 * is generated in xparameters.h
	 */
	Status = TMR_ManagerSelfTestExample(TMR_MANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("TMR_Manager SelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TMR_Manager SelfTestExample\r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
*
* @param	DeviceId is the XPAR_<tmr_manager_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int TMR_ManagerSelfTestExample(u16 DeviceId)
#else
int TMR_ManagerSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;

	/*
	 * Initialize the TMR_Manager driver so that it is ready to use.
	 */
#ifndef SDT
	Status = XTMR_Manager_Initialize(&TMR_Manager, DeviceId);
#else
	Status = XTMR_Manager_Initialize(&TMR_Manager, BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMR_Manager_SelfTest(&TMR_Manager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
