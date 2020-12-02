/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 3.03  YB   08/14/18 Initial release of Repeater ExDes.
*                     Added macro 'XHDCP_MAX_DEVICE_CNT_CTS_HDCP14',
*                     for maximum devices supported for HDCP1.4 CTS.
*                     Added flag 'UpstreamAuthRequestCount'.
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif
#include "xhdmi_example.h"

/************************** Constant Definitions ****************************/
#define XHDCP_DEVICE_ID_SIZE               5    /*< Size in bytes of ReceiverID for HDCP 2.2 or KSV for HDCP 1.4 */
#define XHDCP_MAX_DOWNSTREAM_INTERFACES    32   /*< Maximum number of HDCP downstream interfaces allowed */
#define XHDCP_MAX_DEVICE_CNT_HDCP14        127  /*< Maximum repeater topology device count for HDCP 1.4 */
#define XHDCP_MAX_DEVICE_CNT_CTS_HDCP14    32   /*< Maximum repeater topology device count for HDCP 1.4 CTS tests */
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
  u8  Hdcp2LegacyDeviceDownstream;
  u8  Hdcp1DeviceDownstream;
} XHdcp_Topology;

#if ENABLE_HDCP_PRO
typedef struct {
  u16 Year;
  u8  Month;
  u8  Day;
} XHdcpPro_DateTime;

typedef struct {
  u8 hdcp_2_srm_ver[2];
  XHdcpPro_DateTime DateTime;
  u8 DcpLlcSign[384];
} XHdcpPro_Timestamp;
#endif

typedef struct
{
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  /** Pointer to the HDMI repeater upstream interface */
  XV_HdmiRxSs *UpstreamInstancePtr;
  /** Indicates that upstream interface has been binded */
  u8 UpstreamInstanceBinded;
  /** Flag Indicates that upstream interface is connected */
  u8 UpstreamInstanceConnected;
  /** Flag indicates upstream interface stream is up */
  u8 UpstreamInstanceStreamUp;
  /** Authentication Request count on the upstream interface */
  u32 UpstreamAuthRequestCount;
#endif
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
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
  /** HDCP topology */
  XHdcp_Topology Topology;
  /** Content stream type */
#if ENABLE_HDCP_PRO
  /** Flag to track if the system is a HDCP Professional Repeater. */
  u8 IsHdcpProRepeater;
  /** HDCP Professional Repeater variables. */
  XHdcpPro_Timestamp HdcpProTimestamp;
#endif
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
int  XHdcp_SetUpstream(XHdcp_Repeater *InstancePtr,
       XV_HdmiRxSs *UpstreamInstancePtr);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
int  XHdcp_SetDownstream(XHdcp_Repeater *InstancePtr,
       XV_HdmiTxSs *DownstreamInstancePtr);
#endif
#if defined XPAR_XV_HDMITXSS_NUM_INSTANCES && defined XPAR_XV_HDMIRXSS_NUM_INSTANCES
void XHdcp_SetRepeater(XHdcp_Repeater *InstancePtr, u8 Set);
#if ENABLE_HDCP_PRO
void XHdcp_SetProRepeater(XHdcp_Repeater *InstancePtr, u8 Set);
#endif
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void XHdcp_SetUpstreamCapability(XHdcp_Repeater *InstancePtr, int Protocol);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDCP_H_ */
