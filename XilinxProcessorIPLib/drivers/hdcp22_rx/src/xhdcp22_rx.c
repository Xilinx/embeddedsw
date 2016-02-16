/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp22_rx.c
* @addtogroup hdcp22_rx_v1_0
* @{
* @details
*
* This file contains the main implementation of the Xilinx HDCP 2.2 Receiver
* device driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release
* 1.01  MH   01/15/16 Updated function XHdcp22Rx_SetDdcReauthReq to remove
*                     static. Replaced function XHdcp22Rx_SetDdcHandles
*                     with XHdcp22Rx_SetCallback. Added callback for
*                     authenticated event.
*</pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "stdio.h"
#include "string.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xparameters.h"
#include "xhdcp22_rx_i.h"
#include "xhdcp22_rx.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
/* Functions for initializing subcores */
static int  XHdcp22Rx_InitializeCipher(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_InitializeMmult(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_InitializeRng(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_InitializeTimer(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_ComputeBaseAddress(u32 BaseAddress, u32 SubcoreOffset, u32 *SubcoreAddressPtr);

/* Functions for generating authentication parameters */
static int  XHdcp22Rx_GenerateRrx(XHdcp22_Rx *InstancePtr, u8 *RrxPtr);

/* Functions for performing various tasks during authentication */
static u8   XHdcp22Rx_IsWriteMessageAvailable(XHdcp22_Rx *InstancePtr);
static u8   XHdcp22Rx_IsReadMessageComplete(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_SetDdcMessageSize(XHdcp22_Rx *InstancePtr, u16 MessageSize);
void		XHdcp22Rx_SetDdcReauthReq(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_LoadSessionKey(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_ResetAfterError(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_ResetParams(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_ResetDdc(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_PollMessage(XHdcp22_Rx *InstancePtr);
int         XHdcp22Rx_RsaesOaepDecrypt(XHdcp22_Rx *InstancePtr, const XHdcp22_Rx_KprivRx *KprivRx,
										u8 *EncryptedMessage, u8 *Message, int *MessageLen);

/* Functions for implementing the receiver state machine */
static void *XHdcp22Rx_StateB0(XHdcp22_Rx *InstancePtr);
static void *XHdcp22Rx_StateB1(XHdcp22_Rx *InstancePtr);
static void *XHdcp22Rx_StateB2(XHdcp22_Rx *InstancePtr);
static void *XHdcp22Rx_StateB3(XHdcp22_Rx *InstancePtr);
static void *XHdcp22Rx_StateB4(XHdcp22_Rx *InstancePtr);

/* Functions for processing received messages */
static int  XHdcp22Rx_ProcessMessageAKEInit(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_ProcessMessageAKENoStoredKm(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_ProcessMessageAKEStoredKm(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_ProcessMessageLCInit(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_ProcessMessageSKESendEks(XHdcp22_Rx *InstancePtr);

/* Functions for generating and sending messages */
static int  XHdcp22Rx_SendMessageAKESendCert(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_SendMessageAKESendHPrime(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_SendMessageAKESendPairingInfo(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_SendMessageLCSendLPrime(XHdcp22_Rx *InstancePtr);

/* Functions for stub callbacks */
static void XHdcp22_Rx_StubRunHandler(void *HandlerRef);
static void XHdcp22_Rx_StubSetHandler(void *HandlerRef, u32 Data);
static u32  XHdcp22_Rx_StubGetHandler(void *HandlerRef);

/****************************************************************************/
/**
* Initialize the instance provided by the caller based on the given
* configuration data.
*
* @param 	InstancePtr is a pointer to an XHdcp22_Rx instance.
*			The memory the pointer references must be pre-allocated by
*			the caller. Further calls to manipulate the driver through
*			the HDCP22-RX API must be made with this pointer.
* @param	Config is a reference to a structure containing information
*			about a specific HDCP22-RX device. This function
*			initializes an InstancePtr object for a specific device
*			specified by the contents of Config. This function can
*			initialize multiple instance objects with the use of multiple
*			calls giving different Config information on each call.
* @param	EffectiveAddr is the base address of the device. If address
*			translation is being used, then this parameter must reflect the
*			virtual base address. Otherwise, the physical address should be
*			used.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
int XHdcp22Rx_CfgInitialize(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_Config *ConfigPtr, u32 EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

	int Status;

	/* Clear the instance */
	memset((void *)InstancePtr, 0, sizeof(XHdcp22_Rx));

	/* Copy configuration settings */
	memcpy((void *)&(InstancePtr->Config), (const void *)ConfigPtr, sizeof(XHdcp22_Rx_Config));

	/* Set default values */
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->StateFunc = (XHdcp22_Rx_StateFunc)(&XHdcp22Rx_StateB0);
	InstancePtr->Info.IsEnabled = FALSE;
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_UNAUTHENTICATED;
	InstancePtr->Info.IsNoStoredKm = FALSE;
	InstancePtr->Info.LCInitAttempts = 0;
	InstancePtr->Info.ErrorFlag = XHDCP22_RX_ERROR_FLAG_NONE;
	InstancePtr->Info.ErrorFlagSticky = XHDCP22_RX_ERROR_FLAG_NONE;
	InstancePtr->Info.CurrentState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;
	InstancePtr->Info.NextState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;

	/* Set the callback functions to stubs */
	InstancePtr->Handles.DdcSetAddressCallback = XHdcp22_Rx_StubSetHandler;
	InstancePtr->Handles.IsDdcSetAddressCallbackSet = (FALSE);

	InstancePtr->Handles.DdcSetDataCallback = XHdcp22_Rx_StubSetHandler;
	InstancePtr->Handles.IsDdcSetDataCallbackSet = (FALSE);

	InstancePtr->Handles.DdcGetDataCallback = XHdcp22_Rx_StubGetHandler;
	InstancePtr->Handles.IsDdcGetDataCallbackSet = (FALSE);

	InstancePtr->Handles.DdcGetWriteBufferSizeCallback = XHdcp22_Rx_StubGetHandler;
	InstancePtr->Handles.IsDdcGetWriteBufferSizeCallbackSet = (FALSE);

	InstancePtr->Handles.DdcGetReadBufferSizeCallback = XHdcp22_Rx_StubGetHandler;
	InstancePtr->Handles.IsDdcGetReadBufferSizeCallbackRefSet = (FALSE);

	InstancePtr->Handles.DdcIsWriteBufferEmptyCallback = XHdcp22_Rx_StubGetHandler;
	InstancePtr->Handles.IsDdcIsWriteBufferEmptyCallbackSet = (FALSE);

	InstancePtr->Handles.DdcIsReadBufferEmptyCallback = XHdcp22_Rx_StubGetHandler;
	InstancePtr->Handles.IsDdcIsReadBufferEmptyCallbackSet = (FALSE);

	InstancePtr->Handles.DdcClearReadBufferCallback = XHdcp22_Rx_StubRunHandler;
	InstancePtr->Handles.IsDdcClearReadBufferCallbackSet = (FALSE);

	InstancePtr->Handles.DdcClearWriteBufferCallback = XHdcp22_Rx_StubRunHandler;
	InstancePtr->Handles.IsDdcClearWriteBufferCallbackSet = (FALSE);

	InstancePtr->Handles.IsDdcAllCallbacksSet == (FALSE);

	/* Set RXCAPS repeater mode */
	InstancePtr->RxCaps[0] = 0x02;
	InstancePtr->RxCaps[1] = 0x00;
	InstancePtr->RxCaps[2] = (InstancePtr->Config.Mode == XHDCP22_RX_RECEIVER) ? 0x00 : 0x01;

	/* Reset stored parameters */
	XHdcp22Rx_ResetParams(InstancePtr);

	/* Configure Cipher Instance */
	Status = XHdcp22Rx_InitializeCipher(InstancePtr);
	if(Status == XST_FAILURE)
	{
		return Status;
	}

	/* Configure Mmult Instance */
	Status = XHdcp22Rx_InitializeMmult(InstancePtr);
	if(Status == XST_FAILURE)
	{
		return Status;
	}

	/* Configure Rng Instance */
	Status = XHdcp22Rx_InitializeRng(InstancePtr);
	if(Status == XST_FAILURE)
	{
		return Status;
	}

	/* Configure Timer Instance */
	Status = XHdcp22Rx_InitializeTimer(InstancePtr);
	if(Status == XST_FAILURE)
	{
		return Status;
	}

	/* Reset log */
	XHdcp22Rx_LogReset(InstancePtr, FALSE);

	/* Indicate component has been initialized */
	InstancePtr->IsReady = (XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function resets the HDCP22-RX system to the default state.
* The HDCP22-RX DDC registers are set to their default value
* and the message buffer is reset.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		The DDC message handles must be assigned by the
* 			XHdcp22Rx_SetDdcHandles function prior to calling
* 			this reset function.
******************************************************************************/
int XHdcp22Rx_Reset(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	/* Verify DDC handles assigned */
	Xil_AssertNonvoid(InstancePtr->Handles.IsDdcAllCallbacksSet == TRUE);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_RESET);

	/* Clear message buffer */
	memset(&InstancePtr->MessageBuffer, 0, sizeof(XHdcp22_Rx_Message));
	InstancePtr->MessageSize = 0;

	/* Set default values */
	InstancePtr->StateFunc = (XHdcp22_Rx_StateFunc)(&XHdcp22Rx_StateB0);
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_UNAUTHENTICATED;
	InstancePtr->Info.IsNoStoredKm = FALSE;
	InstancePtr->Info.LCInitAttempts = 0;
	InstancePtr->Info.ErrorFlag = XHDCP22_RX_ERROR_FLAG_NONE;
	InstancePtr->Info.ErrorFlagSticky = XHDCP22_RX_ERROR_FLAG_NONE;
	InstancePtr->Info.CurrentState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;
	InstancePtr->Info.NextState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;

	/* Reset stored parameters */
	XHdcp22Rx_ResetParams(InstancePtr);

	/* Reset DDC registers */
	XHdcp22Rx_ResetDdc(InstancePtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function enables the HDCP22-RX state machine.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		Before enabling the state machine ensure that the instance has
*			been initialized, DDC message handles have been assigned, and keys
*			have been loaded.
******************************************************************************/
int XHdcp22Rx_Enable(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	/* Verify DDC handles assigned */
	Xil_AssertNonvoid(InstancePtr->Handles.IsDdcAllCallbacksSet == TRUE);
	/* Verify keys loaded */
	Xil_AssertNonvoid(InstancePtr->PublicCertPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PrivateKeyPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->NPrimePPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->NPrimeQPtr != NULL);
	/* Verify devices ready */
	Xil_AssertNonvoid(InstancePtr->MmultInst.IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->TimerInst.IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->RngInst.IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->CipherInst.IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_ENABLE);

	/* Enable RNG and Cipher */
	XHdcp22Rng_Enable(&InstancePtr->RngInst);
	XHdcp22Cipher_Enable(&InstancePtr->CipherInst);

	/* Assert enabled flag */
	InstancePtr->Info.IsEnabled = TRUE;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function disables the HDCP22-RX state machine.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		The HDCP22-RX cipher is disabled.
******************************************************************************/
int XHdcp22Rx_Disable(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_DISABLE);

	/* Disable RNG and Cipher */
	XHdcp22Rng_Disable(&InstancePtr->RngInst);
	XHdcp22Cipher_Disable(&InstancePtr->CipherInst);

	/* Deassert device enable flag */
	InstancePtr->Info.IsEnabled = FALSE;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function installs callback functions for the given
* HandlerType:
*
* <pre>
* HandlerType                               Callback Function Type
* -------------------------                 ---------------------------
* (XHDCP22_RX_HANDLER_DDC_SETREGADDR)       DdcSetAddressCallback
* (XHDCP22_RX_HANDLER_DDC_SETREGDATA)       DdcSetDataCallback
* (XHDCP22_RX_HANDLER_DDC_GETREGDATA)       DdcGetDataCallback
* (XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE)      DdcGetWriteBufferSizeCallback
* (XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE)      DdcGetReadBufferSizeCallback
* (XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY)      DdcIsWriteBufferEmptyCallback
* (XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY)      DdcIsReadBufferEmptyCallback
* (XHDCP22_RX_HANDLER_DDC_CLEARRBUF)        DdcClearReadBufferCallback
* (XHDCP22_RX_HANDLER_DDC_CLEARWBUF)        DdcClearWriteBufferCallback
* (XHDCP22_RX_HANDLER_AUTHENTICATED)        AuthenticatedCallback
* </pre>
*
* @param	InstancePtr is a pointer to the HDMI RX core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return
*			- XST_SUCCESS if callback function installed successfully.
*			- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*			installed replaces it with the new handler.
*
******************************************************************************/
int XHdcp22Rx_SetCallback(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_HandlerType HandlerType, void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType > (XHDCP22_RX_HANDLER_UNDEFINED));
	Xil_AssertNonvoid(HandlerType < (XHDCP22_RX_HANDLER_INVALID));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	u32 Status;

	/* Check for handler type */
	switch (HandlerType)
	{
		case (XHDCP22_RX_HANDLER_DDC_SETREGADDR):
			InstancePtr->Handles.DdcSetAddressCallback = (XHdcp22_Rx_SetHandler)CallbackFunc;
			InstancePtr->Handles.DdcSetAddressCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcSetAddressCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_SETREGDATA):
			InstancePtr->Handles.DdcSetDataCallback = (XHdcp22_Rx_SetHandler)CallbackFunc;
			InstancePtr->Handles.DdcSetDataCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcSetDataCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_GETREGDATA):
			InstancePtr->Handles.DdcGetDataCallback = (XHdcp22_Rx_GetHandler)CallbackFunc;
			InstancePtr->Handles.DdcGetDataCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcGetDataCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE):
			InstancePtr->Handles.DdcGetWriteBufferSizeCallback = (XHdcp22_Rx_GetHandler)CallbackFunc;
			InstancePtr->Handles.DdcGetWriteBufferSizeCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcGetWriteBufferSizeCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE):
			InstancePtr->Handles.DdcGetReadBufferSizeCallback = (XHdcp22_Rx_GetHandler)CallbackFunc;
			InstancePtr->Handles.DdcGetReadBufferSizeCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcGetReadBufferSizeCallbackRefSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY):
			InstancePtr->Handles.DdcIsWriteBufferEmptyCallback = (XHdcp22_Rx_GetHandler)CallbackFunc;
			InstancePtr->Handles.DdcIsWriteBufferEmptyCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcIsWriteBufferEmptyCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY):
			InstancePtr->Handles.DdcIsReadBufferEmptyCallback = (XHdcp22_Rx_GetHandler)CallbackFunc;
			InstancePtr->Handles.DdcIsReadBufferEmptyCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcIsReadBufferEmptyCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_CLEARRBUF):
			InstancePtr->Handles.DdcClearReadBufferCallback = (XHdcp22_Rx_RunHandler)CallbackFunc;
			InstancePtr->Handles.DdcClearReadBufferCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcClearReadBufferCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_DDC_CLEARWBUF):
			InstancePtr->Handles.DdcClearWriteBufferCallback = (XHdcp22_Rx_RunHandler)CallbackFunc;
			InstancePtr->Handles.DdcClearWriteBufferCallbackRef = CallbackRef;
			InstancePtr->Handles.IsDdcClearWriteBufferCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		case (XHDCP22_RX_HANDLER_AUTHENTICATED):
			InstancePtr->Handles.AuthenticatedCallback = (XHdcp22_Rx_RunHandler)CallbackFunc;
			InstancePtr->Handles.AuthenticatedCallbackRef = CallbackRef;
			InstancePtr->Handles.IsAuthenticatedCallbackSet = (TRUE);
			Status = (XST_SUCCESS);
			break;
		default:
			Status = (XST_INVALID_PARAM);
			break;
	}

	/* Reset DDC registers only when all handlers have been registered */
    if(InstancePtr->Handles.IsDdcSetAddressCallbackSet &&
		InstancePtr->Handles.IsDdcSetDataCallbackSet &&
		InstancePtr->Handles.IsDdcGetDataCallbackSet &&
		InstancePtr->Handles.IsDdcGetWriteBufferSizeCallbackSet &&
		InstancePtr->Handles.IsDdcGetReadBufferSizeCallbackRefSet &&
		InstancePtr->Handles.IsDdcIsWriteBufferEmptyCallbackSet &&
		InstancePtr->Handles.IsDdcIsReadBufferEmptyCallbackSet &&
		InstancePtr->Handles.IsDdcClearReadBufferCallbackSet &&
		InstancePtr->Handles.IsDdcClearWriteBufferCallbackSet &&
        InstancePtr->Handles.IsDdcAllCallbacksSet == (FALSE))
	{
		InstancePtr->Handles.IsDdcAllCallbacksSet = (TRUE);
		XHdcp22Rx_ResetDdc(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function executes the HDCP22-RX state machine.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		State transitions are logged.
******************************************************************************/
void XHdcp22Rx_Poll(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Run state machine */
	if(InstancePtr->Info.IsEnabled == TRUE)
	{
		InstancePtr->StateFunc = (XHdcp22_Rx_StateFunc)(*InstancePtr->StateFunc)(InstancePtr);
	}

	/* Log state transitions */
	if(InstancePtr->Info.NextState != InstancePtr->Info.CurrentState)
	{
	    XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_STATE, InstancePtr->Info.NextState);
	}
}

/*****************************************************************************/
/**
* This function checks if the HDCP22-RX state machine is enabled.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
u8 XHdcp22Rx_IsEnabled(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->Info.IsEnabled;
}

/*****************************************************************************/
/**
* This function checks if the HDCP22-RX cipher encryption is enabled.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
u8 XHdcp22Rx_IsEncryptionEnabled(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Status;

	Status = XHdcp22Cipher_IsEncrypted(&InstancePtr->CipherInst);

	return (Status) ? TRUE : FALSE;
}

/*****************************************************************************/
/**
* This function checks if the HDCP22-RX state machine is enabled but
* not yet in the Authenticated state.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
u8 XHdcp22Rx_IsInProgress(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return ((InstancePtr->Info.IsEnabled == TRUE) &&
			(InstancePtr->Info.AuthenticationStatus != XHDCP22_RX_STATUS_AUTHENTICATED)) ? (TRUE) : (FALSE);
}

/*****************************************************************************/
/**
* This function checks if the HDCP22-RX state machine is in the
* Authenticated state.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
u8 XHdcp22Rx_IsAuthenticated(XHdcp22_Rx *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Info.AuthenticationStatus == XHDCP22_RX_STATUS_AUTHENTICATED) ? (TRUE) : (FALSE);
}

/*****************************************************************************/
/**
* This function checks if the HDCP22-RX state machine has detected an error
* condition.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
u8 XHdcp22Rx_IsError(XHdcp22_Rx *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Info.ErrorFlagSticky != XHDCP22_RX_ERROR_FLAG_NONE) ? (TRUE) : (FALSE);
}

/*****************************************************************************/
/**
* This function is called when 50 consecutive data island ECC errors are
* detected indicating a link integrity problem. The error flag is set
* indicating to the state machine that REAUTH_REQ bit in the RXSTATUS
* register be set. Setting this flag only takes effect when the
* authentication state machine is in the Authenticated state B4.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		This function can be registered as an asynchronous callback.
******************************************************************************/
void XHdcp22Rx_SetLinkError(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Log error event set flag */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY);
}

/*****************************************************************************/
/**
* This function is called when a DDC read/write burst stops prior to completing
* the expected message size. This will cause the message buffers to be flushed
* and state machine reset.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		This function can be registered as an asynchronous callback.
******************************************************************************/
void XHdcp22Rx_SetDdcError(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Log error event and set flag */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_DDC_BURST);
}

/*****************************************************************************/
/**
* This function sets the DDC RxStatus registers (0x70-0x71) ReauthReq bit
* and clears the link integrity error flag. Setting the ReauthReq bit
* signals the transmitter to re-initiate authentication.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_SetDdcReauthReq(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u8 RxStatus[2];

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_REQAUTH_REQ);

	/* Get RxStatus */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS0_REG);
	RxStatus[0] = InstancePtr->Handles.DdcGetDataCallback(InstancePtr->Handles.DdcGetDataCallbackRef);
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS1_REG);
	RxStatus[1] = InstancePtr->Handles.DdcGetDataCallback(InstancePtr->Handles.DdcGetDataCallbackRef);

	/* Update RxStatus[11] REAUTH_REQ bit */
	*(u16 *)RxStatus |= 0x800;

	/* Set RxStatus */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS0_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, RxStatus[0]);
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS1_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, RxStatus[1]);

    /* Clear link integrity error flag */
	InstancePtr->Info.ErrorFlag &= ~XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY;
}

/*****************************************************************************/
/**
* This function is called when a complete message is available in the
* write message buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		This function can be registered as an asynchronous callback.
******************************************************************************/
void XHdcp22Rx_SetWriteMessageAvailable(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_WRITE_MESSAGE_AVAILABLE);

	/* Update DDC flag */
	InstancePtr->Info.DdcFlag |= XHDCP22_RX_DDC_FLAG_WRITE_MESSAGE_READY;
}

/*****************************************************************************/
/**
* This function is called when a message has been read out of the read
* message buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		This function can be registered as an asynchronous callback.
******************************************************************************/
void XHdcp22Rx_SetReadMessageComplete(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_READ_MESSAGE_COMPLETE);

	/* Update DDC flag */
	InstancePtr->Info.DdcFlag |= XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY;
}

/*****************************************************************************/
/**
* This function is used to load the Lc128 value by copying the contents
* of the array referenced by Lc128Ptr into the cipher.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	Lc128Ptr is a pointer to an array.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LoadLc128(XHdcp22_Rx *InstancePtr, const u8 *Lc128Ptr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Lc128Ptr != NULL);

	XHdcp22Cipher_SetLc128(&InstancePtr->CipherInst, Lc128Ptr,  XHDCP22_RX_LC128_SIZE);
}

/*****************************************************************************/
/**
* This function is used to load the public certificate.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	PublicCertPtr is a pointer to an array.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LoadPublicCert(XHdcp22_Rx *InstancePtr, const u8 *PublicCertPtr)
{
	/* Verify arguments */
	Xil_AssertVoid(PublicCertPtr != NULL);

	InstancePtr->PublicCertPtr = PublicCertPtr;
}

/*****************************************************************************/
/**
* This function is used to load the private key.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	PrivateKeyPtr is a pointer to an array.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LoadPrivateKey(XHdcp22_Rx *InstancePtr, const u8 *PrivateKeyPtr)
{
	/* Verify arguments */
	Xil_AssertVoid(PrivateKeyPtr != NULL);

	InstancePtr->PrivateKeyPtr = PrivateKeyPtr;
}

/*****************************************************************************/
/**
* This function is used to load the Montgomery NprimeP.
*
* @param	NPrimePPtr is a pointer to an array.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LoadMontNPrimeP(XHdcp22_Rx *InstancePtr, const u8 *NPrimePPtr)
{
	/* Verify arguments */
	Xil_AssertVoid(NPrimePPtr != NULL);

	InstancePtr->NPrimePPtr = NPrimePPtr;
}

/*****************************************************************************/
/**
* This function is used to load the Montgomery NprimeQ.
*
* @param	NPrimeQPtr is a pointer to an array.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LoadMontNPrimeQ(XHdcp22_Rx *InstancePtr, const u8 *NPrimeQPtr)
{
	/* Verify arguments */
	Xil_AssertVoid(NPrimeQPtr != NULL);

	InstancePtr->NPrimeQPtr = NPrimeQPtr;
}

/*****************************************************************************/
/**
* This function is a called to initialize the cipher.
*
* @param	InstancePtr is a pointer to the XHdcp22Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_InitializeCipher(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;
	u32 SubcoreBaseAddr;
	XHdcp22_Cipher_Config *CipherConfigPtr = NULL;

	CipherConfigPtr = XHdcp22Cipher_LookupConfig(InstancePtr->Config.CipherDeviceId);
	if(CipherConfigPtr == NULL)
	{
		return XST_FAILURE;
	}

	Status = XHdcp22Rx_ComputeBaseAddress(InstancePtr->Config.BaseAddress, CipherConfigPtr->BaseAddress, &SubcoreBaseAddr);
	Status |= XHdcp22Cipher_CfgInitialize(&InstancePtr->CipherInst, CipherConfigPtr, SubcoreBaseAddr);

	XHdcp22Cipher_SetRxMode(&InstancePtr->CipherInst);

	return Status;
}

/*****************************************************************************/
/**
* This function is a called to initialize the modular multiplier.
*
* @param	InstancePtr is a pointer to the XHdcp22Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_InitializeMmult(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;
	u32 SubcoreBaseAddr;
	XHdcp22_mmult_Config *MmultConfigPtr;

	MmultConfigPtr = XHdcp22_mmult_LookupConfig(InstancePtr->Config.MontMultDeviceId);
	if(MmultConfigPtr == NULL)
	{
		return XST_FAILURE;
	}

	Status = XHdcp22Rx_ComputeBaseAddress(InstancePtr->Config.BaseAddress, MmultConfigPtr->BaseAddress, &SubcoreBaseAddr);
	Status |= XHdcp22_mmult_CfgInitialize(&InstancePtr->MmultInst, MmultConfigPtr, SubcoreBaseAddr);

	return Status;
}

/*****************************************************************************/
/**
* This function is a called to initialize the random number generator.
*
* @param	InstancePtr is a pointer to the XHdcp22Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_InitializeRng(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;
	u32 SubcoreBaseAddr;
	XHdcp22_Rng_Config *RngConfigPtr;

	RngConfigPtr = XHdcp22Rng_LookupConfig(InstancePtr->Config.RngDeviceId);
	if(RngConfigPtr == NULL)
	{
		return XST_FAILURE;
	}

	Status |= XHdcp22Rx_ComputeBaseAddress(InstancePtr->Config.BaseAddress, RngConfigPtr->BaseAddress, &SubcoreBaseAddr);
	Status |= XHdcp22Rng_CfgInitialize(&InstancePtr->RngInst, RngConfigPtr, SubcoreBaseAddr);

	return Status;
}

/*****************************************************************************/
/**
* This function is a called to initialize the timer.
*
* @param	InstancePtr is a pointer to the XHdcp22Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_InitializeTimer(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;
	u32 SubcoreBaseAddr;
	XTmrCtr_Config *TimerConfigPtr;

	TimerConfigPtr = XTmrCtr_LookupConfig(InstancePtr->Config.TimerDeviceId);
	if(TimerConfigPtr == NULL)
	{
		return XST_FAILURE;
	}

	Status |= XHdcp22Rx_ComputeBaseAddress(InstancePtr->Config.BaseAddress, TimerConfigPtr->BaseAddress, &SubcoreBaseAddr);
	XTmrCtr_CfgInitialize(&InstancePtr->TimerInst, TimerConfigPtr, SubcoreBaseAddr);

	return Status;
}

/*****************************************************************************/
/**
* This function computes the subcore absolute address on axi-lite interface
* Subsystem is mapped at an absolute address and all included sub-cores are
* at pre-defined offset from the subsystem base address. To access the subcore
* register map from host CPU an absolute address is required.
*
* @param	BaseAddress is the base address of the subsystem instance
* @param	SubcoreOffset is the offset of the the subcore instance
* @param	SubcoreAddressPtr is the computed base address of the subcore instance
*
* @return	XST_SUCCESS if base address computation is successful and within
* 			subsystem address range else XST_FAILURE
*
******************************************************************************/
static int XHdcp22Rx_ComputeBaseAddress(u32 BaseAddress, u32 SubcoreOffset, u32 *SubcoreAddressPtr)
{
	int Status;
	u32 Address;

	Address = BaseAddress | SubcoreOffset;
	if((Address >= BaseAddress))
	{
		*SubcoreAddressPtr = Address;
		Status = XST_SUCCESS;
	}
	else
	{
		*SubcoreAddressPtr = 0;
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function is used to get a random value Rrx of 64bits for AKEInit.
* When the test mode is set to XHDCP22_RX_TESTMODE_NO_TX a preloaded test
* value is copied into the array with pointer RrxPtr. Otherwise, a
* random value is generated and copied.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	RrxPtr is a pointer to an array where the 64bit Rrx is
* 			to be copied.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_GenerateRrx(XHdcp22_Rx *InstancePtr, u8 *RrxPtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(RrxPtr != NULL);

	XHdcp22Rx_GenerateRandom(InstancePtr, XHDCP22_RX_RRX_SIZE, RrxPtr);

#ifdef _XHDCP22_RX_TEST_
	/* In test mode copy the test vector */
	XHdcp22Rx_TestGenerateRrx(InstancePtr, RrxPtr);
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to check if a complete message is available in
* the write message buffer. The DDC flag is cleared when a message
* available is detected.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
static u8 XHdcp22Rx_IsWriteMessageAvailable(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if write message is available then clear flag */
	if(InstancePtr->Info.DdcFlag & XHDCP22_RX_DDC_FLAG_WRITE_MESSAGE_READY)
	{
		InstancePtr->Info.DdcFlag &= ~XHDCP22_RX_DDC_FLAG_WRITE_MESSAGE_READY;
		return (TRUE);
	}

	return (FALSE);
}

/*****************************************************************************/
/**
* This function is used to check if a complete message has been read
* out of the read message buffer, indicating that the buffer is empty.
* The DDC flag is cleared when a read message complete has been detected.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
******************************************************************************/
static u8 XHdcp22Rx_IsReadMessageComplete(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if read message is complete then clear flag */
	if((InstancePtr->Info.DdcFlag & XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY))
	{
		InstancePtr->Info.DdcFlag &= ~XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY;
		return (TRUE);
	}

	return (FALSE);
}

/*****************************************************************************/
/**
* This function sets the DDC RxStatus registers (0x70-0x71) MessageSize bits.
* The HDCP22-RX state machine calls this function immediately after writing
* a complete message into the read message buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	MessageSize indicates the size in bytes of the message
*			available in the read message buffer.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22Rx_SetDdcMessageSize(XHdcp22_Rx *InstancePtr, u16 MessageSize)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MessageSize <= 0x3FF);

	u8 RxStatus[2];

	/* Get RxStatus */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS0_REG);
	RxStatus[0] = InstancePtr->Handles.DdcGetDataCallback(InstancePtr->Handles.DdcGetDataCallbackRef);
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS1_REG);
	RxStatus[1] = InstancePtr->Handles.DdcGetDataCallback(InstancePtr->Handles.DdcGetDataCallbackRef);

	/* Update RxStatus[9:0] MessageSize bits */
	*(u16 *)RxStatus &= ~0x3FF;
	*(u16 *)RxStatus |= MessageSize;

	/* Set RxStatus */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS0_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, RxStatus[0]);
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS1_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, RxStatus[1]);
}

