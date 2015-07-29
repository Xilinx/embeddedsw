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
* @file xhdcp1x.c
*
* This contains the implementation of the HDCP state machine module
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
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_debug.h"
#include "xhdcp1x_port.h"
#include "xhdcp1x_rx.h"
#include "xhdcp1x_tx.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#if defined(XPAR_XHDMI_TX_NUM_INSTANCES) && (XPAR_XHDMI_TX_NUM_INSTANCES > 0)
	#define INCLUDE_TX
#endif
#if defined(XPAR_XHDMI_RX_NUM_INSTANCES) && (XPAR_XHDMI_RX_NUM_INSTANCES > 0)
	#define INCLUDE_RX
#endif
#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_NUM_INSTANCES > 0)
	#define INCLUDE_RX
	#define INCLUDE_TX
#endif

/**
 * This defines the version of the software driver
 */
#define	DRIVER_VERSION			(0x00010023ul)

/**************************** Type Definitions *******************************/

/************************** Extern Declarations ******************************/

/************************** Global Declarations ******************************/
XHdcp1x_Printf  XHdcp1xDebugPrintf = NULL;
XHdcp1x_LogMsg  XHdcp1xDebugLogMsg = NULL;
XHdcp1x_KsvRevokeCheck  XHdcp1xKsvRevokeCheck = NULL;
XHdcp1x_TimerStart  XHdcp1xTimerStart = NULL;
XHdcp1x_TimerStop  XHdcp1xTimerStop = NULL;
XHdcp1x_TimerDelay  XHdcp1xTimerDelay = NULL;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This queries an interface to determine if it is a receiver
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating receiver (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsRX(InstancePtr)  		(InstancePtr->Common.CfgPtr->IsRx)

/*****************************************************************************/
/**
*
* This queries an interface to determine if it is a transmitter
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating transmitter (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsTX(InstancePtr)  		(!InstancePtr->Common.CfgPtr->IsRx)

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
#define IsDP(InstancePtr)  		(!InstancePtr->Common.CfgPtr->IsHDMI)

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
#define IsHDMI(InstancePtr)  		(InstancePtr->Common.CfgPtr->IsHDMI)

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function determines the adaptor for a specified port device
*
* @param InstancePtr  the device whose adaptor is to be determined
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
int XHdcp1x_CfgInitialize(XHdcp1x *InstancePtr, const XHdcp1x_Config *CfgPtr,
		void* PhyIfPtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Common.IsReady !=
			XIL_COMPONENT_IS_READY);

	/* Initialize InstancePtr */
	memset(InstancePtr, 0, sizeof(XHdcp1x));
	InstancePtr->Common.CfgPtr = CfgPtr;

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxCfgInitialize(&(InstancePtr->Tx),
				CfgPtr, PhyIfPtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxCfgInitialize(&(InstancePtr->Rx),
				CfgPtr, PhyIfPtr);
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	/* Update IsReady */
	if (Status == XST_SUCCESS) {
		InstancePtr->Common.IsReady = XIL_COMPONENT_IS_READY;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function polls an hdcp interface
*
* @param InstancePtr  the interface to poll
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_Poll(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxPoll(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxPoll(&(InstancePtr->Rx));
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function resets an hdcp interface
*
* @param InstancePtr  the interface to reset
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_Reset(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxReset(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxReset(&(InstancePtr->Rx));
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function enables an hdcp interface
*
* @param InstancePtr  the interface to enable
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_Enable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxEnable(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxEnable(&(InstancePtr->Rx));
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function disables an hdcp interface
*
* @param InstancePtr  the interface to disable
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_Disable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxDisable(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxDisable(&(InstancePtr->Rx));
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function updates the state of the underlying physical interface
*
* @param InstancePtr  the interface to update
* @param IsUp  truth value indicating the underlying physical interface state
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_SetPhysicalState(XHdcp1x *InstancePtr, int IsUp)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxSetPhysicalState(&(InstancePtr->Tx), IsUp);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxSetPhysicalState(&(InstancePtr->Rx), IsUp);
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function sets the lane count of a hdcp interface
*
* @param InstancePtr  the interface to update
* @param LaneCount  the lane count
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_SetLaneCount(XHdcp1x *InstancePtr, int LaneCount)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if ((IsTX(InstancePtr)) && (IsDP(InstancePtr))) {
		Status = XHdcp1x_TxSetLaneCount(&(InstancePtr->Tx), LaneCount);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if ((IsRX(InstancePtr)) && (IsDP(InstancePtr))) {
		Status = XHdcp1x_RxSetLaneCount(&(InstancePtr->Rx), LaneCount);
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function initiates authentication of an hdcp interface
*
* @param InstancePtr  the interface to initiate authentication on
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_Authenticate(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxAuthenticate(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxAuthenticate(&(InstancePtr->Rx));
	}
	else
#endif
	{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function queries an interface to determine if authentication is in
* progress
*
* @param InstancePtr  the interface to query
*
* @return
*   Truth value indicating authentication in progress (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_IsInProgress(const XHdcp1x *InstancePtr)
{
	int IsInProgress = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		IsInProgress = XHdcp1x_TxIsInProgress(&(InstancePtr->Tx));
	}
#endif

	return (IsInProgress);
}

/*****************************************************************************/
/**
*
* This function queries an interface to determine if it has successfully
* completed authentication
*
* @param InstancePtr  the interface to query
*
* @return
*   Truth value indicating authenticated (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_IsAuthenticated(const XHdcp1x *InstancePtr)
{
	int IsAuth = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		IsAuth = XHdcp1x_TxIsAuthenticated(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		IsAuth = XHdcp1x_RxIsAuthenticated(&(InstancePtr->Rx));
	}
	else
#endif
	{
		IsAuth = FALSE;
	}

	return (IsAuth);
}

/*****************************************************************************/
/**
*
* This function retrieves the current encryption map of the video streams
* traversing an hdcp interface
*
* @param InstancePtr  the interface to query
*
* @return
*   The current encryption map
*
* @note
*   None.
*
******************************************************************************/
u64 XHdcp1x_GetEncryption(const XHdcp1x *InstancePtr)
{
	u64 StreamMap = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		StreamMap = XHdcp1x_TxGetEncryption(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		StreamMap = XHdcp1x_RxGetEncryption(&(InstancePtr->Rx));
	}
	else
#endif
	{
		StreamMap = 0;
	}

	return (StreamMap);
}

/*****************************************************************************/
/**
*
* This function enables encryption on a series of streams within an hdcp
* interface
*
* @param InstancePtr  the interface to configure
* @param Map  the stream map to enable encryption on
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_EnableEncryption(XHdcp1x *InstancePtr, u64 Map)
{
	int Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxEnableEncryption(&(InstancePtr->Tx), Map);
	}
#endif

	return (Status);
}

/*****************************************************************************/
/**
*
* This function disables encryption on a series of streams within an hdcp
* interface
*
* @param InstancePtr  the interface to configure
* @param Map  the stream map to disable encryption on
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_DisableEncryption(XHdcp1x *InstancePtr, u64 Map)
{
	int Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxDisableEncryption(&(InstancePtr->Tx), Map);
	}
#endif

	return (Status);
}

/*****************************************************************************/
/**
*
* This function sets the key selection vector that is to be used by the hdcp
* cipher
*
* @param InstancePtr  the interface to configure
* @param KeySelect  the key selection vector
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_SetKeySelect(XHdcp1x *InstancePtr, u8 KeySelect)
{
	XHdcp1x_Cipher *CipherPtr = NULL;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		CipherPtr = &(InstancePtr->Tx.Cipher);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		CipherPtr = &(InstancePtr->Rx.Cipher);
	}
	else
#endif
	{
		CipherPtr = NULL;
	}

	/* Set it */
	if (CipherPtr != NULL) {
		Status = XHdcp1x_CipherSetKeySelect(CipherPtr, KeySelect);
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function handles a timeout on an hdcp interface
*
* @param InstancePtr  the interface
*
* @return
*  void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_HandleTimeout(void *InstancePtr)
{
	XHdcp1x *HdcpPtr = InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(HdcpPtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(HdcpPtr)) {
		XHdcp1x_TxHandleTimeout(&(HdcpPtr->Tx));
	}
#endif

	return;
}


/*****************************************************************************/
/**
*
* This function sets the debug printf function for the module
*
* @param PrintfFunc  the printf function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetDebugPrintf(XHdcp1x_Printf PrintfFunc)
{
	XHdcp1xDebugPrintf = PrintfFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function sets the debug log message function for the module
*
* @param LogFunc  the debug logging function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetDebugLogMsg(XHdcp1x_LogMsg LogFunc)
{
	XHdcp1xDebugLogMsg = LogFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function sets the KSV revocation list check function for the module
*
* @param RevokeCheckFunc  the KSV revocation list check function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetKsvRevokeCheck(XHdcp1x_KsvRevokeCheck RevokeCheckFunc)
{
	XHdcp1xKsvRevokeCheck = RevokeCheckFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function sets timer start function for the module
*
* @param TimerStartFunc  the timer start function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetTimerStart(XHdcp1x_TimerStart TimerStartFunc)
{
	XHdcp1xTimerStart = TimerStartFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function sets timer stop function for the module
*
* @param TimerStopFunc  the timer stop function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetTimerStop(XHdcp1x_TimerStop TimerStopFunc)
{
	XHdcp1xTimerStop = TimerStopFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function sets timer busy delay function for the module
*
* @param TimerDelayFunc  the timer busy delay function
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_SetTimerDelay(XHdcp1x_TimerDelay TimerDelayFunc)
{
	XHdcp1xTimerDelay = TimerDelayFunc;
	return;
}

/*****************************************************************************/
/**
*
* This function retrieves the version of the hdcp driver software
*
* @return
*   The software driver version
*
* @note
*   None.
*
******************************************************************************/
u32 XHdcp1x_GetDriverVersion(void)
{
	return (DRIVER_VERSION);
}

/*****************************************************************************/
/**
*
* This function retrieves the cipher version of an hdcp interface
*
* @param InstancePtr  the interface to query
*
* @return
*   The cipher version used by the interface
*
* @note
*   None.
*
******************************************************************************/
u32 XHdcp1x_GetVersion(const XHdcp1x *InstancePtr)
{
	const XHdcp1x_Cipher *CipherPtr = NULL;
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		CipherPtr = &(InstancePtr->Tx.Cipher);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		CipherPtr = &(InstancePtr->Rx.Cipher);
	}
	else
#endif
	{
		CipherPtr = NULL;
	}

	/* Set it */
	if (CipherPtr != NULL) {
		Version = XHdcp1x_CipherGetVersion(CipherPtr);
	}

	return (Version);
}

/*****************************************************************************/
/**
*
* This function performs a debug display of an hdcp instance
*
* @param InstancePtr  the interface to display
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_Info(const XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		XHdcp1x_TxInfo(&(InstancePtr->Tx));
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		XHdcp1x_RxInfo(&(InstancePtr->Rx));
	}
	else
#endif
	{
		XHDCP_DEBUG_PRINTF("unknown interface type\r\n");
	}

	return;
}
