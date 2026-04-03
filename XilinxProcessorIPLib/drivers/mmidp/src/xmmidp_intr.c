/*******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
	u32 RegVal = 0;

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
	u32 RegVal = 0;

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
	u32 RegVal = 0;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_HPD_INTERRUPT_ENABLE);
	RegVal |= Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_HPD_INTERRUPT_ENABLE, RegVal);
}

/******************************************************************************/
/**
 * This function disables XMMIDP_GENERAL_INTERRUPT_0 bits based on the mask
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Mask specifies the interrupt bits to disable
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GeneralInterruptDisable(XMmiDp *InstancePtr, u32 Mask)
{
	u32 RegVal;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_GEN_INT_ENABLE0);
	RegVal &= ~Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT_ENABLE0, RegVal);
}

/******************************************************************************/
/**
 * This function disables XMMIDP_HPD_INTERRUPT_ENABLE bits based on the mask
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Mask specifies the interrupt bits to disable
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_HpdInterruptDisable(XMmiDp *InstancePtr, u32 Mask)
{
	u32 RegVal;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_HPD_INTERRUPT_ENABLE);
	RegVal &= ~Mask;

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
	if (CallbackFunc == NULL) {
		xil_printf("[HPD] WARNING: HpdIrq callback is NULL, handler not set\r\n");
		return;
	}

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
	if (CallbackFunc == NULL) {
		xil_printf("[HPD] WARNING: HotPlug callback is NULL, handler not set\r\n");
		return;
	}

	InstancePtr->HpdHotPlugHandler = CallbackFunc;
	InstancePtr->HpdHotPlugCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hpd hot unplug
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
void XMmiDp_SetHpdHotUnplugHandler(XMmiDp *InstancePtr,
				   XMmiDp_HpdHotUnplugHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	if (CallbackFunc == NULL) {
		xil_printf("[HPD] WARNING: HotUnplug callback is NULL, handler not set\r\n");
		return;
	}

	InstancePtr->HpdHotUnplugHandler = CallbackFunc;
	InstancePtr->HpdHotUnplugCallbackRef = CallbackRef;
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
	u32 HpdIen;

	Xil_AssertVoid(InstancePtr != NULL);

	IntrStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				    XMMIDP_HPD_STATUS0);
	HpdIen = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_HPD_INTERRUPT_ENABLE);

	xil_printf("[HPD-ISR] HPD_STATUS0=0x%08X", IntrStatus);
	if (IntrStatus & XMMIDP_HPD_STATUS0_HPD_IRQ_MASK)
		xil_printf(" IRQ");
	if (IntrStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK)
		xil_printf(" PLUG");
	if (IntrStatus & XMMIDP_HPD_STATUS0_HOT_UNPLUG_MASK)
		xil_printf(" UNPLUG");
	if (IntrStatus & XMMIDP_HPD_STATUS0_UNPLUG_ERR_MASK)
		xil_printf(" UNPLUG_ERR");
	xil_printf(" state=%d\r\n",
		   (IntrStatus & XMMIDP_HPD_STATUS0_STATE_MASK) >>
		   XMMIDP_HPD_STATUS0_STATE_SHIFT);

	/* Write-1-to-clear the HPD status bits */
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_HPD_STATUS0, IntrStatus);

	if ((IntrStatus & XMMIDP_HPD_STATUS0_HPD_IRQ_MASK) &&
	    (InstancePtr->HpdIrqHandler != NULL))
		InstancePtr->HpdIrqHandler(InstancePtr->HpdIrqCallbackRef);

	if ((IntrStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK) &&
	    (InstancePtr->HpdHotPlugHandler != NULL))
		InstancePtr->HpdHotPlugHandler(InstancePtr->HpdHotPlugCallbackRef);

	if ((IntrStatus & XMMIDP_HPD_STATUS0_HOT_UNPLUG_MASK) &&
	    (InstancePtr->HpdHotUnplugHandler != NULL))
		InstancePtr->HpdHotUnplugHandler(InstancePtr->HpdHotUnplugCallbackRef);

	/*
	 * Re-enable HPD interrupts after handling.
	 * After hotplug:   enable IRQ + UNPLUG (detect disconnect while connected)
	 * After hotunplug: enable IRQ + PLUG + UNPLUG (detect reconnect)
	 * After IRQ only:  preserve existing enable bits
	 */
	if (IntrStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK)
		HpdIen |= XMMIDP_HPD_IRQ_EN_MASK |
			  XMMIDP_HPD_UNPLUG_EN_MASK;

	if (IntrStatus & XMMIDP_HPD_STATUS0_HOT_UNPLUG_MASK)
		HpdIen |= XMMIDP_HPD_IRQ_EN_MASK |
			  XMMIDP_HPD_PLUG_EN_MASK |
			  XMMIDP_HPD_UNPLUG_EN_MASK;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_HPD_INTERRUPT_ENABLE, HpdIen);

}

