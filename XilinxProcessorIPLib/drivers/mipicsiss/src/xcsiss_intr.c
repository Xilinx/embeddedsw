/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
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
* @file xcsiss_intr.c
* @addtogroup csiss
* @{
* @details
*
* This is the interrupt handling part of the Xilinx MIPI CSI Rx Subsystem
* device driver. The interrupt registration and handler are defined here.
* The callbacks are registered for events which are interrupts clubbed together
* on the basis of the CSI specification. Refer to CSI driver for the event
* groups.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vs    07/27/15   Initial Release

* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xcsi.h"
#include "xcsiss.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/**************************** Local Global ***********************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
*
* This function is the interrupt handler for the MIPI CSI Rx Subsystem.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCsiSs_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XCsiSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCsiSs_IntrHandler(void *InstancePtr)
{
	XCsiSs *XCsiSsPtr = (XCsiSs *)((void *)InstancePtr);

	Xil_AssertVoid(XCsiSsPtr != NULL);
	Xil_AssertVoid(XCsiSsPtr->CsiPtr != NULL);

	XCsi_IntrHandler(XCsiSsPtr->CsiPtr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
*
* HandlerType              Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XCSI_HANDLER_DPHY        A DPHY Level Error has been detected.
*
* XCSI_HANDLER_PROTLVL     A Protocol Level Error has been detected.
*
* XCSI_HANDLER_PKTLVL      A Packet Level Error has been detected.
*
* XCSI_HANDLER_SHORTPACKET A Short packet has been received or the
* 			   Short Packet FIFO is full.
*
* XCSI_HANDLER_FRAMERECVD  A Frame has been received
*
* XCSI_HANDLER_OTHERERROR  Any other type of interrupt has occured like
* 			   Stream Line Buffer Full, Incorrect Lanes, etc
*
* </pre>
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param 	HandlerType is the type of call back to be registered.
*
* @param	CallBackFunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
*
* @param 	CallBackRef is a void pointer to data to be referenced to
* 		by the CallBackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
u32 XCsiSs_SetCallBack(XCsiSs *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef)
{
	u32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);

	Status = XCsi_SetCallBack(InstancePtr->CsiPtr, HandlerType,
					CallBackFunc, CallBackRef);

	return Status;
}

/** @} */
