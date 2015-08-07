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
* @file xhdcp1x_tx.c
*
* This contains the main implementation file for the Xilinx HDCP transmit
* state machine
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_debug.h"
#include "xhdcp1x_platform.h"
#include "xhdcp1x_port.h"
#if defined(XPAR_XHDMI_TX_NUM_INSTANCES) && (XPAR_XHDMI_TX_NUM_INSTANCES > 0)
#include "xhdcp1x_port_hdmi.h"
#else
#include "xhdcp1x_port_dp.h"
#endif
#include "xhdcp1x_tx.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

#define FLAG_PHY_UP		(1u << 0)  /**< Flag to track physical state */
#define FLAG_IS_REPEATER	(1u << 1)  /**< Flag to track repeater state */

#define TMO_5MS			(   5u)    /**< Timeout value for 5ms */
#define TMO_100MS		( 100u)    /**< Timeout value for 100ms */
#define TMO_1SECOND		(1000u)    /**< Timeout value for 1s */

/**************************** Type Definitions *******************************/

typedef enum {
	EVENT_NULL,
	EVENT_AUTHENTICATE,
	EVENT_CHECK,
	EVENT_DISABLE,
	EVENT_ENABLE,
	EVENT_LINKDOWN,
	EVENT_PHYDOWN,
	EVENT_PHYUP,
	EVENT_POLL,
	EVENT_TIMEOUT,
} tEvent;

typedef enum {
	STATE_DISABLED,
	STATE_DETERMINERXCAPABLE,
	STATE_EXCHANGEKSVS,
	STATE_COMPUTATIONS,
	STATE_VALIDATERX,
	STATE_AUTHENTICATED,
	STATE_LINKINTEGRITYCHECK,
	STATE_TESTFORREPEATER,
	STATE_WAITFORREADY,
	STATE_READKSVLIST,
	STATE_UNAUTHENTICATED,
	STATE_PHYDOWN,
} tState;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* This queries an interface to determine if it is Display Port (DP).
*
* @param	InstancePtr is the instance to query.
*
* @return	Truth value indicating DP (TRUE) or not (FALSE).
*
* @note		None.
*
******************************************************************************/
#define IsDP(InstancePtr)  		(!InstancePtr->Config.IsHDMI)

/*****************************************************************************/
/**
* This queries an interface to determine if it is HDMI.
*
* @param	InstancePtr is the instance to query.
*
* @return	Truth value indicating HDMI (TRUE) or not (FALSE).
*
* @note		None.
*
******************************************************************************/
#define IsHDMI(InstancePtr)  		(InstancePtr->Config.IsHDMI)

/*************************** Function Prototypes *****************************/

