/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xtmr_manager_intr.c
* @addtogroup Overview
* @{
*
* This file contains interrupt-related functions for the TMR Manager component
* (XTMR_Manager).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.10  ml   25/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xtmr_manager.h"
#include "xtmr_manager_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static void SemInterruptHandler(XTMR_Manager *InstancePtr);

/************************** Variable Definitions ****************************/

/** @cond INTERNAL_DOC */
typedef void (*Handler)(XTMR_Manager *InstancePtr);
/** @endcond */

/****************************************************************************/
/*
* Sets the handler that will be called when an event (interrupt) occurs.
* See xtmr_manager_sem.c for the canonical doxygen documentation.
*/
void XTMR_Manager_SetHandler(XTMR_Manager *InstancePtr,
				XTMR_Manager_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/****************************************************************************/
/**
* This function is the interrupt handler for the TMR Manager driver.
* It must be connected to an interrupt system by the user such that it is
* called when an interrupt for any TMR Manager occurs. This function
* does not save or restore the processor context such that the user must
* ensure this occurs.
*
* @param[in] InstancePtr contains a pointer to the instance of the core that
*		the interrupt is for.
*
******************************************************************************/
void XTMR_Manager_InterruptHandler(XTMR_Manager *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	SemInterruptHandler(InstancePtr);
}

/****************************************************************************/
/**
* This function handles the interrupt when SEM status changes.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
*
*****************************************************************************/
static void SemInterruptHandler(XTMR_Manager *InstancePtr)
{


	/*
	 * Update the stats to reflect the interrupt
	 */
	InstancePtr->Stats.InterruptCount++;
}

/*****************************************************************************/
/*
* Disables the core interrupt.
* See xtmr_manager_sem.c for the canonical doxygen documentation.
*/
void XTMR_Manager_DisableInterrupt(XTMR_Manager *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the SEM interrupt mask register to disable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, 0);
	InstancePtr->SemImr = 0;
}

/*****************************************************************************/
/*
* Enables the core interrupt for SEM status signal changes.
* See xtmr_manager_sem.c for the canonical doxygen documentation.
*/
void XTMR_Manager_EnableInterrupt(XTMR_Manager *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the SEM interrupt maskregister to enable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, Mask);
	InstancePtr->SemImr = Mask;
}

/** @} */
