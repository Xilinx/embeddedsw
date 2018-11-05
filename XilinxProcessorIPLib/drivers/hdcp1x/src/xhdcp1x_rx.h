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
* @file xhdcp1x_rx.h
* @addtogroup hdcp1x_v4_2
* @{
*
* This file provides the interface of the HDCP RX state machine
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 1.10  MG     01/18/16 Added function XHdcp1x_RxIsEnabled and
*                       XHdcp1x_RxIsInProgress.
* 1.20  MG     02/25/16 Added function XHdcp1x_RxSetCallback.
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added functions:
*                       XHdcp1x_RxDownstreamReady, XHdcp1x_RxGetRepeaterInfo,
*                       XHdcp1x_RxDownstreamReadyCallback,
*                       XHdcp1x_RxSetCallBack.
* 3.1   yas    07/28/16 Repeater functionality extended to support HDMI.
*                       Removed the XHdcp1x_RxDownstreamReadyCallback.
*                       Added fucntions,
*                       XHdcp1x_RxSetRepeaterBcaps,XHdcp1x_RxIsInComputations,
*                       XHdcp1x_RxIsInWaitforready, XHdcp1x_RxHandleTimeout,
*                       XHdcp1x_RxStartTimer, XHdcp1x_RxStopTimer,
*                       XHdcp1x_RxBusyDelay, XHdcp1x_RxSetTopologyUpdate,
*                       XHdcp1x_RxSetTopology, XHdcp1x_RxSetTopologyKSVList,
*                       XHdcp1x_RxSetTopologyDepth,
*                       XHdcp1x_RxSetTopologyDeviceCnt,
*                       XHdcp1x_RxSetTopologyMaxCascadeExceeded,
*                       XHdcp1x_RxSetTopologyMaxDevsExceeded,
*                       XHdcp1x_RxCheckEncryptionChange.
* 4.1   yas    11/10/16 Added function XHdcp1x_RxSetHdmiMode.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_RX_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_RX_H

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

void XHdcp1x_RxInit(XHdcp1x *InstancePtr);
int XHdcp1x_RxSetCallback(XHdcp1x *InstancePtr,
		XHdcp1x_HandlerType HandlerType, void *CallbackFunc,
		void *CallbackRef);

int XHdcp1x_RxPoll(XHdcp1x *InstancePtr);
int XHdcp1x_RxSetRepeaterBcaps(XHdcp1x *InstancePtr, u8 IsRptr);

int XHdcp1x_RxReset(XHdcp1x *InstancePtr);
int XHdcp1x_RxEnable(XHdcp1x *InstancePtr);
int XHdcp1x_RxDisable(XHdcp1x *InstancePtr);

int XHdcp1x_RxSetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_RxSetLaneCount(XHdcp1x *InstancePtr, int LaneCount);
int XHdcp1x_RxDownstreamReady(XHdcp1x *InstancePtr);
int XHdcp1x_RxAuthenticate(XHdcp1x *InstancePtr);
int XHdcp1x_RxIsAuthenticated(const XHdcp1x *InstancePtr);
int XHdcp1x_RxIsInComputations(const XHdcp1x *InstancePtr);
int XHdcp1x_RxIsInWaitforready(const XHdcp1x *InstancePtr);
int XHdcp1x_RxIsEnabled(const XHdcp1x *InstancePtr);
int XHdcp1x_RxIsInProgress(const XHdcp1x *InstancePtr);

u64 XHdcp1x_RxGetEncryption(const XHdcp1x *InstancePtr);

void XHdcp1x_RxHandleTimeout(XHdcp1x *InstancePtr);
int XHdcp1x_RxInfo(const XHdcp1x *InstancePtr);
int XHdcp1x_RxGetRepeaterInfo(XHdcp1x *InstancePtr,
		XHdcp1x_RepeaterExchange *RepeaterInfoPtr);

void XHdcp1x_RxSetTopologyMaxDevsExceeded(XHdcp1x *InstancePtr, u8 Value);
void XHdcp1x_RxSetTopologyMaxCascadeExceeded(XHdcp1x *InstancePtr, u8 Value);
void XHdcp1x_RxSetTopologyDeviceCnt(XHdcp1x *InstancePtr, u32 Value);
void XHdcp1x_RxSetTopologyDepth(XHdcp1x *InstancePtr, u32 Value);
void XHdcp1x_RxSetTopologyKSVList(XHdcp1x *InstancePtr, u8 *ListPtr,
		u32 ListSize);
void XHdcp1x_RxSetTopologyUpdate(XHdcp1x *InstancePtr);
void XHdcp1x_RxSetTopology(XHdcp1x *InstancePtr,
		const XHdcp1x_RepeaterExchange *TopologyPtr);
void XHdcp1x_RxSetHdmiMode(XHdcp1x *InstancePtr, u8 Value);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_RX_H */
/** @} */
