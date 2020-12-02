/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/*
 * xhdcp22_example.h
 *
 *  Created on: Dec 17, 2018
 *      Author: jbaniset
 */

#ifndef XHDCP22_EXAMPLE_H_
#define XHDCP22_EXAMPLE_H_

#include "xdprxss.h"
#include "xdptxss.h"

#define XHDCP_DEVICE_ID_SIZE               5    /*< Size in bytes of ReceiverID for HDCP 2.2 or KSV for HDCP 1.4 */
#define XHDCP_MAX_DOWNSTREAM_INTERFACES    32   /*< Maximum number of HDCP downstream interfaces allowed */
#define XHDCP_MAX_DEVICE_CNT_HDCP14        127  /*< Maximum repeater topology device count for HDCP 1.4 */
#define XHDCP_MAX_DEPTH_HDCP14             7    /*< Maximum repeater topology depth for HDCP 1.4 */
#define XHDCP_MAX_DEVICE_CNT_HDCP22        31   /*< Maximum repeater topology device count for HDCP 2.2 */
#define XHDCP_MAX_DEPTH_HDCP22             4    /*< Maximum repeater topology depth for HDCP 2.2 */

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


typedef struct
{
#ifdef XPAR_XDPRXSS_NUM_INSTANCES
  /** Pointer to the DP repeater upstream interface */
  XDpRxSs *UpstreamInstancePtr;
  /** Indicates that upstream interface has been binded */
  u8 UpstreamInstanceBinded;
  /** Flag Indicates that upstream interface is connected */
  u8 UpstreamInstanceConnected;
  /** Flag indicates upstream interface stream is up */
  u8 UpstreamInstanceStreamUp;
#endif
/* This should be enabled with if both RX and TX
 * HDCP22 are enable din the same design*/
#if 1
#if XPAR_XDPTXSS_NUM_INSTANCES
  /** Array of pointers to each DP repeater downstream interface */
  XDpTxSs *DownstreamInstancePtr[XHDCP_MAX_DOWNSTREAM_INTERFACES];
  /** Count of DP repeater downstream interfaces binded */
  u8 DownstreamInstanceBinded;
  /** Flag indicates downstream interface is connected */
  u32 DownstreamInstanceConnected;
  /** Flag indicates downstream interface stream is up */
  u32 DownstreamInstanceStreamUp;
#endif
#if defined (XPAR_XDPTXSS_NUM_INSTANCES) && defined (XPAR_XDPRXSS_NUM_INSTANCES)
  /** HDCP topology */
  XHdcp_Topology Topology;
  /** Content stream type */
#endif
#ifdef XPAR_XDPTXSS_NUM_INSTANCES
  /** Enforce content blocking */
  u8 EnforceBlocking;
  u8 StreamType;
#endif
#endif /*#if 0*/
  /** Flag indicates that HDCP repeater is ready */
  u32 IsReady;
} XHdcp22_Repeater;

/**
* These constants specify the HDCP key types
*/
typedef enum
{
  XV_DPRXSS_KEY_HDCP22_LC128,     /**< HDCP 2.2 LC128 */
  XV_DPRXSS_KEY_HDCP22_PRIVATE   /**< HDCP 2.2 Private */
} XV_DpRxSs_Hdcp22KeyType;

int XHdcp22_SetUpstream(XHdcp22_Repeater *InstancePtr,
      XDpRxSs *UpstreamInstancePtr);
void XHdcp22_Poll(XHdcp22_Repeater *InstancePtr);

int XHdcp22_LoadKeys_rx(uint8_t *Hdcp22Lc128, uint32_t Hdcp22Lc128Size,
		uint8_t *Hdcp22RxPrivateKey, uint32_t Hdcp22RxPrivateKeySize);
void XV_DpRxSs_Hdcp22SetKey(XDpRxSs *InstancePtr,
		XV_DpRxSs_Hdcp22KeyType KeyType, u8 *KeyPtr);


/**
* These constants specify the HDCP key types
*/
typedef enum
{
  XV_DPTXSS_KEY_HDCP22_LC128,     /**< HDCP 2.2 LC128 */
  XV_DPTXSS_KEY_HDCP22_SRM,		  /**< HDCP 2.2 SRM */
  XV_DPTXSS_KEY_INVALID           /**< Invalid Key */
} XV_DpTxSs_Hdcp22KeyType;

int XHdcp22_LoadKeys_tx(u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size);
void XV_DpTxSs_Hdcp22SetKey(XDpTxSs *InstancePtr,
		XV_DpTxSs_Hdcp22KeyType KeyType, u8 *KeyPtr);


#endif /* XHDCP22_EXAMPLE_H_ */
