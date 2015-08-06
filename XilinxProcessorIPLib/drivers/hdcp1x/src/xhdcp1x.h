/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
#include "xstatus.h"
#include "xtmrctr.h"

/************************** Constant Definitions *****************************/

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
* This typedef contains configuration information for the HDCP core.
*/
typedef struct {
	u16 DeviceId;		/**< Device instance ID. */
	u32 BaseAddress;	/**< The base address of the core  */
	u32 SysFrequency;	/**< The main clock frequency of the core */
	u16 IsRx;	        /**< Flag indicating the core direction */
	u16 IsHDMI;	        /**< Flag indicating if the core is meant to
					work with HDMI. */
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
	const XHdcp1x_Config *CfgPtr;		/**< The cipher core config */
	u32 IsReady;				/**< The ready flag */
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
 * This typedef contains an instance of the HDCP port
 */
typedef struct XHdcp1x_PortStruct {
	const XHdcp1x_Config *CfgPtr;	/**< The cipher core config */
	const struct XHdcp1x_PortPhyIfAdaptorS *Adaptor; /**< Port adaptor */
	void *PhyIfPtr;			/**< The port's physical interface */
	u32 IsReady;			/**< The ready flag */
	XHdcp1x_Callback AuthCallback;	/**< (Re)Authentication callback */
	void *AuthRef;			/**< (Re)Authentication reference */
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
 * This typedef defines the elements that are common to both RX and TX HDCP
 * interfaces. The fields within this typedef must align with both the RX
 * and TX definitions
 */
typedef struct {
	const XHdcp1x_Config *CfgPtr;	/**< The cipher core config */
	u32 IsReady;			/**< The ready flag */
	XHdcp1x_Port Port;		/**< The interface's port */
} XHdcp1x_Common;

/**
 * This typedef contains the transmit HDCP interface
 */
typedef struct {
	const XHdcp1x_Config *CfgPtr;	/**< The cipher core config */
	u32 IsReady;			/**< The ready flag */
	XHdcp1x_Port Port;		/**< The interface's port */
	u32 CurrentState;		/**< The interface's current state */
	u32 PreviousState;		/**< The interface's previous state */
	u64 StateHelper;		/**< The interface's state helper */
	u16 Flags;			/**< The interface flags */
	u16 PendingEvents;		/**< The bit map of pending events */
	u64 EncryptionMap;		/**< The configured encryption map */
	XHdcp1x_TxStats Stats;		/**< The interface's statistics */
} XHdcp1x_Tx;

/**
 * This typedef contains the receive HDCP interface
 */
typedef struct {
	const XHdcp1x_Config *CfgPtr;	/**< The cipher core config */
	u32 IsReady;			/**< The ready flag */
	XHdcp1x_Port Port;		/**< The interface's port */
	u32 CurrentState;		/**< The interface's current state */
	u32 PreviousState;		/**< The interface's previous state */
	u16 Flags;			/**< The interface flags */
	u16 PendingEvents;		/**< The bit map of pending events */
	XHdcp1x_RxStats Stats;		/**< The interface's statistics */
} XHdcp1x_Rx;

/**
 * This typedef contains an instance of an HDCP interface
 */
typedef struct {
	XHdcp1x_Config Config;		/**< The core config */
	XHdcp1x_Cipher Cipher;		/**< The interface's cipher */
	union {
		XHdcp1x_Common Common;	/**< The common interface elements */
		XHdcp1x_Tx Tx;		/**< The transmit interface elements */
		XHdcp1x_Rx Rx;		/**< The receive interface elements */
	};
} XHdcp1x;

/**
 * This typedef defines the function interface that is to be used for checking
 * a specific KSV against the platforms revocation list
 */
typedef int (*XHdcp1x_KsvRevokeCheck)(const XHdcp1x *InstancePtr, u64 Ksv);

/**
 * This typedef defines the function interface that is to be used for starting
 * a one shot timer on behalf of an HDCP interface within the underlying
 * platform
 */
typedef int (*XHdcp1x_TimerStart)(const XHdcp1x *InstancePtr, u16 TmoInMs);

/**
 * This typedef defines the function interface that is to be used for stopping
 * a timer on behalf of an HDCP interface
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
		void *PhyIfPtr);

int XHdcp1x_SelfTest(XHdcp1x *InstancePtr);

int XHdcp1x_Poll(XHdcp1x *InstancePtr);

int XHdcp1x_Reset(XHdcp1x *InstancePtr);
int XHdcp1x_Enable(XHdcp1x *InstancePtr);
int XHdcp1x_Disable(XHdcp1x *InstancePtr);

int XHdcp1x_SetPhysicalState(XHdcp1x *InstancePtr, int IsUp);
int XHdcp1x_SetLaneCount(XHdcp1x *InstancePtr, int LaneCount);

int XHdcp1x_Authenticate(XHdcp1x *InstancePtr);
int XHdcp1x_IsInProgress(const XHdcp1x *InstancePtr);
int XHdcp1x_IsAuthenticated(const XHdcp1x *InstancePtr);

u64 XHdcp1x_GetEncryption(const XHdcp1x *InstancePtr);
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
