/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_katclient.c
*
* This file contains the implementation of the client interface functions for
* KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  07/18/22 Initial release
* 5.1   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_katclient.h"
#include "xsecure_plat_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform decrypt KAT on AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM	 - On invalid argument
 *	-	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key
 *							write fails
 *	-	XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR - Error when AES
 * 							decrypt init fails
 *	-	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag
 * 					not matched with user provided tag
 *	-	XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data
 * 					not matched with expected data
 *
 ******************************************************************************/
int XSecure_AesDecryptKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_AES_DECRYPT_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform KAT on AES DPA
 *          countermeasure KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_AESKAT_INVALID_PARAM	- Invalid Argument
 *	-	XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR - Error when
 * 						AESDPACM key write fails
 *	-	XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR - Error when
 * 						AESDPACM key load fails
 *	-	XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR - Error when
 * 						AESDPACM sss configuration fails
 *	-	XSECURE_AESDPACM_KAT_FAILED_ERROR - Error when AESDPACM KAT fails
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptCmKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_AES_DECRYPT_CM_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform RSA encrypt KAT
 *
 * @param	InstancePtr	- Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when
 *					RSA data not matched with expected data
 *
 ******************************************************************************/
int XSecure_RsaPublicEncKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_RSA_PUB_ENC_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform SHA3 KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_SHA3_INVALID_PARAM 	 - On invalid argument
 *	-	XSECURE_SHA3_LAST_UPDATE_ERROR - Error when SHA3 last update fails
 *	-	XSECURE_SHA3_KAT_FAILED_ERROR	 - Error when SHA3 hash not matched with
 *					   expected hash
 *	-	XSECURE_SHA3_PMC_DMA_UPDATE_ERROR - Error when DMA driver fails to update
 *					      the data to SHA3
 *	-	XSECURE_SHA3_FINISH_ERROR 	 - Error when SHA3 finish fails
 *
 ******************************************************************************/
int XSecure_Sha3Kat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_SHA3_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform ECC sign verify KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CurveClass		- Type of elliptic curve class(Prime - 0, Binary - 1)
 *
 * @return
 *	-	XST_SUCCESS - On success
 * 	-	XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key
 * 							is not valid
 *	-	XSECURE_ELLIPTIC_KAT_FAILED_ERROR - When elliptic KAT
 *							fails
 *
 ******************************************************************************/
int XSecure_EllipticSignVerifyKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT;
	Payload[2U] = CurveClass;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform encrypt KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM	 - On invalid argument
 *	-	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key
 *							write fails
 *	-	XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR - Error when AES
 * 							decrypt init fails
 *	-	XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data
 * 					not matched with expected data
 *
 ******************************************************************************/
int XSecure_AesEncryptKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_AES_ENCRYPT_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to Perform RSA private decrypt KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR - Error when
 *					RSA data not matched with expected data
 *
 ******************************************************************************/
int XSecure_RsaPrivateDecKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_RSA_PRIVATE_DEC_KAT;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform ECC sign generate KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CurveClass  Type of elliptic curve class(Prime - 0, Binary - 1)
 *
 * @return
 *	-	XST_SUCCESS - On success
 * 	-	XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key
 * 							is not valid
 *	-	XSECURE_ELLIPTIC_KAT_FAILED_ERROR - When elliptic KAT
 *							fails
 *
 ******************************************************************************/
int XSecure_EllipticSignGenKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_ELLIPTIC_SIGN_GEN_KAT;
	Payload[2U] = CurveClass;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
