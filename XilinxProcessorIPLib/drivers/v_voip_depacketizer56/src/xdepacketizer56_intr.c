/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xdepacketizer56_intr.c
*
* This file contains interrupt related functions for Xilinx VoIP ST2022-6
* Depacketizer core.
* Please see xdepacketizer56.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00   mmo   02/12/16 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdepacketizer56.h"

/************************** Constant Definitions *****************************/
#define XDEPACKTIZER56_DEBUG_PRINT 0

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XDepacketizer56_DatagramMismatchHandler
                                              (XDepacketizer56 *InstancePtr);
static void XDepacketizer56_BufferEmptyHandler(XDepacketizer56 *InstancePtr);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the VoIP ST2022-6 Depacketizer
* driver.
*
* This handler reads the pending interrupt from VoIP ST2022-6 Depacketizer,
* determines the source of the interrupts, clears the interrupts and calls
* callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDepacketizer56_SetCallback() during initialization phase. An example
* delivered with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XDepacketizer56 instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDepacketizer56_IntrHandler(void *InstancePtr)
{
    u32 Data;
    XDepacketizer56 *XDepacketizer56Ptr = (XDepacketizer56 *)InstancePtr;

    /* Verify arguments */
    Xil_AssertVoid(XDepacketizer56Ptr != NULL);
    Xil_AssertVoid(XDepacketizer56Ptr->IsReady == XIL_COMPONENT_IS_READY);

#if XDEPACKTIZER56_DEBUG_PRINT > 0
    xil_printf("Interrupt for De-Packetizer 56\n\r");
#endif

    /* Video Lock */
    Data = XDepacketizer56_ReadReg(XDepacketizer56Ptr->Config.BaseAddress,
            (XDEPACKETIZER56_INTR_STATUS_REG_OFFSET)) &
                    (XDEPACKETIZER56_INTR_STATUS_DATAGRAM_FRAME_MISMATCH_MASK);

    /* Check for IRQ flag set */
    if (Data) {
        XDepacketizer56Ptr->DatagramperFrameMismatchInterrupt = (TRUE);

        /* Jump to Video Lock handler */
        XDepacketizer56_DatagramMismatchHandler(XDepacketizer56Ptr);

#if XDEPACKTIZER56_DEBUG_PRINT > 0
        xil_printf("Datagram Per Frame Mismatch Interrupt Served\n\r");
#endif
    }

    /* Video Lock */
    Data = ((XDepacketizer56_ReadReg(XDepacketizer56Ptr->Config.BaseAddress,
            (XDEPACKETIZER56_INTR_STATUS_REG_OFFSET)) &
                    (XDEPACKETIZER56_INTR_STATUS_BUFFER_EMPTY_MASK)) >>
                        (XDEPACKETIZER56_INTR_STATUS_BUFFER_EMPTY_SHIFT));

    /* Check for IRQ flag set */
    if (Data) {
        XDepacketizer56Ptr->BufferEmptyInterrupt = (TRUE);

        /* Jump to Video Lock handler */
        XDepacketizer56_BufferEmptyHandler(XDepacketizer56Ptr);

#if XDEPACKTIZER56_DEBUG_PRINT > 0
        xil_printf("Buffer Empty Interrupt Served\n\r");
#endif
    }

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                 Callback Function Type
* -------------------------   -----------------------------------------------
* </pre>
*
* @param    InstancePtr is a pointer to the VoIP ST2022-6 Depacketizer core
*       instance.
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
int XDepacketizer56_SetCallback(XDepacketizer56 *InstancePtr,
                                    XDepacketizer56_HandlerType HandlerType,
                                         void *CallbackFunc, void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XDEPACKETIZER56_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {

        case (XDEPACKETIZER56_HANDLER_DATAGRAM_PER_FRAME_MISMATCH):
            InstancePtr->DatagramMismatchCallback =
                                        (XDepacketizer56_Callback)CallbackFunc;
            InstancePtr->DatagramMismatchRef = CallbackRef;
            InstancePtr->IsDatagramMismatchCallbackSet = (TRUE);
            Status = (XST_SUCCESS);
            break;

        case (XDEPACKETIZER56_HANDLER_BUFFER_EMPTY):
            InstancePtr->BufferEmptyCallback =
                                        (XDepacketizer56_Callback)CallbackFunc;
            InstancePtr->BufferEmptyRef = CallbackRef;
            InstancePtr->IsBufferEmptyCallbackSet = (TRUE);
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
* This function is the VoIP ST2022-6 Depacketizer Error Reporting Interrupt
* Handler for Incoming Packet Datagram Per Frame Mismatch.
*
* This handler reads corresponding event interrupt from the Datagram Per Frame
* Mismatch Interrupt Status register. It determines the source of the
* interrupts and calls according callbacks.
*
* @param    InstancePtr is a pointer to the VoIP ST2022-6 Depacketizer core
*       instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void XDepacketizer56_DatagramMismatchHandler
                                                 (XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    /* Callback */
    if (InstancePtr->IsDatagramMismatchCallbackSet) {
        InstancePtr->DatagramMismatchCallback
                                            (InstancePtr->DatagramMismatchRef);
    }

    /* Expected Datagrame */
    RegValue = XDepacketizer56_ReadReg(InstancePtr->Config.BaseAddress,
            (XDEPACKETIZER56_ERROR_STATUS_REG_OFFSET)) &
                    (XDEPACKETIZER56_ERROR_STATUS_DET_DATAGRAM_PER_FRAME_MASK);

    /* Clear Interrupt Register */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XDEPACKETIZER56_INTR_CLEAR_REG_OFFSET),
           (XDEPACKETIZER56_INTR_STATUS_DATAGRAM_FRAME_MISMATCH_MASK));

    InstancePtr->DatagramperFrameMismatchInterrupt = (FALSE);
}

/*****************************************************************************/
/**
*
* This function is the VoIP ST2022-6 Depacketizer Buffer Empty Reporting
* Interrupt Handler.
*
* This handler reads corresponding event interrupt from the Buffer Empty
* Interrupt Status register. It determines the source of the interrupts and
* calls according callbacks.
*
* @param    InstancePtr is a pointer to the VoIP ST2022-6 Depacketizer core
*       instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void XDepacketizer56_BufferEmptyHandler(XDepacketizer56 *InstancePtr)
{
    u32 RegValue;

    /* Callback */
    if (InstancePtr->IsBufferEmptyCallbackSet) {
        InstancePtr->BufferEmptyCallback (InstancePtr->BufferEmptyRef);
    }

    /* Clear Interrupt Register */
    XDepacketizer56_WriteReg((InstancePtr->Config.BaseAddress),
       (XDEPACKETIZER56_INTR_CLEAR_REG_OFFSET),
            (XDEPACKETIZER56_INTR_STATUS_BUFFER_EMPTY_MASK));

    InstancePtr->BufferEmptyInterrupt = (FALSE);
}
