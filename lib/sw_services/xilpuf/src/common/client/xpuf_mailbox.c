/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 2.1   skg  11/07/22 Added In Body comments
*       am   02/17/23 Fixed HIS_COMF violations
*       kpt  08/03/23 Fix security review comments
* 2.4   ng   04/30/24 Fixed doxygen comments
* 2.8   tbk  06/22/26 Added input parameter validation
*       tus  08/07/26 Added optional IPI response argument to XPuf_ProcessMailbox
*
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xpuf_mailbox_apis XilPuf Mailbox APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xpuf_mailbox.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
 * @brief 	This function sends IPI request to the target module and gets the
 * 		response from it.
 *
 * @param	MailboxPtr	Pointer to mailbox instance
 * @param	MsgPtr		Pointer to the payload message
 * @param	MsgLen		Length of the message
 * @param	Response	Pointer to store the IPI response. When non-NULL,
 * 				the buffer must hold RESPONSE_ARG_CNT words. Word 0 is
 * 				the IPI status and the following words are payload data.
 *
 * @return
 *		- XST_SUCCESS if the IPI send and receive is successful
 *		- XST_INVALID_PARAM if any input parameter is invalid
 *		- XST_FAILURE if there is a failure
 *
 * @note	Payload consists of API ID and call arguments to be written
 * 		in IPI buffer.
 *
 ****************************************************************************/
int XPuf_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen, u32 *Response)
{
	volatile int Status = XST_FAILURE;
	u32 LocalResp[RESPONSE_ARG_CNT] = {0U};

	if (Response == NULL) {
		Response = LocalResp;
	}

	/** - Validate input parameters. */
	if ((MailboxPtr == NULL) || (MsgPtr == NULL) || (MsgLen == 0U)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Send CDO to PLM through IPI. Return XST_FAILURE if sending data failed. */
	Status = (int)XMailbox_SendData(MailboxPtr, XPUF_TARGET_IPI_INT_MASK, MsgPtr, MsgLen,
				XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/** - Wait for IPI response from PLM and return the status of the IPI response. */
	Status = (int)XMailbox_Recv(MailboxPtr, XPUF_TARGET_IPI_INT_MASK, Response, RESPONSE_ARG_CNT,
				XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = (int)Response[0];

END:
	return Status;
}
/** @} */
