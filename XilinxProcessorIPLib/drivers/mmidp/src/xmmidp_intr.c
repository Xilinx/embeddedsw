/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_intr.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0	  ck   03/14/25  Initial release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <xstatus.h>
#include <sleep.h>

#include "xmmidp.h"

/******************************************************************************/
/**
 * This function enables XMMIDP_GENERAL_INTERRUPT_0 bits based on the mask
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Mask specifies the interrupt bits to enable
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GeneralInterruptEnable(XMmiDp *InstancePtr, u32 Mask)
{
	u32 RegVal = 0;;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_GEN_INT0);
	RegVal |= Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT0, RegVal);
}

/******************************************************************************/
/**
 * This function enables XMMIDP_HPD_INTERRUPT_ENABLE bits based on the mask
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Mask specifies the interrupt bits to enable
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_HpdInterruptEnable(XMmiDp *InstancePtr, u32 Mask)
{
	u32 RegVal = 0;;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_HPD_INTERRUPT_ENABLE);
	RegVal |= Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_HPD_INTERRUPT_ENABLE, RegVal);
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hpd irq event
 * interrupt occurs.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetIrqHpdHandler(XMmiDp *InstancePtr,
			     XMmiDp_IrqHpdHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IrqHpdHandler = CallbackFunc;
	InstancePtr->IrqHpdCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the HPD Event interrupt handler for the XMmiDp driver.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_HpdEventHandler(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0);

	if (Status & XMMIDP_HPD_STATUS0_HPD_IRQ_MASK) {
		InstancePtr->IrqHpdHandler(InstancePtr->IrqHpdCallbackRef);
	}

}
/** @} */
