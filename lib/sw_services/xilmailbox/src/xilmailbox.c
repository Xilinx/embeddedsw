/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox.c
 * @brief Implements the XilMailbox library top-level functions.
 *
 * This file contains the definitions for the Xilinx mailbox library top-level
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
 * 1.11  ht   11/12/24    Update description of MsgLen
 * 1.12  ht   09/02/25    IPI operation relied on the RemoteId stored in XMailbox
 * 			  Instance, which could lead to race conditions and data
 * 			  corruption when handling multiple requests from
 * 			  different IPI channels concurrently.
 * 			  Use RemoteId based APIs to support concurrent IPI
 * 			  channel requests.
 * 1.14  vn   08/26/26    Fix Doxygen warnings.
 * 1.14  vn   09/30/26    Align Doxygen comments with AMD standards
 *
 *</pre>
 *
 *****************************************************************************/

/** @addtogroup xilmailbox Overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xilmailbox.h"

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Triggers an IPI to a destination CPU.
 *
 * @param[in,out] InstancePtr Pointer to the XMailbox instance.
 * @param[in]     RemoteId Mask of the CPU to which the IPI is triggered.
 * @param[in]     Is_Blocking Flag that, when set, triggers the notification
 *                in blocking mode.
 *
 * @return
 *                - XST_SUCCESS if successful.
 *                - XST_FAILURE if unsuccessful.
 *
 ****************************************************************************/
u32 XMailbox_Send(XMailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking)
{
	u32 Status = XST_FAILURE;

	/* Validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	InstancePtr->Agent.RemoteId = RemoteId;
	Status = InstancePtr->XMbox_IPI_SendById(InstancePtr, RemoteId,
						 Is_Blocking);
	return Status;
}

/*****************************************************************************/
/**
 * @brief Sends an IPI message to a destination CPU.
 *
 * @param[in,out] InstancePtr Pointer to the XMailbox instance.
 * @param[in]     RemoteId Mask of the CPU to which the IPI is triggered.
 * @param[in]     BufferPtr Pointer to the buffer that contains the message to
 *                send.
 * @param[in]     MsgLen Number of messages (each message is 4 bytes).
 * @param[in]     BufferType Type of buffer (XILMBOX_MSG_TYPE_REQ or
 *                XILMBOX_MSG_TYPE_RESP).
 * @param[in]     Is_Blocking Flag that, when set, triggers the notification
 *                in blocking mode.
 *
 * @return
 *                - XST_SUCCESS if successful.
 *                - XST_FAILURE if unsuccessful.
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
	Status = InstancePtr->XMbox_IPI_SendDataById(InstancePtr, BufferPtr,
						     MsgLen, BufferType,
						     RemoteId, Is_Blocking);
	/* Return statement */
	return Status;
}

/*****************************************************************************/
/**
 * @brief Reads an IPI message.
 *
 * @param[in,out] InstancePtr Pointer to the XMailbox instance.
 * @param[in]     SourceId Mask for the CPU that sent the message.
 * @param[out]    BufferPtr Pointer to the buffer in which to store the read
 *                message.
 * @param[in]     MsgLen Number of messages (each message is 4 bytes).
 * @param[in]     BufferType Type of buffer (XILMBOX_MSG_TYPE_REQ or
 *                XILMBOX_MSG_TYPE_RESP).
 *
 * @return
 *                - XST_SUCCESS if successful.
 *                - XST_FAILURE if unsuccessful.
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
	Status = InstancePtr->XMbox_IPI_RecvById(InstancePtr, BufferPtr,
						 MsgLen, SourceId, BufferType);

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
* @brief Installs an asynchronous callback function for the given HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XMAILBOX_RECV_HANDLER	   Recv handler
* XMAILBOX_ERROR_HANDLER   Error handler
*
* </pre>
*
* @param[out]  InstancePtr Pointer to the XMailbox instance.
* @param[in]   HandlerType Specifies which callback to attach.
* @param[in]   CallBackFuncPtr Address of the callback function.
* @param[in]   CallBackRefPtr User data item passed to the callback function
*              when it is invoked.
*
* @return
*              - XST_SUCCESS when the handler is installed.
*              - XST_FAILURE when HandlerType is invalid.
*
* @note        Invoking this function for a handler that has already been
*              installed replaces it with the new handler.
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
* @brief Sets the shared memory location for IPI usage.
*
* @param[in,out] InstancePtr Pointer to the XMailbox instance.
* @param[in]     Address Address of the shared memory location.
* @param[in]     Size Size of the memory location.
*
* @return
*                - XST_SUCCESS if the memory is set for IPI usage.
*                - XST_FAILURE on failure.
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
* @brief Returns the shared memory location for IPI usage.
*
* @param[in]   InstancePtr Pointer to the XMailbox instance.
* @param[out]  Address Pointer to the address of the variable to which the
*              memory is assigned.
*
* @return      Size of the memory allocated for IPI usage.
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
* @brief Releases the shared memory.
*
* @param[out]  InstancePtr Pointer to the XMailbox instance.
*
* @return
*              - XST_SUCCESS if the memory is released.
*              - XST_FAILURE if the memory is not released.
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
/** @} */
