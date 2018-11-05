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
* @file xv_frmbufrd_l2_intr.c
* @addtogroup v_frmbuf_rd_v2_0
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
#include "xv_frmbufrd_l2.h"


/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                     Callback Function Type
* -----------------------         --------------------------------------------------
* (XVFRMBUFRD_HANDLER_DONE)       DoneCallback
* (XVFRMBUFRD_HANDLER_READY)      ReadyCallback
*
* @param    InstancePtr is a pointer to the Frame Buffer Read core instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*           callback function when it is invoked.
*
* @return
*           - XST_SUCCESS if callback function installed successfully.
*           - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note   Invoking this function for a handler that already has been
*         installed replaces it with the new handler.
*
******************************************************************************/
int XVFrmbufRd_SetCallback(XV_FrmbufRd_l2 *InstancePtr, u32 HandlerType,
                           void *CallbackFunc, void *CallbackRef)
{
  u32 Status;
  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(HandlerType >= (XVFRMBUFRD_HANDLER_DONE));
  Xil_AssertNonvoid(CallbackFunc != NULL);
  Xil_AssertNonvoid(CallbackRef != NULL);

  /* Check for handler type */
  switch (HandlerType) {
    case (XVFRMBUFRD_HANDLER_DONE):
      InstancePtr->FrameDoneCallback = (XVFrmbufRd_Callback)CallbackFunc;
      InstancePtr->CallbackDoneRef = CallbackRef;
      Status = (XST_SUCCESS);
      break;
    case (XVFRMBUFRD_HANDLER_READY):
      InstancePtr->FrameReadyCallback = (XVFrmbufRd_Callback)CallbackFunc;
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
* This function is the interrupt handler for the frame buffer read core driver.
*
* This handler clears the pending interrupt and determined if the source is
* frame done signal. If yes, starts the next frame processing and calls the
* registered callback function
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XVFrmbufRd_SetCallback() during initialization phase.
*
* @param    InstancePtr is a pointer to the core instance that just
*           interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVFrmbufRd_InterruptHandler(void *InstancePtr)
{
  XV_FrmbufRd_l2 *FrmbufRdPtr = (XV_FrmbufRd_l2 *)InstancePtr;
  u32 Status;

  /* Verify arguments */
  Xil_AssertVoid(FrmbufRdPtr != NULL);
  Xil_AssertVoid(FrmbufRdPtr->FrmbufRd.IsReady == XIL_COMPONENT_IS_READY);

  /* Get the interrupt source */
  Status = XV_frmbufrd_InterruptGetStatus(&FrmbufRdPtr->FrmbufRd);

  /* Check for Done Signal */
  if(Status & XVFRMBUFRD_IRQ_DONE_MASK) {
    /* Clear the interrupt */
    XV_frmbufrd_InterruptClear(&FrmbufRdPtr->FrmbufRd, XVFRMBUFRD_IRQ_DONE_MASK);
    //Call user registered callback function, if any
    if(FrmbufRdPtr->FrameDoneCallback) {
          FrmbufRdPtr->FrameDoneCallback(FrmbufRdPtr->CallbackDoneRef);
    }
    XV_frmbufrd_Start(&FrmbufRdPtr->FrmbufRd);
  }

  /* Check for Ready Signal */
  if(Status & XVFRMBUFRD_IRQ_READY_MASK) {
    /* Clear the interrupt */
    XV_frmbufrd_InterruptClear(&FrmbufRdPtr->FrmbufRd, XVFRMBUFRD_IRQ_READY_MASK);
    //Call user registered callback function, if any
    if(FrmbufRdPtr->FrameReadyCallback) {
          FrmbufRdPtr->FrameReadyCallback(FrmbufRdPtr->CallbackReadyRef);
    }
    XV_frmbufrd_Start(&FrmbufRdPtr->FrmbufRd);
  }
}
/** @} */
