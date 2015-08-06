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
* @file xhdcp1x.c
*
* This contains the implementation of the HDCP state machine module
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

XHdcp1x_Printf XHdcp1xDebugPrintf = NULL;
XHdcp1x_LogMsg XHdcp1xDebugLogMsg = NULL;
XHdcp1x_KsvRevokeCheck XHdcp1xKsvRevokeCheck = NULL;
XHdcp1x_TimerStart XHdcp1xTimerStart = NULL;
XHdcp1x_TimerStop XHdcp1xTimerStop = NULL;
XHdcp1x_TimerDelay XHdcp1xTimerDelay = NULL;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* This queries an interface to determine if it is a receiver.
*
* @param	InstancePtr is the instance to query.
*
* @return	Truth value indicating receiver (TRUE) or not (FALSE).
*
* @note		None.
*
******************************************************************************/
#define IsRX(InstancePtr) ((InstancePtr)->Config.IsRx)

/*****************************************************************************/
/**
* This queries an interface to determine if it is a transmitter.
*
* @param	InstancePtr is the instance to query.
*
* @return	Truth value indicating transmitter (TRUE) or not (FALSE).
*
* @note		None.
*
******************************************************************************/
#define IsTX(InstancePtr) (!(InstancePtr)->Config.IsRx)

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
#define IsDP(InstancePtr) (!(InstancePtr)->Config.IsHDMI)

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
#define IsHDMI(InstancePtr) ((InstancePtr)->Config.IsHDMI)

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function retrieves the configuration for this HDCP instance and fills
* in the InstancePtr->Config structure.
*
* @param	InstancePtr is the device whose adaptor is to be determined.
* @param	CfgPtr is the configuration of the instance.
* @param	PhyIfPtr is pointer to the underlying physical interface.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_CfgInitialize(XHdcp1x *InstancePtr, const XHdcp1x_Config *CfgPtr,
		void *PhyIfPtr)
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
	InstancePtr->Config = *CfgPtr;

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		Status = XHdcp1x_TxCfgInitialize(InstancePtr, CfgPtr, PhyIfPtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxCfgInitialize(InstancePtr, CfgPtr, PhyIfPtr);
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
* This function polls an HDCP interface.
*
* @param	InstancePtr is the interface to poll.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxPoll(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxPoll(InstancePtr);
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
* This function resets an HDCP interface.
*
* @param InstancePtr is the interface to reset.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxReset(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxReset(InstancePtr);
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
* This function enables an HDCP interface.
*
* @param	InstancePtr is the interface to enable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxEnable(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxEnable(InstancePtr);
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
* This function disables an HDCP interface.
*
* @param	InstancePtr is the interface to disable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxDisable(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxDisable(InstancePtr);
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
* This function updates the state of the underlying physical interface.
*
* @param	InstancePtr is the interface to update.
* @param	IsUp indicates the state of the underlying physical interface.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxSetPhysicalState(InstancePtr, IsUp);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxSetPhysicalState(InstancePtr, IsUp);
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
* This function sets the lane count of a hdcp interface
*
* @param	InstancePtr is the interface to update.
* @param	LaneCount is the lane count.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxSetLaneCount(InstancePtr, LaneCount);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if ((IsRX(InstancePtr)) && (IsDP(InstancePtr))) {
		Status = XHdcp1x_RxSetLaneCount(InstancePtr, LaneCount);
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
* This function initiates authentication of an HDCP interface.
*
* @param	InstancePtr is the interface to initiate authentication on.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxAuthenticate(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		Status = XHdcp1x_RxAuthenticate(InstancePtr);
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
* This function queries an interface to determine if authentication is in
* progress.
*
* @param	InstancePtr is the interface to query.
*
* @return	Truth value indicating authentication in progress (TRUE) or not
*		(FALSE).
*
* @note		None.
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
		IsInProgress = XHdcp1x_TxIsInProgress(InstancePtr);
	}
#endif

	return (IsInProgress);
}

/*****************************************************************************/
/**
* This function queries an interface to determine if it has successfully
* completed authentication.
*
* @param	InstancePtr is the interface to query.
*
* @return	Truth value indicating authenticated (TRUE) or not (FALSE).
*
* @note		None.
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
		IsAuth = XHdcp1x_TxIsAuthenticated(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		IsAuth = XHdcp1x_RxIsAuthenticated(InstancePtr);
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
* This function retrieves the current encryption map of the video streams
* traversing an hdcp interface.
*
* @param InstancePtr is the interface to query.
*
* @return	The current encryption map.
*
* @note		None.
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
		StreamMap = XHdcp1x_TxGetEncryption(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		StreamMap = XHdcp1x_RxGetEncryption(InstancePtr);
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
* This function enables encryption on a series of streams within an HDCP
* interface.
*
* @param	InstancePtr is the interface to configure.
* @param	Map is the stream map to enable encryption on.
*
* @return	XST_SUCCESS if successful.
*		XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxEnableEncryption(InstancePtr, Map);
	}
#endif

	return (Status);
}

/*****************************************************************************/
/**
* This function disables encryption on a series of streams within an HDCP
* interface.
*
* @param	InstancePtr is the interface to configure.
* @param	Map is the stream map to disable encryption on.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
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
		Status = XHdcp1x_TxDisableEncryption(InstancePtr, Map);
	}
#endif

	return (Status);
}

/*****************************************************************************/
/**
* This function sets the key selection vector that is to be used by the HDCP
* cipher.
*
* @param	InstancePtr is the interface to configure.
* @param	KeySelect is the key selection vector.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_SetKeySelect(XHdcp1x *InstancePtr, u8 KeySelect)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XHdcp1x_CipherSetKeySelect(InstancePtr, KeySelect);

	return (Status);
}

/*****************************************************************************/
/**
* This function handles a timeout on an HDCP interface.
*
* @param	InstancePtr is the interface.
*
* @return	None.
*
* @note		None.
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
		XHdcp1x_TxHandleTimeout(HdcpPtr);
	}
#endif
}


/*****************************************************************************/
/**
* This function sets the debug printf function for the module.
*
* @param	PrintfFunc is the printf function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetDebugPrintf(XHdcp1x_Printf PrintfFunc)
{
	XHdcp1xDebugPrintf = PrintfFunc;
}

/*****************************************************************************/
/**
* This function sets the debug log message function for the module.
*
* @param	LogFunc is the debug logging function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetDebugLogMsg(XHdcp1x_LogMsg LogFunc)
{
	XHdcp1xDebugLogMsg = LogFunc;
}

/*****************************************************************************/
/**
* This function sets the KSV revocation list check function for the module.
*
* @param	RevokeCheckFunc is the KSV revocation list check function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetKsvRevokeCheck(XHdcp1x_KsvRevokeCheck RevokeCheckFunc)
{
	XHdcp1xKsvRevokeCheck = RevokeCheckFunc;
}

/*****************************************************************************/
/**
* This function sets timer start function for the module.
*
* @param	TimerStartFunc is the timer start function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetTimerStart(XHdcp1x_TimerStart TimerStartFunc)
{
	XHdcp1xTimerStart = TimerStartFunc;
}

/*****************************************************************************/
/**
* This function sets timer stop function for the module.
*
* @param	TimerStopFunc is the timer stop function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetTimerStop(XHdcp1x_TimerStop TimerStopFunc)
{
	XHdcp1xTimerStop = TimerStopFunc;
}

/*****************************************************************************/
/**
* This function sets timer busy delay function for the module.
*
* @param	TimerDelayFunc is the timer busy delay function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_SetTimerDelay(XHdcp1x_TimerDelay TimerDelayFunc)
{
	XHdcp1xTimerDelay = TimerDelayFunc;
}

/*****************************************************************************/
/**
* This function retrieves the version of the HDCP driver software.
*
* @return	The software driver version.
*
* @note		None.
*
******************************************************************************/
u32 XHdcp1x_GetDriverVersion(void)
{
	return (DRIVER_VERSION);
}

/*****************************************************************************/
/**
* This function retrieves the cipher version of an HDCP interface.
*
* @param	InstancePtr is the interface to query.
*
* @return	The cipher version used by the interface
*
* @note		None.
*
******************************************************************************/
u32 XHdcp1x_GetVersion(const XHdcp1x *InstancePtr)
{
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Version = XHdcp1x_CipherGetVersion(InstancePtr);

	return (Version);
}

/*****************************************************************************/
/**
*
* This function performs a debug display of an HDCP instance.
*
* @param	InstancePtr is the interface to display.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdcp1x_Info(const XHdcp1x *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

#if defined(INCLUDE_TX)
	/* Check for TX */
	if (IsTX(InstancePtr)) {
		XHdcp1x_TxInfo(InstancePtr);
	}
	else
#endif
#if defined(INCLUDE_RX)
	/* Check for RX */
	if (IsRX(InstancePtr)) {
		XHdcp1x_RxInfo(InstancePtr);
	}
	else
#endif
	{
		XHDCP1X_DEBUG_PRINTF("unknown interface type\r\n");
	}
}
