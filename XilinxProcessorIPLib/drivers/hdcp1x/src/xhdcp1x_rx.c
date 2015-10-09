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
* @file xhdcp1x_rx.c
* @addtogroup hdcp1x_v1_0
* @{
*
* This contains the main implementation file for the Xilinx HDCP receive state
* machine
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_debug.h"
#include "xhdcp1x_port.h"
#if defined(XPAR_XV_HDMIRX_NUM_INSTANCES) && (XPAR_XV_HDMIRX_NUM_INSTANCES > 0)
#include "xhdcp1x_port_hdmi.h"
#else
#include "xhdcp1x_port_dp.h"
#endif
#include "xhdcp1x_rx.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

#define XVPHY_FLAG_PHY_UP	(1u << 0)  /**< Flag to track physical state */

/**************************** Type Definitions *******************************/

typedef enum {
	XHDCP1X_EVENT_NULL,
	XHDCP1X_EVENT_AUTHENTICATE,
	XHDCP1X_EVENT_CHECK,
	XHDCP1X_EVENT_DISABLE,
	XHDCP1X_EVENT_ENABLE,
	XHDCP1X_EVENT_PHYDOWN,
	XHDCP1X_EVENT_PHYUP,
	XHDCP1X_EVENT_POLL,
	XHDCP1X_EVENT_UPDATERi,
} XHdcp1x_EventType;

typedef enum {
	XHDCP1X_STATE_DISABLED,
	XHDCP1X_STATE_UNAUTHENTICATED,
	XHDCP1X_STATE_COMPUTATIONS,
	XHDCP1X_STATE_AUTHENTICATED,
	XHDCP1X_STATE_LINKINTEGRITYFAILED,
	XHDCP1X_STATE_PHYDOWN,
} XHdcp1x_StateType;

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes *****************************/

static void XHdcp1x_RxDebugLog(const XHdcp1x *InstancePtr, const char *LogMsg);
static void XHdcp1x_RxPostEvent(XHdcp1x *InstancePtr, XHdcp1x_EventType Event);
static void XHdcp1x_RxAuthCallback(void *Parameter);
static void XHdcp1x_RxLinkFailCallback(void *Parameter);
static void XHdcp1x_RxRiUpdateCallback(void *Parameter);
static void XHdcp1x_RxSetCheckLinkState(XHdcp1x *InstancePtr, int IsEnabled);
static void XHdcp1x_RxEnableState(XHdcp1x *InstancePtr);
static void XHdcp1x_RxDisableState(XHdcp1x *InstancePtr);
static void XHdcp1x_RxStartComputations(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxPollForComputations(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxUpdateRi(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxCheckLinkIntegrity(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxReportLinkIntegrityFailure(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunDisabledState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunUnauthenticatedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunComputationsState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunAuthenticatedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunLinkIntegrityFailedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxRunPhysicalLayerDownState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxEnterState(XHdcp1x *InstancePtr, XHdcp1x_StateType State,
		XHdcp1x_StateType *NextStatePtr);
static void XHdcp1x_RxExitState(XHdcp1x *InstancePtr, XHdcp1x_StateType State);
static void XHdcp1x_RxDoTheState(XHdcp1x *InstancePtr, XHdcp1x_EventType Event);
static void XHdcp1x_RxProcessPending(XHdcp1x *InstancePtr);
static const char *XHdcp1x_RxStateToString(XHdcp1x_StateType State);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes a HDCP receiver state machine.
*
* @param	InstancePtr is the receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_RxInit(XHdcp1x *InstancePtr)
{
	XHdcp1x_StateType DummyState = XHDCP1X_STATE_DISABLED;

	/* Update theHandler */
	InstancePtr->Rx.PendingEvents = 0;

	/* Kick the state machine */
	XHdcp1x_RxEnterState(InstancePtr, XHDCP1X_STATE_DISABLED, &DummyState);
}

/*****************************************************************************/
/**
* This function polls the HDCP receiver module.
*
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
 ******************************************************************************/
int XHdcp1x_RxPoll(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Process any pending events */
	XHdcp1x_RxProcessPending(InstancePtr);

	/* Poll it */
	XHdcp1x_RxDoTheState(InstancePtr, XHDCP1X_EVENT_POLL);

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
int XHdcp1x_RxReset(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Reset it */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_DISABLE);
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_ENABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function enables a HDCP receive interface.
*
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxEnable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post it */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_ENABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function disables a HDCP receive interface.
*
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxDisable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post it */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_DISABLE);

	return (Status);
}

/*****************************************************************************/
/**
* This function updates the physical state of an HDCP interface.
*
* @param	InstancePtr is the receiver instance.
* @param	IsUp is truth value indicating the status of physical interface.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxSetPhysicalState(XHdcp1x *InstancePtr, int IsUp)
{
	int Status = XST_SUCCESS;
	XHdcp1x_EventType Event = XHDCP1X_EVENT_PHYDOWN;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Event */
	if (IsUp) {
		Event = XHDCP1X_EVENT_PHYUP;
	}

	/* Post it */
	XHdcp1x_RxPostEvent(InstancePtr, Event);

	return (Status);
}

/*****************************************************************************/
/**
* This function set the lane count of an hdcp interface.
*
* @param	InstancePtr is the receiver instance.
* @param	LaneCount is the number of lanes of the interface.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxSetLaneCount(XHdcp1x *InstancePtr, int LaneCount)
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
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxAuthenticate(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Post the re-authentication request */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_AUTHENTICATE);

	return (Status);
}

/*****************************************************************************/
/**
* This function queries an interface to check if its been authenticated.
*
* @param	InstancePtr is the receiver instance.
*
* @return	Truth value indicating authenticated (true) or not (false).
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxIsAuthenticated(const XHdcp1x *InstancePtr)
{
	int IsAuthenticated = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine IsAuthenticated */
	if (InstancePtr->Rx.CurrentState == XHDCP1X_STATE_AUTHENTICATED) {
		IsAuthenticated = TRUE;
	}

	return (IsAuthenticated);
}

/*****************************************************************************/
/**
* This function retrieves the current encryption stream map.
*
* @param	InstancePtr is the receiver instance.
*
* @return	The current encryption stream map.
*
* @note		None.
*
******************************************************************************/
u64 XHdcp1x_RxGetEncryption(const XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Get it */
	return (XHdcp1x_CipherGetEncryption(InstancePtr));
}

/*****************************************************************************/
/**
* This function implements the debug display output for receiver instances.
*
* @param	InstancePtr is the receiver instance.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_RxInfo(const XHdcp1x *InstancePtr)
{
	u64 LocalKsv = 0;
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Display it */
	XHDCP1X_DEBUG_PRINTF("Type:            ");
	if (InstancePtr->Config.IsHDMI) {
		XHDCP1X_DEBUG_PRINTF("hdmi-rx\r\n");
	}
	else {
		XHDCP1X_DEBUG_PRINTF("dp-rx\r\n");
	}
	XHDCP1X_DEBUG_PRINTF("Current State:   %s\r\n",
		XHdcp1x_RxStateToString(InstancePtr->Rx.CurrentState));
	XHDCP1X_DEBUG_PRINTF("Previous State:  %s\r\n",
		XHdcp1x_RxStateToString(InstancePtr->Rx.PreviousState));
	XHDCP1X_DEBUG_PRINTF("Encrypted?:      %s\r\n",
		XHdcp1x_IsEncrypted(InstancePtr) ? "Yes" : "No");
	XHDCP1X_DEBUG_PRINTF("Flags:           %04X\r\n",
		InstancePtr->Rx.Flags);
	Version = XHdcp1x_GetDriverVersion();
	XHDCP1X_DEBUG_PRINTF("Driver Version:  %d.%02d.%02d\r\n",
		((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
		(Version & 0xFFu));
	Version = XHdcp1x_CipherGetVersion(InstancePtr);
	XHDCP1X_DEBUG_PRINTF("Cipher Version:  %d.%02d.%02d\r\n",
		((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
		(Version & 0xFFu));
	LocalKsv = XHdcp1x_CipherGetLocalKsv(InstancePtr);
	XHDCP1X_DEBUG_PRINTF("Local KSV:       %02lX", (LocalKsv >> 32));
	XHDCP1X_DEBUG_PRINTF("%08lX\r\n", (LocalKsv & 0xFFFFFFFFu));

	XHDCP1X_DEBUG_PRINTF("\r\n");
	XHDCP1X_DEBUG_PRINTF("Rx Stats\r\n");
	XHDCP1X_DEBUG_PRINTF("Auth Attempts:   %d\r\n",
		InstancePtr->Rx.Stats.AuthAttempts);
	XHDCP1X_DEBUG_PRINTF("Link Failures:   %d\r\n",
		InstancePtr->Rx.Stats.LinkFailures);
	XHDCP1X_DEBUG_PRINTF("Ri Updates:      %d\r\n",
		InstancePtr->Rx.Stats.RiUpdates);

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
* @param	LogMsg  is the message to log.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxDebugLog(const XHdcp1x *InstancePtr, const char *LogMsg)
{
	char Label[16];

	/* Format Label */
	snprintf(Label, 16, "hdcp-rx(%d) - ", InstancePtr->Config.DeviceId);

	/* Log it */
	XHDCP1X_DEBUG_LOGMSG(Label);
	XHDCP1X_DEBUG_LOGMSG(LogMsg);
	XHDCP1X_DEBUG_LOGMSG("\r\n");
}

/*****************************************************************************/
/**
* This function posts an event to a state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to post.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxPostEvent(XHdcp1x *InstancePtr, XHdcp1x_EventType Event)
{
	/* Check for disable and clear any pending enable */
	if (Event == XHDCP1X_EVENT_DISABLE) {
		InstancePtr->Rx.PendingEvents &= ~(1u << XHDCP1X_EVENT_ENABLE);
	}
	/* Check for phy-down and clear any pending phy-up */
	else if (Event == XHDCP1X_EVENT_PHYDOWN) {
		InstancePtr->Rx.PendingEvents &= ~(1u << XHDCP1X_EVENT_PHYUP);
	}

	/* Post it */
	InstancePtr->Rx.PendingEvents |= (1u << Event);
}

/*****************************************************************************/
/**
* This function acts as the re-authentication callback for a state machine.
*
* @param	Parameter is the parameter specified during registration.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxAuthCallback(void *Parameter)
{
	XHdcp1x *InstancePtr = Parameter;

	/* Post the re-authentication request */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_AUTHENTICATE);
}

/*****************************************************************************/
/**
* This function acts as the link failure callback for a state machine.
*
* @param	Parameter is the parameter specified during registration.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxLinkFailCallback(void *Parameter)
{
	XHdcp1x *InstancePtr = Parameter;

	/* Post the check request */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_CHECK);
}