static void DebugLog(const XHdcp1x *InstancePtr, const char *LogMsg);
static void PostEvent(XHdcp1x *InstancePtr, tEvent Event);
static void StartTimer(XHdcp1x *InstancePtr, u16 TimeoutInMs);
static void StopTimer(XHdcp1x *InstancePtr);
static void BusyDelay(XHdcp1x *InstancePtr, u16 DelayInMs);
static void ReauthenticateCallback(void *Parameter);
static void CheckLinkCallback(void *Parameter);
static void SetCheckLinkState(XHdcp1x *InstancePtr, int IsEnabled);
static void EnableEncryption(XHdcp1x *InstancePtr);
static void DisableEncryption(XHdcp1x *InstancePtr);
static void Enable(XHdcp1x *InstancePtr);
static void Disable(XHdcp1x *InstancePtr);
static void CheckRxCapable(const XHdcp1x *InstancePtr, tState *NextStatePtr);
static u64 GenerateAn(XHdcp1x *InstancePtr);
static int IsKsvValid(u64 Ksv);
static void ExchangeKsvs(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void StartComputations(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void PollForComputations(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void ValidateRx(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void CheckLinkIntegrity(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void TestForRepeater(XHdcp1x *InstancePtr, tState *NextStatePtr);
static void PollForWaitForReady(XHdcp1x *InstancePtr, tState *NextStatePtr);
static int ValidateKsvList(XHdcp1x *InstancePtr, u16 RepeaterInfo);
static void ReadKsvList(XHdcp1x *InstancePtr, tState *NextStatePtr);
static int IsAuthenticated(const XHdcp1x *InstancePtr);
static void RunDisabledState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunDetermineRxCapableState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunExchangeKsvsState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunComputationsState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunValidateRxState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunAuthenticatedState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunLinkIntegrityCheckState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunTestForRepeaterState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunWaitForReadyState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunReadKsvListState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunUnauthenticatedState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void RunPhysicalLayerDownState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr);
static void EnterState(XHdcp1x *InstancePtr, tState State,
		tState *NextStatePtr);
static void ExitState(XHdcp1x *InstancePtr, tState State);
static void DoTheState(XHdcp1x *InstancePtr, tEvent Event);
static void ProcessPending(XHdcp1x *InstancePtr);
static const char *StateToString(tState State);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes a transmit state machine.
*
* @param	InstancePtr is the receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_TxInit(XHdcp1x *InstancePtr)
{
	tState DummyState = STATE_DISABLED;

	/* Update theHandler */
	InstancePtr->Tx.PendingEvents = 0;

	/* Kick the state machine */
	EnterState(InstancePtr, STATE_DISABLED, &DummyState);
}

/*****************************************************************************/
/**
* This function polls an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxPoll(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Process any pending events */
	ProcessPending(InstancePtr);

	/* Poll it */
	DoTheState(InstancePtr, EVENT_POLL);

	return (Status);
}

/*****************************************************************************/
/**
* This function resets an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		This function disables and then re-enables the interface.
*
******************************************************************************/
int XHdcp1x_TxReset(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Reset it */
	PostEvent(InstancePtr, EVENT_DISABLE);
	PostEvent(InstancePtr, EVENT_ENABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function enables an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxEnable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post it */
	PostEvent(InstancePtr, EVENT_ENABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function disables an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxDisable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post it */
	PostEvent(InstancePtr, EVENT_DISABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function updates the physical state of an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
* @param	IsUp is truth value indicating the status of physical interface.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxSetPhysicalState(XHdcp1x *InstancePtr, int IsUp)
{
	int Status = XST_SUCCESS;
	tEvent Event = EVENT_PHYDOWN;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Event */
	if (IsUp) {
		Event = EVENT_PHYUP;
	}

	/* Post it */
	PostEvent(InstancePtr, Event);

	return (Status);
}

/*****************************************************************************/
/**
* This function set the lane count of an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
* @param	LaneCount is the number of lanes of the interface.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxSetLaneCount(XHdcp1x *InstancePtr, int LaneCount)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(LaneCount > 0);

	/* Set it */
	return (XHdcp1x_CipherSetNumLanes(InstancePtr, LaneCount));
}

/*****************************************************************************/
/**
* This function initiates authentication on an interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxAuthenticate(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post the re-authentication request */
	PostEvent(InstancePtr, EVENT_AUTHENTICATE);

	return (Status);
}

/*****************************************************************************/
/**
* This function queries an interface to check if authentication is still in
* progress.
*
* @param	InstancePtr is the transmitter instance.
*
* @return	Truth value indicating in progress (true) or not (false).
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxIsInProgress(const XHdcp1x *InstancePtr)
{
	int IsInProgress = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Which state? */
	switch (InstancePtr->Tx.CurrentState) {
		/* For the "steady" states */
		case STATE_DISABLED:
		case STATE_UNAUTHENTICATED:
		case STATE_AUTHENTICATED:
		case STATE_LINKINTEGRITYCHECK:
			IsInProgress = FALSE;
			break;

		/* Otherwise */
		default:
			IsInProgress = TRUE;
			break;
	}

	return (IsInProgress);
}

/*****************************************************************************/
/**
* This function queries an interface to check if its been authenticated.
*
* @param	InstancePtr is the transmitter instance.
*
* @return	Truth value indicating authenticated (true) or not (false).
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxIsAuthenticated(const XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (IsAuthenticated(InstancePtr));
}

/*****************************************************************************/
/**
* This function retrieves the current encryption stream map.
*
* @param	InstancePtr the transmitter instance.
*
* @return	The current encryption stream map.
*
* @note		None.
*
******************************************************************************/
u64 XHdcp1x_TxGetEncryption(const XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Tx.EncryptionMap);
}

/*****************************************************************************/
/**
* This function enables encryption on set of streams on an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
* @param	StreamMap is the bit map of streams to enable encryption on.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxEnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Update InstancePtr */
	InstancePtr->Tx.EncryptionMap |= StreamMap;

	/* Check for authenticated */
	if (IsAuthenticated(InstancePtr)) {
		EnableEncryption(InstancePtr);
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function disables encryption on set of streams on an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
* @param	StreamMap is the bit map of streams to disable encryption on.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxDisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Disable it */
	Status = XHdcp1x_CipherDisableEncryption(InstancePtr, StreamMap);

	/* Update InstancePtr */
	if (Status == XST_SUCCESS) {
		InstancePtr->Tx.EncryptionMap &= ~StreamMap;
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function handles a timeout on an HDCP interface.
*
* @param	InstancePtr is the transmitter instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_TxHandleTimeout(XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Post the timeout */
	PostEvent(InstancePtr, EVENT_TIMEOUT);
}

/*****************************************************************************/
/**
* This function implements the debug display output for transmit instances.
*
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_TxInfo(const XHdcp1x *InstancePtr)
{
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Display it */
	XHDCP1X_DEBUG_PRINTF("Type:            ");
	if (IsHDMI(InstancePtr)) {
		XHDCP1X_DEBUG_PRINTF("hdmi-tx\r\n");
	}
	else {
		XHDCP1X_DEBUG_PRINTF("dp-tx\r\n");
	}
	XHDCP1X_DEBUG_PRINTF("Current State:   %s\r\n",
			StateToString(InstancePtr->Tx.CurrentState));
	XHDCP1X_DEBUG_PRINTF("Previous State:  %s\r\n",
			StateToString(InstancePtr->Tx.PreviousState));
	XHDCP1X_DEBUG_PRINTF("State Helper:    %016llX\r\n",
			InstancePtr->Tx.StateHelper);
	XHDCP1X_DEBUG_PRINTF("Flags:           %04X\r\n",
			InstancePtr->Tx.Flags);
	XHDCP1X_DEBUG_PRINTF("Encryption Map:  %016llX\r\n",
			InstancePtr->Tx.EncryptionMap);
	Version = XHdcp1x_GetDriverVersion();
	XHDCP1X_DEBUG_PRINTF("Driver Version:  %d.%02d.%02d\r\n",
			((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
			(Version & 0xFFu));
	Version = XHdcp1x_CipherGetVersion(InstancePtr);
	XHDCP1X_DEBUG_PRINTF("Cipher Version:  %d.%02d.%02d\r\n",
			((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
			(Version & 0xFFu));
	XHDCP1X_DEBUG_PRINTF("\r\n");
	XHDCP1X_DEBUG_PRINTF("Tx Stats\r\n");
	XHDCP1X_DEBUG_PRINTF("Auth Passed:     %d\r\n",
			InstancePtr->Tx.Stats.AuthPassed);
	XHDCP1X_DEBUG_PRINTF("Auth Failed:     %d\r\n",
			InstancePtr->Tx.Stats.AuthFailed);
	XHDCP1X_DEBUG_PRINTF("Reauth Requests: %d\r\n",
			InstancePtr->Tx.Stats.ReauthRequested);
	XHDCP1X_DEBUG_PRINTF("Check Passed:    %d\r\n",
			InstancePtr->Tx.Stats.LinkCheckPassed);
	XHDCP1X_DEBUG_PRINTF("Check Failed:    %d\r\n",
			InstancePtr->Tx.Stats.LinkCheckFailed);
	XHDCP1X_DEBUG_PRINTF("Read Failures:   %d\r\n",
			InstancePtr->Tx.Stats.ReadFailures);

	XHDCP1X_DEBUG_PRINTF("\r\n");
	XHDCP1X_DEBUG_PRINTF("Cipher Stats\r\n");
	XHDCP1X_DEBUG_PRINTF("Int Count:       %d\r\n",
			InstancePtr->Cipher.Stats.IntCount);

	XHDCP1X_DEBUG_PRINTF("\r\n");
	XHDCP1X_DEBUG_PRINTF("Port Stats\r\n");
	XHDCP1X_DEBUG_PRINTF("Int Count:       %d\r\n",
			InstancePtr->Port.Stats.IntCount);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function logs a debug message on behalf of a handler state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	LogMsg is the message to log.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DebugLog(const XHdcp1x *InstancePtr, const char *LogMsg)
{
	char Label[16];

	/* Format Label */
	snprintf(Label, 16, "hdcp-tx(%d) - ", InstancePtr->Config.DeviceId);

	/* Log it */
	XHDCP1X_DEBUG_LOGMSG(Label);
	XHDCP1X_DEBUG_LOGMSG(LogMsg);
	XHDCP1X_DEBUG_LOGMSG("\r\n");
}

/*****************************************************************************/
/**
* This function posts an event to a state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to post.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void PostEvent(XHdcp1x *InstancePtr, tEvent Event)
{
	/* Check for disable and clear any pending enable */
	if (Event == EVENT_DISABLE) {
		InstancePtr->Tx.PendingEvents &= ~(1u << EVENT_ENABLE);
	}
	/* Check for phy-down and clear any pending phy-up */
	else if (Event == EVENT_PHYDOWN) {
		InstancePtr->Tx.PendingEvents &= ~(1u << EVENT_PHYUP);
	}

	/* Post it */
	InstancePtr->Tx.PendingEvents |= (1u << Event);
}

/*****************************************************************************/
/**
* This function starts a state machine's timer.
*
* @param	InstancePtr is the state machine.
* @param	TimeoutInMs is the timeout in milli-seconds.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StartTimer(XHdcp1x *InstancePtr, u16 TimeoutInMs)
{
	/* Start it */
	XHdcp1x_PlatformTimerStart(InstancePtr, TimeoutInMs);
}

/*****************************************************************************/
/**
* This function stops a state machine's timer.
*
* @param	InstancePtr is the state machine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StopTimer(XHdcp1x *InstancePtr)
{
	/* Stop it */
	XHdcp1x_PlatformTimerStop(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function busy delays a state machine.
*
* @param	InstancePtr is the state machine.
* @param	TimeoutInMs is the delay time in milli-seconds.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void BusyDelay(XHdcp1x *InstancePtr, u16 DelayInMs)
{
	/* Busy wait */
	XHdcp1x_PlatformTimerBusy(InstancePtr, DelayInMs);
}

/*****************************************************************************/
/**
* This function acts as the reauthentication callback for a state machine.
*
* @param	Parameter is the parameter specified during registration.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ReauthenticateCallback(void *Parameter)
{
	XHdcp1x *InstancePtr = Parameter;

	/* Update statistics */
	InstancePtr->Tx.Stats.ReauthRequested++;

	/* Post the re-authentication request */
	PostEvent(InstancePtr, EVENT_AUTHENTICATE);
}

/*****************************************************************************/
/**
* This function acts as the check link callback for a state machine.
*
* @param	Parameter is the parameter specified during registration.
*
* @return	Mpme/
*
* @note		None.
*
******************************************************************************/
static void CheckLinkCallback(void *Parameter)
{
	XHdcp1x *InstancePtr = Parameter;

	/* Post the check request */
	PostEvent(InstancePtr, EVENT_CHECK);
}

/*****************************************************************************/
/**
* This function sets the check link state of the handler.
*
* @param	InstancePtr is the HDCP state machine.
* @param	IsEnabled is truth value indicating on/off.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SetCheckLinkState(XHdcp1x *InstancePtr, int IsEnabled)
{
	/* Check for HDMI */
	if (IsHDMI(InstancePtr)) {
		/* Check for enabled */
		if (IsEnabled) {
			/* Register Callback */
			XHdcp1x_CipherSetCallback(InstancePtr,
					XHDCP1X_CIPHER_HANDLER_Ri_UPDATE,
					&CheckLinkCallback, InstancePtr);

			/* Enable it */
			XHdcp1x_CipherSetRiUpdate(InstancePtr, TRUE);
		}
		/* Otherwise */
		else {
			/* Disable it */
			XHdcp1x_CipherSetRiUpdate(InstancePtr, FALSE);
		}
	}
}

/*****************************************************************************/
/**
* This function enables encryption for a state machine.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	None.
*
* @note		This function inserts a 5ms delay for things to settle when
*		encryption is actually being disabled.
*
******************************************************************************/
static void EnableEncryption(XHdcp1x *InstancePtr)
{
	/* Check for encryption enabled */
	if (InstancePtr->Tx.EncryptionMap != 0) {
		u64 StreamMap = 0;

		/* Determine StreamMap */
		StreamMap =
			XHdcp1x_CipherGetEncryption(InstancePtr);

		/* Check if there is something to do */
		if (StreamMap != InstancePtr->Tx.EncryptionMap) {
			/* Wait a bit */
			BusyDelay(InstancePtr, TMO_5MS);

			/* Enable it */
			XHdcp1x_CipherEnableEncryption(InstancePtr,
					InstancePtr->Tx.EncryptionMap);
		}
	}
}

/*****************************************************************************/
/**
* This function disables encryption for a state machine.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	None.
*
* @note		This function inserts a 5ms delay for things to settle when
*		encryption is actually being disabled.
*
******************************************************************************/
static void DisableEncryption(XHdcp1x *InstancePtr)
{
	u64 StreamMap = XHdcp1x_CipherGetEncryption(InstancePtr);

	/* Check if encryption actually enabled */
	if (StreamMap != 0) {
		/* Update StreamMap for all stream */
		StreamMap = (u64)(-1);

		/* Disable it all */
		XHdcp1x_CipherDisableEncryption(InstancePtr, StreamMap);

		/* Wait at least a frame */
		BusyDelay(InstancePtr, TMO_5MS);
	}
}

/*****************************************************************************/
/**
* This function enables a state machine.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Enable(XHdcp1x *InstancePtr)
{
	/* Clear statistics */
	memset(&(InstancePtr->Tx.Stats), 0, sizeof(InstancePtr->Tx.Stats));

	/* Enable the crypto engine */
	XHdcp1x_CipherEnable(InstancePtr);

	/* Register the re-authentication callback */
	XHdcp1x_PortSetCallback(InstancePtr,
			XHDCP1X_PORT_HANDLER_AUTHENTICATE,
			&ReauthenticateCallback, InstancePtr);

	/* Enable the hdcp port */
	XHdcp1x_PortEnable(InstancePtr);
}

/*****************************************************************************/
/**
* This function disables a state machine.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Disable(XHdcp1x *InstancePtr)
{
	/* Disable the hdcp port */
	XHdcp1x_PortDisable(InstancePtr);

	/* Disable the cryto engine */
	XHdcp1x_CipherDisable(InstancePtr);

	/* Disable the timer */
	StopTimer(InstancePtr);

	/* Update InstancePtr */
	InstancePtr->Tx.Flags &= ~FLAG_IS_REPEATER;
	InstancePtr->Tx.StateHelper = 0;
	InstancePtr->Tx.EncryptionMap = 0;
}

/*****************************************************************************/
/**
* This function checks to ensure that the remote end is HDCP capable.
*
* @param	InstancePtr is the HDCP state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void CheckRxCapable(const XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	/* Check for capable */
	if (XHdcp1x_PortIsCapable(InstancePtr)) {
		/* Log */
		DebugLog(InstancePtr, "rx hdcp capable");

		/* Update NextStatePtr */
		*NextStatePtr = STATE_EXCHANGEKSVS;
	}
	else {
		/* Log */
		DebugLog(InstancePtr, "rx not capable");

		/* Update NextStatePtr */
		*NextStatePtr = STATE_UNAUTHENTICATED;
	}
}

/*****************************************************************************/
/**
* This function generates the An from a random number generator.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	A 64-bit pseudo random number (An).
*
* @note		None.
*
******************************************************************************/
static u64 GenerateAn(XHdcp1x *InstancePtr)
{
	u64 An = 0;

	/* Attempt to generate An */
	if (XHdcp1x_CipherDoRequest(InstancePtr, XHDCP1X_CIPHER_REQUEST_RNG) ==
			XST_SUCCESS) {
		/* Wait until done */
		while (!XHdcp1x_CipherIsRequestComplete(InstancePtr));

		/* Update theAn */
		An = XHdcp1x_CipherGetMi(InstancePtr);
	}

	/* Check if zero */
	if (An == 0) {
		An = 0x351F7175406A74Dull;
	}

	return (An);
}

/*****************************************************************************/
/**
* This function validates a KSV value as having 20 1s and 20 0s.
*
* @param	Ksv is the value to validate.
*
* @return	Truth value indicating valid (TRUE) or not (FALSE).
*
* @note		None.
*
******************************************************************************/
static int IsKsvValid(u64 Ksv)
{
	int IsValid = FALSE;
	int NumOnes = 0;

	/* Determine NumOnes */
	while (Ksv != 0) {
		if ((Ksv & 1) != 0) {
			NumOnes++;
		}
		Ksv >>= 1;
	}

	/* Check for 20 1s */
	if (NumOnes == 20) {
		IsValid = TRUE;
	}

	return (IsValid);
}

/*****************************************************************************/
/**
* This function exchanges the ksvs between the two ends of the link.
*
* @param	InstancePtr is the HDCP state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ExchangeKsvs(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	u8 Buf[8];

	/* Initialize Buf */
	memset(Buf, 0, 8);

	/* Update NextStatePtr - assume failure */
	*NextStatePtr = STATE_UNAUTHENTICATED;

	/* Read the Bksv from remote end */
	if (XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_BKSV,
			Buf, 5) > 0) {
		u64 RemoteKsv = 0;

		/* Determine theRemoteKsv */
		XHDCP1X_PORT_BUF_TO_UINT(RemoteKsv, Buf,
				XHDCP1X_PORT_SIZE_BKSV * 8);

		/* Check for invalid */
		if (!IsKsvValid(RemoteKsv)) {
			DebugLog(InstancePtr, "Bksv invalid");
		}
		/* Check for revoked */
		else if (XHdcp1x_PlatformIsKsvRevoked(InstancePtr, RemoteKsv)) {
			DebugLog(InstancePtr, "Bksv is revoked");
		}
		/* Otherwise we're good to go */
		else {
			u64 LocalKsv = 0;
			u64 An = 0;

			/* Check for repeater and update InstancePtr */
			if (XHdcp1x_PortIsRepeater(InstancePtr)) {
				InstancePtr->Tx.Flags |= FLAG_IS_REPEATER;
			}
			else {
				InstancePtr->Tx.Flags &= ~FLAG_IS_REPEATER;
			}

			/* Generate theAn */
			An = GenerateAn(InstancePtr);

			/* Save theAn into the state helper for use later */
			InstancePtr->Tx.StateHelper = An;

			/* Determine theLocalKsv */
			LocalKsv = XHdcp1x_CipherGetLocalKsv(InstancePtr);

			/* Load the cipher with the remote ksv */
			XHdcp1x_CipherSetRemoteKsv(InstancePtr, RemoteKsv);

			/* Send An to remote */
			XHDCP1X_PORT_UINT_TO_BUF(Buf, An,
					XHDCP1X_PORT_SIZE_AN * 8);
			XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AN,
					Buf, XHDCP1X_PORT_SIZE_AN);

			/* Send AKsv to remote */
			XHDCP1X_PORT_UINT_TO_BUF(Buf, LocalKsv,
					XHDCP1X_PORT_SIZE_AKSV * 8);
			XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AKSV,
					Buf, XHDCP1X_PORT_SIZE_AKSV);

			/* Update NextStatePtr */
			*NextStatePtr = STATE_COMPUTATIONS;
		}
	}
	/* Otherwise */
	else {
		/* Update the statistics */
		InstancePtr->Tx.Stats.ReadFailures++;
	}
}

