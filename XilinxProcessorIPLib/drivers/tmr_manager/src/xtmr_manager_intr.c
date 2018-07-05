/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_intr.c
* @addtogroup tmr_manager_v1_2
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

typedef void (*Handler)(XTMR_Manager *InstancePtr);

/****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver. The purpose of the handler is to allow application
* specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return	None.
*
* @note		There is no assert on the CallBackRef since the driver doesn't
*		know what it is (nor should it)
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
*
* This function is the interrupt handler for the TMR Manager driver.
* It must be connected to an interrupt system by the user such that it is
* called when an interrupt for any TMR Manager occurs. This function
* does not save or restore the processor context such that the user must
* ensure this occurs.
*
* @param	InstancePtr contains a pointer to the instance of the core that
*		the interrupt is for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTMR_Manager_InterruptHandler(XTMR_Manager *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	SemInterruptHandler(InstancePtr);
}

/****************************************************************************/
/**
*
* This function handles the interrupt when SEM status changes.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		None.
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
/**
*
* This function disables the core interrupt. After calling this function,
* data may still be received by the core but no interrupt will be generated
* since the hardware device has no way to disable the receiver.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
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
/**
*
* This function enables the core interrupt such that an interrupt will occur
* when any of the SEM status signals indicated by the mask are changed.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	Mask is a mask indicating bits that should give an interrupt.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
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
