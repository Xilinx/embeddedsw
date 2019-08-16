
/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilmailbox.c
 * @addtogroup xilmailbox_v1_1
 * @{
 * @details
 *
 * This file contains the definitions for xilinx mailbox library top level
 * functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  14/02/19    Initial Release
 * 1.1   sd   16/08/19    Initialise status variable
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilmailbox.h"

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function triggers an IPI to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param RemoteId is the Mask of the CPU to which IPI is to be triggered
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return
 *	- XST_SUCCESS if successful
 *	- XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 XMailbox_Send(XMailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking)
{
	u32 Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	InstancePtr->Agent.RemoteId = RemoteId;
	Status = InstancePtr->XMbox_IPI_Send(InstancePtr, Is_Blocking);
	return Status;
}

/*****************************************************************************/
/**
 * This function sends an IPI message to a destination CPU
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param RemoteId is the Mask of the CPU to which IPI is to be triggered
 * @param BufferPtr is the pointer to Buffer which contains the message to be sent
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer (XILMBOX_MSG_TYPE_REQ (OR)
 *	  XILMBOX_MSG_TYPE_RESP)
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return
 *	- XST_SUCCESS if successful
 *	- XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 XMailbox_SendData(XMailbox *InstancePtr, u32 RemoteId, void *BufferPtr,
		      u32 MsgLen, u8 BufferType, u8 Is_Blocking)
{
	u32 Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(MsgLen <= XMAILBOX_MAX_MSG_LEN);
	Xil_AssertNonvoid((BufferType == XILMBOX_MSG_TYPE_REQ) ||
			  (BufferType == XILMBOX_MSG_TYPE_RESP));

	InstancePtr->Agent.RemoteId = RemoteId;
	Status = InstancePtr->XMbox_IPI_SendData(InstancePtr, BufferPtr,
						 MsgLen, BufferType,
						 Is_Blocking);
	return Status;
}

/*****************************************************************************/
/**
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the XMailbox instance
 * @param SourceId is the Mask for the CPU which has sent the message
 * @param BufferPtr is the pointer to Buffer to which the read message needs
 *	  to be stored
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer (XILMBOX_MSG_TYPE_REQ or
 *	  XILMBOX_MSG_TYPE_RESP)
 *
 * @return
 *	- XST_SUCCESS if successful
 *	- XST_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 XMailbox_Recv(XMailbox *InstancePtr, u32 SourceId, void *BufferPtr,
		  u32 MsgLen, u8 BufferType)
{
	u32 Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(MsgLen <= XMAILBOX_MAX_MSG_LEN);
	Xil_AssertNonvoid((BufferType == XILMBOX_MSG_TYPE_REQ) ||
			  (BufferType == XILMBOX_MSG_TYPE_RESP));

	InstancePtr->Agent.SourceId = SourceId;
	Status = InstancePtr->XMbox_IPI_Recv(InstancePtr, BufferPtr, MsgLen,
					     BufferType);
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("Error while receiving message %s", __func__);
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XMAILBOX_RECV_HANDLER	   Recv handler
* XMAILBOX_ERROR_HANDLER   Error handler
*
* </pre>
*
* @param	InstancePtr is a pointer to the XMailbox instance.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
s32 XMailbox_SetCallBack(XMailbox *InstancePtr, XMailbox_Handler HandlerType,
			 void *CallBackFuncPtr, void *CallBackRefPtr)
{
	s32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackFuncPtr != NULL);
	Xil_AssertNonvoid(CallBackRefPtr != NULL);
	Xil_AssertNonvoid((HandlerType == XMAILBOX_RECV_HANDLER) ||
			  (HandlerType == XMAILBOX_ERROR_HANDLER));

	/*
	 * Calls the respective callback function corresponding to
	 * the handler type
	 */
	switch (HandlerType) {
		case XMAILBOX_RECV_HANDLER:
			InstancePtr->RecvHandler =
				(XMailbox_RecvHandler)((void *)CallBackFuncPtr);
			InstancePtr->RecvRefPtr = CallBackRefPtr;
			Status = (XST_SUCCESS);
			break;

		case XMAILBOX_ERROR_HANDLER:
			InstancePtr->ErrorHandler =
				(XMailbox_ErrorHandler)((void *)CallBackFuncPtr);
			InstancePtr->ErrorRefPtr = CallBackRefPtr;
			Status = (XST_SUCCESS);
			break;
		default:
			Status = (XST_INVALID_PARAM);
			break;
	}

	return Status;
}