/*****************************************************************************/
/**
* This function initiates the computations for a state machine.
*
* @param	InstancePtr is the HDCP receiver state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StartComputations(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	u64 Value = 0;
	u32 X = 0;
	u32 Y = 0;
	u32 Z = 0;

	/* Log */
	DebugLog(InstancePtr, "starting computations");

	/* Update Value with An */
	Value = InstancePtr->Tx.StateHelper;

	/* Load the cipher B registers with An */
	X = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Y = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Z = (u32) (Value & 0x000000FFul);
	if ((InstancePtr->Tx.Flags & FLAG_IS_REPEATER) != 0) {
		Z |= (1ul << 8);
	}
	XHdcp1x_CipherSetB(InstancePtr, X, Y, Z);

	/* Initiate the block cipher */
	XHdcp1x_CipherDoRequest(InstancePtr, XHDCP1X_CIPHER_REQUEST_BLOCK);
}

/*****************************************************************************/
/**
* This function polls the progress of the computations for a state machine.
*
* @param	InstancePtr is the HDCP state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void PollForComputations(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	/* Check for done */
	if (XHdcp1x_CipherIsRequestComplete(InstancePtr)) {
		DebugLog(InstancePtr, "computations complete");
		*NextStatePtr = STATE_VALIDATERX;
	}
	else {
		DebugLog(InstancePtr, "waiting for computations");
	}
}

/*****************************************************************************/
/**
* This function validates the attached receiver.
*
* @param	InstancePtr is the HDCP state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ValidateRx(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	u8 Buf[2];
	int NumTries = 3;

	/* Update NextStatePtr */
	*NextStatePtr = STATE_UNAUTHENTICATED;

	/* Attempt to read Ro */
	do {
		/* Read the remote Ro' */
		if (XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_RO,
				Buf, 2) > 0) {
			char LogBuf[32];
			u16 RemoteRo = 0;
			u16 LocalRo = 0;

			/* Determine RemoteRo */
			XHDCP1X_PORT_BUF_TO_UINT(RemoteRo, Buf, 2 * 8);

			/* Determine theLLocalRoocalRo */
			LocalRo = XHdcp1x_CipherGetRo(InstancePtr);

			/* Compare the Ro values */
			if (LocalRo == RemoteRo) {

				/* Determine theLogBuf */
				snprintf(LogBuf, 32, "rx valid Ro/Ro' (%04X)",
						LocalRo);

				/* Update NextStatePtr */
				*NextStatePtr = STATE_TESTFORREPEATER;
			}
			/* Otherwise */
			else {
				/* Determine theLogBuf */
				snprintf(LogBuf, 32, "Ro/Ro' mismatch (%04X/"
						"%04X)", LocalRo, RemoteRo);

				/* Update statistics if the last attempt */
				if (NumTries == 1)
					InstancePtr->Tx.Stats.AuthFailed++;
			}

			/* Log */
			DebugLog(InstancePtr, LogBuf);
		}
		/* Otherwise */
		else {
			/* Log */
			DebugLog(InstancePtr, "Ro' read failure");

			/* Update the statistics */
			InstancePtr->Tx.Stats.ReadFailures++;
		}

		/* Update for loop */
		NumTries--;
	}
	while ((*NextStatePtr == STATE_UNAUTHENTICATED) && (NumTries > 0));
}

/*****************************************************************************/
/**
* This function checks the integrity of a HDCP link.
*
* @param	InstancePtr is the hdcp state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void CheckLinkIntegrity(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	u8 Buf[2];
	int NumTries = 3;

	/* Update theNextState */
	*NextStatePtr = STATE_DETERMINERXCAPABLE;

	/* Iterate through the tries */
	do {
		/* Read the remote Ri' */
		if (XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_RO,
				Buf, 2) > 0) {

			char LogBuf[48];
			u16 RemoteRi = 0;
			u16 LocalRi = 0;

			/* Determine theRemoteRo */
			XHDCP1X_PORT_BUF_TO_UINT(RemoteRi, Buf, 16);

			/* Determine theLocalRi */
			LocalRi = XHdcp1x_CipherGetRi(InstancePtr);

			/* Compare the local and remote values */
			if (LocalRi == RemoteRi) {
				*NextStatePtr = STATE_AUTHENTICATED;
				snprintf(LogBuf, 48, "link check passed Ri/Ri'"
						"(%04X)", LocalRi);
			}
			/* Check for last attempt */
			else if (NumTries == 1) {
				snprintf(LogBuf, 48, "link check failed Ri/Ri'"
						"(%04X/%04X)", LocalRi,
						RemoteRi);
			}

			/* Log */
			DebugLog(InstancePtr, LogBuf);
		}
		else {
			DebugLog(InstancePtr, "Ri' read failure");
			InstancePtr->Tx.Stats.ReadFailures++;
		}

		/* Update for loop */
		NumTries--;
	}
	while ((*NextStatePtr != STATE_AUTHENTICATED) && (NumTries > 0));

	/* Check for success */
	if (*NextStatePtr == STATE_AUTHENTICATED) {
		InstancePtr->Tx.Stats.LinkCheckPassed++;
	}
	else {
		InstancePtr->Tx.Stats.LinkCheckFailed++;
	}
}

/*****************************************************************************/
/**
* This function checks the remote end to see if its a repeater.
*
* @param	InstancePtr is the HDCP state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		The implementation of this function enables encryption when a
*		repeater is detected downstream. The standard is ambiguous as to
*		the handling of this specific case by this behaviour is required
*		in order to pass the Unigraf compliance test suite.
*
******************************************************************************/
static void TestForRepeater(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	/* Check for repeater */
	if (XHdcp1x_PortIsRepeater(InstancePtr)) {
		u8 Buf[XHDCP1X_PORT_SIZE_AINFO];

		/* Update InstancePtr */
		InstancePtr->Tx.Flags |= FLAG_IS_REPEATER;

		/* Clear AINFO */
		memset(Buf, 0, XHDCP1X_PORT_SIZE_AINFO);
		XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO, Buf,
				XHDCP1X_PORT_SIZE_AINFO);

		/* Update NextStatePtr */
		*NextStatePtr = STATE_WAITFORREADY;

		/* Log */
		DebugLog(InstancePtr, "repeater detected");

		/* Enable authentication if needed */
		EnableEncryption(InstancePtr);
	}
	else {
		/* Update InstancePtr */
		InstancePtr->Tx.Flags &= ~FLAG_IS_REPEATER;

		/* Update NextStatePtr */
		*NextStatePtr = STATE_AUTHENTICATED;
	}
}

