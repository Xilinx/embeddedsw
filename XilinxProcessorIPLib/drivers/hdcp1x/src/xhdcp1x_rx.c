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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* This contains the main implementation file for the Xilinx HDCP receive state
* machine
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* 1.01         07/23/15 Additional documentation and formating
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
#if defined(XPAR_XHDMI_RX_NUM_INSTANCES) && (XPAR_XHDMI_RX_NUM_INSTANCES > 0)
	#include "xhdcp1x_port_hdmi.h"
#else
	#include "xhdcp1x_port_dp.h"
#endif
#include "xhdcp1x_rx.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define FLAG_PHY_UP		(1u << 0)  /**< Flag to track physical state */

/**************************** Type Definitions *******************************/
typedef enum
{
	EVENT_NULL,
	EVENT_AUTHENTICATE,
	EVENT_CHECK,
	EVENT_DISABLE,
	EVENT_ENABLE,
	EVENT_PHYDOWN,
	EVENT_PHYUP,
	EVENT_POLL,
	EVENT_UPDATERi,

} tEvent;

typedef enum
{
	STATE_DISABLED,
	STATE_UNAUTHENTICATED,
	STATE_COMPUTATIONS,
	STATE_AUTHENTICATED,
	STATE_LINKINTEGRITYFAILED,
	STATE_PHYDOWN,

} tState;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This queries an interface to determine if it is Display Port (DP)
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating DP (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsDP(InstancePtr)  		(!InstancePtr->CfgPtr->IsHDMI)

/*****************************************************************************/
/**
*
* This queries an interface to determine if it is HDMI
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating HDMI (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsHDMI(InstancePtr)  		(InstancePtr->CfgPtr->IsHDMI)

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function logs a debug message on behalf of a handler state machine
*
* @param InstancePtr  the receiver instance
* @param LogMsg  the message to log
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void DebugLog(const XHdcp1x_Rx *InstancePtr, const char *LogMsg)
{
	char Label[16];

	/* Format Label */
	snprintf(Label, 16, "hdcp-rx(%d) - ", InstancePtr->CfgPtr->DeviceId);

	/* Log it */
	XHDCP1X_DEBUG_LOGMSG(Label);
	XHDCP1X_DEBUG_LOGMSG(LogMsg);
	XHDCP1X_DEBUG_LOGMSG("\r\n");

	return;
}

/*****************************************************************************/
/**
*
* This function posts an event to a state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to post
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void PostEvent(XHdcp1x_Rx *InstancePtr, tEvent Event)
{
	/* Check for disable and clear any pending enable */
	if (Event == EVENT_DISABLE) {
		InstancePtr->PendingEvents &= ~(1u << EVENT_ENABLE);
	}
	/* Check for phy-down and clear any pending phy-up */
	else if (Event == EVENT_PHYDOWN) {
		InstancePtr->PendingEvents &= ~(1u << EVENT_PHYUP);
	}

	/* Post it */
	InstancePtr->PendingEvents |= (1u << Event);

	return;
}

/*****************************************************************************/
/**
*
* This function acts as the re-authentication callback for a state machine
*
* @param Parameter  the parameter specified during registration
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void AuthCallback(void *Parameter)
{
	XHdcp1x_Rx *InstancePtr = Parameter;

	/* Post the re-authentication request */
	PostEvent(InstancePtr, EVENT_AUTHENTICATE);

	return;
}

/*****************************************************************************/
/**
*
* This function acts as the link failure callback for a state machine
*
* @param Parameter  the parameter specified during registration
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void LinkFailCallback(void *Parameter)
{
	XHdcp1x_Rx *InstancePtr = Parameter;

	/* Post the check request */
	PostEvent(InstancePtr, EVENT_CHECK);

	return;
}

/*****************************************************************************/
/**
*
* This function acts as the Ri register update callback for a state machine
*
* @param Parameter  the parameter specified during registration
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RiUpdateCallback(void *Parameter)
{
	XHdcp1x_Rx *InstancePtr = Parameter;

	/* Post the update Ri request */
	PostEvent(InstancePtr, EVENT_UPDATERi);

	return;
}

