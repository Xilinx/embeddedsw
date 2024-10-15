/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_elliptic_client.c
*
* This file contains the implementation of elliptic client interface APIs for
* Versal Net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   har  06/15/23 Initial release
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_client_apis XilSecure ECDSA Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_mailbox.h"
#include "xsecure_plat_elliptic_client.h"
#include "xsecure_plat_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to generate shared secret using
 * 		Elliptic Curve Diffie–Hellman Key Exchange (ECDH)
 *
* @param	InstancePtr	Pointer to the client instance
* @param	CrvType		Type of elliptic curve
* @param	PrivateKey	Pointer to the private key buffer
* @param	PublicKey	Pointer to the public key buffer
* @param	SharedSecret	Pointer to the output buffer which shall
* 				be used to store shared secret
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_GenSharedSecret(XSecure_ClientInstance *InstancePtr, u32 CrvType, const u8* PrivateKey,
	const u8* PublicKey, u8 *SharedSecret)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_MAX_MSG_LEN];
	u64 PrvtKeyAddr = (u64)(UINTPTR)PrivateKey;
	u64 PubKeyAddr = (u64)(UINTPTR)PublicKey;
	u64 SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_GEN_SHARED_SECRET);
	Payload[1U] = CrvType;
	Payload[2U] = (u32)PrvtKeyAddr;
	Payload[3U] = (u32)(PrvtKeyAddr >> XSECURE_ADDR_HIGH_SHIFT);
	Payload[4U] = (u32)PubKeyAddr;
	Payload[5U] = (u32)(PubKeyAddr >> XSECURE_ADDR_HIGH_SHIFT);
	Payload[6U] = (u32)SharedSecretAddr;
	Payload[7U] = (u32)(SharedSecretAddr >> XSECURE_ADDR_HIGH_SHIFT);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_GenSharedSecret
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
/** @} */
