/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox.c
 * @addtogroup xilmailbox Overview
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
 * 1.3   sd   03/03/21    Doxygen Fixes
 * 1.4   sd   23/06/21    Fix MISRA-C warnings
 * 1.6   kpt  03/16/22    Added shared memory API's for IPI utilization
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

	/* Validate the input arguments */
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

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(MsgLen <= XMAILBOX_MAX_MSG_LEN);
	Xil_AssertNonvoid((BufferType == XILMBOX_MSG_TYPE_REQ) ||
			  (BufferType == XILMBOX_MSG_TYPE_RESP));

	/* Initialize the destination CPU */
	InstancePtr->Agent.RemoteId = RemoteId;

	/* Send IPI message to a destination CPU */
	Status = InstancePtr->XMbox_IPI_SendData(InstancePtr, BufferPtr,
			MsgLen, BufferType,
			Is_Blocking);
	/* Return statement */
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

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(MsgLen <= XMAILBOX_MAX_MSG_LEN);
	Xil_AssertNonvoid((BufferType == XILMBOX_MSG_TYPE_REQ) ||
			  (BufferType == XILMBOX_MSG_TYPE_RESP));

	/* Initialize the source CPU */
	InstancePtr->Agent.SourceId = SourceId;

	/* Read IPI message from the buffer */
	Status = InstancePtr->XMbox_IPI_Recv(InstancePtr, BufferPtr, MsgLen,
					     BufferType);

	/* Check if there is any error while receiving message */
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("Error while receiving message %s", __func__);
	}
	/* Return statement */
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
* @param	CallBackFuncPtr is the address of the callback function.
* @param	CallBackRefPtr is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_FAILURE when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
s32 XMailbox_SetCallBack(XMailbox *InstancePtr, XMailbox_Handler HandlerType,
			 void *CallBackFuncPtr, void *CallBackRefPtr)
{
	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackFuncPtr != NULL);
	Xil_AssertNonvoid(CallBackRefPtr != NULL);
	Xil_AssertNonvoid((HandlerType == XMAILBOX_RECV_HANDLER) ||
			  (HandlerType == XMAILBOX_ERROR_HANDLER));

	/*
	 * Calls the respective callback function corresponding to
	 * the handler type
	 */
	if (HandlerType == XMAILBOX_RECV_HANDLER) {
		/* Call Recv handler */
		InstancePtr->RecvHandler =
			(XMailbox_RecvHandler)((void *)CallBackFuncPtr);
		InstancePtr->RecvRefPtr = CallBackRefPtr;
	} else {
		/* Call Error handler */
		InstancePtr->ErrorHandler =
			(XMailbox_ErrorHandler)((void *)CallBackFuncPtr);
		InstancePtr->ErrorRefPtr = CallBackRefPtr;
	}

	/* Return statement */
	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief	This function sets the shared memory location for IPI usage
*
* @param	InstancePtr is a pointer to the XMailbox instance.
* @param	Address	Address of shared memory location
* @param	Size	Size of the memory location
*
* @return
*	-	XST_SUCCESS - if memory is set for IPI usage
*	-	XST_FAILURE - On failure
*
******************************************************************************/
u32 XMailbox_SetSharedMem(XMailbox *InstancePtr, u64 Address, u32 Size)
{
	u32 Status = XST_FAILURE;

	/* Set the shared memory segment */
	if (InstancePtr != NULL) {
		if (InstancePtr->SharedMem.SharedMemState != XMAILBOX_SHARED_MEM_INITIALIZED) {
			InstancePtr->SharedMem.Address = Address;
			InstancePtr->SharedMem.Size = Size;
			InstancePtr->SharedMem.SharedMemState = XMAILBOX_SHARED_MEM_INITIALIZED;
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* @brief	This function returns the shared memory location for IPI usage
*
* @param	InstancePtr is a pointer to the XMailbox instance.
* @param	Address	Pointer to the address of the variable for
* 			which memory needs to be assigned
*
* @return
* 	- Size	Size of the memory allocated for IPI usage
*
******************************************************************************/
u32 XMailbox_GetSharedMem(XMailbox *InstancePtr, u64 **Address)
{
	u32 Size = 0U;

	/* Get the size of shared memory location */
	if (InstancePtr != NULL) {
		if (InstancePtr->SharedMem.SharedMemState == XMAILBOX_SHARED_MEM_INITIALIZED) {
			*Address = (u64 *)(UINTPTR)InstancePtr->SharedMem.Address;
			Size = InstancePtr->SharedMem.Size;
		}
	}

	return Size;
}

/*****************************************************************************/
/**
*
* @brief	This function releases the shared memory
*
* @return
*	-	XST_SUCCESS - if memory is released
*	-	XST_FAILURE - if memory is not released
*
******************************************************************************/
int XMailbox_ReleaseSharedMem(XMailbox *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Release the shared memory */
	if (InstancePtr != NULL) {
		InstancePtr->SharedMem.Address = 0U;
		InstancePtr->SharedMem.Size = 0U;
		InstancePtr->SharedMem.SharedMemState = XMAILBOX_SHARED_MEM_UNINITIALIZED;
		Status = XST_SUCCESS;
	}

	return Status;
}