/*****************************************************************************/
/**
* This function loads the HDCP22-RX cipher after session key exchange
* and set the Ks and Riv parameters received from SKESendEks message.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_LoadSessionKey(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Set Ks and Riv */
	XHdcp22Cipher_SetKs(&InstancePtr->CipherInst, InstancePtr->Params.Ks, XHDCP22_RX_KS_SIZE);
	XHdcp22Cipher_SetRiv(&InstancePtr->CipherInst, InstancePtr->Params.Riv, XHDCP22_RX_RIV_SIZE);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO, XHDCP22_RX_LOG_INFO_ENCRYPTION_ENABLE);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function resets the HDCP22-RX system after an error event. The
* driver calls this function when recovering from error conditions.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22Rx_ResetAfterError(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Log info event */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_FORCE_RESET);

	/* Reset cipher */
	XHdcp22Cipher_Disable(&InstancePtr->CipherInst);
	XHdcp22Cipher_Enable(&InstancePtr->CipherInst);

	/* Clear message buffer */
	memset(&InstancePtr->MessageBuffer, 0, sizeof(XHdcp22_Rx_Message));
	InstancePtr->MessageSize = 0;

	/* Set default values */
	InstancePtr->StateFunc = (XHdcp22_Rx_StateFunc)(&XHdcp22Rx_StateB0);
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_UNAUTHENTICATED;
	InstancePtr->Info.IsNoStoredKm = FALSE;
	InstancePtr->Info.LCInitAttempts = 0;
	InstancePtr->Info.DdcFlag = XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY;
	InstancePtr->Info.CurrentState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;
	InstancePtr->Info.NextState = XHDCP22_RX_STATE_B0_WAIT_AKEINIT;

	/* Reset parameters */
	XHdcp22Rx_ResetParams(InstancePtr);
}

