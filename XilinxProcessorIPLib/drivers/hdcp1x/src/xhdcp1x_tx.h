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
* @file xhdcp1x_tx.h
* @addtogroup hdcp1x_v2_0
* @{
*
* This file provides the interface of the HDCP TX state machine
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

#ifndef XHDCP1X_TX_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_TX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XHdcp1x_TxInit(XHdcp1x *InstancePtr);

int XHdcp1x_TxPoll(XHdcp1x *InstancePtr);

int XHdcp1x_TxReset(XHdcp1x *InstancePtr);
int XHdcp1x_TxEnable(XHdcp1x *InstancePtr);
int XHdcp1x_TxDisable(XHdcp1x *InstancePtr);

u64 XHdcp1x_TxGetEncryption(const XHdcp1x *InstancePtr);
int XHdcp1x_TxEnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
int XHdcp1x_TxDisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);

int XHdcp1x_TxSetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_TxSetLaneCount(XHdcp1x *InstancePtr, int LaneCount);

int XHdcp1x_TxAuthenticate(XHdcp1x *InstancePtr);
int XHdcp1x_TxIsInProgress(const XHdcp1x *InstancePtr);
int XHdcp1x_TxIsAuthenticated(const XHdcp1x *InstancePtr);

void XHdcp1x_TxHandleTimeout(XHdcp1x *InstancePtr);

int XHdcp1x_TxInfo(const XHdcp1x *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_TX_H */
/** @} */