/******************************************************************************/
/**
 * This function is the top-level general interrupt handler for the XMmiDp
 * driver. It reads the GENERAL_INTERRUPT register, dispatches HPD events
 * to XMmiDp_HpdInterruptHandler, and clears all pending event bits.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_GeneralInterruptHandler(XMmiDp *InstancePtr)
{
	u32 GenIntrStatus;
	u32 GenIntrEnable;
	u32 ActiveEvents;

	Xil_AssertVoid(InstancePtr != NULL);

	GenIntrStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				       XMMIDP_GEN_INT0);
	GenIntrEnable = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				       XMMIDP_GEN_INT_ENABLE0);

	ActiveEvents = GenIntrStatus & GenIntrEnable;
	if (!ActiveEvents)
		return;

	/* Write-1-to-clear all active event bits */
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT0, ActiveEvents);

	if (ActiveEvents & XMMIDP_GEN_INT0_HPD_EVENT_MASK)
		XMmiDp_HpdInterruptHandler(InstancePtr);

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_OVRFLW_STREAM0_MASK)
		xil_printf("[ISR] Video FIFO overflow stream 0\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_UNDRFLW_STREAM0_MASK)
		xil_printf("[ISR] Video FIFO underflow stream 0\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_AUD_FIFO_OVRFLW_STREAM0_MASK)
		xil_printf("[ISR] Audio FIFO overflow stream 0\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_OVRFLW_STREAM1_MASK)
		xil_printf("[ISR] Video FIFO overflow stream 1\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_UNDRFLW_STREAM1_MASK)
		xil_printf("[ISR] Video FIFO underflow stream 1\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_AUD_FIFO_OVRFLW_STREAM1_MASK)
		xil_printf("[ISR] Audio FIFO overflow stream 1\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_OVRFLW_STREAM2_MASK)
		xil_printf("[ISR] Video FIFO overflow stream 2\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_UNDRFLW_STREAM2_MASK)
		xil_printf("[ISR] Video FIFO underflow stream 2\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_AUD_FIFO_OVRFLW_STREAM2_MASK)
		xil_printf("[ISR] Audio FIFO overflow stream 2\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_OVRFLW_STREAM3_MASK)
		xil_printf("[ISR] Video FIFO overflow stream 3\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_VID_FIFO_UNDRFLW_STREAM3_MASK)
		xil_printf("[ISR] Video FIFO underflow stream 3\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_AUD_FIFO_OVRFLW_STREAM3_MASK)
		xil_printf("[ISR] Audio FIFO overflow stream 3\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_DSC_EVENT_MASK)
		xil_printf("[ISR] DSC event\r\n");

	if (ActiveEvents & XMMIDP_GEN_INT0_PM_EVENT_MASK)
		xil_printf("[ISR] PM event\r\n");
}

/******************************************************************************/
/**
 * This function enables SDP_STATUS_ENABLE bits based on the mask
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Mask specifies the interrupt bits to enable
 *
 * @return	None
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SdpStatusInterruptEnable(XMmiDp *InstancePtr, u32 Mask)
{
	u32 RegVal;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_SDP_STATUS_EN);
	RegVal |= Mask;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_SDP_STATUS_EN, RegVal);
}
/** @} */