/*****************************************************************************/
/**
* This function resets the HDCP22-RX parameters stored in memory during the
* authentication process.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		This function is called each time an AKE_Init message is
*			received.
******************************************************************************/
static void XHdcp22Rx_ResetParams(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Clear stored parameters */
	memset(InstancePtr->Params.Kd,     0, sizeof(InstancePtr->Params.Kd));
	memset(InstancePtr->Params.Km,     0, sizeof(InstancePtr->Params.Km));
	memset(InstancePtr->Params.Ks,     0, sizeof(InstancePtr->Params.Ks));
	memset(InstancePtr->Params.Riv,    0, sizeof(InstancePtr->Params.Riv));
	memset(InstancePtr->Params.Rn,     0, sizeof(InstancePtr->Params.Rn));
	memset(InstancePtr->Params.Rrx,    0, sizeof(InstancePtr->Params.Rrx));
	memset(InstancePtr->Params.Rtx,    0, sizeof(InstancePtr->Params.Rtx));
	memset(InstancePtr->Params.RxCaps, 0, sizeof(InstancePtr->Params.RxCaps));
	memset(InstancePtr->Params.TxCaps, 0, sizeof(InstancePtr->Params.TxCaps));
}

/*****************************************************************************/
/**
* This function resets the HDCP22-RX DDC registers to their default values
* and clears the read/write message buffers. The DDC status flag is set to
* it's initial state and the DDC error flags are cleared.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22Rx_ResetDdc(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Handles.IsDdcAllCallbacksSet == TRUE);

	/* Set HDCP2Version register */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_VERSION_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, 0x04);

	/* Set RXSTATUS registers */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS0_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, 0x00);
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_RXSTATUS1_REG);
	InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, 0x00);

	/* Reset read/write message buffers */
	InstancePtr->Handles.DdcClearReadBufferCallback(InstancePtr->Handles.DdcClearReadBufferCallbackRef);
	InstancePtr->Handles.DdcClearWriteBufferCallback(InstancePtr->Handles.DdcClearWriteBufferCallbackRef);

	/* Initialize DDC status flag */
	InstancePtr->Info.DdcFlag = XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY;

	/* Clear DDC error flags */
	InstancePtr->Info.ErrorFlag &= ~XHDCP22_RX_ERROR_FLAG_DDC_BURST;
}

