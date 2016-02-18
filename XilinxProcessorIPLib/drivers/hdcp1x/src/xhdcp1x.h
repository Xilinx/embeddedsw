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
* @file xhdcp1x.h
* @addtogroup hdcp1x_v3_0
* @{
* @details
*
* This header file contains the external declarations associated with the
* HDCP interface driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 2.00  als    09/30/15 Added EffectiveAddr argument to XHdcp1x_CfgInitialize.
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added constants:
*                       XHDCP1X_RPTR_MAX_CASCADE and
*                       XHDCP1X_RPTR_MAX_DEVS_COUNT.
*                       Added enumeration type:
*                       XHdcp1x_RepeaterStateMachineHandlerType.
*                       Added typedef data type XHdcp1x_Ksv.
*                       Added structure XHdcp1x_RepeaterExchange.
*                       Updated core structure XHdcp1x_Tx, XHdcp1x_Rx and
*                       XHdcp1x for Repeater support.
*                       Added following functions:
*                       XHdcp1x_DownstreamReady, XHdcp1x_GetRepeaterInfo,
*                       XHdcp1x_SetCallBack, XHdcp1x_ReadDownstream.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xhdcp1x_hw.h"
#include "xstatus.h"
#include "xtmrctr.h"

/************************** Constant Definitions *****************************/

#define XHDCP1X_RPTR_MAX_CASCADE	4	/**< Maximum depth that the
						  *  Repeater can support on
						  *  the downstream
						  *  interface */
#define XHDCP1X_RPTR_MAX_DEVS_COUNT	32	/**< Maximum devices that can
						  *  be cascaded to the
						  *  Repeater */

/**************************** Type Definitions *******************************/

/**
 * This typedef defines the callback interface that is to be used for
 * interrupts within this driver
 */
typedef void (*XHdcp1x_Callback)(void *CallbackRef);

/**
 * This typedef defines the function interface that is to be used for debug
 * print statements within this driver
 */
typedef void (*XHdcp1x_Printf)(const char *fmt, ...);

/**
 * This typedef defines the function interface that is to be used for debug
 * log message statements within this driver
 */
typedef void (*XHdcp1x_LogMsg)(const char *fmt, ...);

/**
 * This enumerates the call back for the HDCP Repeater Tx state machine
 */
typedef enum {
	XHDCP1X_RPTR_HDLR_DOWNSTREAM_READY = 1,
	XHDCP1X_RPTR_HDLR_REPEATER_EXCHANGE,
	XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
} XHdcp1x_RepeaterStateMachineHandlerType;

/**
* This typedef contains configuration information for the HDCP core.
*/
typedef struct {
	u16 DeviceId;		/**< Device instance ID. */
	u32 BaseAddress;	/**< The base address of the core  */
	u32 SysFrequency;	/**< The main clock frequency of the core */
	u16 IsRx;		/**< Flag indicating the core direction */
	u16 IsHDMI;		/**< Flag indicating if the core is meant to
				  *  work with HDMI. */
} XHdcp1x_Config;

/**
* This typedef defines the statistics collected by a cipher instance
*/
typedef struct {
	u32 IntCount;		/**< The number of interrupts detected */
} XHdcp1x_CipherStats;

/**
* This typedef contains an instance of the HDCP cipher core
*/
typedef struct {
	XHdcp1x_Callback LinkFailCallback;	/**< Link fail callback */
	void *LinkFailRef;			/**< Link fail reference */
	u32 IsLinkFailCallbackSet;		/**< Link fail config flag */
	XHdcp1x_Callback RiUpdateCallback;	/**< Ri update callback */
	void *RiUpdateRef;			/**< Ri update reference */
	u32 IsRiUpdateCallbackSet;		/**< Ri update config flag */
	XHdcp1x_CipherStats Stats;		/**< Cipher statistics */
} XHdcp1x_Cipher;

/**
 * This typedef defines the statistics collected by a port instance
 */
typedef struct {
	u32 IntCount;		/**< The number of interrupts detected */
} XHdcp1x_PortStats;

/**
 * Forward declaration of a structure defined within xhdcxp1x_port.h
 */
struct XHdcp1x_PortPhyIfAdaptorS;

/**
 * This typedef defines a memory to store a Key Selection Vector (KSV)
 */
typedef u64 XHdcp1x_Ksv;

/**
 * This typedef contains an instance of the HDCP Repeater values to
 * exchanged between HDCP Tx and HDCP Rx
 */
typedef struct {
	u32 V[5];	/**< 20 bytes value of SHA 1 hash, V'H0, V'H1, V'H2 ,
			  *  V'H3 ,V'H4 read from the downstream Repeater*/
	XHdcp1x_Ksv KsvList[32];	/**< An array of 32 elements each
					  *  of 64 bits to store the KSVs
					  *  for the KSV FIFO*/
	u8 Depth;	/**< Depth of the Repeater's downstream topology*/
	u8 DeviceCount;		/**< Number of downstream devices attached
				  *  to the Repeater*/
} XHdcp1x_RepeaterExchange;

/**
 * This typedef contains an instance of the HDCP port
 */
typedef struct XHdcp1x_PortStruct {
	const struct XHdcp1x_PortPhyIfAdaptorS *Adaptor;/**< Port adaptor */
	void *PhyIfPtr;		/**< The port's physical interface */
	XHdcp1x_Callback AuthCallback;	/**< (Re)Authentication callback */
	void *AuthRef;		/**< (Re)Authentication reference */
	u32 IsAuthCallbackSet;		/**< (Re)Authentication config flag */
	XHdcp1x_PortStats Stats;	/**< Port statistics */
} XHdcp1x_Port;

/**
 * This typedef defines the statistics collected transmit port instance
 */
typedef struct {
	u32 AuthFailed;		/**< Num of failed authentication attempts */
	u32 AuthPassed;		/**< Num of passed authentication attempts */
	u32 ReauthRequested;	/**< Num of rxd re-authentication requests */
	u32 ReadFailures;	/**< Num of remote read failures */
	u32 LinkCheckPassed;	/**< Num of link verifications that passed */
	u32 LinkCheckFailed;	/**< Num of link verifications that failed */
} XHdcp1x_TxStats;

/**
 * This typedef defines the statistics collected receive port instance
 */
typedef struct {
	u32 AuthAttempts;	/**< Num of rxd authentication requests */
	u32 LinkFailures;	/**< Num of link verifications that failed */
	u32 RiUpdates;		/**< Num of Ri updates performed */
} XHdcp1x_RxStats;

/**
 * This typedef contains the transmit HDCP interface
 */
typedef struct {
	u32 CurrentState;	/**< The interface's current state */
	u32 PreviousState;	/**< The interface's previous state */
	u64 StateHelper;	/**< The interface's state helper */
	u16 Flags;		/**< The interface flags */
	u16 PendingEvents;	/**< The bit map of pending events */
	u64 EncryptionMap;	/**< The configured encryption map */
	XHdcp1x_TxStats Stats;	/**< The interface's statistics */
	XHdcp1x_Callback DownstreamReadyCallback;/**< (Repeater)Downstream
						   *  Ready callback */
	void *DownstreamReadyRef;	/**< (Repeater)Downstream Ready
					  *  reference */
	u32 IsDownStreamReadyCallbackSet;	/**< (Repeater)Check to
						  *  determine if Downstream
						  *  Ready callback is set
						  *  flag*/
	XHdcp1x_Callback RepeaterExchangeCallback;/**< (Repeater)Exchange
						    *  Repeater Values
						    *  callback */
	void *RepeaterExchangeRef;	/**< (Repeater)Exchange Repeater
					  *  Values reference */
	u32 IsRepeaterExchangeCallbackSet;	/**< (Repeater)Check to
						  *  determine if Exchange
						  *  Repeater Values
						  *  callback is set flag */
	u16 DownstreamReady;/**< The downstream interface's status flag */
} XHdcp1x_Tx;

/**
 * This typedef contains the receive HDCP interface
 */
typedef struct {
	u32 CurrentState;	/**< The interface's current state */
	u32 PreviousState;	/**< The interface's previous state */
	u16 Flags;		/**< The interface flags */
	u16 PendingEvents;	/**< The bit map of pending events */
	XHdcp1x_RxStats Stats;	/**< The interface's statistics */
	XHdcp1x_Callback RepeaterDownstreamAuthCallback;/**< (Repeater)Post
							  *  authenticate to
							  *  downstream,
							  *  second part of
							  *  authentication
							  *  start callback */
	void *RepeaterDownstreamAuthRef;/**< (Repeater)Post authenticate to
					  *  downstream, second part of
					  *  authentication start reference */
	u32 IsRepeaterDownStreamAuthCallbackSet;/**< (Repeater)Is "Post
						  *  authenticate to
						  *  downstream to trigger
						  *  second part of
						  *  authentication" callback
						  *  set flag*/
} XHdcp1x_Rx;

/**
 * This typedef contains an instance of an HDCP interface
 */
typedef struct {
	XHdcp1x_Config Config;	/**< The core config */
	XHdcp1x_Cipher Cipher;	/**< The interface's cipher */
	XHdcp1x_Port Port;	/**< The interface's port */
	union {
		XHdcp1x_Tx Tx;	/**< The transmit interface elements */
		XHdcp1x_Rx Rx;	/**< The receive interface elements */
	};
	u32 IsReady;		/**< The ready flag */
	u32 IsRepeater;		/**< The IsRepeater flag determines if the
				  *  HDCP is part of a Repeater system
				  *  or a standalone interface */
	XHdcp1x_RepeaterExchange RepeaterValues;/**< The Repeater value to
						  *  be exchanged between
						  *  Tx and Rx */
	void *Hdcp1xRef;		/**< A void reference pointer for
					  *  association of a external core
					  *  in our case a timer with the
					  *  hdcp instance */
} XHdcp1x;

/**
 * This typedef defines the function interface that is to be used
 * for checking a specific KSV against the platforms revocation list
 */
typedef int (*XHdcp1x_KsvRevokeCheck)(const XHdcp1x *InstancePtr, u64 Ksv);

/**
 * This typedef defines the function interface that is to be used
 * for starting a one shot timer on behalf of an HDCP interface within
 * the underlying platform
 */
typedef int (*XHdcp1x_TimerStart)(const XHdcp1x *InstancePtr, u16 TmoInMs);

/**
 * This typedef defines the function interface that is to be used
 * for stopping a timer on behalf of an HDCP interface
 */
typedef int (*XHdcp1x_TimerStop)(const XHdcp1x *InstancePtr);

/**
 * This typedef defines the function interface that is to be used for
 * performing a busy delay on behalf of an HDCP interface
 */
typedef int (*XHdcp1x_TimerDelay)(const XHdcp1x *InstancePtr, u16 DelayInMs);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

XHdcp1x_Config *XHdcp1x_LookupConfig(u16 DeviceId);

int XHdcp1x_CfgInitialize(XHdcp1x *InstancePtr, const XHdcp1x_Config *CfgPtr,
		void *PhyIfPtr, u32 EffectiveAddr);

int XHdcp1x_SelfTest(XHdcp1x *InstancePtr);

int XHdcp1x_Poll(XHdcp1x *InstancePtr);

int XHdcp1x_DownstreamReady(XHdcp1x *InstancePtr);
int XHdcp1x_GetRepeaterInfo(XHdcp1x *InstancePtr,
		XHdcp1x_RepeaterExchange *RepeaterInfoPtr);
u32 XHdcp1x_SetCallBack(XHdcp1x *InstancePtr, u32 HandlerType,
		XHdcp1x_Callback CallBackFunc, void *CallBackRef);

int XHdcp1x_Reset(XHdcp1x *InstancePtr);
int XHdcp1x_Enable(XHdcp1x *InstancePtr);
int XHdcp1x_Disable(XHdcp1x *InstancePtr);

int XHdcp1x_SetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_SetLaneCount(XHdcp1x *InstancePtr, int LaneCount);

int XHdcp1x_Authenticate(XHdcp1x *InstancePtr);
int XHdcp1x_ReadDownstream(XHdcp1x *InstancePtr);
int XHdcp1x_IsInProgress(const XHdcp1x *InstancePtr);
int XHdcp1x_IsAuthenticated(const XHdcp1x *InstancePtr);

u64 XHdcp1x_GetEncryption(const XHdcp1x *InstancePtr);
int XHdcp1x_IsEncrypted(const XHdcp1x *InstancePtr);
int XHdcp1x_EnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
int XHdcp1x_DisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);

int XHdcp1x_SetKeySelect(XHdcp1x *InstancePtr, u8 KeySelect);

void XHdcp1x_HandleTimeout(void *InstancePtr);
void XHdcp1x_CipherIntrHandler(void *InstancePtr);
void XHdcp1x_PortIntrHandler(void *InstancePtr, u32 IntCause);

void XHdcp1x_SetDebugPrintf(XHdcp1x_Printf PrintfFunc);
void XHdcp1x_SetDebugLogMsg(XHdcp1x_LogMsg LogFunc);

void XHdcp1x_SetKsvRevokeCheck(XHdcp1x_KsvRevokeCheck RevokeCheckFunc);
void XHdcp1x_SetTimerStart(XHdcp1x_TimerStart TimerStartFunc);
void XHdcp1x_SetTimerStop(XHdcp1x_TimerStop TimerStopFunc);
void XHdcp1x_SetTimerDelay(XHdcp1x_TimerDelay TimerDelayFunc);

u32 XHdcp1x_GetDriverVersion(void);
u32 XHdcp1x_GetVersion(const XHdcp1x *InstancePtr);
void XHdcp1x_Info(const XHdcp1x *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_H */
/** @} */
