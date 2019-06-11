/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
#include "xhdcp22_rx.h"

/************************** Constant Definitions *****************************/

#define XDPRXSS_HDCP22_MAX_QUEUE_SIZE 16

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for the HDCP22 core.
*/
typedef struct {
	u16 DeviceId;   /**< Device ID of the sub-core */
	UINTPTR AbsAddr; /**< Absolute Base Address of the Sub-cores*/
} XDpRxSs_Hdcp22_Config;

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
	u16 IsPresent;  /**< Flag to indicate if sub-core is present in
			  the design*/
	XDpRxSs_Hdcp22_Config Hdcp22Config; /**< HDCP22 core configuration */
} XDpRxSs_Hdcp22SubCore;

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
 * Xhdcp22_Rx_DpcdFlag in xhdcp22_rx_i.h*/
typedef enum {
	XDPRX_HDCP22_XHDCP22_RX_DPCD_FLAG_NONE,		/**< Clear DPCD flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_INIT_RCVD,	/**< Ake_Init Msg Rcvd
								  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_NO_STORED_KM_RCVD,/**< Ake_No_Stored_Km
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_AKE_STORED_KM_RCVD,	/**< Ake_Stored_Km
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_LC_INIT_RCVD,		/**< Lc_Init Msg Rcvd
							  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_SKE_SEND_EKS_RCVD,	/**< Ske_Send_Eks
							  Msg Rcvd Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_HPRIME_READ_DONE,	/**< H' Msg Read done
							  Flag*/
	XDPRX_HDCP22_RX_DPCD_FLAG_PAIRING_INFO_READ_DONE,/**< Pairing info
							   Read done Flag*/
} XDpRxSs_Hdcp22DpcdEvents;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XDpRxSs_SubcoreInitHdcp22(void *InstancePtr);
void XDpRxSs_Hdcp22Poll(void *Instance);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /*#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)*/

#endif /* XDPRXSS_HDCP22_H_ */