/*****************************************************************************/
/**
* This function uses polling to read a complete message out of the read
* message buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Size of message read out of read message buffer.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_PollMessage(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Size = 0;
	int Offset = 0;

	/* Get message */
	if(XHdcp22Rx_IsWriteMessageAvailable(InstancePtr) == TRUE)
	{
		Size = InstancePtr->Handles.DdcGetWriteBufferSizeCallback(InstancePtr->Handles.DdcGetWriteBufferSizeCallbackRef);

		InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_WRITE_REG);

		for(Offset = 0; Offset < Size; Offset++)
		{
			InstancePtr->MessageBuffer[Offset] = InstancePtr->Handles.DdcGetDataCallback(InstancePtr->Handles.DdcGetDataCallbackRef);
		}
	}

	return Size;
}

/*****************************************************************************/
/**
* This function implements the HDCP22-RX state B0 (Unauthenticated). In this
* state the receiver is awaiting the reception of AKE_Init from the
* transmitter to trigger the authentication protocol.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Pointer to the function that implements the next state.
*
* @note		When an unexpected or malformed message is received
*			the system is reset to a known state to allow graceful
*			recovery from error.
******************************************************************************/
static void *XHdcp22Rx_StateB0(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Update state */
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_UNAUTHENTICATED;
	InstancePtr->Info.CurrentState = InstancePtr->Info.NextState;

	/* Check error condition */
	if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_DDC_BURST)
	{
		XHdcp22Rx_ResetDdc(InstancePtr);
		XHdcp22Rx_ResetAfterError(InstancePtr);
		return XHdcp22Rx_StateB0;
	}

	/* Check if message is available */
	InstancePtr->MessageSize = XHdcp22Rx_PollMessage(InstancePtr);

	/* Message handling */
	if(InstancePtr->MessageSize > 0)
	{
		switch(MsgPtr->MsgId)
		{
		case XHDCP22_RX_MSG_ID_AKEINIT: /* Transition B0->B1 */
			Status = XHdcp22Rx_ProcessMessageAKEInit(InstancePtr);
			if(Status == XST_SUCCESS)
			{
			    InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDCERT;
			    return XHdcp22Rx_StateB1;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		default:
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		}
	}

	return XHdcp22Rx_StateB0;
}

