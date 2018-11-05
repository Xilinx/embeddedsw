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
* @file xhdcp1x_tx.h
* @addtogroup hdcp1x_v4_2
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
* 1.10  MG     01/18/16 Added function XHdcp1x_TxIsEnabled.
* 1.20  MG     02/25/16 Added function XHdcp1x_TxSetCallback.
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added the following functions:
*                       XHdcp1x_TxReadDownstream, XHdcp1x_TxSetCallBack,
*                       XHdcp1x_TxTriggerDownstreamAuth.
* 3.1   yas    06/15/16 Repeater functionality extended to support HDMI.
*                       Added fucntions,
*                       XHdcp1x_TxIsInComputations,XHdcp1x_TxIsInWaitforready,
*                       XHdcp1x_TxIsDownstrmCapable, XHdcp1x_TxIsRepeater,
*                       XHdcp1x_TxEnableBlank, XHdcp1x_TxDisableBlank,
*                       XHdcp1x_TxGetTopologyKSVList,
*                       XHdcp1x_TxGetTopologyDepth,
*                       XHdcp1x_TxGetTopologyDeviceCnt,
*                       XHdcp1x_TxGetTopologyMaxCascadeExceeded,
*                       XHdcp1x_TxGetTopologyBKSV,
*                       XHdcp1x_TxGetTopologyMaxDevsExceeded,
*                       XHdcp1x_TxGetTopology
* 4.1   yas    22/04/16 Added function XHdcp1x_TxSetHdmiMode.
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
int XHdcp1x_TxSetCallback(XHdcp1x *InstancePtr,
	XHdcp1x_HandlerType HandlerType, void *CallbackFunc,
	void *CallbackRef);

int XHdcp1x_TxPoll(XHdcp1x *InstancePtr);

int XHdcp1x_TxReset(XHdcp1x *InstancePtr);
int XHdcp1x_TxEnable(XHdcp1x *InstancePtr);
int XHdcp1x_TxDisable(XHdcp1x *InstancePtr);

int XHdcp1x_TxIsDownstrmCapable(const XHdcp1x *InstancePtr);
u64 XHdcp1x_TxGetEncryption(const XHdcp1x *InstancePtr);
int XHdcp1x_TxEnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
int XHdcp1x_TxDisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
void XHdcp1x_TxSetHdmiMode(XHdcp1x *InstancePtr, u8 Value);

int XHdcp1x_TxSetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_TxSetLaneCount(XHdcp1x *InstancePtr, int LaneCount);

int XHdcp1x_TxAuthenticate(XHdcp1x *InstancePtr);
int XHdcp1x_TxReadDownstream(XHdcp1x *InstancePtr);
int XHdcp1x_TxIsInProgress(const XHdcp1x *InstancePtr);
int XHdcp1x_TxIsAuthenticated(const XHdcp1x *InstancePtr);
int XHdcp1x_TxIsInComputations(const XHdcp1x *InstancePtr);
int XHdcp1x_TxIsInWaitforready(const XHdcp1x *InstancePtr);
int XHdcp1x_TxIsEnabled(const XHdcp1x *InstancePtr);
void XHdcp1x_TxHandleTimeout(XHdcp1x *InstancePtr);

int XHdcp1x_TxInfo(const XHdcp1x *InstancePtr);
void XHdcp1x_TxTriggerDownstreamAuth(void *Parameter);

void XHdcp1x_TxEnableBlank(XHdcp1x *InstancePtr);
void XHdcp1x_TxDisableBlank(XHdcp1x *InstancePtr);
int XHdcp1x_TxIsRepeater(XHdcp1x *InstancePtr);

XHdcp1x_RepeaterExchange *XHdcp1x_TxGetTopology(XHdcp1x *InstancePtr);
u32 XHdcp1x_TxGetTopologyMaxDevsExceeded(XHdcp1x *InstancePtr);
u8 *XHdcp1x_TxGetTopologyBKSV(XHdcp1x *InstancePtr);
u32 XHdcp1x_TxGetTopologyMaxCascadeExceeded(XHdcp1x *InstancePtr);
u32 XHdcp1x_TxGetTopologyDeviceCnt(XHdcp1x *InstancePtr);
u32 XHdcp1x_TxGetTopologyDepth(XHdcp1x *InstancePtr);
u8 *XHdcp1x_TxGetTopologyKSVList(XHdcp1x *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_TX_H */
/** @} */