/*****************************************************************************/
/**
* This function acts as the Ri register update callback for a state machine.
*
* @param	Parameter is the parameter specified during registration.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRiUpdateCallback(void *Parameter)
{
	XHdcp1x *InstancePtr = Parameter;

	/* Post the update Ri request */
	XHdcp1x_RxPostEvent(InstancePtr, XHDCP1X_EVENT_UPDATERi);
}

/*****************************************************************************/
/**
* This function sets the check link state of the handler.
*
* @param	InstancePtr is the receiver instance.
* @param	IsEnabled is truth value indicating on/off.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxSetCheckLinkState(XHdcp1x *InstancePtr, int IsEnabled)
{
	/* Check for HDMI */
	if (InstancePtr->Config.IsHDMI) {
		XHdcp1x_CipherSetRiUpdate(InstancePtr, IsEnabled);
	}
	/* Check for DP */
	else {
		XHdcp1x_CipherSetLinkStateCheck(InstancePtr, IsEnabled);
	}
}

/*****************************************************************************/
/**
* This function enables a receiver state machine.
*
* @param	InstancePtr is the receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxEnableState(XHdcp1x *InstancePtr)
{
	u64 MyKsv = 0;
	u8 Buf[8];

	/* Disable and register the link failure callback */
	XHdcp1x_CipherSetLinkStateCheck(InstancePtr, FALSE);
	XHdcp1x_CipherSetCallback(InstancePtr,
			XHDCP1X_CIPHER_HANDLER_LINK_FAILURE,
			&XHdcp1x_RxLinkFailCallback, InstancePtr);

	/* Disable and register the Ri callback */
	XHdcp1x_CipherSetRiUpdate(InstancePtr, FALSE);
	XHdcp1x_CipherSetCallback(InstancePtr,
			XHDCP1X_CIPHER_HANDLER_Ri_UPDATE,
			&XHdcp1x_RxRiUpdateCallback, InstancePtr);

	/* Enable the crypto engine */
	XHdcp1x_CipherEnable(InstancePtr);

	/* Read MyKsv */
	MyKsv = XHdcp1x_CipherGetLocalKsv(InstancePtr);

	/* If unknown - try against for good luck */
	if (MyKsv == 0) {
		MyKsv = XHdcp1x_CipherGetLocalKsv(InstancePtr);
	}

	/* Initialize Bksv */
	memset(Buf, 0, 8);
	XHDCP1X_PORT_UINT_TO_BUF(Buf, MyKsv, XHDCP1X_PORT_SIZE_BKSV * 8);
	XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BKSV, Buf,
			XHDCP1X_PORT_SIZE_BKSV);

	/* Register the re-authentication callback */
	XHdcp1x_PortSetCallback(InstancePtr, XHDCP1X_PORT_HANDLER_AUTHENTICATE,
			&XHdcp1x_RxAuthCallback, InstancePtr);

	/* Enable the hdcp port */
	XHdcp1x_PortEnable(InstancePtr);
}