/*****************************************************************************/
/**
* This function implements the HDCP22-RX state B1 (ComputeKm). In this
* state the receiver makes the AKESendCert message available for reading
* by the transmitter in response to AKEInit. If AKENoStoredKm is received,
* the receiver decrypts Km with KprivRx and calculates HPrime. If AKEStoredKm
* is received it decrypts Ekh(Km) to derive Km and calculate HPrime. It makes
* AKESendHPrime message available for reading immediately after computation
* of HPrime to ensure that the message is received by the transmitter within
* the specified 1s timeout at the transmitter for NoStoredKm and 200ms timeout
* for StoredKm. When AKENoStoredKm is received the AKESendPairingInfo message
* is made available to the transmitter for reading after within 200ms after
* AKESendHPrime message is received by the transmitter.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Pointer to the function that implements the next state.
*
* @note		When an unexpected or malformed message is received the
*			system is reset to the default state B0 to allow graceful
*			recovery from error.
******************************************************************************/
static void *XHdcp22Rx_StateB1(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Update state */
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_COMPUTE_KM;
	InstancePtr->Info.CurrentState = InstancePtr->Info.NextState;

	/* Check error condition */
	if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_DDC_BURST)
	{
		XHdcp22Rx_ResetDdc(InstancePtr);
		XHdcp22Rx_ResetAfterError(InstancePtr);
		return XHdcp22Rx_StateB0;
	}

	/* Check if message is available */
	InstancePtr->MessageSize = XHdcp22Rx_PollMessage(InstancePtr);

	/* Message handling */
	if(InstancePtr->MessageSize > 0)
	{
		switch(MsgPtr->MsgId)
		{
		case XHDCP22_RX_MSG_ID_AKEINIT:
			Status = XHdcp22Rx_ProcessMessageAKEInit(InstancePtr);
			if(Status == XST_SUCCESS)
			{
			    InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDCERT;
			    break;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		case XHDCP22_RX_MSG_ID_AKENOSTOREDKM:
			if(InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B1_WAIT_AKEKM)
			{
				Status = XHdcp22Rx_ProcessMessageAKENoStoredKm(InstancePtr);
				if(Status == XST_SUCCESS)
				{
					InstancePtr->Info.IsNoStoredKm = TRUE;
					InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME;
					break;
				}
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKENOSTOREDKM);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		case XHDCP22_RX_MSG_ID_AKESTOREDKM:
			if(InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B1_WAIT_AKEKM)
			{
				Status = XHdcp22Rx_ProcessMessageAKEStoredKm(InstancePtr);
				if(Status == XST_SUCCESS)
				{
					InstancePtr->Info.IsNoStoredKm = FALSE;
					InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME;
					break;
				}
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKESTOREDKM);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		case XHDCP22_RX_MSG_ID_LCINIT: /* Transition B1->B2 */
			if(InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B1_WAIT_LCINIT)
			{
				Status = XHdcp22Rx_ProcessMessageLCInit(InstancePtr);
				if(Status == XST_SUCCESS)
				{
					InstancePtr->Info.NextState = XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME;
					return XHdcp22Rx_StateB2;
				}
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_LCINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		default:
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		}
	}

	/* Message send */
	switch(InstancePtr->Info.NextState)
	{
	case XHDCP22_RX_STATE_B1_SEND_AKESENDCERT:
		if(XHdcp22Rx_IsReadMessageComplete(InstancePtr) == TRUE)
		{
			Status = XHdcp22Rx_SendMessageAKESendCert(InstancePtr);
			InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_WAIT_AKEKM;
		}
		break;
	case XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME:
		if(XHdcp22Rx_IsReadMessageComplete(InstancePtr) == TRUE)
		{
			Status = XHdcp22Rx_SendMessageAKESendHPrime(InstancePtr);
			InstancePtr->Info.NextState = (InstancePtr->Info.IsNoStoredKm) ?
				XHDCP22_RX_STATE_B1_SEND_AKESENDPAIRINGINFO : XHDCP22_RX_STATE_B1_WAIT_LCINIT;
		}
		break;
	case XHDCP22_RX_STATE_B1_SEND_AKESENDPAIRINGINFO:
		if(XHdcp22Rx_IsReadMessageComplete(InstancePtr) == TRUE)
		{
			Status = XHdcp22Rx_SendMessageAKESendPairingInfo(InstancePtr);
			InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_WAIT_LCINIT;
		}
		break;
	default:
		break;
	}

	return XHdcp22Rx_StateB1;
}

/*****************************************************************************/
/**
* This function implements the HDCP22-RX state B2 (Compute_LPrime). The
* receiver computes LPrime required during locality check and makes
* LCSendLPrime message available for reading by the transmitter.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Pointer to the function that implements the next state.
*
* @note		When an unexpected or malformed message is received
*			the system is reset to the default state B0 to allow
*			graceful recovery from error.
******************************************************************************/
static void *XHdcp22Rx_StateB2(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Update state */
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_COMPUTE_LPRIME;
	InstancePtr->Info.CurrentState = InstancePtr->Info.NextState;

	/* Check error condition */
	if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_DDC_BURST)
	{
		XHdcp22Rx_ResetDdc(InstancePtr);
		XHdcp22Rx_ResetAfterError(InstancePtr);
		return XHdcp22Rx_StateB0;
	}

	/* Check if message is available */
	InstancePtr->MessageSize = XHdcp22Rx_PollMessage(InstancePtr);

	/* Message handling */
	if(InstancePtr->MessageSize > 0)
	{
		switch(MsgPtr->MsgId)
		{
		case XHDCP22_RX_MSG_ID_AKEINIT: /* Transition B2->B1 */
			Status = XHdcp22Rx_ProcessMessageAKEInit(InstancePtr);
			if(Status == XST_SUCCESS)
			{
				InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDCERT;
				return XHdcp22Rx_StateB1;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		case XHDCP22_RX_MSG_ID_LCINIT: /* Transition B2->B2 */
			/* Maximum of 1024 locality check attempts allowed */
			if(InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME ||
				InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS)
			{
				if(InstancePtr->Info.LCInitAttempts <= XHDCP22_RX_MAX_LCINIT)
				{
					Status = XHdcp22Rx_ProcessMessageLCInit(InstancePtr);
					if(Status == XST_SUCCESS)
					{
						InstancePtr->Info.NextState = XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME;
						break;
					}
					break;
				}
			    XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MAX_LCINIT_ATTEMPTS);
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_LCINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		case XHDCP22_RX_MSG_ID_SKESENDEKS: /* Transition B2->B3 */
			if(InstancePtr->Info.CurrentState == XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS)
			{
			InstancePtr->Info.NextState = XHDCP22_RX_STATE_B3_COMPUTE_KS;
			return XHdcp22Rx_StateB3;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_SKESENDEKS);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		default:
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		}
	}

	/* Message send */
	switch(InstancePtr->Info.NextState)
	{
	case XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME:
		if(XHdcp22Rx_IsReadMessageComplete(InstancePtr) == TRUE)
		{
			Status = XHdcp22Rx_SendMessageLCSendLPrime(InstancePtr);
			InstancePtr->Info.NextState = XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS;
		}
		break;
	default:
		break;
	}

	return XHdcp22Rx_StateB2;
}

/*****************************************************************************/
/**
* This function implements the HDCP22-RX state B3 (ComputKs). The
* receiver decrypts Edkey(Ks) to derive Ks. The cipher is updated
* with the session key and enabled within 200ms after the encrypted
* session key is received.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Pointer to the function that implements the next state.
*
* @note		When an unexpected or malformed message is received the
*			system is reset to the default state B0 to allow graceful
*			recovery from error.
******************************************************************************/
static void *XHdcp22Rx_StateB3(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Update state */
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_COMPUTE_KS;
	InstancePtr->Info.CurrentState = InstancePtr->Info.NextState;

	/* Check error condition */
	if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_DDC_BURST)
	{
		XHdcp22Rx_ResetDdc(InstancePtr);
		XHdcp22Rx_ResetAfterError(InstancePtr);
		return XHdcp22Rx_StateB0;
	}

	/* Compute Ks */
	XHdcp22Rx_ProcessMessageSKESendEks(InstancePtr);

	/* Check if message is available */
	InstancePtr->MessageSize = XHdcp22Rx_PollMessage(InstancePtr);

	/* Message handling */
	if(InstancePtr->MessageSize > 0)
	{
		switch(MsgPtr->MsgId)
		{
		case XHDCP22_RX_MSG_ID_AKEINIT: /* Transition B3->B1 */
			Status = XHdcp22Rx_ProcessMessageAKEInit(InstancePtr);
			if(Status == XST_SUCCESS)
			{
				InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDCERT;
				return XHdcp22Rx_StateB1;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		default:
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		}
	}

	/* Run authenticated callback function */
    if(InstancePtr->Handles.IsAuthenticatedCallbackSet)
	{
		InstancePtr->Handles.AuthenticatedCallback(InstancePtr->Handles.AuthenticatedCallbackRef);
	}

	InstancePtr->Info.NextState = XHDCP22_RX_STATE_B4_AUTHENTICATED;

	return XHdcp22Rx_StateB4; /* Transition B3->B4 */
}

/*****************************************************************************/
/**
* This function implements the HDCP22-RX state B4 (Authenticated). The
* receiver has completed the authentication protocol. An ongoing link
* integrity check is performed external to the HDCP22-RX system to
* detect synchronization mismatches between transmitter and receiver.
* RxStatus REAUTH_REQ bit is set if 50 consecutive data island CRC errors
* are detected. Setting the REAUTH_REQ bit signals the transmitter to
* re-initiate authentication.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	Pointer to the function that implements the next state.
*
* @note		When an unexpected or malformed message is received the
* 			system is reset to the default state B0 to allow graceful
* 			recovery from error.
******************************************************************************/
static void *XHdcp22Rx_StateB4(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Update state */
	InstancePtr->Info.AuthenticationStatus = XHDCP22_RX_STATUS_AUTHENTICATED;
	InstancePtr->Info.CurrentState = InstancePtr->Info.NextState;

	/* Check error condition */
	if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_DDC_BURST)
	{
		XHdcp22Rx_ResetDdc(InstancePtr);
		XHdcp22Rx_ResetAfterError(InstancePtr);
		return XHdcp22Rx_StateB0;
	}
	else if(InstancePtr->Info.ErrorFlag & XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY)
	{
		XHdcp22Rx_SetDdcReauthReq(InstancePtr);
	}

	/* Check if message is available */
	InstancePtr->MessageSize = XHdcp22Rx_PollMessage(InstancePtr);

	/* Message handling */
	if(InstancePtr->MessageSize > 0)
	{
		switch(MsgPtr->MsgId)
		{
		case XHDCP22_RX_MSG_ID_AKEINIT: /* Transition B4->B1 */
			Status = XHdcp22Rx_ProcessMessageAKEInit(InstancePtr);
			if(Status == XST_SUCCESS)
			{
				InstancePtr->Info.NextState = XHDCP22_RX_STATE_B1_SEND_AKESENDCERT;
				return XHdcp22Rx_StateB1;
			}
			XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT);
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		default:
			XHdcp22Rx_ResetAfterError(InstancePtr);
			return XHdcp22Rx_StateB0;
		}
	}

	return XHdcp22Rx_StateB4; /* Transition B4->B4 */
}

