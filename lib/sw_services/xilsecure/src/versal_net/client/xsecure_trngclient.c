/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trngclient.c
*
* This file contains the implementation of the client interface functions for
* TRNG core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   am   06/13/22 Initial release
*       kpt  07/24/22 moved XSecure_TrngKat into xsecure_katclient_plat.c
* 5.1   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_trngclient.h"

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to generate
 *              random number
 *
 * @param	InstancePtr - Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	Errorcode - On failure
 *
 ******************************************************************************/
int XSecure_TrngGenerareRandNum(XSecure_ClientInstance *InstancePtr, u64 RandBufAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if (Size > XSECURE_TRNG_SEC_STRENGTH_IN_BYTES) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_TRNG_GENERATE);
	Payload[1U] = (u32)RandBufAddr;
	Payload[2U] = (u32)(RandBufAddr >> 32);
	Payload[3U] = Size;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
