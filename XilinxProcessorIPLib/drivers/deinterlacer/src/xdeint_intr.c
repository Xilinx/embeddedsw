/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xdeint_intr.c
*
* This code contains interrupt related functions of Xilinx Video
* Deinterlacer (DEINT) device driver. Please see xdeint.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who     Date      Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh 07/10/11 First release
* 2.00a rjh 18/01/12 Updated for v_deinterlacer 2.00
* </pre>
*
******************************************************************************/

#include "xdeint.h"
#include "xil_assert.h"

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Deinterlacer
*
* This handler reads the pending interrupt from the IER/ISR, determines the
* source of the interrupts, calls according callback, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to    handle interrupts and installing the callbacks using
* XDeint_SetCallBack() during initialization phase. An example delivered with
* this driver demonstrates how this could be done.
*
* @param   InstancePtr is a pointer to the XDeint instance that just
*       interrupted.
* @return  None.
* @note       The Error interrupt callback invoked in case an error interrupt or
*       spurious interrupt happens should reset the DEINT device that just
*       interrupted.
*
******************************************************************************/
void XDeint_IntrHandler(void *InstancePtr)
{
    XDeint *XDeintPtr;
    u32 PendingIntr;
    u32 ErrorStatus;

    /* Validate parameters */
    XDeintPtr = (XDeint *) InstancePtr;
    Xil_AssertVoid(XDeintPtr != NULL);
    Xil_AssertVoid(XDeintPtr->IsReady == XIL_COMPONENT_IS_READY);

    /* Get pending interrupts */
    PendingIntr = XDeint_IntrGetPending(XDeintPtr);

    /* Clear pending interrupt(s) */
    XDeint_IntrClear(XDeintPtr, PendingIntr);

    /* A known interrupt has happened */
    if (PendingIntr)
        XDeintPtr->IntCallBack(PendingIntr);

    return;
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function.
*
* @param    InstancePtr is a pointer to the XDeint instance to be worked on.
* @param    CallbackFunc is the address of the callback function.
*
* @return
*  - XST_SUCCESS when handler is installed.
*  - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note
* Invoking this function for a handler that already has been installed replaces
* it with the new handler.
*
******************************************************************************/
int XDeint_SetCallBack(XDeint *InstancePtr,void *CallBackFunc)
{
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(CallBackFunc != NULL);

    InstancePtr->IntCallBack = (XDeint_CallBack) CallBackFunc;

    return XST_SUCCESS;
}