/*****************************************************************************/
/**
* This function processes the AKEInit message received from the transmitter.
* The stored authentication parameters are reset so that no state
* information is stored between authentication attempts. The DDC
* registers are reset to their default value including the REAUTH_REQ
* flag. The cipher is disabled.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_ProcessMessageAKEInit(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Log message read completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKEINIT);

	/* Reset cipher */
	XHdcp22Cipher_Disable(&InstancePtr->CipherInst);
	XHdcp22Cipher_Enable(&InstancePtr->CipherInst);

	/* Reset timer counter */
	XTmrCtr_Reset(&InstancePtr->TimerInst, XHDCP22_RX_TMR_CTR_0);

	/* Check message size */
	if(InstancePtr->MessageSize != sizeof(XHdcp22_Rx_AKEInit))
	{
		XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE);
		return XST_FAILURE;
	}

	/* Reset state variables and DDC registers */
	XHdcp22Rx_ResetParams(InstancePtr);
	XHdcp22Rx_ResetDdc(InstancePtr);

	/* Record Rtx and TxCaps parameters */
	memcpy(InstancePtr->Params.Rtx, MsgPtr->AKEInit.Rtx, XHDCP22_RX_RTX_SIZE);
	memcpy(InstancePtr->Params.TxCaps, MsgPtr->AKEInit.TxCaps, XHDCP22_RX_TXCAPS_SIZE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function generates the AKESendCert message and writes it into the
* read message buffer. After the complete message has been written to the
* buffer the MessageSize is set in the DDC RxStatus register signaling
* the transmitter that the message is available for reading.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		This message must be available for the transmitter with 100ms
*			after receiving AKEInit.
******************************************************************************/
static int XHdcp22Rx_SendMessageAKESendCert(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Offset = 0;
	int Status = XST_SUCCESS;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Generate AKE_Send_Cert message */
	MsgPtr->AKESendCert.MsgId = XHDCP22_RX_MSG_ID_AKESENDCERT;
	memcpy(MsgPtr->AKESendCert.RxCaps, InstancePtr->RxCaps, XHDCP22_RX_RXCAPS_SIZE);
	Status = XHdcp22Rx_GenerateRrx(InstancePtr, MsgPtr->AKESendCert.Rrx);
	memcpy(MsgPtr->AKESendCert.CertRx, InstancePtr->PublicCertPtr, XHDCP22_RX_CERT_SIZE);

	/* Write message to read buffer */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_READ_REG);
	for(Offset = 0; Offset < sizeof(XHdcp22_Rx_AKESendCert); Offset++)
	{
		InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, InstancePtr->MessageBuffer[Offset]);
	}

	/* Write message size signaling completion */
	XHdcp22Rx_SetDdcMessageSize(InstancePtr, sizeof(XHdcp22_Rx_AKESendCert));

	/* Record Rrx and RxCaps */
	memcpy(InstancePtr->Params.Rrx, MsgPtr->AKESendCert.Rrx, XHDCP22_RX_RRX_SIZE);
	memcpy(InstancePtr->Params.RxCaps, MsgPtr->AKESendCert.RxCaps, XHDCP22_RX_RXCAPS_SIZE);

	/* Log message write completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKESENDCERT);

	return Status;
}

/*****************************************************************************/
/**
* This function processes the AKENoStoredKm message received from the
* transmitter. The RSAES-OAEP operation decrypts Km with the receiver
* private key.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_ProcessMessageAKENoStoredKm(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Size;
	int Status = XST_SUCCESS;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Log message read completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKENOSTOREDKM);

	/* Check message size */
	if(InstancePtr->MessageSize != sizeof(XHdcp22_Rx_AKENoStoredKm))
	{
		XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE);
		return XST_FAILURE;
	}

	/* Compute Km, Perform RSA decryption */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KM);
	Status = XHdcp22Rx_RsaesOaepDecrypt(InstancePtr, (XHdcp22_Rx_KprivRx *)(InstancePtr->PrivateKeyPtr),
										MsgPtr->AKENoStoredKm.EKpubKm, InstancePtr->Params.Km, &Size);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KM_DONE);

	return (Status == XST_SUCCESS && Size == XHDCP22_RX_KM_SIZE) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
* This function processes the AKEStoredKm message received from the
* transmitter. Decrypts Ekh(Km) using AES with the received m as
* input and Kh as key into the AES module.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_ProcessMessageAKEStoredKm(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Log message read completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKESTOREDKM);

	/* Check message size */
	if(InstancePtr->MessageSize != sizeof(XHdcp22_Rx_AKEStoredKm))
	{
		XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE);
		return XST_FAILURE;
	}

	/* Compute Km */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KM);
	XHdcp22Rx_ComputeEkh(InstancePtr->PrivateKeyPtr, MsgPtr->AKEStoredKm.EKhKm, MsgPtr->AKEStoredKm.M, InstancePtr->Params.Km);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KM_DONE);

	return Status;
}

