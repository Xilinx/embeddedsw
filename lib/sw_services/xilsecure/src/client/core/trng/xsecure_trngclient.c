/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.2   am   04/01/23 Added XST_INVALID_PARAM error code for invalid parameters
*       am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*       am   07/31/23 Fixed typo for XSecure_TrngGenerareRandNum function
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*
* </pre>
*
*
******************************************************************************/
/**
* @addtogroup xsecure_trng_client_apis XilSecure TRNG Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_trngclient.h"

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to generate random number
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	RandBufAddr	Rand Buffer address to store the generated random number.
 * @param	Size		Number of random bytes needs to be generated.
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XST_INVALID_PARAM  If any input parameters are invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_TrngGenerateRandNum(XSecure_ClientInstance *InstancePtr, u64 RandBufAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (Size > XSECURE_TRNG_SEC_STRENGTH_IN_BYTES) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_TRNG_GENERATE);
	Payload[1U] = (u32)RandBufAddr;
	Payload[2U] = (u32)(RandBufAddr >> 32);
	Payload[3U] = Size;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_TrngGenerateRandNum
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/** @} */
