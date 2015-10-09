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
/*****************************************************************************/
/**
*
* @file xhdcp1x_cipher_intr.c
* @addtogroup hdcp1x_v2_0
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

/************************** Function Definitions *****************************/

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
