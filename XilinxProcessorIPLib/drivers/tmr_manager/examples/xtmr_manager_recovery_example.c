/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
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
* 1.7   adk  04/04/25 Ported example to the SDT flow.
* 1.10  ml   09/24/26 Fix initialize TMR Manager before first
*                     xil_printf to prevent reset loop; initialize
*                     Error_Inject to 0 (was uninitialized).
* 1.10  ml   25/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
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
#ifndef SDT
#define TMR_MANAGER_DEVICE_ID		XPAR_TMR_MANAGER_0_DEVICE_ID	/**< TMR Manager device ID. */
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_DEVICE_ID	/**< TMR Inject device ID. */
#else
#define TMR_MANAGER_DEVICE_ID		XPAR_TMR_MANAGER_0_BASEADDR	/**< TMR Manager base address. */
#define TMR_INJECT_DEVICE_ID		XPAR_TMR_INJECT_0_BASEADDR	/**< TMR Inject device ID. */
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define ERROR_INJECT_CPU_ID	1	/**< CPU ID used for error injection. */
/************************** Function Prototypes ******************************/
#ifndef SDT
int TMR_ManagerRecoveryExample(u16 TMRManagerDeviceId, u16 TMRInjectDeviceId);
#else
int TMR_ManagerRecoveryExample(UINTPTR TMRManagerBaseAddress, UINTPTR TMRInjectBaseAddress);
#endif
static void TMR_Manager_BreakHandler(void *CallBackRef);

/************************** Variable Definitions *****************************/
XTMR_Manager TMR_Manager;	/**< Instance of the TMR_Manager device */
XTMR_Inject TMR_Inject;	/**< Instance of the TMR_Inject device */
volatile int BreakCount = 0;

/*****************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Initialize TMR Manager FIRST before anything else so that the
	 * break/recovery vector is installed before any TMR fault can fire.
	 * TMR_ManagerRecoveryExample() skips re-initialization to avoid
	 * overwriting the saved cold-reset vector with the TMR handler.
	 */
	Status = XTMR_Manager_Initialize(&TMR_Manager, TMR_MANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XTMR_Manager_SetPreResetHandler(&TMR_Manager,
					(void *)TMR_Manager_BreakHandler,
					&TMR_Manager);

	xil_printf("\r\n--- Entering main() --- \r\n");
	/*
	 * Run the TMR_Manager self test example, specify the Device ID that
	 * is generated in xparameters.h
	 */
	Status = TMR_ManagerRecoveryExample(TMR_MANAGER_DEVICE_ID,
					    TMR_INJECT_DEVICE_ID);
	xil_printf("\n\rTMR Manager Recovery Example Test %s\r\n",
		   (Status == XST_SUCCESS) ? "passed" : "failed");

	xil_printf("\n\r--- Exiting main() --- \r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* This static function updates break vector event count.
*
* @param[in] CallBackRef is the callback reference passed from the break
* 		vector handler, which in our case is a pointer to the driver
* 		instance.
*
******************************************************************************/
static void TMR_Manager_BreakHandler(void *CallBackRef)
{
	(void)CallBackRef;
	BreakCount++;
}

/*****************************************************************************/
/**
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
* @param[in] TMRManagerBaseAddress is the base address of the TMR_Manager device.
* @param[in] TMRInjectBaseAddress is the base address of the TMR_Inject device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
****************************************************************************/
#ifndef SDT
int TMR_ManagerRecoveryExample(u16 TMRManagerDeviceId, u16 TMRInjectDeviceId)
#else
int TMR_ManagerRecoveryExample(UINTPTR TMRManagerBaseAddress, UINTPTR TMRInjectBaseAddress)
#endif
{
	int Status;
	u32 Error_Inject = 0; /**< Fix: initialize to 0 (was uninitialized) */
#ifndef TESTAPP_GEN
	(void)TMRManagerBaseAddress;
#endif

	/*
	 * Initialize the TMR_Inject driver so that it is ready to use.
	 * Note: injectmask() inside this driver must run from LMB memory
	 * (see lscript.ld) for TMR Inject IP to intercept the fetch.
	 */
#ifndef SDT
	Status = XTMR_Inject_Initialize(&TMR_Inject, TMRInjectDeviceId);
#else
	Status = XTMR_Inject_Initialize(&TMR_Inject, TMRInjectBaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * In TESTAPP_GEN mode main() is not generated, so initialize here.
	 * In the normal flow main() already called XTMR_Manager_Initialize
	 * (to install the break vector before any TMR fault can fire); calling
	 * it a second time would overwrite the saved cold-reset vector with the
	 * TMR handler itself, causing a reset loop on cold reset.
	 */
#ifdef TESTAPP_GEN
#ifndef SDT
	Status = XTMR_Manager_Initialize(&TMR_Manager, TMRManagerDeviceId);
#else
	Status = XTMR_Manager_Initialize(&TMR_Manager, TMRManagerBaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMR_Manager_SelfTest(&TMR_Manager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Configure Break Delay Initialization Register to zero so that
	 * break occurs immediately after fault detection.
	 */
	XTMR_Manager_Configure_BrkDelay(&TMR_Manager, 0);

	/**< Configure pre reset handler (re-set after re-init above) */
	XTMR_Manager_SetPreResetHandler(&TMR_Manager, (void *)TMR_Manager_BreakHandler,
					&TMR_Manager);

	/**< Inject the fault — Error_Inject=0 injects bit 2 of address 0x0 */
	XTMR_Inject_Enable(&TMR_Inject, ERROR_INJECT_CPU_ID);
	XTMR_Inject_InjectBit(&TMR_Inject, Error_Inject, 2);

	/**< Wait for break vector */
	while (BreakCount == 0);

	if (TMR_Manager.Stats.RecoveryCount == (u32)BreakCount) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}
