/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_mailbox.c
*
* This file contains the implementation of the xilmailbox generic interface APIs
* for xilocp client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xocp_mailbox.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to the target module and gets
 *          the response from it
 *
 * @param   MailboxPtr - Pointer to mailbox instance
 * @param   MsgPtr - Pointer to the payload message
 * @param   MsgLen - Length of the message
 *
 * @return
 *          - XST_SUCCESS - If the IPI send and receive is successful
 *          - XST_FAILURE - If there is a failure
 *
 * @note   Payload consists of API id and call arguments to be written in
 *         IPI buffer
 *
 ****************************************************************************/
int XOcp_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen)
{
	int Status = XST_FAILURE;
	u32 Response[RESPONSE_ARG_CNT];

	/**
	 * Sends IPI CDO to PLM, returns XST_FAILURE if sending data is failed
	 */
	Status = (int)XMailbox_SendData(MailboxPtr, XOCP_TARGET_IPI_INT_MASK, MsgPtr, MsgLen,
		XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Waits for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then error is returned otherwise,
	 * it returns the status of the IPI response
	 */
	Status = (int)XMailbox_Recv(MailboxPtr, XOCP_TARGET_IPI_INT_MASK, Response, RESPONSE_ARG_CNT,
		XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)Response[0];

END:
	return Status;
}