/*****************************************************************************/
/**
* This function disables a receiver state machine.
*
* @param	InstancePtr is the receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxDisableState(XHdcp1x *InstancePtr)
{
	/* Disable the hdcp cipher and port */
	XHdcp1x_PortDisable(InstancePtr);
	XHdcp1x_CipherDisable(InstancePtr);

	/* Clear statistics */
	memset(&(InstancePtr->Rx.Stats), 0, sizeof(InstancePtr->Rx.Stats));
}

/*****************************************************************************/
/**
* This function initiates the computations for a receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxStartComputations(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr)
{
	u8 Buf[8];
	u64 Value = 0;
	u32 X = 0;
	u32 Y = 0;
	u32 Z = 0;

	/* Log */
	XHdcp1x_RxDebugLog(InstancePtr, "starting computations");

	/* Update statistics */
	InstancePtr->Rx.Stats.AuthAttempts++;

	/* Determine theAKsv */
	memset(Buf, 0, 8);
	XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_AKSV, Buf,
			XHDCP1X_PORT_SIZE_AKSV);
	XHDCP1X_PORT_BUF_TO_UINT(Value, Buf, XHDCP1X_PORT_SIZE_AKSV * 8);

	/* Load the cipher with the remote ksv */
	XHdcp1x_CipherSetRemoteKsv(InstancePtr, Value);

	/* Update theU64Value with An */
	memset(Buf, 0, 8);
	XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_AN, Buf,
			XHDCP1X_PORT_SIZE_AN);
	XHDCP1X_PORT_BUF_TO_UINT(Value, Buf, XHDCP1X_PORT_SIZE_AN * 8);

	/* Load the cipher B registers with An */
	X = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Y = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Z = (u32) (Value & 0x000000FFul);
	XHdcp1x_CipherSetB(InstancePtr, X, Y, Z);

	/* Initiate the block cipher */
	XHdcp1x_CipherDoRequest(InstancePtr, XHDCP1X_CIPHER_REQUEST_BLOCK);
}

