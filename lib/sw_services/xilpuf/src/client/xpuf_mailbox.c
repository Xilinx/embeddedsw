/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_mailbox.c
*
* This file contains the implementation of the xilmailbox generic interface APIs for
* xilpuf client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       kpt  01/13/22 Removed hardcoded IPI device id
*       am   02/18/22 Fixed COMF code complexity violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xpuf_mailbox.h"

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
 *	-	XST_SUCCESS - If the IPI send and receive is successful
 *	-	XST_FAILURE - If there is a failure
 *
 * @note	Payload  consists of API id and call arguments to be written
 * 		in IPI buffer
 *
 ****************************************************************************/
int XPuf_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen)
{
	int Status = XST_FAILURE;
	u32 Response[RESPONSE_ARG_CNT];

	Status = (int)XMailbox_SendData(MailboxPtr, XPUF_TARGET_IPI_INT_MASK, MsgPtr, MsgLen,
				XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)XMailbox_Recv(MailboxPtr, XPUF_TARGET_IPI_INT_MASK, Response, RESPONSE_ARG_CNT,
				XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)Response[0];

END:
	return Status;
}
