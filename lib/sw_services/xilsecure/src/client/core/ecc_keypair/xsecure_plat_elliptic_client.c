/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file client/core/ecc_keypair/xsecure_plat_elliptic_client.c
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
*       yog  03/18/25 Defined a structure and updated the ECDH input params to that structure.
* 5.7   tvp  04/30/26 Added XSecure_EllipticPrivateKeyGenerate
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
#include "xsecure_defs.h"

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
	u32 Payload[PAYLOAD_ARG_CNT];
	XSecure_EcdhParams *EcdhParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u64 PrvtKeyAddr = (u64)(UINTPTR)PrivateKey;
	u64 PubKeyAddr = (u64)(UINTPTR)PublicKey;
	u64 SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Link shared memory of size EcdsaParams to EcdsaParams structure for IPI usage.
	 *   Validates the size of the shared memory whether the required size is available or not.
	 */
	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EcdhParams);
	if ((EcdhParams == NULL) || (MemSize < sizeof(XSecure_EcdhParams))) {
		goto END;
	}

	EcdhParams->CurveType = CrvType;
	EcdhParams->PubKeyAddrHigh = (u32)(PubKeyAddr >> XSECURE_ADDR_HIGH_SHIFT);
	EcdhParams->PubKeyAddrLow = (u32)PubKeyAddr;
	EcdhParams->PrivKeyAddrHigh = (u32)(PrvtKeyAddr >> XSECURE_ADDR_HIGH_SHIFT);
	EcdhParams->PrivKeyAddrLow = (u32)PrvtKeyAddr;
	EcdhParams->SharedSecretAddrHigh = (u32)(SharedSecretAddr >> XSECURE_ADDR_HIGH_SHIFT);
	EcdhParams->SharedSecretAddrLow = (u32)SharedSecretAddr;
	Buffer = (u64)(UINTPTR)EcdhParams;

	XSecure_DCacheFlushRange(EcdhParams, sizeof(XSecure_EcdhParams));

	/** - Fill IPI Payload */
	XSECURE_PACK_PAYLOAD2(Payload, XSECURE_API_GEN_SHARED_SECRET,
				Buffer,
				(Buffer >> XSECURE_ADDR_HIGH_SHIFT));

	/**
	 * - Send an IPI request to the PLM by using the CDO command to call XSecure_GenSharedSecret
	 *   API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request for ECC private key generation.
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	ParamsPtr	Pointer to the XSecure_EccPrivateKeyParams
 *				structure.
 *
 * @return
 *		 - XST_SUCCESS  On Success.
 *		 - XST_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  If there is a failure.
 *
 ******************************************************************************/
int XSecure_EllipticPrivateKeyGenerate(XSecure_ClientInstance *InstancePtr,
		const XSecure_EccPrivateKeyParams *ParamsPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 Addr = (u64)(UINTPTR)ParamsPtr;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) ||
	    (ParamsPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSECURE_PACK_PAYLOAD3(Payload, XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN,
			(u32)Addr, (u32)(Addr >> XSECURE_ADDR_HIGH_SHIFT),
			(u32)sizeof(XSecure_EccPrivateKeyParams));

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
			PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */
