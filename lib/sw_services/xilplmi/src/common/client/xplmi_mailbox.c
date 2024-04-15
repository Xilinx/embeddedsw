/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_mailbox.c
 *
 * This file contains the implementation of the xilmailbox generic interface APIs for
 * xilplmi client library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
  * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       am   04/04/24 Fixed doxygen warnings
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xplmi_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to the target module and gets the response from it
 *
 * @param	ClientPtr	Pointer to mailbox instance
 * @param	MsgPtr		Pointer to the payload message
 * @param	MsgLen		Length of the message
 *
 * @return
 *			 - XST_SUCCESS - If the IPI send and receive is successful
 *			 - XST_FAILURE - If there is a failure
 *
 * @note	Payload  consists of API id and call arguments to be written in IPI buffer
 *
 **************************************************************************************************/
int XPlmi_ProcessMailbox(XPlmi_ClientInstance *ClientPtr, u32 *MsgPtr, u32 MsgLen)
{
	int Status = XST_FAILURE;

    /**
	 * - Send IPI CDO to PLM. Return XST_FAILURE if sending data failed
	 */
	Status = (int)XMailbox_SendData((XMailbox *)ClientPtr->MailboxPtr, XPLMI_TARGET_IPI_INT_MASK,
				MsgPtr, MsgLen, XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 * - Wait for IPI response from PLM  with a default timeout of 300 seconds.
     * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response
	 */
	Status = (int)XMailbox_Recv((XMailbox *)ClientPtr->MailboxPtr, XPLMI_TARGET_IPI_INT_MASK,
					(ClientPtr->Response), RESPONSE_ARG_CNT,XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)ClientPtr->Response[0];

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function sets the instance of mailbox
*
* @param	InstancePtr	Pointer to the client instance
* @param	MailboxPtr	Pointer to the mailbox instance
*
* @return
*			 - XST_SUCCESS	On successful initialization
*			 - XST_FAILURE	On failure
*
 *************************************************************************************************/
int XPlmi_ClientInit(XPlmi_ClientInstance* InstancePtr, XMailbox* MailboxPtr)
{
	int Status = XST_FAILURE;

    /**
	 *  - Perform input parameter validation on InstancePtr,if not NULL initialize the InstancePtr
	 *  Return XST_FAILURE if NULL
	 */
	if (InstancePtr != NULL) {
			InstancePtr->MailboxPtr = MailboxPtr;
			Status = XST_SUCCESS;
	}

	return Status;
}