/*****************************************************************************/
/**
* This function polls the progress of the computations for a state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxPollForComputations(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr)
{
	/* Check for done */
	if (XHdcp1x_CipherIsRequestComplete(InstancePtr)) {
		u8 Buf[4];
		u16 Ro = 0;

		/* Log */
		XHdcp1x_RxDebugLog(InstancePtr, "computations complete");

		/* Read theRo */
		Ro = XHdcp1x_CipherGetRo(InstancePtr);

		/* Initialize Buf */
		memset(Buf, 0, 4);
		XHDCP1X_PORT_UINT_TO_BUF(Buf, Ro, 16);

		/* Update the value of Ro' */
		XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_RO, Buf, 2);

#if defined(XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE)
		/* Update the Bstatus to indicate Ro' available */
		XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
				XHDCP1X_PORT_SIZE_BSTATUS);
		Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE;
		XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
				XHDCP1X_PORT_SIZE_BSTATUS);
#endif
		/* Update NextStatePtr */
		*NextStatePtr = XHDCP1X_STATE_AUTHENTICATED;
	}
	else {
		XHdcp1x_RxDebugLog(InstancePtr, "waiting for computations");
	}
}

/*****************************************************************************/
/**
* This function updates the Ro'/Ri' register of the state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		This function has to save the value of Ri (in RememberRi) as the
*		macro that converts from a uint16 to a HDCP buffer destroys the
*		original value.
*
******************************************************************************/
static void XHdcp1x_RxUpdateRi(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr)
{
	char LogBuf[20];
	u8 Buf[4];
	u16 Ri = 0;
	u16 RememberRi = 0;

	/* Read Ri */
	Ri = XHdcp1x_CipherGetRi(InstancePtr);

	/* Update RememberRi */
	RememberRi = Ri;

	/* Initialize theBuf */
	memset(Buf, 0, 4);
	XHDCP1X_PORT_UINT_TO_BUF(Buf, Ri, XHDCP1X_PORT_SIZE_RO * 8);

	/* Update the value of Ro' */
	XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_RO, Buf, sizeof(Ri));

