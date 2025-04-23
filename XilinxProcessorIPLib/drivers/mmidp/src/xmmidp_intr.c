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
 * This function clears XMMIDP_GENERAL_INT0 HPD_EVENT bit
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearGeneralHpdEvent(XMmiDp *InstancePtr)
{
	u32 RegVal = 0;;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_GEN_INT0);

	RegVal |= XMMIDP_GEN_INT0_HPD_EVENT_MASK;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT0, RegVal);
}
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
				XMMIDP_GEN_INT_ENABLE0);
	RegVal |= Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT_ENABLE0, RegVal);
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
void XMmiDp_SetHpdIrqHandler(XMmiDp *InstancePtr,
			     XMmiDp_HpdIrqHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->HpdIrqHandler = CallbackFunc;
	InstancePtr->HpdIrqCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hpd hotplug
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
void XMmiDp_SetHpdHotPlugHandler(XMmiDp *InstancePtr,
				 XMmiDp_HpdHotPlugHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->HpdHotPlugHandler = CallbackFunc;
	InstancePtr->HpdHotPlugCallbackRef = CallbackRef;
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
void XMmiDp_HpdInterruptHandler(XMmiDp *InstancePtr)
{
	u32 IntrStatus;
	u32 HpdIrq;
	u32 HpdHotPlug;
	u32 HpdStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	IntrStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				    XMMIDP_HPD_STATUS0);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_HPD_STATUS0, IntrStatus);

	HpdStatus = (IntrStatus & XMMIDP_HPD_STATUS0_STATUS_MASK) >>
		    XMMIDP_HPD_STATUS0_STATUS_SHIFT;
	HpdIrq = (IntrStatus & XMMIDP_HPD_STATUS0_HPD_IRQ_MASK) >>
		 XMMIDP_HPD_STATUS0_HPD_IRQ_SHIFT;
	HpdHotPlug =  (IntrStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK) >>
		      XMMIDP_HPD_STATUS0_HOT_PLUG_SHIFT;

	if (HpdStatus & HpdIrq) {
		InstancePtr->HpdIrqHandler(InstancePtr->HpdIrqCallbackRef);
	}

	if (HpdStatus & HpdHotPlug) {
		InstancePtr->HpdHotPlugHandler(InstancePtr->HpdHotPlugCallbackRef);
	}

}
/** @} */
