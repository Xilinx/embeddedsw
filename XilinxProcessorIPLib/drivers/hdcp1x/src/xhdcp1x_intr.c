/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xhdcp1x_cipher_intr.c
* @addtogroup hdcp1x_v3_0
* @{
*
* This file contains interrupt related functions for Xilinx HDCP core.
* Please see xhdcp_cipher.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 3.0   yas    02/13/16 Added function XHdcp1x_SetCallBack.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_port.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_NUM_INSTANCES > 0)
#define INCLUDE_RX
#define INCLUDE_TX
#endif

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function sets the callback for High-Bandwidth Content Protection
* (HDCP) Tx interface to write the Repeater values read from downstream
* to upstream.
*
* @param	InstancePtr is a pointer to the HDCP core instance.
* @param	HandlerType is the handler which describes the call back type.
* @param	CallBackFunc is a pointer to the call back function.
* @param	CallBackRef is a vod pointer to the call back reference.
*
* @return
*		- XST_SUCCESS, if HDCP i/f enabled successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdcp1x_SetCallBack(XHdcp1x *InstancePtr, u32 HandlerType,
		XHdcp1x_Callback CallBackFunc, void *CallBackRef)
{
	u32 Status;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	if (!InstancePtr->Config.IsRx) {
		Status = XHdcp1x_TxSetCallBack(InstancePtr, HandlerType,
					CallBackFunc, CallBackRef);
	}
	else
#endif
#if defined(INCLUDE_RX)
	if (InstancePtr->Config.IsRx) {
		Status = XHdcp1x_RxSetCallBack(InstancePtr, HandlerType,
					CallBackFunc, CallBackRef);
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function is the cipher interrupt handler for the HDCP module.
*
* @param	InstancePtr is the device instance that just interrupted.
*
* @return	None.
*
* @note		This function just forwards the interrupt along to the
*		corresponding cipher core.
*
******************************************************************************/
void XHdcp1x_CipherIntrHandler(void *InstancePtr)
{
	XHdcp1x *HdcpPtr = InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(HdcpPtr != NULL);
	Xil_AssertVoid(HdcpPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Dispatch it to the corresponding cipher */
	XHdcp1x_CipherHandleInterrupt(HdcpPtr);
}

/*****************************************************************************/
/**
* This function is the port interrupt handler for the HDCP module.
*
* @param	InstancePtr is the device instance that just interrupted.
* @param	IntCause is the interrupt cause bit map.
*
* @return	None.
*
* @note		This function just forwards the interrupt along to the
*		corresponding cipher core.
*
******************************************************************************/
void XHdcp1x_PortIntrHandler(void *InstancePtr, u32 IntCause)
{
	XHdcp1x *HdcpPtr = InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(HdcpPtr != NULL);
	Xil_AssertVoid(HdcpPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Dispatch it to the corresponding port */
	XHdcp1x_PortHandleInterrupt(HdcpPtr, IntCause);
}

/** @} */
