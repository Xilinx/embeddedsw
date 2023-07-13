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
* 5.2   am   04/01/23 Added XST_INVALID_PARAM error code for invalid parameters
*       am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*       kpt  07/09/23 Added APIs related to Key wrap and unwrap
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
static int XSecure_UpdateCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
		u32 CryptoMask, u32 ApiId);
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit of HNIC
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
int XSecure_UpdateHnicCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask)
{
	return((XSecure_UpdateCryptoStatus(InstancePtr, CryptoStatusOp, CryptoMask,
			XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS)));
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit of of CPM 5N
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
int XSecure_UpdateCpm5NCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask)
{
	return((XSecure_UpdateCryptoStatus(InstancePtr, CryptoStatusOp, CryptoMask,
			XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS)));
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit of PCIDE
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
int XSecure_UpdatePcideCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask)
{
	return((XSecure_UpdateCryptoStatus(InstancePtr, CryptoStatusOp, CryptoMask,
			XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS)));
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit of PKI
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
int XSecure_UpdatePkiCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
	 u32 CryptoMask)
{
	return((XSecure_UpdateCryptoStatus(InstancePtr, CryptoStatusOp, CryptoMask,
			XSECURE_API_UPDATE_PKI_CRYPTO_STATUS)));
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to set crypto status bit(s) of module
 *
 * @param	InstancePtr  		Pointer to the client instance
 * @param   CryptoStatusOp		Operation to set or clear crypto status bit
 * @param   CryptoMask  		Mask to set or clear crypto status bit
 * @param	ApiId				API ID of the IPI command
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode	- On failure
 *
 ******************************************************************************/
static int XSecure_UpdateCryptoStatus(XSecure_ClientInstance *InstancePtr, XSecure_CryptoStatusOp CryptoStatusOp,
		u32 CryptoMask, u32 ApiId)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((CryptoStatusOp != XSECURE_CRYPTO_STATUS_SET) && (CryptoStatusOp != XSECURE_CRYPTO_STATUS_CLEAR)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, ApiId);
	Payload[1U] = CryptoStatusOp;
	Payload[2U] = CryptoMask;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to get RSA public key for key wrap.
 *
 * @param	InstancePtr - Pointer to the client instance
 * @param  	PubKey - Pointer to the XSecure_RsaPubKeyAddr instance
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode - On failure
 *
 ******************************************************************************/
int XSecure_GetRsaPublicKeyForKeyWrap(XSecure_ClientInstance *InstancePtr, XSecure_RsaPubKeyAddr *PubKey)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];
	u64 PubKeyAddr;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (PubKey == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	PubKeyAddr = (u64)(UINTPTR)PubKey;
	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_GET_KEY_WRAP_RSA_PUBLIC_KEY);
	Payload[1U] = (u32)(PubKeyAddr >> 32U);
	Payload[2U] = (u32)PubKeyAddr;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to unwrap the wrapped AES key.
 *
 * @param	InstancePtr - Pointer to the client instance
 * @param  	KeyWrapData - Pointer to the XSecure_KeyWrapData instance
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	Errorcode - On failure
 *
 ******************************************************************************/
int XSecure_KeyUnwrap(XSecure_ClientInstance *InstancePtr, XSecure_KeyWrapData *KeyWrapData)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_3U];
	u64 KeyWrapAddr;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (KeyWrapData == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	KeyWrapAddr = (u64)(UINTPTR)KeyWrapData;
	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KEY_UNWRAP);
	Payload[1U] = (u32)(KeyWrapAddr >> 32U);
	Payload[2U] = (u32)KeyWrapAddr;

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
