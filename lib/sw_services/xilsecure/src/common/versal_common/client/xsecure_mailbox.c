/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_mailbox.c
*
* This file contains the implementation of the xilmailbox generic interface APIs for
* xilsecure client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Renamed XSecure_ConfigIpi as XSecure_SetIpi
*                     Added XSecure_InitializeIpi
*       am   05/22/21 Resolved MISRA C violations
* 4.6   har  07/14/21 Fixed doxygen warnings
*       kpt  09/27/21 Fixed compilation warnings
* 4.7   kpt  01/13/21 Added API's to set and get the shared memory
*       am   03/08/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"

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
 * @param	MsgPtr		Pointer to the payload message
 * @param	MsgLen		Length of the message
 *
 * @return
 *	-	XST_SUCCESS - If the IPI send and receive is successful
 *	-	XST_FAILURE - If there is a failure
 *
 * @note	Payload  consists of API id and call arguments to be written
 * 		in IPI buffer
 *
 ****************************************************************************/
int XSecure_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen)
{
	int Status = XST_FAILURE;
	u32 Response[RESPONSE_ARG_CNT];

	Status = (int)XMailbox_SendData(MailboxPtr, XSECURE_TARGET_IPI_INT_MASK, MsgPtr, MsgLen,
				XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)XMailbox_Recv(MailboxPtr, XSECURE_TARGET_IPI_INT_MASK, Response, RESPONSE_ARG_CNT,
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
*
* This function sets the instance of mailbox
*
* @param InstancePtr Pointer to the client instance
* @param MailboxPtr Pointer to the mailbox instance
*
* @return
* 	- XST_SUCCESS	On successful initialization
* 	- XST_FAILURE	On failure
*
******************************************************************************/
int XSecure_ClientInit(XSecure_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr)
{
	int Status = XST_FAILURE;

	if (InstancePtr != NULL){
			InstancePtr->MailboxPtr = MailboxPtr;
			InstancePtr->SlrIndex = 0U;
			Status = XST_SUCCESS;
	}

	return Status;
}