/*****************************************************************************/
/**
*
* This function sets the check link state of the handler
*
* @param InstancePtr  the receiver instance
* @param IsEnabled  truth value indicating on/off
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void SetCheckLinkState(XHdcp1x_Rx *InstancePtr, int IsEnabled)
{
	XHdcp1x_Cipher *CipherPtr = &(InstancePtr->Cipher);

	/* Check for DP */
	if (IsDP(InstancePtr)) {
		XHdcp1x_CipherSetLinkStateCheck(CipherPtr, IsEnabled);
	}
	/* Check for HDMI */
	else if (IsHDMI(InstancePtr)) {
		XHdcp1x_CipherSetRiUpdate(CipherPtr, IsEnabled);
	}

	/* Return */
	return;
}

/*****************************************************************************/
/**
*
* This function enables a receiver state machine
*
* @param InstancePtr  the receiver instance
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Enable(XHdcp1x_Rx *InstancePtr)
{
	XHdcp1x_Cipher *CipherPtr = &(InstancePtr->Cipher);
	XHdcp1x_Port *PortPtr = &(InstancePtr->Port);
	u64 MyKsv = 0;
	u8 Buf[8];

	/* Disable and register the link failure callback */
	XHdcp1x_CipherSetLinkStateCheck(CipherPtr, FALSE);
	XHdcp1x_CipherSetCallback(CipherPtr,
			XHDCP1X_CIPHER_HANDLER_LINK_FAILURE,
			&LinkFailCallback, InstancePtr);

	/* Disable and register the Ri callback */
	XHdcp1x_CipherSetRiUpdate(CipherPtr, FALSE);
	XHdcp1x_CipherSetCallback(CipherPtr,
			XHDCP1X_CIPHER_HANDLER_Ri_UPDATE,
			&RiUpdateCallback, InstancePtr);

	/* Enable the crypto engine */
	XHdcp1x_CipherEnable(CipherPtr);

	/* Read MyKsv */
	MyKsv = XHdcp1x_CipherGetLocalKsv(CipherPtr);

	/* If unknown - try against for good luck */
	if (MyKsv == 0) {
		MyKsv = XHdcp1x_CipherGetLocalKsv(CipherPtr);
	}

	/* Initialize Bksv */
	memset(Buf, 0, 8);
	XHDCP1X_PORT_UINT_TO_BUF(Buf, MyKsv, XHDCP1X_PORT_SIZE_BKSV*8);
	XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_BKSV, Buf,
			XHDCP1X_PORT_SIZE_BKSV);

	/* Register the re-authentication callback */
	XHdcp1x_PortSetCallback(PortPtr, XHDCP1X_PORT_HANDLER_AUTHENTICATE,
			&AuthCallback, InstancePtr);

	/* Enable the hdcp port */
	XHdcp1x_PortEnable(PortPtr);

	return;
}

/*****************************************************************************/
/**
*
* This function disables a receiver state machine
*
* @param InstancePtr  the receiver instance
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Disable(XHdcp1x_Rx *InstancePtr)
{
	/* Disable the hdcp cipher and port */
	XHdcp1x_PortDisable(&(InstancePtr->Port));
	XHdcp1x_CipherDisable(&(InstancePtr->Cipher));

	/* Clear statistics */
	memset(&(InstancePtr->Stats), 0, sizeof(InstancePtr->Stats));

	return;
}

