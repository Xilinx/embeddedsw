/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
#include "xhdcp22_tx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/

#define XDPTXSS_HDCP_MAX_QUEUE_SIZE	   16

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for the HDCP22 core.
*/
typedef struct {
	u16 DeviceId;   /**< Device ID of the sub-core */
	UINTPTR AbsAddr; /**< Absolute Base Address of the Sub-cores*/
} XDpTxSs_Hdcp22_Config;

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
	u16 IsPresent;  /**< Flag to indicate if sub-core is present
			  in the design*/
	XDpTxSs_Hdcp22_Config Hdcp22Config; /**< HDCP22 core configuration */
} XDpTxSs_Hdcp22SubCore;

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
int XDpTxSs_HdcpAuthRequest(void *Instance);
int XDpTxSs_HdcpPoll(void *Instance);
int XDpTxSs_HdcpPushEvent(void *Instance, XDpTxSs_HdcpEvent Event);
u8 XDpTxSs_IsSinkHdcp22Capable(void *Instance);

#endif /*(XPAR_XHDCP22_TX_NUM_INSTANCES > 0)*/

#ifdef __cplusplus
}
#endif

#endif /* HDCP22_XDPTXSS_HDCP22_H_ */