/*****************************************************************************/
/**
* This function computes HPrime, generates AKESendHPrime message, and
* writes it into the read message buffer. After the complete message has
* been written to the buffer the MessageSize is set in the DDC RxStatus
* register signaling the transmitter that the message is available for
* reading.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		This message must be available for the transmitter within
*			1s after receiving AKENoStoredKm or 200ms after receiving
*			AKEStoredKm.
******************************************************************************/
static int XHdcp22Rx_SendMessageAKESendHPrime(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Offset;
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Compute H Prime */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME);
	XHdcp22Rx_ComputeHPrime(InstancePtr->Params.Rrx, InstancePtr->Params.RxCaps,
			InstancePtr->Params.Rtx, InstancePtr->Params.TxCaps, InstancePtr->Params.Km,
			InstancePtr->Params.Kd, MsgPtr->AKESendHPrime.HPrime);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME_DONE);

	/* Generate AKE_Send_H_prime message */
	MsgPtr->AKESendHPrime.MsgId = XHDCP22_RX_MSG_ID_AKESENDHPRIME;

	/* Write message to buffer */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_READ_REG);
	for(Offset = 0; Offset < sizeof(XHdcp22_Rx_AKESendHPrime); Offset++)
	{
		InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, InstancePtr->MessageBuffer[Offset]);
	}

	/* Write message size signaling completion */
	XHdcp22Rx_SetDdcMessageSize(InstancePtr, sizeof(XHdcp22_Rx_AKESendHPrime));

	/* Record HPrime */
	memcpy(InstancePtr->Params.HPrime, MsgPtr->AKESendHPrime.HPrime, XHDCP22_RX_HPRIME_SIZE);

	/* Log message write completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKESENDHPRIME);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function computes Ekh(Km), generates AKESendPairingInfo message,
* and writes it into the read message buffer. After the complete message
* has been written to the buffer the MessageSize is set in the DDC RxStatus
* register signaling the transmitter that the message is available for
* reading.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		The AKESendPairingInfo message is sent only in response to
*			receiving AKENoStoredKm message. This message must be
*			available for the transmitter within 200ms after sending
*			AKESendHPrime.
******************************************************************************/
static int XHdcp22Rx_SendMessageAKESendPairingInfo(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 M[XHDCP22_RX_RTX_SIZE+XHDCP22_RX_RRX_SIZE];
	u8 EKhKm[XHDCP22_RX_EKH_SIZE];
	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;
	int Offset;
	int Status = XST_SUCCESS;

	/* Concatenate M = (Rtx || Rrx) */
	memcpy(M, InstancePtr->Params.Rtx, XHDCP22_RX_RTX_SIZE);
	memcpy(M+XHDCP22_RX_RTX_SIZE, InstancePtr->Params.Rrx, XHDCP22_RX_RRX_SIZE);

	/* Compute Ekh */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH);
	XHdcp22Rx_ComputeEkh(InstancePtr->PrivateKeyPtr, InstancePtr->Params.Km, M, EKhKm);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH_DONE);

	/* Generate AKE_Send_Pairing_Info message */
	MsgPtr->AKESendPairingInfo.MsgId = XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO;
	memcpy(MsgPtr->AKESendPairingInfo.EKhKm, EKhKm, XHDCP22_RX_EKH_SIZE);

	/* Write message to buffer */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_READ_REG);
	for(Offset = 0; Offset < sizeof(XHdcp22_Rx_AKESendPairingInfo); Offset++)
	{
		InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, InstancePtr->MessageBuffer[Offset]);
	}

	/* Write message size signaling completion */
	XHdcp22Rx_SetDdcMessageSize(InstancePtr, sizeof(XHdcp22_Rx_AKESendPairingInfo));

	/* Record Ekh */
	memcpy(InstancePtr->Params.EKh, EKhKm, XHDCP22_RX_EKH_SIZE);

	/* Log message write completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO);

	return Status;
}

/*****************************************************************************/
/**
* This function processes the LCInit message received from the
* transmitter. The locality check attempts is incremented for
* each LCInit message received. The random nonce Rn is recorded.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
******************************************************************************/
static int XHdcp22Rx_ProcessMessageLCInit(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Log message read completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_LCINIT);

	/* Check message size */
	if(InstancePtr->MessageSize != sizeof(XHdcp22_Rx_LCInit))
	{
		XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE);
		return XST_FAILURE;
	}

	/* Update locality check attempts */
	InstancePtr->Info.LCInitAttempts++;

	/* Record Rn parameter */
	memcpy(InstancePtr->Params.Rn, MsgPtr->LCInit.Rn, XHDCP22_RX_RN_SIZE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function computes LPrime, generates LCSendLPrime message,
* and writes it into the read message buffer. After the complete message
* has been written to the buffer the MessageSize is set in the DDC RxStatus
* register signaling the transmitter that the message is available for
* reading.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		The LCSendLPrime message must be available for the transmitter
*			within 20ms after receiving LCInit.
******************************************************************************/
static int XHdcp22Rx_SendMessageLCSendLPrime(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;
	int Offset;

	/* Compute LPrime */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME);
	XHdcp22Rx_ComputeLPrime(InstancePtr->Params.Rn, InstancePtr->Params.Kd, InstancePtr->Params.Rrx, MsgPtr->LCSendLPrime.LPrime);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME_DONE);

	/* Generate LC_Send_L_prime message */
	MsgPtr->LCSendLPrime.MsgId = XHDCP22_RX_MSG_ID_LCSENDLPRIME;

	/* Write message to buffer */
	InstancePtr->Handles.DdcSetAddressCallback(InstancePtr->Handles.DdcSetAddressCallbackRef, XHDCP22_RX_DDC_READ_REG);
	for(Offset = 0; Offset < sizeof(XHdcp22_Rx_LCSendLPrime); Offset++)
	{
		InstancePtr->Handles.DdcSetDataCallback(InstancePtr->Handles.DdcSetDataCallbackRef, InstancePtr->MessageBuffer[Offset]);
	}

	/* Write message size signaling completion */
	XHdcp22Rx_SetDdcMessageSize(InstancePtr, sizeof(XHdcp22_Rx_LCSendLPrime));

	/* Record LPrime parameter */
	memcpy(InstancePtr->Params.LPrime, MsgPtr->LCSendLPrime.LPrime, XHDCP22_RX_LPRIME_SIZE);

	/* Log message write completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_LCSENDLPRIME);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function processes the SKESendEks message received from the
* transmitter. The session key Ks is decrypted and written to the
* cipher.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		The cipher must be enabled within 200ms after receiving
*			SKESendEks.
******************************************************************************/
static int XHdcp22Rx_ProcessMessageSKESendEks(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XHdcp22_Rx_Message *MsgPtr = (XHdcp22_Rx_Message*)InstancePtr->MessageBuffer;

	/* Log message read completion */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_INFO_MESSAGE, XHDCP22_RX_MSG_ID_SKESENDEKS);

	/* Check message size */
	if(InstancePtr->MessageSize != sizeof(XHdcp22_Rx_SKESendEks))
	{
		XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_ERROR, XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE);
		return XST_FAILURE;
	}

	/* Compute Ks */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KS);
	XHdcp22Rx_ComputeKs(InstancePtr->Params.Rrx, InstancePtr->Params.Rtx, InstancePtr->Params.Km,
						InstancePtr->Params.Rn, MsgPtr->SKESendEks.EDkeyKs, InstancePtr->Params.Ks);
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_DEBUG, XHDCP22_RX_LOG_DEBUG_COMPUTE_KS_DONE);

	/* Record Riv parameter */
	memcpy(InstancePtr->Params.Riv, MsgPtr->SKESendEks.Riv, XHDCP22_RX_RIV_SIZE);

	/* Load cipher session key */
	XHdcp22Rx_LoadSessionKey(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function clears the log pointers.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	Verbose allows to add debug logging.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LogReset(XHdcp22_Rx *InstancePtr, u8 Verbose)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Log.Head = 0;
	InstancePtr->Log.Tail = 0;
	InstancePtr->Log.Verbose = Verbose;

	/* Reset and start the logging timer. */
	/* Note: This timer increments continuously and will wrap at 42 second (100 Mhz clock) */
	XTmrCtr_Reset(&InstancePtr->TimerInst, XHDCP22_RX_TMR_CTR_0);
	XTmrCtr_Start(&InstancePtr->TimerInst, XHDCP22_RX_TMR_CTR_0);
}


/*****************************************************************************/
/**
* This function returns the time expired since a log reset was called.
*
* @param  InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return The expired logging time in useconds.
*
* @note   None.
******************************************************************************/
u32 XHdcp22Rx_LogGetTimeUSecs(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 PeriodUsec = InstancePtr->TimerInst.Config.SysClockFreqHz * 1e-6;
	return (XTmrCtr_GetValue(&InstancePtr->TimerInst, XHDCP22_RX_TMR_CTR_0) / PeriodUsec);
}


/*****************************************************************************/
/**
* This function writes HDCP22-RX log event into buffer. If the log event
* is of type error, the sticky error flag is set. The sticky error flag
* is used to keep history of error conditions.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
* @param	Evt specifies an action to be carried out.
* @param	Data specifies the information that gets written into log
*			buffer.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LogWr(XHdcp22_Rx *InstancePtr, u16 Evt, u16 Data)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt < XHDCP22_RX_LOG_EVT_INVALID);

	int LogBufSize = 0;

	/* When logging a debug event check if verbose is set to true */
	if (InstancePtr->Log.Verbose == FALSE && Evt == XHDCP22_RX_LOG_EVT_DEBUG) {
		return;
	}

	/* Write data and event into log buffer */
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].Data = Data;
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].LogEvent = Evt;
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].TimeStamp =
			XHdcp22Rx_LogGetTimeUSecs(InstancePtr);

	/* Update head pointer if reached to end of the buffer */
	LogBufSize = sizeof(InstancePtr->Log.LogItems)/sizeof(XHdcp22_Rx_LogItem);
	if (InstancePtr->Log.Head == (u8)(LogBufSize) - 1) {
		/* Clear pointer */
		InstancePtr->Log.Head = 0;
	} else {
		/* Increment pointer */
		InstancePtr->Log.Head++;
	}

	/* Check tail pointer. When the two pointer are equal, then the buffer
	* is full.In this case then increment the tail pointer as well to
	* remove the oldest entry from the buffer.
	*/
	if (InstancePtr->Log.Tail == InstancePtr->Log.Head) {
		if (InstancePtr->Log.Tail == (u8)(LogBufSize) - 1) {
			InstancePtr->Log.Tail = 0;
		} else {
			InstancePtr->Log.Tail++;
		}
	}

	/* Update sticky error flag */
	if(Evt == XHDCP22_RX_LOG_EVT_ERROR)
	{
		InstancePtr->Info.ErrorFlagSticky |= (u32)Data;
	}
}


/*****************************************************************************/
/**
* This function provides the log information from the log buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return
*           - Content of log buffer if log pointers are not equal.
*           - Otherwise Zero.
*
* @note		None.
******************************************************************************/
XHdcp22_Rx_LogItem* XHdcp22Rx_LogRd(XHdcp22_Rx *InstancePtr)
{
	XHdcp22_Rx_LogItem* LogPtr;
	int LogBufSize = 0;
	u8 Tail = 0;
	u8 Head = 0;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Tail = InstancePtr->Log.Tail;
	Head = InstancePtr->Log.Head;

	/* Check if there is any data in the log and return a NONE defined log item */
	LogBufSize = sizeof(InstancePtr->Log.LogItems)/sizeof(XHdcp22_Rx_LogItem);
	if (Tail == Head) {
		LogPtr = &InstancePtr->Log.LogItems[Tail];
		LogPtr->Data = 0;
		LogPtr->LogEvent = XHDCP22_RX_LOG_EVT_NONE;
		LogPtr->TimeStamp = 0;
		return LogPtr;
	}

	LogPtr = &InstancePtr->Log.LogItems[Tail];

	/* Increment tail pointer */
	if (Tail == (u8)(LogBufSize) - 1) {
		InstancePtr->Log.Tail = 0;
	}
	else {
		InstancePtr->Log.Tail++;
	}
	return LogPtr;
}