/*****************************************************************************/
/**
*
* This function initiates the computations for a receiver state machine
*
* @param InstancePtr  the receiver instance
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void StartComputations(XHdcp1x_Rx *InstancePtr, tState *NextStatePtr)
{
	XHdcp1x_Cipher *CipherPtr = &(InstancePtr->Cipher);
	XHdcp1x_Port *PortPtr = &(InstancePtr->Port);
	u8 Buf[8];
	u64 Value = 0;
	u32 X = 0;
	u32 Y = 0;
	u32 Z = 0;

	/* Log */
	DebugLog(InstancePtr, "starting computations");

	/* Update statistics */
	InstancePtr->Stats.AuthAttempts++;

	/* Determine theAKsv */
	memset(Buf, 0, 8);
	XHdcp1x_PortRead(PortPtr, XHDCP1X_PORT_OFFSET_AKSV, Buf,
			XHDCP1X_PORT_SIZE_AKSV);
	XHDCP1X_PORT_BUF_TO_UINT(Value, Buf, XHDCP1X_PORT_SIZE_AKSV*8);

	/* Load the cipher with the remote ksv */
	XHdcp1x_CipherSetRemoteKsv(CipherPtr, Value);

	/* Update theU64Value with An */
	memset(Buf, 0, 8);
	XHdcp1x_PortRead(PortPtr, XHDCP1X_PORT_OFFSET_AN, Buf,
			XHDCP1X_PORT_SIZE_AN);
	XHDCP1X_PORT_BUF_TO_UINT(Value, Buf, XHDCP1X_PORT_SIZE_AN*8);

	/* Load the cipher B registers with An */
	X = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Y = (u32) (Value & 0x0FFFFFFFul);
	Value >>= 28;
	Z = (u32) (Value & 0x000000FFul);
	XHdcp1x_CipherSetB(CipherPtr, X, Y, Z);

	/* Initiate the block cipher */
	XHdcp1x_CipherDoRequest(CipherPtr, XHDCP1X_CIPHER_REQUEST_BLOCK);

	/* Return */
	return;
}

/*****************************************************************************/
/**
*
* This function polls the progress of the computations for a state machine
*
* @param InstancePtr  the receiver instance
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void PollForComputations(XHdcp1x_Rx *InstancePtr, tState *NextStatePtr)
{
	XHdcp1x_Cipher *CipherPtr = &(InstancePtr->Cipher);

	/* Check for done */
	if (XHdcp1x_CipherIsRequestComplete(CipherPtr)) {
		XHdcp1x_Port *PortPtr = &(InstancePtr->Port);
		u8 Buf[4];
		u16 Ro = 0;

		/* Log */
		DebugLog(InstancePtr, "computations complete");

		/* Read theRo */
		Ro = XHdcp1x_CipherGetRo(CipherPtr);

		/* Initialize Buf */
		memset(Buf, 0, 4);
		XHDCP1X_PORT_UINT_TO_BUF(Buf, Ro, 16);

		/* Update the value of Ro' */
		XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_RO, Buf, 2);

#if defined(XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE)
		/* Update the Bstatus to indicate Ro' available */
		XHdcp1x_PortRead(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
				XHDCP1X_PORT_SIZE_BSTATUS);
		Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE;
		XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
				XHDCP1X_PORT_SIZE_BSTATUS);
#endif

		/* Update NextStatePtr */
		*NextStatePtr = STATE_AUTHENTICATED;
	}
	else {
		DebugLog(InstancePtr, "waiting for computations");
	}

	return;
}

/*****************************************************************************/
/**
*
* This function updates the Ro'/Ri' register of the state machine
*
* @param InstancePtr  the receiver instance
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   This function has to save the value of Ri (in RememberRi) as the macro
*   that converts from a uint16 to a HDCP buffer destroys the original value
*
******************************************************************************/
static void UpdateRi(XHdcp1x_Rx *InstancePtr, tState *NextStatePtr)
{
	XHdcp1x_Cipher *CipherPtr = &(InstancePtr->Cipher);
	XHdcp1x_Port *PortPtr = &(InstancePtr->Port);
	char LogBuf[20];
	u8 Buf[4];
	u16 Ri = 0;
	u16 RememberRi = 0;

	/* Read Ri */
	Ri = XHdcp1x_CipherGetRi(CipherPtr);

	/* Update RememberRi */
	RememberRi = Ri;

	/* Initialize theBuf */
	memset(Buf, 0, 4);
	XHDCP1X_PORT_UINT_TO_BUF(Buf, Ri, XHDCP1X_PORT_SIZE_RO*8);

	/* Update the value of Ro' */
	XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_RO, Buf, sizeof(Ri));

