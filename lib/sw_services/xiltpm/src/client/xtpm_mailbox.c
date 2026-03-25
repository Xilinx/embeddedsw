/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_mailbox.c
*
* This file contains the implementation of the xilmailbox generic interface APIs for
* xilTPM client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pre  03/09/26 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xtpm_mailbox_apis XilTPM mailbox APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xtpm_mailbox.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
 * @brief  This function sends IPI request to the target module and gets the
 * response from it
 *
 * @param	MailboxPtr	Pointer to mailbox instance
 * @param	MsgPtr		Pointer to the payload message
 * @param	MsgLen		Length of the message
 *
 * @return
 * 		- XST_SUCCESS  If the IPI send and receive is successful
 * 		- XST_FAILURE  If there is a failure
 *
 * @note	Payload  consists of API id and call arguments to be written
 * 		in IPI buffer
 *
 ****************************************************************************/
int XTpm_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen)
{
	int Status = XST_FAILURE;
	u32 Response[RESPONSE_ARG_CNT];

	/**
	 *  Send IPI CDO to PLM.
	 *  Return XST_FAILURE, if failure in IPI send request.
	 */
	Status = (int)XMailbox_SendData(MailboxPtr, XTPM_TARGET_IPI_INT_MASK, MsgPtr, MsgLen,
				XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Wait for IPI response from PLM  with a default timeout of 2 seconds.
	 * If the timeout exceeds, then error is returned otherwise it returns the status of the IPI response.
	 */
	Status = (int)XMailbox_Recv(MailboxPtr, XTPM_TARGET_IPI_INT_MASK, Response, RESPONSE_ARG_CNT,
				XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)Response[0];

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function sets the instance of mailbox
*
* @param 	InstancePtr	Pointer to the client instance
* @param 	MailboxPtr 	Pointer to the mailbox instance
*
* @return
* 		- XST_SUCCESS  On successful initialization
* 		- XST_FAILURE  On failure
*
******************************************************************************/
int XTpm_ClientInit(XTpm_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr) {
	int Status = XST_FAILURE;

        /**
	 *  Perform input parameter validation on InstancePtr.
	 *  If not NULL initialize the InstancePtr, else return XST_FAILURE.
	 */
	if (InstancePtr != NULL) {
			InstancePtr->MailboxPtr = MailboxPtr;
			InstancePtr->SlrIndex = 0U;
			Status = XST_SUCCESS;
	}

	return Status;
}
/** @} */
