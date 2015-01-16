/*******************************************************************************
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdprx_intr.c
 *
 * This file contains functions related to XDprx interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdprx.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function installs a callback function for when a video mode change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVmChangeHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVmChangeHandler = CallbackFunc;
	InstancePtr->IntrVmChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the power state interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrPowerStateHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrPowerStateHandler = CallbackFunc;
	InstancePtr->IntrPowerStateCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a no video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrNoVideoHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrNoVideoHandler = CallbackFunc;
	InstancePtr->IntrNoVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a vertical blanking
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVBlankHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVBlankHandler = CallbackFunc;
	InstancePtr->IntrVBlankCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training lost interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTrainingLostHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTrainingLostHandler = CallbackFunc;
	InstancePtr->IntrTrainingLostCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a valid video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVideoHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVideoHandler = CallbackFunc;
	InstancePtr->IntrVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training done interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTrainingDoneHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTrainingDoneHandler = CallbackFunc;
	InstancePtr->IntrTrainingDoneCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a bandwidth change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrBwChangeHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrBwChangeHandler = CallbackFunc;
	InstancePtr->IntrBwChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 1
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp1Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp1Handler = CallbackFunc;
	InstancePtr->IntrTp1CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 2
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp2Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp2Handler = CallbackFunc;
	InstancePtr->IntrTp2CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 3
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp3Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp3Handler = CallbackFunc;
	InstancePtr->IntrTp3CallbackRef = CallbackRef;
}