#if defined(XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE)
	/* Update the Bstatus to indicate Ro' available */
	XHdcp1x_PortRead(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
	Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_RO_AVAILABLE;
	XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
#endif

	/* Update statistics */
	InstancePtr->Stats.RiUpdates++;

	/* Determine theLogBuf */
	snprintf(LogBuf, 20, "update Ri (%04X)", RememberRi);

	/* Log */
	DebugLog(InstancePtr, LogBuf);

	return;
}

/*****************************************************************************/
/**
*
* This functions handles check the integrity of the link
*
* @param InstancePtr  the receiver instance
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None
*
******************************************************************************/
static void CheckLinkIntegrity(XHdcp1x_Rx *InstancePtr, tState *NextStatePtr)
{
	if (XHdcp1x_CipherIsLinkUp(&(InstancePtr->Cipher))) {
		*NextStatePtr = STATE_AUTHENTICATED;
	}
	else {
		*NextStatePtr = STATE_LINKINTEGRITYFAILED;
	}

	return;
}

/*****************************************************************************/
/**
*
* This functions reports the failure of link integrity
*
* @param InstancePtr  the receiver instance
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None
*
******************************************************************************/
static void ReportLinkIntegrityFailure(XHdcp1x_Rx *InstancePtr,
		tState *NextStatePtr)
{
#if defined(XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE)
	XHdcp1x_Port *PortPtr = &(InstancePtr->Port);
	u8 Buf[XHDCP1X_PORT_SIZE_BSTATUS];

	/* Update the Bstatus register */
	XHdcp1x_PortRead(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
	Buf[0] |= XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE;
	XHdcp1x_PortWrite(PortPtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf,
			XHDCP1X_PORT_SIZE_BSTATUS);
#endif

	/* Log */
	DebugLog(InstancePtr, "link integrity failed");

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "disabled" state of the receiver state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunDisabledState(XHdcp1x_Rx *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {

	/* For enable */
	case EVENT_ENABLE:
		*NextStatePtr = STATE_UNAUTHENTICATED;
		if ((InstancePtr->Flags & FLAG_PHY_UP) == 0)
			*NextStatePtr = STATE_PHYDOWN;
		break;
	/* For physical layer down */
	case EVENT_PHYDOWN:
		InstancePtr->Flags &= ~FLAG_PHY_UP;
	break;
	/* For physical layer up */
	case EVENT_PHYUP:
		InstancePtr->Flags |= FLAG_PHY_UP;
		break;
	/* Otherwise */
	default:
		/* Do nothing */
		break;
	}

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "unauthenticated" state of the receiver state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunUnauthenticatedState(XHdcp1x_Rx *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {

	/* For authenticate */
	case EVENT_AUTHENTICATE:
		*NextStatePtr = STATE_COMPUTATIONS;
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

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "computations" state of the receiver state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunComputationsState(XHdcp1x_Rx *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {

	/* For authenticate */
	case EVENT_AUTHENTICATE:
		StartComputations(InstancePtr, NextStatePtr);
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

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "authenticated" state of the receiver state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunAuthenticatedState(XHdcp1x_Rx *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {

	/* For authenticate */
	case EVENT_AUTHENTICATE:
		*NextStatePtr = STATE_COMPUTATIONS;
		break;
	/* For check */
	case EVENT_CHECK:
		CheckLinkIntegrity(InstancePtr, NextStatePtr);
		break;
	/* For disable */
	case EVENT_DISABLE:
		*NextStatePtr = STATE_DISABLED;
		break;
	/* For physical layer down */
	case EVENT_PHYDOWN:
		*NextStatePtr = STATE_PHYDOWN;
		break;
	/* For update Ri */
	case EVENT_UPDATERi:
		UpdateRi(InstancePtr, NextStatePtr);
		break;
	/* Otherwise */
	default:
		/* Do nothing */
		break;
	}

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "link integrity failed" state of the receiver state
* machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunLinkIntegrityFailedState(XHdcp1x_Rx *InstancePtr, tEvent Event,
		tState *NextStatePtr)
{
	/* Which event? */
	switch (Event) {

	/* For authenticate */
	case EVENT_AUTHENTICATE:
		*NextStatePtr = STATE_COMPUTATIONS;
		break;
	/* For check */
	case EVENT_CHECK:
		CheckLinkIntegrity(InstancePtr, NextStatePtr);
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

	return;
}

/*****************************************************************************/
/**
*
* This function runs the "physical layer down" state of the receiver state
* machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void RunPhysicalLayerDownState(XHdcp1x_Rx *InstancePtr, tEvent Event,
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
		break;
	/* Otherwise */
	default:
		/* Do nothing */
		break;
	}

	return;
}

/*****************************************************************************/
/**
*
* This function enters a hdcp receiver state
*
* @param InstancePtr  the receiver instance
* @param State  the state to enter
* @param NextStatePtr  the next state
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void EnterState(XHdcp1x_Rx *InstancePtr, tState State,
		tState *NextStatePtr)
{
	/* Which state? */
	switch (State) {

	/* For the disabled state */
	case STATE_DISABLED:
		Disable(InstancePtr);
		break;
	/* For the unauthenticated state */
	case STATE_UNAUTHENTICATED:
		InstancePtr->Flags |= FLAG_PHY_UP;
		break;
	/* For the computations state */
	case STATE_COMPUTATIONS:
		StartComputations(InstancePtr, NextStatePtr);
		break;
	/* For the authenticated state */
	case STATE_AUTHENTICATED:
		DebugLog(InstancePtr, "authenticated");
		SetCheckLinkState(InstancePtr, TRUE);
		break;
	/* For the link integrity failed state */
	case STATE_LINKINTEGRITYFAILED:
		InstancePtr->Stats.LinkFailures++;
		ReportLinkIntegrityFailure(InstancePtr, NextStatePtr);
		break;
	/* For physical layer down */
	case STATE_PHYDOWN:
		InstancePtr->Flags &= ~FLAG_PHY_UP;
		XHdcp1x_CipherDisable(&(InstancePtr->Cipher));
		break;
	/* Otherwise */
	default:
		/* Do nothing */
		break;
	}

	return;
}

/*****************************************************************************/
/**
*
* This function exits a hdcp receiver state
*
* @param InstancePtr  the receiver instance
* @param State  the state to exit
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void ExitState(XHdcp1x_Rx *InstancePtr, tState State)
{
	/* Which state? */
	switch (State) {

	/* For the disabled state */
	case STATE_DISABLED:
		Enable(InstancePtr);
		break;
	/* For the authenticated state */
	case STATE_AUTHENTICATED:
		SetCheckLinkState(InstancePtr, FALSE);
		break;
	/* For physical layer down */
	case STATE_PHYDOWN:
		XHdcp1x_CipherEnable(&(InstancePtr->Cipher));
		break;
	/* Otherwise */
	default:
		/* Do nothing */
		break;
	}

	return;
}

/*****************************************************************************/
/**
*
* This function drives a hdcp receiver state machine
*
* @param InstancePtr  the receiver instance
* @param Event  the event to process
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void DoTheState(XHdcp1x_Rx *InstancePtr, tEvent Event)
{
	tState NextState = InstancePtr->CurrentState;

	/* Which state? */
	switch (InstancePtr->CurrentState) {

	/* For the disabled state */
	case STATE_DISABLED:
		RunDisabledState(InstancePtr, Event, &NextState);
		break;
	/* For the unauthenticated state */
	case STATE_UNAUTHENTICATED:
		RunUnauthenticatedState(InstancePtr, Event, &NextState);
		break;
	/* For the computations state */
	case STATE_COMPUTATIONS:
		RunComputationsState(InstancePtr, Event, &NextState);
		break;
	/* For the authenticated state */
	case STATE_AUTHENTICATED:
		RunAuthenticatedState(InstancePtr, Event, &NextState);
		break;
	/* For the link integrity failed state */
	case STATE_LINKINTEGRITYFAILED:
		RunLinkIntegrityFailedState(InstancePtr, Event, &NextState);
		break;
	/* For the physical layer down state */
	case STATE_PHYDOWN:
		RunPhysicalLayerDownState(InstancePtr, Event, &NextState);
		break;
	/* Otherwise */
	default:
		break;
	}

	/* Check for state change */
	while (InstancePtr->CurrentState != NextState) {

		/* Perform the state transition */
		ExitState(InstancePtr, InstancePtr->CurrentState);
		InstancePtr->PreviousState = InstancePtr->CurrentState;
		InstancePtr->CurrentState = NextState;
		EnterState(InstancePtr, InstancePtr->CurrentState, &NextState);
	}

	return;
}

/*****************************************************************************/
/**
*
* This function initializes a hdcp receiver state machine
*
* @param InstancePtr  the receiver instance
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Init(XHdcp1x_Rx *InstancePtr)
{
	tState DummyState = STATE_DISABLED;

	/* Update theHandler */
	InstancePtr->PendingEvents = 0;

	/* Kick the state machine */
	EnterState(InstancePtr, STATE_DISABLED, &DummyState);

	return;
}

/*****************************************************************************/
/**
*
* This function processes the events pending on a state machine
*
* @param InstancePtr  the receiver instance
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void ProcessPending(XHdcp1x_Rx *InstancePtr)
{
	/* Check for any pending events */
	if (InstancePtr->PendingEvents != 0) {
		u16 Pending = InstancePtr->PendingEvents;
		tEvent Event = EVENT_NULL;

		/* Update InstancePtr */
		InstancePtr->PendingEvents = 0;

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

	return;
}

/*****************************************************************************/
/**
*
* This function initializes the hdcp receiver module
*
* @param InstancePtr  the receiver instance
* @param CfgPtr  the configuration of the instance
* @param PhyIfPtr  pointer to the underlying physical interface
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxCfgInitialize(XHdcp1x_Rx *InstancePtr,
		const XHdcp1x_Config *CfgPtr, void* PhyIfPtr)
{
	XHdcp1x_Cipher *CipherPtr = NULL;
	XHdcp1x_Port *PortPtr = NULL;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(PhyIfPtr != NULL);

	/* Initialize InstancePtr */
	memset(InstancePtr, 0, sizeof(XHdcp1x_Rx));
	InstancePtr->CfgPtr = CfgPtr;

	/* Initialize cipher, port and state machine */
	CipherPtr = &InstancePtr->Cipher;
	PortPtr = &InstancePtr->Port;
	Status = XHdcp1x_PortCfgInitialize(PortPtr, CfgPtr, PhyIfPtr);
	if (Status == XST_SUCCESS) {
		Status = XHdcp1x_CipherCfgInitialize(CipherPtr, CfgPtr);
		if (Status == XST_SUCCESS) {
			Init(InstancePtr);
		}
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function polls the hdcp receiver module
*
* @param InstancePtr  the receiver instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
 ******************************************************************************/
int XHdcp1x_RxPoll(XHdcp1x_Rx *InstancePtr)
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
*
* This function resets an hdcp interface
*
* @param InstancePtr  the transmitter instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   This function disables and then re-enables the interface.
*
 ******************************************************************************/
int XHdcp1x_RxReset(XHdcp1x_Rx *InstancePtr)
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
*
* This function enables a hdcp receive interface
*
* @param InstancePtr  the receiver instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxEnable(XHdcp1x_Rx *InstancePtr)
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
*
* This function disables a hdcp receive interface
*
* @param InstancePtr  the receiver instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxDisable(XHdcp1x_Rx *InstancePtr)
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
*
* This function updates the physical state of an hdcp interface
*
* @param InstancePtr  the receiver instance
* @param IsUp  truth value indicating the status of physical interface
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxSetPhysicalState(XHdcp1x_Rx *InstancePtr, int IsUp)
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
*
* This function set the lane count of an hdcp interface
*
* @param InstancePtr  the receiver instance
* @param LaneCount  the number of lanes of the interface
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxSetLaneCount(XHdcp1x_Rx *InstancePtr, int LaneCount)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(LaneCount > 0);

	/* Set it */
	return (XHdcp1x_CipherSetNumLanes(&(InstancePtr->Cipher), LaneCount));
}

/*****************************************************************************/
/**
*
* This function initiates authentication on an interface
*
* @param InstancePtr  the receiver instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxAuthenticate(XHdcp1x_Rx *InstancePtr)
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
*
* This function queries an interface to check if its been authenticated
*
* @param InstancePtr  the receiver instance
*
* @return
*   Truth value indicating authenticated (true) or not (false)
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxIsAuthenticated(const XHdcp1x_Rx *InstancePtr)
{
	int IsAuthenticated = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine IsAuthenticated */
	if (InstancePtr->CurrentState == STATE_AUTHENTICATED) {
		IsAuthenticated = TRUE;
	}

	return (IsAuthenticated);
}

/*****************************************************************************/
/**
*
* This function retrieves the current encryption stream map
*
* @param InstancePtr  the receiver instance
*
* @return
*   The current encryption stream map
*
* @note
*   None.
*
******************************************************************************/
u64 XHdcp1x_RxGetEncryption(const XHdcp1x_Rx *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Get it */
	return (XHdcp1x_CipherGetEncryption(&(InstancePtr->Cipher)));
}

/*****************************************************************************/
/**
*
* This function converts from a state to a display string
*
* @param State  the state to convert
*
* @return
*   The corresponding display string
*
* @note
*   None.
*
******************************************************************************/
static const char* StateToString(tState State)
{
	const char* String = NULL;

	/* Which state? */
	switch (State) {

	case STATE_DISABLED:
		String = "disabled";
		break;
	case STATE_UNAUTHENTICATED:
		String = "unauthenticated";
		break;
	case STATE_COMPUTATIONS:
		String = "computations";
		break;
	case STATE_AUTHENTICATED:
		String = "authenticated";
		break;
	case STATE_LINKINTEGRITYFAILED:
		String = "link-integrity-failed";
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

/*****************************************************************************/
/**
*
* This function implements the debug display output for receiver instances
*
* @param InstancePtr  the receiver instance
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_RxInfo(const XHdcp1x_Rx *InstancePtr)
{
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Display it */
	XHDCP1X_DEBUG_PRINTF("Type:            ");
	if (IsHDMI(InstancePtr)) {
		XHDCP1X_DEBUG_PRINTF("hdmi-rx\r\n");
	}
	else {
		XHDCP1X_DEBUG_PRINTF("dp-rx\r\n");
	}
	XHDCP1X_DEBUG_PRINTF("Current State:   %s\r\n",
			StateToString(InstancePtr->CurrentState));
	XHDCP1X_DEBUG_PRINTF("Previous State:  %s\r\n",
			StateToString(InstancePtr->PreviousState));
	XHDCP1X_DEBUG_PRINTF("Flags:           %04X\r\n",
			InstancePtr->Flags);
	Version = XHdcp1x_GetDriverVersion();
	XHDCP1X_DEBUG_PRINTF("Driver Version:  %d.%02d.%02d\r\n",
			((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
			(Version & 0xFFu));
	Version = XHdcp1x_CipherGetVersion(&(InstancePtr->Cipher));
	XHDCP1X_DEBUG_PRINTF("Cipher Version:  %d.%02d.%02d\r\n",
			((Version >> 16) &0xFFFFu), ((Version >> 8) & 0xFFu),
			(Version & 0xFFu));
	XHDCP1X_DEBUG_PRINTF("\r\n");
	XHDCP1X_DEBUG_PRINTF("Rx Stats\r\n");
	XHDCP1X_DEBUG_PRINTF("Auth Attempts:   %d\r\n",
			InstancePtr->Stats.AuthAttempts);
	XHDCP1X_DEBUG_PRINTF("Link Failures:   %d\r\n",
			InstancePtr->Stats.LinkFailures);
	XHDCP1X_DEBUG_PRINTF("Ri Updates:      %d\r\n",
			InstancePtr->Stats.RiUpdates);

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
