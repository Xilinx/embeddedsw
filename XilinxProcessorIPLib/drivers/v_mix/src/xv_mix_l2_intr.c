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
* @file xv_mix_l2_intr.c
* @addtogroup v_mix_v3_0
* @{
*
* The functions in this file provides interrupt handler and associated
* functions that are consumed by layer-2
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   12/14/15   Initial Release
*             02/12/16   Move user call back before frame start trigger
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_mix_l2.h"


/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for Mixer interrupt
*
* @param    InstancePtr is a pointer to the mixer core instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*           callback function when it is invoked.
*
* @return
*         XST_SUCCESS if callback function installed successfully.
*
* @note   Invoking this function for a handler that already has been
*         installed replaces it with the new handler.
*
******************************************************************************/
int XVMix_SetCallback(XV_Mix_l2 *InstancePtr, void *CallbackFunc, void *CallbackRef)
{
  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(CallbackFunc != NULL);
  Xil_AssertNonvoid(CallbackRef != NULL);

  InstancePtr->FrameDoneCallback = (XVMix_Callback)CallbackFunc;
  InstancePtr->CallbackRef = CallbackRef;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the mixer core driver.
*
* This handler clears the pending interrupt and determined if the source is
* frame done signal. If yes, starts the next frame processing and calls the
* registered callback function
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XVMix_SetCallback() during initialization phase.
*
* @param	InstancePtr is a pointer to the core instance that just
*		    interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVMix_InterruptHandler(void *InstancePtr)
{
  XV_Mix_l2 *MixPtr = (XV_Mix_l2 *)InstancePtr;
  u32 Status;

  /* Verify arguments */
  Xil_AssertVoid(MixPtr != NULL);
  Xil_AssertVoid(MixPtr->Mix.IsReady == XIL_COMPONENT_IS_READY);

  /* Get the interrupt source */
  Status = XV_mix_InterruptGetStatus(&MixPtr->Mix);

  /* Clear the interrupt */
  XV_mix_InterruptClear(&MixPtr->Mix, XVMIX_IRQ_DONE_MASK);

  /* Check for Done Signal */
  if(Status & XVMIX_IRQ_DONE_MASK) {
    //Call user registered callback function, if any
    if(MixPtr->FrameDoneCallback) {
	      MixPtr->FrameDoneCallback(MixPtr->CallbackRef);
    }
    XV_mix_Start(&MixPtr->Mix);
  }
}
/** @} */
