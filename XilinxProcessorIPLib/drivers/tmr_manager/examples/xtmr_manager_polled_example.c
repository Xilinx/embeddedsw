/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xtmr_manager_polled_example.c
*
* This file contains a design example using the TMR_Manager driver (XTMR_Manager)
* and hardware device using the polled mode.
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
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
#include "xstatus.h"
#include "xtmr_manager.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMR_MANAGER_DEVICE_ID	XPAR_TMR_MANAGER_0_DEVICE_ID
#else
#define TMR_MANAGER_DEVICE_ID	XPAR_TMR_MANAGER_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int TMR_ManagerPolledExample(u16 DeviceId);
#else
int TMR_ManagerPolledExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XTMR_Manager TMR_Manager;	/* Instance of the TMR_Manager Device */


/*****************************************************************************/
/**
*
* Main function to call the TMR Manager polled example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMR_Manager polled example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = TMR_ManagerPolledExample(TMR_MANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("TMR_Manager PolledExample Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TMR_Manager Polled Example\r\n");
	return XST_SUCCESS;

}


/****************************************************************************/
/**
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
* This function polls the TMR_Manager and does not require the use of
* interrupts.
*
* @param	DeviceId is the Device ID of the TMR_Manager and is the
*		XPAR_<tmr_manager_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
*
* @note		None
*
****************************************************************************/
#ifndef SDT
int TMR_ManagerPolledExample(u16 DeviceId)
#else
int TMR_ManagerPolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	unsigned int SentCount;
	unsigned int ReceivedCount = 0;
	int Index;

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
