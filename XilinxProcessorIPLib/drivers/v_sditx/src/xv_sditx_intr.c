/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

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
	u32 Data;
	XV_SdiTx *SdiTxPtr = (XV_SdiTx *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(SdiTxPtr != NULL);
	Xil_AssertVoid(SdiTxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Video Lock */
	Data = XV_SdiTx_ReadReg(SdiTxPtr->Config.BaseAddress,
				(XV_SDITX_INT_STS_OFFSET));

	/* Check for IRQ flag set */
	if (Data & XV_SDITX_INT_GT_RST_DONE_MASK) {
		/* Clear event flag */
		XV_SdiTx_WriteReg(SdiTxPtr->Config.BaseAddress,
					(XV_SDITX_INT_CLR_OFFSET),
					(XV_SDITX_INT_GT_RST_DONE_MASK));
		XV_SdiTx_WriteReg(SdiTxPtr->Config.BaseAddress,
					(XV_SDITX_INT_CLR_OFFSET), 0x0);

		/* Jump to GT reset done interrupt handler */
		if (XV_SdiTx_ReadReg(SdiTxPtr->Config.BaseAddress,
					XV_SDITX_STS_SB_TX_TDATA_OFFSET) &
					XV_SDITX_STS_SB_TX_TDATA_GT_TX_RESETDONE_MASK) {
			SdiTx_GtTxRstDoneIntrHandler(SdiTxPtr);
		}
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
* (XV_SDITX_HANDLER_GTRESET_DONE)    GtRstDoneCallback
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
		InstancePtr->IsGtRstDoneCallbackSet = (TRUE);
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
* @param	Interrupt to be enabled.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiTx_IntrEnable(XV_SdiTx *InstancePtr, u32 Interrupt)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_INT_MSK_OFFSET));
	Data &= ~Interrupt;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_INT_MSK_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the selected interrupt.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
* @param	Interrupt to be disabled.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiTx_IntrDisable(XV_SdiTx *InstancePtr, u32 Interrupt)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_INT_MSK_OFFSET));
	Data |= Interrupt;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_INT_MSK_OFFSET), (Data));
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
	if (InstancePtr->IsGtRstDoneCallbackSet) {
		InstancePtr->GtRstDoneCallback(InstancePtr->GtRstDoneRef);
	}
}
