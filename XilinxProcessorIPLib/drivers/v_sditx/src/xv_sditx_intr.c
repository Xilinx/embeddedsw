/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditx_intr.c
*
* This file contains interrupt related functions for Xilinx SDI TX core.
* Please see xv_sditx.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   jsr    07/17/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sditx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void SdiTx_GtTxRstDoneIntrHandler(XV_SdiTx *InstancePtr);
static void SdiTx_OverFlowIntrHandler(XV_SdiTx *InstancePtr);
static void SdiTx_UnderFlowIntrHandler(XV_SdiTx *InstancePtr);
static void SdiTx_CeAlignErrIntrHandler(XV_SdiTx *InstancePtr);
static void SdiTx_Axi4sVidLockIntrHandler(XV_SdiTx *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the SDI Tx core
*
* @param	InstancePtr is the XV_SdiTx instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XV_SdiTx_GetIntrEnable(XV_SdiTx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDITX_IER_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts pending in the
* Interrupt Status Register of the SDI Tx core
*
* @param	InstancePtr is the XV_SdiTx instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XV_SdiTx_GetIntrStatus(XV_SdiTx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDITX_ISR_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the SDI Tx core
*
* @param	InstancePtr is the XV_SdiTx instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
*		to be cleared in the Interrupt Status register
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XV_SdiTx_InterruptClear(XV_SdiTx *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDITX_ISR_ALLINTR_MASK))) == 0);

	Mask &= XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDITX_ISR_OFFSET);

	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDITX_ISR_OFFSET,
				Mask & XV_SDITX_ISR_ALLINTR_MASK);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI TX driver.
*
* This handler reads the pending interrupt for GT reset done interrupt from
* Tx IP, determines the source of the interrupts, clears the
* interrupts and calls callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XV_SdiTx_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XV_SdiTx instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_IntrHandler(void *InstancePtr)
{
	u32 ActiveIntr;
	u32 Mask;

	XV_SdiTx *SdiTxPtr = (XV_SdiTx *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(SdiTxPtr != NULL);
	Xil_AssertVoid(SdiTxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts */
	ActiveIntr = XV_SdiTx_GetIntrStatus(SdiTxPtr);

	/* Read ISR register  for Gtresetdone*/
	Mask = ActiveIntr & XV_SDITX_ISR_GTTX_RSTDONE_MASK;

	/* Check for GTTX_RSTDONE IRQ flag set */
	if (Mask) {

		/* Jump to GT reset done interrupt handler */
		if (XV_SdiTx_ReadReg(SdiTxPtr->Config.BaseAddress,
					XV_SDITX_SB_TX_STS_TDATA_OFFSET) &
					XV_SDITX_SB_TX_STS_TDATA_GT_TX_RESETDONE_MASK) {
			SdiTx_GtTxRstDoneIntrHandler(SdiTxPtr);
		}

		/* Clear handled interrupt(s) */
		XV_SdiTx_InterruptClear(SdiTxPtr, Mask);
	}

	/* Read ISR register  for OVERRFLOW*/
	Mask = ActiveIntr & XV_SDITX_ISR_OVERFLOW_MASK;

	/* Check for OVERFLOW IRQ flag set */
	if (Mask) {

		/* Jump to OVERFLOW interrupt handler */
		SdiTx_OverFlowIntrHandler(SdiTxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiTx_InterruptClear(SdiTxPtr, Mask);
	}

	/* Read ISR register  for UNDERFLOW*/
	Mask = ActiveIntr & XV_SDITX_ISR_UNDERFLOW_MASK;

	/* Check for UNDERFLOW IRQ flag set */
	if (Mask) {

		/* Jump to UNDERFLOW interrupt handler */
		SdiTx_UnderFlowIntrHandler(SdiTxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiTx_InterruptClear(SdiTxPtr, Mask);
	}

	/* Read ISR register  for CE_ALIGN_ERROR*/
	Mask = ActiveIntr & XV_SDITX_ISR_TX_CE_ALIGN_ERR_MASK;

	/* Check for CE_ALIGN_ERROR IRQ flag set */
	if (Mask) {

		/* Jump to CE_ALIGN_ERROR interrupt handler */
		SdiTx_CeAlignErrIntrHandler(SdiTxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiTx_InterruptClear(SdiTxPtr, Mask);
	}

	/* Read ISR register  for Axi4S Video lock*/
	Mask = ActiveIntr & XV_SDITX_ISR_AXI4S_VID_LOCK_MASK;

	/* Check for Axi4s Video lock IRQ flag set */
	if (Mask) {

		/* Jump to Axi4s video lock interrupt handler */
		SdiTx_Axi4sVidLockIntrHandler(SdiTxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiTx_InterruptClear(SdiTxPtr, Mask);
	}
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                       Callback Function Type
* -------------------------         -------------------------------------------
* (XV_SDITX_HANDLER_GTRESET_DONE)	GtRstDoneCallback
* (XV_SDITX_HANDLER_OVERFLOW)		OverFlowCallback
* (XV_SDITX_HANDLER_UNDERFLOW)		UnderFlowCallback
* </pre>
*
* @param    InstancePtr is a pointer to the SDI TX core instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_SdiTx_SetCallback(XV_SdiTx *InstancePtr, u32 HandlerType,
void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_SDITX_HANDLER_GTRESET_DONE));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
	/* Stream down */
	case (XV_SDITX_HANDLER_GTRESET_DONE):
		InstancePtr->GtRstDoneCallback = (XV_SdiTx_Callback)CallbackFunc;
		InstancePtr->GtRstDoneRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Overflow */
	case (XV_SDITX_HANDLER_OVERFLOW):
		InstancePtr->OverFlowCallback = (XV_SdiTx_Callback)CallbackFunc;
		InstancePtr->OverFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* UnderFlow */
	case (XV_SDITX_HANDLER_UNDERFLOW):
		InstancePtr->UnderFlowCallback = (XV_SdiTx_Callback)CallbackFunc;
		InstancePtr->UnderFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* CE align errors */
	case (XV_SDITX_HANDLER_CEALIGN):
		InstancePtr->CeAlignErrCallback = (XV_SdiTx_Callback)CallbackFunc;
		InstancePtr->CeAlignErrRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Axi4s video lock  */
	case (XV_SDITX_HANDLER_AXI4SVIDLOCK):
		InstancePtr->Axi4sVidLockCallback = (XV_SdiTx_Callback)CallbackFunc;
		InstancePtr->Axi4sVidLockRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;
	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function enables the selected interrupt.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
* @param	Mask to be enabled.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiTx_IntrEnable(XV_SdiTx *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDITX_IER_ALLINTR_MASK))) == 0);

	Mask |= XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDITX_IER_OFFSET);

	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDITX_IER_OFFSET,
			Mask & XV_SDITX_IER_ALLINTR_MASK);
}

