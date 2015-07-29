/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_cipher_intr.c
*
* This file contains interrupt related functions for Xilinx HDCP core.
* Please see xhdcp1x_cipher.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdcp1x_cipher_hw.h"
#include "xhdcp1x_cipher.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define RegRead(InstancePtr, Offset)  \
	XHdcp1x_CipherReadReg(InstancePtr->CfgPtr->BaseAddress, Offset)

#define RegWrite(InstancePtr, Offset, Value)  \
	XHdcp1x_CipherWriteReg(InstancePtr->CfgPtr->BaseAddress, Offset, Value)

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                             Callback Function Type
* ------------------------------------    ------------------------------------
* (XHDCP1X_CIPHER_HANDLER_LINK_FAILURE)   LinkFailCallback
* (XHDCP1X_CIPHER_HANDLER_Ri_UPDATE)      RiUpdateCallback
* </pre>
*
* @param	InstancePtr is a pointer to the HDCP cipher core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XHdcp1x_CipherSetCallback(XHdcp1x_Cipher *InstancePtr, u32 HandlerType,
                XHdcp1x_Callback CallbackFunc, void *CallbackRef)
{
	/* Locals */
	u32 Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= XHDCP1X_CIPHER_HANDLER_LINK_FAILURE);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {

	/* Link Failure Callback */
	case (XHDCP1X_CIPHER_HANDLER_LINK_FAILURE):
		InstancePtr->LinkFailCallback = CallbackFunc;
		InstancePtr->LinkFailRef = CallbackRef;
		InstancePtr->IsLinkFailCallbackSet = (TRUE);
		break;

		/* Ri Update Callback */
	case (XHDCP1X_CIPHER_HANDLER_Ri_UPDATE):
		InstancePtr->RiUpdateCallback = CallbackFunc;
		InstancePtr->RiUpdateRef = CallbackRef;
		InstancePtr->IsRiUpdateCallbackSet = (TRUE);
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	/* Return */
	return (Status);
}

/*****************************************************************************/
/**
*
* This function enables/disables the reporting of link check state changes
*
* @param InstancePtr  the cipher core instance
* @param IsEnabled  enable/disable link state change notifications
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherSetLinkStateCheck(XHdcp1x_Cipher *InstancePtr, int IsEnabled)
{
	int Status = XST_SUCCESS;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check DP receive */
	if (XHdcp1x_CipherIsDP(InstancePtr) &&
			XHdcp1x_CipherIsRX(InstancePtr)) {

		u32 Val = 0;

		/* Clear any pending link state failure interrupt */
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_STATUS,
		                XHDCP1X_CIPHER_BITMASK_INTERRUPT_LINK_FAIL);

		/* Update it */
		Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK);
		if (IsEnabled) {
			Val &= ~XHDCP1X_CIPHER_BITMASK_INTERRUPT_LINK_FAIL;
		}
		else {
			Val |= XHDCP1X_CIPHER_BITMASK_INTERRUPT_LINK_FAIL;
		}
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK, Val);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function enables/disables the reporting of Ri update notifications
*
* @param InstancePtr  the cipher core instance
* @param IsEnabled  enable/disable Ri update notifications
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherSetRiUpdate(XHdcp1x_Cipher *InstancePtr, int IsEnabled)
{
	int Status = XST_SUCCESS;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check HDMI receive */
	if (XHdcp1x_CipherIsHDMI(InstancePtr)) {

		u32 Val = 0;

		/* Clear any pending link state failure interrupt */
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_STATUS,
		                XHDCP1X_CIPHER_BITMASK_INTERRUPT_Ri_UPDATE);

		/* Update theDevice */
		Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK);
		if (IsEnabled) {
			Val &= ~XHDCP1X_CIPHER_BITMASK_INTERRUPT_Ri_UPDATE;
		}
		else {
			Val |= XHDCP1X_CIPHER_BITMASK_INTERRUPT_Ri_UPDATE;
		}
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK, Val);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the cipher core driver
*
* @param InstancePtr  the cipher core instance
*
* @return
*   void
*
* @note
*   none
*
******************************************************************************/
void XHdcp1x_CipherHandleInterrupt(void *InstancePtr)
{
	XHdcp1x_Cipher *HdcpCipherPtr = (XHdcp1x_Cipher *) InstancePtr;
	u32 Pending = 0;

	/* Verify arguments */
	Xil_AssertVoid(HdcpCipherPtr != NULL);
	Xil_AssertVoid(HdcpCipherPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Determine Pending */
	Pending = RegRead(HdcpCipherPtr, XHDCP1X_CIPHER_REG_INTERRUPT_STATUS);
	Pending &= ~RegRead(HdcpCipherPtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK);

	/* Check for pending */
	if (Pending != 0) {

		/* Clear Pending */
		RegWrite(HdcpCipherPtr, XHDCP1X_CIPHER_REG_INTERRUPT_STATUS,
				Pending);

		/* Update statistics */
		HdcpCipherPtr->Stats.IntCount++;

		/* Check for link integrity failure */
		if (Pending & XHDCP1X_CIPHER_BITMASK_INTERRUPT_LINK_FAIL) {

			/* Invoke callback if set */
			if (HdcpCipherPtr->IsLinkFailCallbackSet)
				(*HdcpCipherPtr->LinkFailCallback)(
						HdcpCipherPtr->LinkFailRef);
		}

		/* Check for change to Ri register */
		if (Pending & XHDCP1X_CIPHER_BITMASK_INTERRUPT_Ri_UPDATE) {

			/* Invoke callback if set */
			if (HdcpCipherPtr->IsRiUpdateCallbackSet)
				(*HdcpCipherPtr->RiUpdateCallback)(
						HdcpCipherPtr->RiUpdateRef);
		}
	}

	return;
}
