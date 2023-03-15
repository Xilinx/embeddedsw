/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_client.c
*
* This file contains the implementation of platform specific client interface functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.1   kpt  07/18/22 Initial release
*       am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_client.h"
#include "xsecure_plat_defs.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   NodeId				Nodeid of the module
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
int XSecure_UpdateCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
		u32 NodeId, u32 CryptoMask)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((CryptoStatusOp != XSECURE_CRYPTO_STATUS_SET) && (CryptoStatusOp != XSECURE_CRYPTO_STATUS_CLEAR)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_UPDATE_CRYPTO_STATUS);
	Payload[1U] = CryptoStatusOp;
	Payload[2U] = NodeId;
	Payload[3U] = CryptoMask;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
