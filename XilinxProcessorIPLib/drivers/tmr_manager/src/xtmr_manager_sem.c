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


/************************** Variable Definitions ****************************/

/** @cond INTERNAL_DOC */
typedef void (*Handler)(XTMR_Manager *InstancePtr);
/** @endcond */


/****************************************************************************/
/**
* @brief Sets the handler that will be called when an interrupt event occurs.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
* @param[in] FuncPtr is the pointer to the callback function.
* @param[in] CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
*****************************************************************************/
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
* @brief Disables the core interrupt.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
*
*****************************************************************************/
void XTMR_Manager_DisableInterrupt(XTMR_Manager *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to disable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, 0);
}

/****************************************************************************/
/**
* @brief Enables the core interrupt such that an interrupt will occur
* when any of the unmasked SEM status signals change value.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
* @param[in] mask is a bitmask indicating which SEM signals trigger an interrupt.
*
*****************************************************************************/
void XTMR_Manager_EnableInterrupt(XTMR_Manager *InstancePtr, u32 mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to enable the interrupts.
	 */
	XTMR_Manager_WriteReg(InstancePtr->RegBaseAddress,
				XTM_SEMIMR_OFFSET, mask);
}

/** @} */