/*****************************************************************************/
/**
*
* This function disables the selected interrupt.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
* @param	Mask to be disabled.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiTx_IntrDisable(XV_SdiTx *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDITX_IER_ALLINTR_MASK))) == 0);

	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDITX_IER_OFFSET,
			~Mask & XV_SDITX_IER_ALLINTR_MASK);

}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Video Lock Event.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void SdiTx_GtTxRstDoneIntrHandler(XV_SdiTx *InstancePtr)
{
	/* Call stream up callback */
	if (InstancePtr->GtRstDoneCallback) {
		InstancePtr->GtRstDoneCallback(InstancePtr->GtRstDoneRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Tx over flow Event.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void SdiTx_OverFlowIntrHandler(XV_SdiTx *InstancePtr)
{
	/* Call OverFlow callback */
	if (InstancePtr->OverFlowCallback) {
		InstancePtr->OverFlowCallback(InstancePtr->OverFlowRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Tx under flow Event.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void SdiTx_UnderFlowIntrHandler(XV_SdiTx *InstancePtr)
{
	/* Call UnderFlow callback */
	if (InstancePtr->UnderFlowCallback) {
		InstancePtr->UnderFlowCallback(InstancePtr->UnderFlowRef);
	}
}


/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Tx CE Align Error Event.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void SdiTx_CeAlignErrIntrHandler(XV_SdiTx *InstancePtr)
{
	/* Call CE Align Error callback */
	if (InstancePtr->CeAlignErrCallback) {
		InstancePtr->CeAlignErrCallback(InstancePtr->CeAlignErrRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Axi4s bridge lock Event.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void SdiTx_Axi4sVidLockIntrHandler(XV_SdiTx *InstancePtr)
{
	/* Call Axi4s Video lock callback */
	if (InstancePtr->Axi4sVidLockCallback) {
		InstancePtr->Axi4sVidLockCallback(InstancePtr->Axi4sVidLockRef);
	}
}
