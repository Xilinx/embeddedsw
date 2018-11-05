/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xv_frmbufwr_l2_intr.c
* @addtogroup v_frmbuf_wr_v2_0
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
* 1.00  vyc   04/05/17   Initial Release
* 3.00  vyc   04/04/18   Add interrupt handler for ap_ready
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_frmbufwr_l2.h"


/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                     Callback Function Type
* -----------------------         --------------------------------------------------
* (XVFRMBUFWR_HANDLER_DONE)       DoneCallback
* (XVFRMBUFWR_HANDLER_READY)      ReadyCallback
*
* @param    InstancePtr is a pointer to the Frame Buffer Write core instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*           callback function when it is invoked.
*
* @return
*           - XST_SUCCESS if callback function installed successfully.
*           - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*           installed replaces it with the new handler.
*
******************************************************************************/
int XVFrmbufWr_SetCallback(XV_FrmbufWr_l2 *InstancePtr, u32 HandlerType,
                           void *CallbackFunc, void *CallbackRef)
{
  u32 Status;

  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(HandlerType >= (XVFRMBUFWR_HANDLER_DONE));
  Xil_AssertNonvoid(CallbackFunc != NULL);
  Xil_AssertNonvoid(CallbackRef != NULL);

  /* Check for handler type */
  switch (HandlerType) {
    case (XVFRMBUFWR_HANDLER_DONE):
      InstancePtr->FrameDoneCallback = (XVFrmbufWr_Callback)CallbackFunc;
      InstancePtr->CallbackDoneRef = CallbackRef;
      Status = (XST_SUCCESS);
      break;
    case (XVFRMBUFWR_HANDLER_READY):
      InstancePtr->FrameReadyCallback = (XVFrmbufWr_Callback)CallbackFunc;
      InstancePtr->CallbackReadyRef = CallbackRef;
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
* This function is the interrupt handler for the frame buffer write core driver.
*
* This handler clears the pending interrupt and determined if the source is
* frame done signal. If yes, starts the next frame processing and calls the
* registered callback function
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XVFrmbufWr_SetCallback() during initialization phase.
*
* @param    InstancePtr is a pointer to the core instance that just
*           interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVFrmbufWr_InterruptHandler(void *InstancePtr)
{
  XV_FrmbufWr_l2 *FrmbufWrPtr = (XV_FrmbufWr_l2 *)InstancePtr;
  u32 Status;

  /* Verify arguments */
  Xil_AssertVoid(FrmbufWrPtr != NULL);
  Xil_AssertVoid(FrmbufWrPtr->FrmbufWr.IsReady == XIL_COMPONENT_IS_READY);

  /* Get the interrupt source */
  Status = XV_frmbufwr_InterruptGetStatus(&FrmbufWrPtr->FrmbufWr);

  /* Check for Done Signal */
  if(Status & XVFRMBUFWR_IRQ_DONE_MASK) {
    /* Clear the interrupt */
    XV_frmbufwr_InterruptClear(&FrmbufWrPtr->FrmbufWr, XVFRMBUFWR_IRQ_DONE_MASK);
    //Call user registered callback function, if any
    if(FrmbufWrPtr->FrameDoneCallback) {
          FrmbufWrPtr->FrameDoneCallback(FrmbufWrPtr->CallbackDoneRef);
    }
    XV_frmbufwr_Start(&FrmbufWrPtr->FrmbufWr);
  }

  /* Check for Ready Signal */
  if(Status & XVFRMBUFWR_IRQ_READY_MASK) {
    /* Clear the interrupt */
    XV_frmbufwr_InterruptClear(&FrmbufWrPtr->FrmbufWr, XVFRMBUFWR_IRQ_READY_MASK);
    //Call user registered callback function, if any
    if(FrmbufWrPtr->FrameReadyCallback) {
          FrmbufWrPtr->FrameReadyCallback(FrmbufWrPtr->CallbackReadyRef);
    }
    XV_frmbufwr_Start(&FrmbufWrPtr->FrmbufWr);
  }
}
/** @} */