#if defined(XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE)
	/* Update the Bstatus to indicate Ro' available */
	XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
	Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE;
	XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
#endif

	/* Update statistics */
	InstancePtr->Rx.Stats.RiUpdates++;

	/* Determine theLogBuf */
	snprintf(LogBuf, 20, "update Ri (%04X)", RememberRi);

	/* Log */
	XHdcp1x_RxDebugLog(InstancePtr, LogBuf);
}

/*****************************************************************************/
/**
* This functions handles check the integrity of the link.
*
* @param	InstancePtr is the receiver instance.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxCheckLinkIntegrity(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr)
{
	if (XHdcp1x_CipherIsLinkUp(InstancePtr)) {
		*NextStatePtr = XHDCP1X_STATE_AUTHENTICATED;
	}
	else {
		*NextStatePtr = XHDCP1X_STATE_LINKINTEGRITYFAILED;
	}
}

/*****************************************************************************/
/**
* This functions reports the failure of link integrity.
*
* @param	InstancePtr is the receiver instance.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxReportLinkIntegrityFailure(XHdcp1x *InstancePtr,
		XHdcp1x_StateType *NextStatePtr)
{
#if defined(XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE)
	u8 Buf[XHDCP1X_PORT_SIZE_BSTATUS];

	/* Update the Bstatus register */
	XHdcp1x_PortRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
	Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE;
	XHdcp1x_PortWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
#endif

	/* Log */
	XHdcp1x_RxDebugLog(InstancePtr, "link integrity failed");
}