/*****************************************************************************/
/**
* This function polls a state machine in the "wait for ready" state.
*
* @param	InstancePtr is the hdcp state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void PollForWaitForReady(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	u16 RepeaterInfo = 0;
	int Status = XST_SUCCESS;

	/* Attempt to read the repeater info */
	Status = XHdcp1x_PortGetRepeaterInfo(InstancePtr, &RepeaterInfo);
	if (Status == XST_SUCCESS) {
		/* Check that neither cascade or device numbers exceeded */
		if ((RepeaterInfo & 0x0880u) == 0) {
			/* Check for at least one attached device */
			if ((RepeaterInfo & 0x007Fu) != 0) {
				/* Update InstancePtr */
				InstancePtr->Tx.StateHelper = RepeaterInfo;

				/* Update NextStatePtr */
				*NextStatePtr = STATE_READKSVLIST;

				/* Log */
				DebugLog(InstancePtr, "devices attached: "
						"ksv list ready");
			}
			/* Otherwise */
			else {
				/* Update NextStatePtr */
				*NextStatePtr = STATE_AUTHENTICATED;

				/* Log */
				DebugLog(InstancePtr, "no attached devices");
			}
		}
		/* Check for cascade exceeded */
		else {
			/* Update NextStatePtr */
			*NextStatePtr = STATE_UNAUTHENTICATED;

			/* Log */
			if ((RepeaterInfo & 0x0800u) != 0) {
				DebugLog(InstancePtr, "max cascade exceeded");
			}
			else {
				DebugLog(InstancePtr, "max devices exceeded");
			}
		}
	}
}

/*****************************************************************************/
/**
* This function validates the ksv list from an attached repeater.
*
* @param	InstancePtr is the hdcp state machine.
* @param	RepeaterInfo is the repeater information.
*
* @return	Truth value indicating valid (TRUE) or invalid (FALSE).
*
* @note		None.
*
******************************************************************************/
static int ValidateKsvList(XHdcp1x *InstancePtr, u16 RepeaterInfo)
{
	SHA1Context Sha1Context;
	u8 Buf[24];
	int NumToRead = 0;
	int IsValid = FALSE;

	/* Initialize Buf */
	memset(Buf, 0, 24);

	/* Initialize Sha1Context */
	SHA1Reset(&Sha1Context);

	/* Assume success */
	IsValid = TRUE;

	/* Determine theNumToRead */
	NumToRead = ((RepeaterInfo & 0x7Fu)*5);

	/* Read the ksv list */
	do {
		int NumThisTime = XHDCP1X_PORT_SIZE_KSVFIFO;

		/* Truncate if necessary */
		if (NumThisTime > NumToRead) {
			NumThisTime = NumToRead;
		}

		/* Read the next chunk of the list */
		if (XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_KSVFIFO,
				Buf, NumThisTime) > 0) {
			/* Update the calculation of V */
			SHA1Input(&Sha1Context, Buf, NumThisTime);
		}
		else {
			/* Update the statistics */
			InstancePtr->Tx.Stats.ReadFailures++;

			/* Update IsValid */
			IsValid = FALSE;
		}

		/* Update for loop */
		NumToRead -= NumThisTime;
	}
	while ((NumToRead > 0) && (IsValid));

	/* Check for success */
	if (IsValid) {
		u64 Mo = 0;
		u8 Sha1Result[SHA1HashSize];

		/* Insert RepeaterInfo into the SHA-1 transform */
		Buf[0] = (u8) (RepeaterInfo & 0xFFu);
		Buf[1] = (u8) ((RepeaterInfo >> 8) & 0xFFu);
		SHA1Input(&Sha1Context, Buf, 2);

		/* Insert the Mo into the SHA-1 transform */
		Mo = XHdcp1x_CipherGetMo(InstancePtr);
		XHDCP1X_PORT_UINT_TO_BUF(Buf, Mo, 64);
		SHA1Input(&Sha1Context, Buf, 8);

		/* Finalize the SHA-1 result and confirm success */
		if (SHA1Result(&Sha1Context, Sha1Result) == shaSuccess) {
			u8 Offset = XHDCP1X_PORT_OFFSET_VH0;
			const u8 *Sha1Buf = Sha1Result;
			int NumIterations = (SHA1HashSize >> 2);

			/* Iterate through the SHA-1 chunks */
			do {
				u32 CalcValue = 0;
				u32 ReadValue = 0;

				/* Determine CalcValue */
				CalcValue = *Sha1Buf++;
				CalcValue <<= 8;
				CalcValue |= *Sha1Buf++;
				CalcValue <<= 8;
				CalcValue |= *Sha1Buf++;
				CalcValue <<= 8;
				CalcValue |= *Sha1Buf++;

				/* Read the value from the far end */
				if (XHdcp1x_PortRead(InstancePtr, Offset, Buf,
						4) > 0) {
					/* Determine ReadValue */
					XHDCP1X_PORT_BUF_TO_UINT(ReadValue,
							Buf, 32);
				}
				else {
					/* Update ReadValue */
					ReadValue = 0;

					/* Update the statistics */
					InstancePtr->Tx.Stats.ReadFailures++;
				}

				/* Check for mismatch */
				if (CalcValue != ReadValue) {
					IsValid = FALSE;
				}

				/* Update for loop */
				Offset += 4;
				NumIterations--;
			}
			while (NumIterations > 0);
		}
		/* Otherwise */
		else {
			IsValid = FALSE;
		}
	}

	return (IsValid);
}

/*****************************************************************************/
/**
* This function reads the ksv list from an attached repeater.
*
* @param	InstancePtr is the hdcp state machine.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ReadKsvList(XHdcp1x *InstancePtr, tState *NextStatePtr)
{
	int NumAttempts = 3;
	int KsvListIsValid = FALSE;
	u16 RepeaterInfo = 0;

	/* Determine RepeaterInfo */
	RepeaterInfo = (u16)(InstancePtr->Tx.StateHelper & 0x0FFFu);

	/* Iterate through the attempts */
	do {
		/* Attempt to validate the ksv list */
		KsvListIsValid = ValidateKsvList(InstancePtr, RepeaterInfo);

		/* Update for loop */
		NumAttempts--;
	}
	while ((NumAttempts > 0) && (!KsvListIsValid));

	/* Check for success */
	if (KsvListIsValid) {
		/* Log */
		DebugLog(InstancePtr, "ksv list validated");

		/* Update NextStatePtr */
		*NextStatePtr = STATE_AUTHENTICATED;
	}
	else {
		/* Log */
		DebugLog(InstancePtr, "ksv list invalid");

		/* Update NextStatePtr */
		*NextStatePtr = STATE_UNAUTHENTICATED;
	}
}

