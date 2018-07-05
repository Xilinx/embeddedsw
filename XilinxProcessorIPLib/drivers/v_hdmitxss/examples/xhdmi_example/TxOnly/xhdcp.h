/******************************************************************************
*
* Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp.h
*
* This is the main header file for the Xilinx HDCP abstraction layer.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   05/24/16 First Release
* 1.01  MH   06/16/17 Removed authentication request flag.
*</pre>
*
*****************************************************************************/
#ifndef _XHDCP_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDCP_H_

#ifdef __cplusplus
extern "C" {
#endif



/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xparameters.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif

/************************** Constant Definitions ****************************/
#define XHDCP_DEVICE_ID_SIZE               5    /*< Size in bytes of ReceiverID for HDCP 2.2 or KSV for HDCP 1.4 */
#define XHDCP_MAX_DOWNSTREAM_INTERFACES    32   /*< Maximum number of HDCP downstream interfaces allowed */
#define XHDCP_MAX_DEVICE_CNT_HDCP14        127  /*< Maximum repeater topology device count for HDCP 1.4 */
#define XHDCP_MAX_DEPTH_HDCP14             7    /*< Maximum repeater topology depth for HDCP 1.4 */
#define XHDCP_MAX_DEVICE_CNT_HDCP22        31   /*< Maximum repeater topology device count for HDCP 2.2 */
#define XHDCP_MAX_DEPTH_HDCP22             4    /*< Maximum repeater topology depth for HDCP 2.2 */

/************************** Variable Declaration ****************************/

/**************************** Type Definitions ******************************/
typedef struct
{
  int DeviceCnt;
  int Depth;
  u8  DeviceList[XHDCP_MAX_DEVICE_CNT_HDCP14][XHDCP_DEVICE_ID_SIZE];
  u8  MaxDevsExceeded;
  u8  MaxCascadeExceeded;
  u8  Hdcp20RepeaterDownstream;
  u8  Hdcp1DeviceDownstream;
} XHdcp_Topology;

typedef struct
{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /** Array of pointers to each HDMI repeater downstream interface */
  XV_HdmiTxSs *DownstreamInstancePtr[XHDCP_MAX_DOWNSTREAM_INTERFACES];
  /** Count of HDMI repeater downstream interfaces binded */
  u8 DownstreamInstanceBinded;
  /** Flag indicates downstream interface is connected */
  u32 DownstreamInstanceConnected;
  /** Flag indicates downstream interface stream is up */
  u32 DownstreamInstanceStreamUp;
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /** Enforce content blocking */
  u8 EnforceBlocking;
  u8 StreamType;
#endif
  /** Flag indicates that HDCP repeater is ready */
  u32 IsReady;
} XHdcp_Repeater;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

// Functions used for initialization
int  XHdcp_Initialize(XHdcp_Repeater *InstancePtr);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
int  XHdcp_SetDownstream(XHdcp_Repeater *InstancePtr,
       XV_HdmiTxSs *DownstreamInstancePtr);
#endif

// Functions used to process callback events
void XHdcp_StreamUpCallback(void *HdcpInstancePtr);
void XHdcp_StreamDownCallback(void *HdcpInstancePtr);
void XHdcp_StreamConnectCallback(void *HdcpInstancePtr);
void XHdcp_StreamDisconnectCallback(void *HdcpInstancePtr);

// Other functions
void XHdcp_Poll(XHdcp_Repeater *InstancePtr);
void XHdcp_DisplayInfo(XHdcp_Repeater *InstancePtr, u8 Verbose);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void XHdcp_Authenticate(XHdcp_Repeater *InstancePtr);
void XHdcp_EnableEncryption(XHdcp_Repeater *InstancePtr, u8 Set);
void XHdcp_SetDownstreamCapability(XHdcp_Repeater *InstancePtr, int Protocol);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDCP_H_ */