/*****************************************************************************/
/**
* This function prints the contents of the log buffer.
*
* @param	InstancePtr is a pointer to the XHdcp22_Rx core instance.
*
* @return	None.
*
* @note		None.
******************************************************************************/
void XHdcp22Rx_LogDisplay(XHdcp22_Rx *InstancePtr)
{
	XHdcp22_Rx_LogItem* LogPtr;
	char str[255];
	u64 TimeStampPrev = 0;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n-------HDCP22 RX log start-------\r\n");
	strcpy(str, "UNDEFINED");
	do {
		/* Read log data */
		LogPtr = XHdcp22Rx_LogRd(InstancePtr);

		/* Print timestamp */
		if(LogPtr->LogEvent != XHDCP22_RX_LOG_EVT_NONE)
		{
			if(LogPtr->TimeStamp < TimeStampPrev) TimeStampPrev = 0;
			xil_printf("[%8ld:", LogPtr->TimeStamp);
			xil_printf("%8ld] ", (LogPtr->TimeStamp - TimeStampPrev));
			TimeStampPrev = LogPtr->TimeStamp;
		}

		/* Print log event */
		switch(LogPtr->LogEvent) {
		case(XHDCP22_RX_LOG_EVT_NONE):
			xil_printf("-------HDCP22 RX log end-------\r\n\r\n");
			break;
		case XHDCP22_RX_LOG_EVT_INFO:
			switch(LogPtr->Data)
			{
			/* Print General Log Data */
			case XHDCP22_RX_LOG_INFO_RESET:
				strcpy(str, "Asserted [RESET]"); break;
			case XHDCP22_RX_LOG_INFO_ENABLE:
				strcpy(str, "State machine [ENABLED]"); break;
			case XHDCP22_RX_LOG_INFO_DISABLE:
				strcpy(str, "State machine [DISABLED]"); break;
			case XHDCP22_RX_LOG_INFO_REQAUTH_REQ:
				strcpy(str, "Asserted [REAUTH_REQ]"); break;
			case XHDCP22_RX_LOG_INFO_ENCRYPTION_ENABLE:
				strcpy(str, "Asserted [ENCRYPTION_ENABLE]"); break;
			case XHDCP22_RX_LOG_INFO_WRITE_MESSAGE_AVAILABLE:
				strcpy(str, "Write message available"); break;
			case XHDCP22_RX_LOG_INFO_READ_MESSAGE_COMPLETE:
				strcpy(str, "Read message complete"); break;
			}
			xil_printf("%s\r\n", str);
			break;
		case XHDCP22_RX_LOG_EVT_INFO_STATE:
			switch(LogPtr->Data)
			{
			/* Print State Log Data */
			case XHDCP22_RX_STATE_B0_WAIT_AKEINIT:
				strcpy(str, "B0_WAIT_AKEINIT"); break;
			case XHDCP22_RX_STATE_B1_SEND_AKESENDCERT:
				strcpy(str, "B1_SEND_AKESENDCERT"); break;
			case XHDCP22_RX_STATE_B1_WAIT_AKEKM:
				strcpy(str, "B1_WAIT_AKEKM"); break;
			case XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME:
				strcpy(str, "B1_SEND_AKESENDHPRIME"); break;
			case XHDCP22_RX_STATE_B1_SEND_AKESENDPAIRINGINFO:
				strcpy(str, "B1_SEND_AKESENDPAIRINGINFO"); break;
			case XHDCP22_RX_STATE_B1_WAIT_LCINIT:
				strcpy(str, "B1_WAIT_LCINIT"); break;
			case XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME:
				strcpy(str, "B2_SEND_LCSENDLPRIME"); break;
			case XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS:
				strcpy(str, "B2_WAIT_SKESENDEKS"); break;
			case XHDCP22_RX_STATE_B3_COMPUTE_KS:
				strcpy(str, "B3_COMPUTE_KS"); break;
			case XHDCP22_RX_STATE_B4_AUTHENTICATED:
				strcpy(str, "B4_AUTHENTICATED"); break;
			}
			xil_printf("Current state [%s]\r\n", str);
			break;
		case XHDCP22_RX_LOG_EVT_INFO_MESSAGE:
			switch(LogPtr->Data)
			{
			/* Print Message Log Data */
			case XHDCP22_RX_MSG_ID_AKEINIT:
				strcpy(str, "Received message [AKEINIT]"); break;
			case XHDCP22_RX_MSG_ID_AKESENDCERT:
				strcpy(str, "Sent message [AKESENDCERT]"); break;
			case XHDCP22_RX_MSG_ID_AKENOSTOREDKM:
				strcpy(str, "Received message [AKENOSTOREDKM]"); break;
			case XHDCP22_RX_MSG_ID_AKESTOREDKM:
				strcpy(str, "Received message [AKESTOREDKM]"); break;
			case XHDCP22_RX_MSG_ID_AKESENDHPRIME:
				strcpy(str, "Sent message [AKESENDHPRIME]"); break;
			case XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO:
				strcpy(str, "Sent message [AKESENDPAIRINGINFO]"); break;
			case XHDCP22_RX_MSG_ID_LCINIT:
				strcpy(str, "Received message [LCINIT]"); break;
			case XHDCP22_RX_MSG_ID_LCSENDLPRIME:
				strcpy(str, "Sent message [LCSENDLPRIME]"); break;
			case XHDCP22_RX_MSG_ID_SKESENDEKS:
				strcpy(str, "Received message [SKESENDEKS]"); break;
			}
			xil_printf("%s\r\n", str);
			break;
		case XHDCP22_RX_LOG_EVT_DEBUG:
			switch(LogPtr->Data)
			{
			/* Print Debug Log Data */
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA:
				strcpy(str, "COMPUTE_RSA"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA_DONE:
				strcpy(str, "COMPUTE_RSA_DONE"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_KM:
				strcpy(str, "COMPUTE_KM"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_KM_DONE:
				strcpy(str, "COMPUTE_KM_DONE"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME:
				strcpy(str, "COMPUTE_HPRIME"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME_DONE:
				strcpy(str, "COMPUTE_HPRIME_DONE"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH:
				strcpy(str, "COMPUTE_EKHKM"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH_DONE:
				strcpy(str, "COMPUTE_EKHKM_DONE"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME:
				strcpy(str, "COMPUTE_LPRIME"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME_DONE:
				strcpy(str, "COMPUTE_LPRIME_DONE"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_KS:
				strcpy(str, "COMPUTE_KS"); break;
			case XHDCP22_RX_LOG_DEBUG_COMPUTE_KS_DONE:
				strcpy(str, "COMPUTE_KS_DONE"); break;
			}
			xil_printf("Debug: Event [%s]\r\n", str);
			break;
		case XHDCP22_RX_LOG_EVT_ERROR:
			switch(LogPtr->Data)
			{
			/* Print Error Log Data */
			case XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE:
				strcpy(str, "Received message with unexpected size"); break;
			case XHDCP22_RX_ERROR_FLAG_FORCE_RESET:
				strcpy(str, "Forcing reset after error"); break;
			case XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT:
				strcpy(str, "Problem processing received message [AKEINIT]"); break;
			case XHDCP22_RX_ERROR_FLAG_PROCESSING_AKENOSTOREDKM:
				strcpy(str, "Problem processing received message [AKENOSTOREDKM]"); break;
			case XHDCP22_RX_ERROR_FLAG_PROCESSING_AKESTOREDKM:
				strcpy(str, "Problem processing received message [AKESTOREDKM]"); break;
			case XHDCP22_RX_ERROR_FLAG_PROCESSING_LCINIT:
				strcpy(str, "Problem processing received message [LCINIT]"); break;
			case XHDCP22_RX_ERROR_FLAG_PROCESSING_SKESENDEKS:
				strcpy(str, "Problem processing received message [SKESENDEKS]"); break;
			case XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY:
				strcpy(str, "Detected problem with link integrity"); break;
			case XHDCP22_RX_ERROR_FLAG_DDC_BURST:
				strcpy(str, "Detected problem with DDC burst read/write"); break;
			case XHDCP22_RX_ERROR_FLAG_MAX_LCINIT_ATTEMPTS:
				strcpy(str, "Exceeded maximum LCINIT attempts"); break;
			}
			xil_printf("Error: %s\r\n", str);
			break;
		case XHDCP22_RX_LOG_EVT_USER:
			xil_printf("User: %d\r\n", LogPtr->Data);
			break;
		default:
			xil_printf("Error: Unknown log event\r\n");
			break;
		}
	} while (LogPtr->LogEvent != XHDCP22_RX_LOG_EVT_NONE);
}

/*****************************************************************************/
/**
* This function is a stub for the run handler. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param	HandlerRef is a callback reference passed in by the upper
*			layer when setting the callback functions, and passed back to
*			the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22_Rx_StubRunHandler(void *HandlerRef)
{
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
* This function is a stub for the set handler. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param	HandlerRef is a callback reference passed in by the upper
*			layer when setting the callback functions, and passed back to
*			the upper layer when the callback is invoked.
* @param	Data is a value to be set.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22_Rx_StubSetHandler(void *HandlerRef, u32 Data)
{
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
* This function is a stub for the set handler. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param	HandlerRef is a callback reference passed in by the upper
*			layer when setting the callback functions, and passed back to
*			the upper layer when the callback is invoked.
*
* @return	Returns the get value.
*
* @note		None.
******************************************************************************/
static u32 XHdcp22_Rx_StubGetHandler(void *HandlerRef)
{
	Xil_AssertNonvoidAlways();
}

/** @} */
