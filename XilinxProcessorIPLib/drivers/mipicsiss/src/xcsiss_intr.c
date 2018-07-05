/******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsiss_intr.c
* @addtogroup csiss_v1_1
* @{
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
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/27/15 Initial release
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


/************************** Function Definitions *****************************/

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
	XCsiSs *XCsiSsPtr = (XCsiSs *)InstancePtr;

	/* Verify arguments. */
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
* HandlerType                Invoked by this driver when:
* -------------------------  --------------------------------------------------
* XCSISS_HANDLER_DPHY        A DPHY Level Error has been detected.
* XCSISS_HANDLER_PROTLVL     A Protocol Level Error has been detected.
* XCSISS_HANDLER_PKTLVL      A Packet Level Error has been detected.
* XCSISS_HANDLER_SHORTPACKET A Short packet has been received or the
*                            Short Packet FIFO is full.
* XCSISS_HANDLER_FRAMERECVD  A Frame has been received
* XCSISS_HANDLER_OTHERERROR  Any other type of interrupt has occured like
*                            Stream Line Buffer Full, Incorrect Lanes, etc
* </pre>
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	HandlerType is the type of call back to be registered.
* @param	CallbackFunc is the pointer to a call back funtion which
*		is called when a particular event occurs.
* @param	CallbackRef is a void pointer to data to be referenced to
*		by the CallBackFunc
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
****************************************************************************/
u32 XCsiSs_SetCallBack(XCsiSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	Status = XCsi_SetCallBack(InstancePtr->CsiPtr, HandlerType,
					CallbackFunc, CallbackRef);

	return Status;
}

/*****************************************************************************/
/**
* This function is used to disable the interrupts in the CSI core.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
* @param	IntrMask Indicates Mask for enable interrupts.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsiSs_IntrDisable(XCsiSs *InstancePtr, u32 IntrMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->CsiPtr != NULL);
	Xil_AssertVoid((IntrMask & ~XCSISS_ISR_ALLINTR_MASK) == 0);

	XCsi_IntrDisable(InstancePtr->CsiPtr, IntrMask);
}
/** @} */
