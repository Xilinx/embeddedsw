/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_hdcp22.h
*
* This is the header file for Xilinx DisplayPort Receiver Subsystem sub-core,
* is High-Bandwidth Content Protection 2.2 (HDCP2.2).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 jb  02/18/19 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_HDCP22_H_
#define XDPRXSS_HDCP22_H_ /**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)
#include "xhdcp22_rx_dp.h"

/************************** Constant Definitions *****************************/

#define XDPRXSS_HDCP22_MAX_QUEUE_SIZE 16

/**************************** Type Definitions *******************************/
/**
* These constants specify the HDCP22 Events
*/
typedef enum
{
	XDPRXSS_HDCP22_NO_EVT,
	XDPRXSS_HDCP22_CONNECT_EVT,
	XDPRXSS_HDCP22_DISCONNECT_EVT,
	XDPRXSS_HDCP22_INVALID_EVT
} XDpRxSs_Hdcp22Event;

/**
 * These constants specify the HDCP key types
 */
typedef enum
{
	XDPRXSS_KEY_HDCP22_LC128,     /**< HDCP 2.2 LC128 */
	XDPRXSS_KEY_HDCP22_PRIVATE,   /**< HDCP 2.2 Private */
} XDpRxSs_Hdcp22KeyType;

typedef struct
{
	XDpRxSs_Hdcp22Event Queue[XDPRXSS_HDCP22_MAX_QUEUE_SIZE]; /**< Data */
	u8                    Tail;      /**< Tail pointer */
	u8                    Head;      /**< Head pointer */
} XDpRxSs_Hdcp22EventQueue;

/* The order of the enums in this should be as same as
 * Xhdcp22_Rx_DpcdFlag in xhdcp22_rx_dp_i.h*/
typedef enum {
	XDPRX_HDCP22_XHDCP22_RX_DPCD_FLAG_NONE,		/**< Clear DPCD flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_INIT_RCVD = 0x001,	/**< Ake_Init Msg Rcvd
								  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_NO_STORED_KM_RCVD = 0x002,/**< Ake_No_Stored_Km
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_STORED_KM_RCVD = 0x004,	/**< Ake_Stored_Km
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_LC_INIT_RCVD = 0x008,		/**< Lc_Init Msg Rcvd
							  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_SKE_SEND_EKS_RCVD = 0x010,	/**< Ske_Send_Eks
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_HPRIME_READ_DONE = 0x020,	/**< H' Msg Read done
							  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_PAIRING_INFO_READ_DONE = 0x040,/**< Pairing info
							   Read done Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_RPTR_RCVID_LST_ACK_READ_DONE = 0x080,/**< Repeater
								Receiver ID List Ack Read done Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_RPTR_STREAM_MANAGE_READ_DONE = 0x100,/**< Repeater
								Stream manage Read done Flag*/

} XDpRxSs_Hdcp22DpcdEvents;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XDpRxSs_SubcoreInitHdcp22(void *InstancePtr);
void XDpRxSs_Hdcp22Poll(void *Instance);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /*#if (XPAR_XHDCP22_RX_DP_NUM_INSTANCES > 0)*/

#endif /* XDPRXSS_HDCP22_H_ */
