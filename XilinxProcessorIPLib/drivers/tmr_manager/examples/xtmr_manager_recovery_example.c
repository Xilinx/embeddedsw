/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_recovery_example.c
*
* This file contains the example using tmr_manager and tmr_inject drivers
* to perform an error injection and recovery.
*
* H/W Requirements:
* In order to test this example at the hardware design level both TMR
* Manager and TMR Inject IP needs to be present, and TMR Manager Recover
* signal needs to be connected to Microblaze processor Suspend port.
*
* S/W Requirements:
* To inject the error using the tmr inject IP, As per hardware
* implementation, the error function which injects the error
* should be executed from the lmb memory. Inorder to run this
* examples text and data sections in linker should point to
* LMB memory.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.3	adk  02/23/22 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xtmr_manager.h"
#include "xtmr_inject.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMR_MANAGER_DEVICE_ID		XPAR_TMR_MANAGER_0_DEVICE_ID
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_DEVICE_ID


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define ERROR_INJECT_CPU_ID	1

/************************** Function Prototypes ******************************/
int TMR_ManagerRecoveryExample(u16 TMRManagerDeviceId, u16 TMRInjectDeviceId);
static void TMR_Manager_BreakHandler(void *CallBackRef);

/************************** Variable Definitions *****************************/
XTMR_Manager TMR_Manager;	/* Instance of the TMR_Manager device */
XTMR_Inject TMR_Inject;	/* Instance of the TMR_Inject device */
volatile int BreakCount = 0;

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

	xil_printf("\r\n--- Entering main() --- \r\n");
	/*
	 * Run the TMR_Manager self test example, specify the Device ID that
	 * is generated in xparameters.h
	 */
	Status = TMR_ManagerRecoveryExample(TMR_MANAGER_DEVICE_ID,
					    TMR_INJECT_DEVICE_ID);
	xil_printf("\n\rTMR Manager Recovery Example Test %s\r\n",
		   (Status == XST_SUCCESS) ? "passed":"failed");

	xil_printf("\n\r--- Exiting main() --- \r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This static function updates break vector event count.
*
* @param	CallBackRef is the callback reference passed from the break
* 		vector handler, which in our case is a pointer to the driver
* 		instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TMR_Manager_BreakHandler(void *CallBackRef)
{
	BreakCount++;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
*
* @param	TMRManagerDeviceId is the XPAR_<tmr_manager_instance>_DEVICE_ID
* 		value from xparameters.h.
* @param	TMRInjectDeviceId is the XPAR_<tmr_inject_instance>_DEVICE_ID
* 		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int TMR_ManagerRecoveryExample(u16 TMRManagerDeviceId, u16 TMRInjectDeviceId)
{
	int Status;
	u32 Error_Inject;

	/*
	 * Initialize the TMR_Inject driver so that it is ready to use.
	 */
	Status = XTMR_Inject_Initialize(&TMR_Inject, TMRInjectDeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the TMR_Manager driver so that it is ready to use.
	 */
	Status = XTMR_Manager_Initialize(&TMR_Manager, TMRManagerDeviceId);
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

	/*
	 * Configure Break Delay Initialization Register to zero so that
	 * break occurs immediately
	 */
	XTMR_Manager_Configure_BrkDelay(&TMR_Manager, 0);

	/* Configure pre reset handler */
	XTMR_Manager_SetPreResetHandler(&TMR_Manager, (void *)TMR_Manager_BreakHandler,
					&TMR_Manager);

	/* Inject the fault */
	XTMR_Inject_Enable(&TMR_Inject, ERROR_INJECT_CPU_ID);
	XTMR_Inject_InjectBit(&TMR_Inject, Error_Inject, 2);

	/* Wait for break vector */
	while (BreakCount == 0);

	if (TMR_Manager.Stats.RecoveryCount == BreakCount)
		return XST_SUCCESS;
	else
		return XST_FAILURE;
}
