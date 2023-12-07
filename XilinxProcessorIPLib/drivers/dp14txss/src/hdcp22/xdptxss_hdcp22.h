/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_hdcp22.h
*
* This is the header file for Xilinx DisplayPort Transmitter Subsystem
* sub-core, is High-Bandwidth Content Protection (HDCP22).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 jb  02/21/19 Initial release.
* </pre>
*
******************************************************************************/
#ifndef HDCP22_XDPTXSS_HDCP22_H_
#define HDCP22_XDPTXSS_HDCP22_H_	/**< Prevent circular inclusions
				  *  by using protection macros */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"
#if (XPAR_XHDCP22_TX_DP_NUM_INSTANCES > 0)
#include "xhdcp22_tx_dp.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/

#define XDPTXSS_HDCP_MAX_QUEUE_SIZE	   16

/**************************** Type Definitions *******************************/
/**
* These constants specify the HDCP Events
*/
typedef enum
{
	XDPTXSS_HDCP_NO_EVT,
	XDPTXSS_HDCP_CONNECT_EVT,
	XDPTXSS_HDCP_DISCONNECT_EVT,
	XDPTXSS_HDCP_AUTHENTICATE_EVT,
	XDPTXSS_HDCP_INVALID_EVT
} XDpTxSs_HdcpEvent;

typedef struct
{
	XDpTxSs_HdcpEvent Queue[XDPTXSS_HDCP_MAX_QUEUE_SIZE]; /**< Data */
	u8	Tail;      /**< Tail pointer */
	u8	Head;      /**< Head pointer */
} XDpTxSs_HdcpEventQueue;

/**
 * These constants specify the HDCP key types
 */
typedef enum
{
	XDPTXSS_KEY_HDCP22_LC128,	/**< HDCP 2.2 LC128 */
	XDPTXSS_KEY_HDCP22_SRM,		/**< HDCP 2.2 SRM */
	XDPTXSS_KEY_INVALID		/**< Invalid Key */
} XDpTxSs_Hdcp22KeyType;

/************************** Variable Declarations ****************************/


/************************** Function Prototypes ******************************/
int XDpTxSs_SubcoreInitHdcp22(void *InstancePtr);
int XDpTxSs_HdcpPoll(void *Instance);
int XDpTxSs_HdcpPushEvent(void *Instance, XDpTxSs_HdcpEvent Event);
u8 XDpTxSs_IsSinkHdcp22Capable(void *Instance);

#endif /*(XPAR_XHDCP22_TX_DP_NUM_INSTANCES > 0)*/

#ifdef __cplusplus
}
#endif

#endif /* HDCP22_XDPTXSS_HDCP22_H_ */