/*****************************************************************************/
/**
* This function queries a a handler to check if its been authenticated.
*
* @param	InstancePtr is the HDCP state machine.
*
* @return	Truth value indicating authenticated (true) or not (false)
*
* @note		None.
*
******************************************************************************/
static int IsAuthenticated(const XHdcp1x *InstancePtr)
{
	int Authenticated = FALSE;

	/* Which state? */
	switch (InstancePtr->Tx.CurrentState) {
		/* For the authenticated and link integrity check states */
		case STATE_AUTHENTICATED:
		case STATE_LINKINTEGRITYCHECK:
			Authenticated = TRUE;
			break;

		/* Otherwise */
		default:
			Authenticated = FALSE;
			break;
	}

	return (Authenticated);
}

/*****************************************************************************/
/**
* This function runs the "disabled" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunDisabledState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For enable */
		case EVENT_ENABLE:
			*NextStatePtr = STATE_UNAUTHENTICATED;
			if ((InstancePtr->Tx.Flags & FLAG_PHY_UP) == 0) {
				*NextStatePtr = STATE_PHYDOWN;
			}
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			InstancePtr->Tx.Flags &= ~FLAG_PHY_UP;
			break;

		/* For physical layer up */
		case EVENT_PHYUP:
			InstancePtr->Tx.Flags |= FLAG_PHY_UP;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "determine rx capable" state of the transmit state
* machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunDetermineRxCapableState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "exchange ksvs" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunExchangeKsvsState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "computations" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunComputationsState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* For poll */
		case EVENT_POLL:
			PollForComputations(InstancePtr, NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "validate-rx" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunValidateRxState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* For timeout */
		case EVENT_TIMEOUT:
			DebugLog(InstancePtr, "validate-rx timeout");
			ValidateRx(InstancePtr, NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "authenticated" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunAuthenticatedState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For check */
		case EVENT_CHECK:
			*NextStatePtr = STATE_LINKINTEGRITYCHECK;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "link-integrity check" state of the transmit state
* machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunLinkIntegrityCheckState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* For poll */
		case EVENT_POLL:
			CheckLinkIntegrity(InstancePtr, NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "test-for-repeater" state of the transmit state
* machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunTestForRepeaterState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* For poll */
		case EVENT_POLL:
			TestForRepeater(InstancePtr, NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "wait-for-ready" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunWaitForReadyState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* For poll */
		case EVENT_POLL:
			PollForWaitForReady(InstancePtr, NextStatePtr);
			break;

		/* For timeout */
		case EVENT_TIMEOUT:
			DebugLog(InstancePtr, "wait-for-ready timeout");
			PollForWaitForReady(InstancePtr, NextStatePtr);
			if (*NextStatePtr == STATE_WAITFORREADY) {
				*NextStatePtr = STATE_UNAUTHENTICATED;
			}
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "read-ksv-list" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunReadKsvListState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "unauthenticated" state of the transmit state machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunUnauthenticatedState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case EVENT_AUTHENTICATE:
			*NextStatePtr = STATE_DETERMINERXCAPABLE;
			break;

		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer down */
		case EVENT_PHYDOWN:
			*NextStatePtr = STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}


/*****************************************************************************/
/**
* This function runs the "physical-layer-down" state of the transmit state
* machine.
*
* @param	InstancePtr is the transmitter instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RunPhysicalLayerDownState(XHdcp1x *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For disable */
		case EVENT_DISABLE:
			*NextStatePtr = STATE_DISABLED;
			break;

		/* For physical layer up */
		case EVENT_PHYUP:
			*NextStatePtr = STATE_UNAUTHENTICATED;
			if (InstancePtr->Tx.EncryptionMap != 0) {
				PostEvent(InstancePtr, EVENT_AUTHENTICATE);
			}
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function enters a state.
*
* @param	InstancePtr is the HDCP state machine.
* @param	State is the state to enter.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void EnterState(XHdcp1x *InstancePtr, tState State, tState *NextStatePtr)
{
	/* Which state? */
	switch (State) {
		/* For the disabled state */
		case STATE_DISABLED:
			Disable(InstancePtr);
			break;

		/* For determine rx capable */
		case STATE_DETERMINERXCAPABLE:
			InstancePtr->Tx.Flags |= FLAG_PHY_UP;
			SetCheckLinkState(InstancePtr, FALSE);
			DisableEncryption(InstancePtr);
			CheckRxCapable(InstancePtr, NextStatePtr);
			break;

		/* For the exchange ksvs state */
		case STATE_EXCHANGEKSVS:
			InstancePtr->Tx.StateHelper = 0;
			ExchangeKsvs(InstancePtr, NextStatePtr);
			break;

		/* For the computations state */
		case STATE_COMPUTATIONS:
			StartComputations(InstancePtr, NextStatePtr);
			break;

		/* For the validate rx state */
		case STATE_VALIDATERX:
			InstancePtr->Tx.StateHelper = 0;
			StartTimer(InstancePtr, TMO_100MS);
			break;

		/* For the wait for ready state */
		case STATE_WAITFORREADY:
			InstancePtr->Tx.StateHelper = 0;
			StartTimer(InstancePtr, (5 * TMO_1SECOND));
			break;

		/* For the read ksv list state */
		case STATE_READKSVLIST:
			ReadKsvList(InstancePtr, NextStatePtr);
			break;

		/* For the authenticated state */
		case STATE_AUTHENTICATED:
			InstancePtr->Tx.StateHelper = 0;
			EnableEncryption(InstancePtr);
			if (InstancePtr->Tx.PreviousState !=
					STATE_LINKINTEGRITYCHECK) {
				InstancePtr->Tx.Stats.AuthPassed++;
				SetCheckLinkState(InstancePtr, TRUE);
				DebugLog(InstancePtr, "authenticated");
			}
			break;

		/* For the link integrity check state */
		case STATE_LINKINTEGRITYCHECK:
			CheckLinkIntegrity(InstancePtr, NextStatePtr);
			break;

		/* For the unauthenticated state */
		case STATE_UNAUTHENTICATED:
			InstancePtr->Tx.Flags &= ~FLAG_IS_REPEATER;
			InstancePtr->Tx.Flags |= FLAG_PHY_UP;
			DisableEncryption(InstancePtr);
			break;

		/* For physical layer down */
		case STATE_PHYDOWN:
			InstancePtr->Tx.Flags &= ~FLAG_PHY_UP;
			DisableEncryption(InstancePtr);
			XHdcp1x_CipherDisable(InstancePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function exits a state.
*
* @param	InstancePtr is the HDCP state machine.
* @param	State is the state to exit.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ExitState(XHdcp1x *InstancePtr, tState State)
{
	/* Which state? */
	switch (State) {
		/* For the disabled state */
		case STATE_DISABLED:
			Enable(InstancePtr);
			break;

		/* For the computations state */
		case STATE_COMPUTATIONS:
			InstancePtr->Tx.StateHelper = 0;
			break;

		/* For the validate rx state */
		case STATE_VALIDATERX:
			StopTimer(InstancePtr);
			break;

		/* For the wait for ready state */
		case STATE_WAITFORREADY:
			StopTimer(InstancePtr);
			break;

		/* For the read ksv list state */
		case STATE_READKSVLIST:
			InstancePtr->Tx.StateHelper = 0;
			break;

		/* For physical layer down */
		case STATE_PHYDOWN:
			XHdcp1x_CipherEnable(InstancePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function drives a transmit state machine.
*
* @param	InstancePtr is the HDCP state machine.
* @param	Event is the event to process.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DoTheState(XHdcp1x *InstancePtr, tEvent Event)
{
	tState NextState = InstancePtr->Tx.CurrentState;

	/* Which state? */
	switch (InstancePtr->Tx.CurrentState) {
		/* For the disabled state */
		case STATE_DISABLED:
			RunDisabledState(InstancePtr, Event, &NextState);
			break;

		/* For determine rx capable state */
		case STATE_DETERMINERXCAPABLE:
			RunDetermineRxCapableState(InstancePtr, Event,
								&NextState);
			break;

		/* For exchange ksvs state */
		case STATE_EXCHANGEKSVS:
			RunExchangeKsvsState(InstancePtr, Event, &NextState);
			break;

		/* For the computations state */
		case STATE_COMPUTATIONS:
			RunComputationsState(InstancePtr, Event, &NextState);
			break;

		/* For the validate rx state */
		case STATE_VALIDATERX:
			RunValidateRxState(InstancePtr, Event, &NextState);
			break;

		/* For the authenticated state */
		case STATE_AUTHENTICATED:
			RunAuthenticatedState(InstancePtr, Event, &NextState);
			break;

		/* For the link integrity check state */
		case STATE_LINKINTEGRITYCHECK:
			RunLinkIntegrityCheckState(InstancePtr, Event,
								&NextState);
			break;

		/* For the test for repeater state */
		case STATE_TESTFORREPEATER:
			RunTestForRepeaterState(InstancePtr, Event, &NextState);
			break;

		/* For the wait for ready state */
		case STATE_WAITFORREADY:
			RunWaitForReadyState(InstancePtr, Event, &NextState);
			break;

		/* For the reads ksv list state */
		case STATE_READKSVLIST:
			RunReadKsvListState(InstancePtr, Event, &NextState);
			break;

		/* For the unauthenticated state */
		case STATE_UNAUTHENTICATED:
			RunUnauthenticatedState(InstancePtr, Event, &NextState);
			break;

		/* For the physical layer down state */
		case STATE_PHYDOWN:
			RunPhysicalLayerDownState(InstancePtr, Event,
								&NextState);
			break;

		/* Otherwise */
		default:
			break;
	}

	/* Check for state change */
	while (InstancePtr->Tx.CurrentState != NextState) {
		/* Perform the state transition */
		ExitState(InstancePtr, InstancePtr->Tx.CurrentState);
		InstancePtr->Tx.PreviousState = InstancePtr->Tx.CurrentState;
		InstancePtr->Tx.CurrentState = NextState;
		EnterState(InstancePtr, InstancePtr->Tx.CurrentState,
								&NextState);
	}
}

/*****************************************************************************/
/**
* This function processes the events pending on a state machine.
*
* @param	InstancePtr is the receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ProcessPending(XHdcp1x *InstancePtr)
{
	/* Check for any pending events */
	if (InstancePtr->Tx.PendingEvents != 0) {
		u16 Pending = InstancePtr->Tx.PendingEvents;
		tEvent Event = EVENT_NULL;

		/* Update InstancePtr */
		InstancePtr->Tx.PendingEvents = 0;

		/* Iterate through thePending */
		do {
			/* Check for a pending event */
			if ((Pending & 1u) != 0) {
				DoTheState(InstancePtr, Event);
			}

			/* Update for loop */
			Pending >>= 1;
			Event++;
		}
		while (Pending != 0);
	}
}

/*****************************************************************************/
/**
* This function converts from a state to a display string.
*
* @param	State is the state to convert.
*
* @return	The corresponding display string.
*
* @note		None.
*
******************************************************************************/
static const char *StateToString(tState State)
{
	const char *String = NULL;

	/* Which state? */
	switch (State) {
		case STATE_DISABLED:
			String = "disabled";
			break;

		case STATE_DETERMINERXCAPABLE:
			String = "determine-rx-capable";
			break;

		case STATE_EXCHANGEKSVS:
			String = "exchange-ksvs";
			break;

		case STATE_COMPUTATIONS:
			String = "computations";
			break;

		case STATE_VALIDATERX:
			String = "validate-rx";
			break;

		case STATE_AUTHENTICATED:
			String = "authenticated";
			break;

		case STATE_LINKINTEGRITYCHECK:
			String = "link-integrity-check";
			break;

		case STATE_TESTFORREPEATER:
			String = "test-for-repeater";
			break;

		case STATE_WAITFORREADY:
			String = "wait-for-ready";
			break;

		case STATE_READKSVLIST:
			String = "read-ksv-list";
			break;

		case STATE_UNAUTHENTICATED:
			String = "unauthenticated";
			break;

		case STATE_PHYDOWN:
			String = "physical-layer-down";
			break;

		default:
			String = "???";
			break;
	}

	return (String);
}
