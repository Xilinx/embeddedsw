/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_katclient.c
*
* This file contains the implementation of the client interface functions for
* KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt  07/18/22 Initial release
* 5.1   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_katclient.h"
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
 * @brief	This function sends IPI request to PLM to perform TRNG KAT and health tests
 *
 * @param	InstancePtr  Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	Errorcode - On failure
 *
 ******************************************************************************/
int XSecure_TrngKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_TRNG_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to set
 *          or clear kat mask of KAT's running on CPM5N, NICSEC.
 *
 * @param	InstancePtr - Pointer to the client instance
 * @param   KatOp		- Operation to set or clear KAT mask
 * @param   NodeId		- Nodeid of the module
 * @param   KatMaskLen  - Length of the KAT mask
 * @param   KatMask     - Pointer to the KAT mask
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode - On failure
 *
 ******************************************************************************/
int XSecure_UpdateKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp, u32 NodeId,
	u32 KatMaskLen, u32 *KatMask)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_7U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((KatOp != XSECURE_API_KAT_SET) && (KatOp != XSECURE_API_KAT_CLEAR)) {
		goto END;
	}

	if ((KatMaskLen < XSECURE_MIN_KAT_MASK_LEN) || (KatMaskLen > XSECURE_MAX_KAT_MASK_LEN)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER((KatMaskLen + XSECURE_KAT_HDR_LEN), XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_UPDATE_KAT_STATUS;
	Payload[2U] = (u32)KatOp;
	Payload[3U] = NodeId;

	Status = Xil_SMemCpy((u8*)&Payload[4U], (KatMaskLen * XSECURE_WORD_LEN), (u8*)KatMask, (KatMaskLen * XSECURE_WORD_LEN),
				(KatMaskLen * XSECURE_WORD_LEN));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}