/*****************************************************************************/
/**
* This function runs the "disabled" state of the receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunDisabledState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For enable */
		case XHDCP1X_EVENT_ENABLE:
			*NextStatePtr = XHDCP1X_STATE_UNAUTHENTICATED;
			if ((InstancePtr->Rx.Flags & XVPHY_FLAG_PHY_UP) == 0) {
				*NextStatePtr = XHDCP1X_STATE_PHYDOWN;
			}
			break;

		/* For physical layer down */
		case XHDCP1X_EVENT_PHYDOWN:
			InstancePtr->Rx.Flags &= ~XVPHY_FLAG_PHY_UP;
			break;

		/* For physical layer up */
		case XHDCP1X_EVENT_PHYUP:
			InstancePtr->Rx.Flags |= XVPHY_FLAG_PHY_UP;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "unauthenticated" state of the receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunUnauthenticatedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case XHDCP1X_EVENT_AUTHENTICATE:
			*NextStatePtr = XHDCP1X_STATE_COMPUTATIONS;
			break;

		/* For disable */
		case XHDCP1X_EVENT_DISABLE:
			*NextStatePtr = XHDCP1X_STATE_DISABLED;
			break;

		/* For physical layer down */
		case XHDCP1X_EVENT_PHYDOWN:
			*NextStatePtr = XHDCP1X_STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "computations" state of the receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunComputationsState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case XHDCP1X_EVENT_AUTHENTICATE:
			XHdcp1x_RxStartComputations(InstancePtr, NextStatePtr);
			break;

		/* For disable */
		case XHDCP1X_EVENT_DISABLE:
			*NextStatePtr = XHDCP1X_STATE_DISABLED;
			break;

		/* For physical layer down */
		case XHDCP1X_EVENT_PHYDOWN:
			*NextStatePtr = XHDCP1X_STATE_PHYDOWN;
			break;

		/* For poll */
		case XHDCP1X_EVENT_POLL:
			XHdcp1x_RxPollForComputations(InstancePtr,
				NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "authenticated" state of the receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunAuthenticatedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case XHDCP1X_EVENT_AUTHENTICATE:
			*NextStatePtr = XHDCP1X_STATE_COMPUTATIONS;
			break;

		/* For check */
		case XHDCP1X_EVENT_CHECK:
			XHdcp1x_RxCheckLinkIntegrity(InstancePtr, NextStatePtr);
			break;

		/* For disable */
		case XHDCP1X_EVENT_DISABLE:
			*NextStatePtr = XHDCP1X_STATE_DISABLED;
			break;

		/* For physical layer down */
		case XHDCP1X_EVENT_PHYDOWN:
			*NextStatePtr = XHDCP1X_STATE_PHYDOWN;
			break;

		/* For update Ri */
		case XHDCP1X_EVENT_UPDATERi:
			XHdcp1x_RxUpdateRi(InstancePtr, NextStatePtr);
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "link integrity failed" state of the receiver state
* machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunLinkIntegrityFailedState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For authenticate */
		case XHDCP1X_EVENT_AUTHENTICATE:
			*NextStatePtr = XHDCP1X_STATE_COMPUTATIONS;
			break;

		/* For check */
		case XHDCP1X_EVENT_CHECK:
			XHdcp1x_RxCheckLinkIntegrity(InstancePtr, NextStatePtr);
			break;

		/* For disable */
		case XHDCP1X_EVENT_DISABLE:
			*NextStatePtr = XHDCP1X_STATE_DISABLED;
			break;

		/* For physical layer down */
		case XHDCP1X_EVENT_PHYDOWN:
			*NextStatePtr = XHDCP1X_STATE_PHYDOWN;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function runs the "physical layer down" state of the receiver state
* machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxRunPhysicalLayerDownState(XHdcp1x *InstancePtr,
		XHdcp1x_EventType Event, XHdcp1x_StateType *NextStatePtr)
{
	/* Which event? */
	switch (Event) {
		/* For disable */
		case XHDCP1X_EVENT_DISABLE:
			*NextStatePtr = XHDCP1X_STATE_DISABLED;
			break;

		/* For physical layer up */
		case XHDCP1X_EVENT_PHYUP:
			*NextStatePtr = XHDCP1X_STATE_UNAUTHENTICATED;
			break;

		/* Otherwise */
		default:
			/* Do nothing */
			break;
	}
}

/*****************************************************************************/
/**
* This function enters a HDCP receiver state.
*
* @param	InstancePtr is the receiver instance.
* @param	State is the state to enter.
* @param	NextStatePtr is the next state.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxEnterState(XHdcp1x *InstancePtr, XHdcp1x_StateType State,
		XHdcp1x_StateType *NextStatePtr)
{
	/* Which state? */
	switch (State) {
		/* For the disabled state */
		case XHDCP1X_STATE_DISABLED:
			XHdcp1x_RxDisableState(InstancePtr);
			break;

		/* For the unauthenticated state */
		case XHDCP1X_STATE_UNAUTHENTICATED:
			InstancePtr->Rx.Flags |= XVPHY_FLAG_PHY_UP;
			break;

		/* For the computations state */
		case XHDCP1X_STATE_COMPUTATIONS:
			XHdcp1x_RxStartComputations(InstancePtr, NextStatePtr);
			break;

		/* For the authenticated state */
		case XHDCP1X_STATE_AUTHENTICATED:
			XHdcp1x_RxDebugLog(InstancePtr, "authenticated");
			XHdcp1x_RxSetCheckLinkState(InstancePtr, TRUE);
			break;

		/* For the link integrity failed state */
		case XHDCP1X_STATE_LINKINTEGRITYFAILED:
			InstancePtr->Rx.Stats.LinkFailures++;
			XHdcp1x_RxReportLinkIntegrityFailure(InstancePtr,
				NextStatePtr);
			break;

		/* For physical layer down */
		case XHDCP1X_STATE_PHYDOWN:
			InstancePtr->Rx.Flags &= ~XVPHY_FLAG_PHY_UP;
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
* This function exits a HDCP receiver state.
*
* @param	InstancePtr is the receiver instance.
* @param	State is the state to exit.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxExitState(XHdcp1x *InstancePtr, XHdcp1x_StateType State)
{
	/* Which state? */
	switch (State) {
		/* For the disabled state */
		case XHDCP1X_STATE_DISABLED:
			XHdcp1x_RxEnableState(InstancePtr);
			break;

		/* For the authenticated state */
		case XHDCP1X_STATE_AUTHENTICATED:
			XHdcp1x_RxSetCheckLinkState(InstancePtr, FALSE);
			break;

		/* For physical layer down */
		case XHDCP1X_STATE_PHYDOWN:
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
* This function drives a HDCP receiver state machine.
*
* @param	InstancePtr is the receiver instance.
* @param	Event is the event to process.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_RxDoTheState(XHdcp1x *InstancePtr, XHdcp1x_EventType Event)
{
	XHdcp1x_StateType NextState = InstancePtr->Rx.CurrentState;

	/* Which state? */
	switch (InstancePtr->Rx.CurrentState) {
		/* For the disabled state */
		case XHDCP1X_STATE_DISABLED:
			XHdcp1x_RxRunDisabledState(InstancePtr, Event,
								&NextState);
			break;

		/* For the unauthenticated state */
		case XHDCP1X_STATE_UNAUTHENTICATED:
			XHdcp1x_RxRunUnauthenticatedState(InstancePtr, Event,
								&NextState);
			break;

		/* For the computations state */
		case XHDCP1X_STATE_COMPUTATIONS:
			XHdcp1x_RxRunComputationsState(InstancePtr, Event,
								&NextState);
			break;

		/* For the authenticated state */
		case XHDCP1X_STATE_AUTHENTICATED:
			XHdcp1x_RxRunAuthenticatedState(InstancePtr, Event,
								&NextState);
			break;

		/* For the link integrity failed state */
		case XHDCP1X_STATE_LINKINTEGRITYFAILED:
			XHdcp1x_RxRunLinkIntegrityFailedState(InstancePtr,
							Event, &NextState);
			break;

		/* For the physical layer down state */
		case XHDCP1X_STATE_PHYDOWN:
			XHdcp1x_RxRunPhysicalLayerDownState(InstancePtr, Event,
								&NextState);
			break;

		/* Otherwise */
		default:
			break;
	}

	/* Check for state change */
	while (InstancePtr->Rx.CurrentState != NextState) {
		/* Perform the state transition */
		XHdcp1x_RxExitState(InstancePtr, InstancePtr->Rx.CurrentState);
		InstancePtr->Rx.PreviousState = InstancePtr->Rx.CurrentState;
		InstancePtr->Rx.CurrentState = NextState;
		XHdcp1x_RxEnterState(InstancePtr, InstancePtr->Rx.CurrentState,
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
static void XHdcp1x_RxProcessPending(XHdcp1x *InstancePtr)
{
	/* Check for any pending events */
	if (InstancePtr->Rx.PendingEvents != 0) {
		u16 Pending = InstancePtr->Rx.PendingEvents;
		XHdcp1x_EventType Event = XHDCP1X_EVENT_NULL;

		/* Update InstancePtr */
		InstancePtr->Rx.PendingEvents = 0;

		/* Iterate through thePending */
		do {
			/* Check for a pending event */
			if ((Pending & 1u) != 0) {
				XHdcp1x_RxDoTheState(InstancePtr, Event);
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
static const char *XHdcp1x_RxStateToString(XHdcp1x_StateType State)
{
	const char *String = NULL;

	/* Which state? */
	switch (State) {
		case XHDCP1X_STATE_DISABLED:
			String = "disabled";
			break;

		case XHDCP1X_STATE_UNAUTHENTICATED:
			String = "unauthenticated";
			break;

		case XHDCP1X_STATE_COMPUTATIONS:
			String = "computations";
			break;

		case XHDCP1X_STATE_AUTHENTICATED:
			String = "authenticated";
			break;

		case XHDCP1X_STATE_LINKINTEGRITYFAILED:
			String = "link-integrity-failed";
			break;

		case XHDCP1X_STATE_PHYDOWN:
			String = "physical-layer-down";
			break;

		default:
			String = "???";
			break;
	}

	return (String);
}
/** @} */
