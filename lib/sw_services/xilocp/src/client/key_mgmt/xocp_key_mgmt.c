/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_key_mgmt.c
*
* This file contains the implementation of the client interface functions for Key Mgmt APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 1.7   rpu  02/18/26 Initial release
*       rpu  03/11/26 Validate input parameters
* </pre>
*
* @note
*
***************************************************************************************************/

/**
 * @addtogroup xocp_key_mgmt_client_apis XilOcp KeyMgmt Client APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xocp_key_mgmt.h"

/************************************* Constant Definitions ***************************************/

/*************************************** Type Definitions *****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/************************************** Function Prototypes ***************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
 * @brief   This function sends IPI request to get 509 certificate.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   GetX509CertAddr - Address of XOcp_X509Cert structure.
  *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 **************************************************************************************************/
int XOcp_GetX509Cert(XOcp_ClientInstance *InstancePtr, u64 GetX509CertAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** - Validate input parameters */
	if ((InstancePtr == NULL) || (GetX509CertAddr == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI payload for XOCP_API_GETX509CERT command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_GETX509CERT, GetX509CertAddr,
				(GetX509CertAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief   This function sends IPI request to attest the data with DevAk.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   AttestWithDevAk - Address of XOcp_AttestWithDevAk structure.
 *
 * @return
 *          - XST_SUCCESS - Attestation with DevAk is successful
 *          - XST_FAILURE - Upon any failure
 *
 **************************************************************************************************/
int XOcp_ClientAttestWithDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttestWithDevAk)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** - Validate input parameters */
	if ((InstancePtr == NULL) || (AttestWithDevAk == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI payload for XOCP_API_ATTESTWITHDEVAK command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_ATTESTWITHDEVAK, AttestWithDevAk,
				(AttestWithDevAk >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function sends IPI request to calculate the hash of the
 * 		Key Wrap buffer and attest with Key Wrap DevAk private key.
 *
 * @param	InstancePtr - Pointer to the client instance
 * @param	AttnPloadAddr - Address of the buffer which should be attested
 * @param	AttnPloadSize - Size of buffer in bytes
 * @param	PubKeyOffset - Offset in provided buffer where public key needs to be stored
 * @param	SignatureAddr - Address of the signature after attestation
 *
 * @return
 *		- XST_SUCCESS - Successfully hashed and attested the data with Key Wrap DevAk private key
 *		- XST_FAILURE - Upon any failure
 *
 **************************************************************************************************/
int XOcp_ClientAttestWithKeyWrapDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttnPloadAddr, u32 AttnPloadSize, u32 PubKeyOffset, u64 SignatureAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** - Validate input parameters */
	if ((InstancePtr == NULL) || (AttnPloadAddr == 0U) || (AttnPloadSize == 0U) ||
		(SignatureAddr == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI payload for XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK command and send the request to Server */
	XOCP_PACK_PAYLOAD6(Payload, XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK,
			AttnPloadAddr, (AttnPloadAddr >> XOCP_ADDR_HIGH_SHIFT),
			AttnPloadSize, PubKeyOffset,
			SignatureAddr, (SignatureAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/**************************************************************************************************/
/**
 *
 * @brief	This function sends IPI request to generate shared secret using
 * 		Elliptic Curve Diffie-Hellman Key Exchange (ECDH). The private
 * 		key used to generate the shared secret is internal DevAK
 * 		private key which is determined by the subsystem from where the
 * 		command is originating.
 *
 * @param	InstancePtr		Pointer to the client instance
 * @param	PubKey			Pointer to the buffer which contains the
 * 					public key to be used for calculating
 * 					shared secret using ECDH.
 * @param	SharedSecret		Pointer to the output buffer which shall
 * 					be used to store shared secret
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - Errorcode  On failure
 *
 **************************************************************************************************/
int XOcp_GenSharedSecretWithDevAk(XOcp_ClientInstance *InstancePtr, const u8* PubKey, u8 *SharedSecret)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 PubKeyAddr = (u64)(UINTPTR)PubKey;
	u64 SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	/** - Validate input parameters */
	if ((InstancePtr == NULL) || (PubKey == NULL) || (SharedSecret == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if ((InstancePtr->MailboxPtr == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI Payload for XOCP_API_GEN_SHARED_SECRET and send request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GEN_SHARED_SECRET, PubKeyAddr,
				(PubKeyAddr >> XOCP_ADDR_HIGH_SHIFT),
				SharedSecretAddr,
				(SharedSecretAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */