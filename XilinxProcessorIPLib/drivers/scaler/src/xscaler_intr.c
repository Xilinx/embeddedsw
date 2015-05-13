/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/**
*
* @file xscaler_intr.c
*
* This code contains interrupt related functions of Xilinx MVI Video Scaler
* device driver. The Scaler device converts a specified rectangular area of an
* input digital video image from one original sampling grid to a desired target
* sampling grid. Please see xscaler.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------------
* 1.00a xd     02/10/09 First release.
* 2.00a xd     12/14/09 Updated Doxygen document tags.
* 7.0   adk    08/22/14 XScaler_IntrHandler and XScaler_SetCallBack APIs were
*                       modified
* </pre>
*
******************************************************************************/

#include "xscaler.h"

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Scaler driver.
*
* This handler calls callback, and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XScaler_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XScaler instance that just
*		interrupted.
* @return	None.
* @note		None.
*
******************************************************************************/
void XScaler_IntrHandler(void *InstancePtr)
{

	XScaler *XScalerPtr = (XScaler *)((void *)InstancePtr);

	/* Validate parameters */
	Xil_AssertVoid(XScalerPtr != NULL);
	Xil_AssertVoid(XScalerPtr->IsReady == XIL_COMPONENT_IS_READY);


	XScalerPtr->CallBack(XScalerPtr->CallBackRef);

	/* Clear pending interrupt(s) */
	XScaler_IntrClear(XScalerPtr);

}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function.
*
* @param	InstancePtr is a pointer to the XScaler instance to be worked
*		on.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XScaler_SetCallBack(XScaler *InstancePtr,
				void *CallBackFunc, void *CallBackRef)
{

	/* Validate arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CallBackFunc != NULL);
	Xil_AssertVoid(CallBackRef != NULL);

	/* Setting the HandlerType */
	InstancePtr->CallBack =
				(XScaler_CallBack)((void *)CallBackFunc);
	InstancePtr->CallBackRef = CallBackRef;

}
