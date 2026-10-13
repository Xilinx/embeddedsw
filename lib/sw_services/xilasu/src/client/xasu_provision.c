/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision.c
 *
 * This file contains the implementation of the client interface functions for the ASU
 * Provisioning interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_provision_client_apis Provisioning Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasu_generic.h"
#include "xasu_provision.h"
#include "xasu_provision_common.h"
#include "xasu_provisioninfo.h"

/************************************ Constant Definitions ***************************************/
#define XASU_PROVISION_XOR_PAYLOAD_SIZE			(24U) /**< XOR payload size in bytes. */
#define XASU_PROVISION_RECORD_MODULE_OFFSET		(0U) /**< Module ID byte offset. */
#define XASU_PROVISION_RECORD_CMD_OFFSET		(1U) /**< Command ID byte offset. */
#define XASU_PROVISION_RECORD_SIZE_OFFSET		(4U) /**< Payload-size field offset. */
#define XASU_PROVISION_RECORD_PAYLOAD_OFFSET		(8U) /**< Payload byte offset. */
#define XASU_PROVISION_STORE_USE_CASE_OFFSET		(12U) /**< STORE_KEY use-case offset. */
#define XASU_PROVISION_STORE_ATTRIBUTES_OFFSET		(13U) /**< STORE_KEY attributes offset. */
#define XASU_PROVISION_STORE_LENGTH_OFFSET		(14U) /**< STORE_KEY length offset. */
#define XASU_PROVISION_STORE_EPOCH_OFFSET		(16U) /**< STORE_KEY epoch offset. */
#define XASU_PROVISION_STORE_USAGE_OFFSET		(20U) /**< STORE_KEY usage-count offset. */
#define XASU_PROVISION_STORE_DATA_OFFSET		(24U) /**< STORE_KEY data offset. */
#define XASU_PROVISION_XOR_SPLIT1_OFFSET		(8U) /**< First split-key ID offset. */
#define XASU_PROVISION_XOR_SPLIT2_OFFSET		(12U) /**< Second split-key ID offset. */
#define XASU_PROVISION_XOR_OUTPUT_OFFSET		(16U) /**< Output-key ID offset. */
#define XASU_PROVISION_XOR_USE_CASE_OFFSET		(20U) /**< Output use-case offset. */
#define XASU_PROVISION_XOR_ATTRIBUTES_OFFSET		(21U) /**< Output attributes offset. */
#define XASU_PROVISION_XOR_LENGTH_OFFSET		(22U) /**< Output key-length offset. */
#define XASU_PROVISION_XOR_EPOCH_OFFSET			(24U) /**< Output epoch offset. */
#define XASU_PROVISION_XOR_USAGE_OFFSET			(28U) /**< Output usage-count offset. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ProvisionEncodeRecordHeader(u8 *BufPtr, u32 BufSize, u8 CmdId,
					    u16 PayloadSize);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Initialize one provisioning instruction record header.
 *
 * @param	BufPtr		Pointer to the output record buffer.
 * @param	BufSize		Capacity of the output record buffer.
 * @param	CmdId		Provisioning instruction command ID.
 * @param	PayloadSize	Record payload size in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the zeroed header is initialized.
 *	- XST_FAILURE, if the buffer is invalid or cannot be cleared.
 *
 *************************************************************************************************/
static s32 XAsu_ProvisionEncodeRecordHeader(u8 *BufPtr, u32 BufSize, u8 CmdId, u16 PayloadSize)
{
	s32 Status = XST_FAILURE;
	u32 RecordSize = XASU_PROVISION_RECORD_HEADER_SIZE + (u32)PayloadSize;

	if ((BufPtr == NULL) || (BufSize < RecordSize)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = Xil_SMemSet(BufPtr, RecordSize, 0U, RecordSize);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	BufPtr[XASU_PROVISION_RECORD_MODULE_OFFSET] = (u8)XASU_MODULE_KEYMANAGER_ID;
	BufPtr[XASU_PROVISION_RECORD_CMD_OFFSET] = CmdId;
	XAsu_ProvisionWriteU16Le(&BufPtr[XASU_PROVISION_RECORD_SIZE_OFFSET], PayloadSize);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Encode one CREATE_KEY_VAULT provisioning instruction record.
 *
 * @param	ParamsPtr	Pointer to the key-vault parameters. VaultId must explicitly select
 *				a user-owned vault and VaultIdAddr must be zero.
 * @param	BufAddr		Address of the output record buffer.
 * @param	BufSize		Capacity of the output record buffer.
 * @param	RecordLenPtr	Pointer that receives the encoded record length.
 *
 * @return
 *	- XST_SUCCESS, if the complete record is encoded.
 *	- XASU_INVALID_ARGUMENT, if an argument or field value is invalid, or an input/output
 *	  range aliases the encoded record.
 *	- XST_FAILURE, if the output buffer cannot be initialized.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionEncodeCreateKeyVault(const XAsu_KeyManagerSubVaultParams *ParamsPtr,
				       u64 BufAddr, u32 BufSize,
				       u32 *RecordLenPtr)
{
	s32 Status = XST_FAILURE;
	u8 *BufPtr = (u8 *)(UINTPTR)BufAddr;
	u32 RecordLen = XASU_PROVISION_RECORD_HEADER_SIZE +
			XASU_PROVISION_CREATE_VAULT_PAYLOAD_SIZE;

	if ((ParamsPtr == NULL) || (BufAddr == 0U) || (RecordLenPtr == NULL) ||
	    (BufSize < RecordLen) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap((u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr), (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_ProvisionValidateCreateKeyVaultParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_ProvisionEncodeRecordHeader(BufPtr, BufSize,
			XASU_KM_CREATE_KEYVAULT_CMD_ID,
			XASU_PROVISION_CREATE_VAULT_PAYLOAD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	BufPtr[XASU_PROVISION_CREATE_VAULT_ID_OFFSET] = ParamsPtr->VaultId;
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_AES)],
				 ParamsPtr->AESKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_IV)],
				 ParamsPtr->IVVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_KDF_HMAC)],
				 ParamsPtr->KDFHmacKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_RSA_PVT)],
				 ParamsPtr->RSAPvtKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_RSA_PUB)],
				 ParamsPtr->RSAPubKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_ECC_PVT)],
				 ParamsPtr->ECCPvtKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_ECC_PUB)],
				 ParamsPtr->ECCPubKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_LMS)],
				 ParamsPtr->LMSKeyVaultCapacity);
	/*
	 * TODO: Encode the reserved SLH-DSA capacity fields when the KeyManager vault parameters
	 * expose SLH-DSA private/public capacities. The zeroed record keeps both fields zero now.
	 */
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_MLDSA_PVT)],
				 ParamsPtr->MldsaPvtKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_MLDSA_PUB)],
				 ParamsPtr->MldsaPubKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_MLKEM_PVT)],
				 ParamsPtr->MlkemPvtKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_MLKEM_PUB)],
				 ParamsPtr->MlkemPubKeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(
				 &BufPtr[XASU_PROVISION_CREATE_CAP_OFFSET(
				 XASU_PROVISION_CREATE_CAP_IDX_X509)],
				 ParamsPtr->X509KeyVaultCapacity);
	XAsu_ProvisionWriteU16Le(&BufPtr[XASU_PROVISION_CREATE_ACCESS_OFFSET],
				 ParamsPtr->AccessRights);
	BufPtr[XASU_PROVISION_CREATE_RESTRICTIONS_OFFSET] = ParamsPtr->Restrictions;
	*RecordLenPtr = RecordLen;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Encode one STORE_KEY provisioning instruction record.
 *
 * @param	ParamsPtr	Pointer to the KeyManager store-key parameters. KeyObjectAddr and
 *				KeyMetadata are encoded. KeyObjectLen must cover the encoded key
 *				object. KeyIdAddr and WrappedInputLen must be zero.
 * @param	BufAddr		Address of the output record buffer.
 * @param	BufSize		Capacity of the output record buffer.
 * @param	RecordLenPtr	Pointer that receives the encoded record length.
 *
 * @return
 *	- XST_SUCCESS, if the record is encoded.
 *	- XASU_INVALID_ARGUMENT, if an argument, length or metadata field is invalid, or an
 *	  input/output range aliases the encoded record.
 *	- XST_FAILURE, if the record header or key bytes cannot be copied.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionEncodeStoreKey(const XAsu_KeyManagerParams *ParamsPtr,
				 u64 BufAddr, u32 BufSize, u32 *RecordLenPtr)
{
	s32 Status = XST_FAILURE;
	u8 *BufPtr = (u8 *)(UINTPTR)BufAddr;
	u32 DataLen = 0U;
	u32 PayloadLen = 0U;
	u32 RecordLen = 0U;

	if ((ParamsPtr == NULL) || (BufAddr == 0U) || (RecordLenPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_ProvisionValidateStoreKeyMetadata(&ParamsPtr->KeyMetadata, &DataLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** The encode copies DataLen bytes, so the caller buffer must cover that object. */
	if (ParamsPtr->KeyObjectLen < DataLen) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	PayloadLen = XASU_PROVISION_STORE_KEY_METADATA_SIZE + DataLen;
	RecordLen = XASU_PROVISION_RECORD_HEADER_SIZE + PayloadLen;
	if ((ParamsPtr->KeyObjectAddr == 0U) || (ParamsPtr->KeyIdAddr != 0U) ||
	    (ParamsPtr->WrappedInputLen != 0U) ||
	    (BufSize < RecordLen) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, ParamsPtr->KeyObjectAddr,
		    DataLen) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap((u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr), (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap((u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr), ParamsPtr->KeyObjectAddr,
		    DataLen) == (u8)XASU_TRUE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_ProvisionEncodeRecordHeader(BufPtr, BufSize,
			XASU_KM_STORE_KEY_CMD_ID, (u16)PayloadLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_RECORD_PAYLOAD_OFFSET],
				 ParamsPtr->KeyMetadata.CompositeKeyId);
	BufPtr[XASU_PROVISION_STORE_USE_CASE_OFFSET] = ParamsPtr->KeyMetadata.KeyUseCase;
	BufPtr[XASU_PROVISION_STORE_ATTRIBUTES_OFFSET] =
		ParamsPtr->KeyMetadata.KeyAttributes;
	XAsu_ProvisionWriteU16Le(&BufPtr[XASU_PROVISION_STORE_LENGTH_OFFSET],
				 ParamsPtr->KeyMetadata.Length);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_STORE_EPOCH_OFFSET],
				 ParamsPtr->KeyMetadata.EpochTime);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_STORE_USAGE_OFFSET],
				 ParamsPtr->KeyMetadata.UsageCount);

	Status = Xil_SMemCpy(&BufPtr[XASU_PROVISION_STORE_DATA_OFFSET], DataLen,
			     (const void *)(UINTPTR)ParamsPtr->KeyObjectAddr, DataLen, DataLen);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	*RecordLenPtr = RecordLen;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Encode one XOR provisioning instruction record.
 *
 * @param	BufAddr		Address of the output record buffer.
 * @param	BufSize		Capacity of the output record buffer.
 * @param	ParamsPtr	Pointer to the input key IDs and output-key metadata.
 *				OutKeyIdAddr must be zero because the output slot is explicit.
 * @param	RecordLenPtr	Pointer that receives the encoded record length.
 *
 * @return
 *	- XST_SUCCESS, if the 32-byte record is encoded.
 *	- XASU_INVALID_ARGUMENT, if an argument, AES output-key length or output buffer is invalid,
 *	  or an input/output range aliases the encoded record.
 *	- XST_FAILURE, if the record header cannot be initialized.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionEncodeXorKeys(u64 BufAddr, u32 BufSize,
				const XAsu_KmXorKeyParams *ParamsPtr,
				u32 *RecordLenPtr)
{
	s32 Status = XST_FAILURE;
	u8 *BufPtr = (u8 *)(UINTPTR)BufAddr;
	u32 RecordLen = XASU_PROVISION_RECORD_HEADER_SIZE + XASU_PROVISION_XOR_PAYLOAD_SIZE;

	if ((BufAddr == 0U) || (ParamsPtr == NULL) || (RecordLenPtr == NULL) ||
	    (BufSize < RecordLen) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap(BufAddr, RecordLen, (u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr)) == (u8)XASU_TRUE) ||
	    (XAsu_IsRangeOverlap((u64)(UINTPTR)RecordLenPtr,
		    (u32)sizeof(*RecordLenPtr), (u64)(UINTPTR)ParamsPtr,
		    (u32)sizeof(*ParamsPtr)) == (u8)XASU_TRUE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_ProvisionValidateXorParams(ParamsPtr) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_ProvisionEncodeRecordHeader(BufPtr, BufSize,
			XASU_KM_XOR_KEY_CMD_ID, (u16)XASU_PROVISION_XOR_PAYLOAD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_XOR_SPLIT1_OFFSET],
				 ParamsPtr->SplitKeyId1);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_XOR_SPLIT2_OFFSET],
				 ParamsPtr->SplitKeyId2);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_XOR_OUTPUT_OFFSET],
				 ParamsPtr->OutKeyMetadata.CompositeKeyId);
	BufPtr[XASU_PROVISION_XOR_USE_CASE_OFFSET] = ParamsPtr->OutKeyMetadata.KeyUseCase;
	BufPtr[XASU_PROVISION_XOR_ATTRIBUTES_OFFSET] = ParamsPtr->OutKeyMetadata.KeyAttributes;
	XAsu_ProvisionWriteU16Le(&BufPtr[XASU_PROVISION_XOR_LENGTH_OFFSET],
				 ParamsPtr->OutKeyMetadata.Length);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_XOR_EPOCH_OFFSET],
				 ParamsPtr->OutKeyMetadata.EpochTime);
	XAsu_ProvisionWriteU32Le(&BufPtr[XASU_PROVISION_XOR_USAGE_OFFSET],
				 ParamsPtr->OutKeyMetadata.UsageCount);
	*RecordLenPtr = RecordLen;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to program a USER eFUSE key.
 *
 *		The key to be programmed can be sourced from the AKV, generated internally by the
 *		ASU using the TRNG (benign provisioning), or supplied directly by the user. It is
 *		always stored in the eFUSE in black (PUF KEK encrypted) form.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure which holds
 *				the USER eFUSE key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionUserEfuse(XAsu_ClientParams *ClientParamPtr,
			    XAsu_ProvisionUserEfuseParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate the provisioning input parameter pointer. */
	if (ParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the USER eFUSE provisioning input parameters. */
	Status = XAsu_ProvisionValidateUserEfuseParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_USER_EFUSE_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_ProvisionUserEfuseParams) /
					XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				  (u32)(sizeof(XAsu_ProvisionUserEfuseParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to provision a Unique Device Endorsement
 *		(UDE) key pair.
 *
 *		The UDE key pair (ECC NIST P-384) is generated inside the ASU so the private key is
 *		never exposed outside the ASU. The private key is encrypted with the PUF KEK and
 *		programmed into the requested eFUSE slot by the ASU (via the PLM), and a self-signed
 *		X.509 certificate over the generated public key is returned to the caller at
 *		ParamsPtr->CertBufAddr with its actual size at ParamsPtr->CertActualSizeAddr.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUdeParams structure which holds the UDE
 *				key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionUde(XAsu_ClientParams *ClientParamPtr, XAsu_ProvisionUdeParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate the provisioning input parameter pointer. */
	if (ParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the UDE provisioning input parameters. */
	Status = XAsu_ProvisionValidateUdeParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function, providing the caller's
	 * CertActualSizeAddr as the response buffer so the certificate size returned by the server
	 * is copied there during the callback.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ParamsPtr->CertActualSizeAddr,
						XASU_PROVISION_UDE_CERT_ACTUAL_SIZE_IN_BYTES,
						XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_UDE_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_ProvisionUdeParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				   (u32)(sizeof(XAsu_ProvisionUdeParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends one atomic command to ASUFW to build an authenticated encrypted
 *		provisioning image from a plaintext instruction-record stream.
 *
 * @param	ClientParamPtr	Pointer to the client parameters.
 * @param	ParamsPtr	Pointer to the record-stream and output-buffer parameters.
 *
 * @return
 *	- XST_SUCCESS, if the request is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if an argument is invalid or the output buffer is too small.
 *	- XASU_INVALID_UNIQUE_ID, if the generated unique ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_WrapProvisionInstructions(XAsu_ClientParams *ClientParamPtr,
				   XAsu_WrapProvisionInstructionsParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate client and wrap-command parameters before allocating a queue entry. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ProvisionValidateWrapParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the status-only callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create the command header and submit the single atomic wrap request. */
	Header = XAsu_CreateHeader(XASU_PROVISION_WRAP_INSTR_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_WrapProvisionInstructionsParams) /
					XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				  (u32)sizeof(XAsu_WrapProvisionInstructionsParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to process a wrapped provisioning image
 *		header. The ASU validates the header, resolves the subsystem's authentication public
 *		key internally, verifies the signature, ECIES-decrypts and CMAC-KDF-derives the chunk
 *		key, then executes the recovered provisioning instructions.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_UnwrapProvisionInstructionsParams structure holding
 *				the provisioning image base and length.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the generated unique ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_UnwrapProvisionInstructions(XAsu_ClientParams *ClientParamPtr,
				     XAsu_UnwrapProvisionInstructionsParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the complete unwrap command before allocating a queue entry. */
	Status = XAsu_ProvisionValidateUnwrapParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. Only a status is returned. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_UNWRAP_INSTR_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_UnwrapProvisionInstructionsParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				  (u32)(sizeof(XAsu_UnwrapProvisionInstructionsParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate and return the Device Provisioning
 *		Certificate (DPC) for the calling subsystem (SAS section 12.16.2).
 *
 *		The ASU checks the internal exportable key vault for the subsystem's provisioning
 *		authentication key pair (ECC NIST P-384) and provisioning wrapping key pair (ECIES
 *		over NIST P-384). Any pair that is missing is generated with the TRNG and stored in
 *		the exportable vault. A UDE-signed X.509 certificate is generated over each public
 *		key, the DPC container (Device DNA, both certificates and a UDE signature over them)
 *		is assembled and returned to the caller at ParamsPtr->DpcBufAddr with its actual size
 *		at ParamsPtr->DpcActualSizeAddr.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_DevProvisionCertParams structure which holds the
 *				DPC generation input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_GetDevProvisionCert(XAsu_ClientParams *ClientParamPtr,
			     XAsu_DevProvisionCertParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate the provisioning input parameter pointer. */
	if (ParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the DPC generation input parameters. */
	Status = XAsu_ProvisionValidateDpcParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function, providing the caller's
	 * DpcActualSizeAddr as the response buffer so the DPC size returned by the server is
	 * copied there during the callback.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ParamsPtr->DpcActualSizeAddr,
						XASU_PROVISION_DPC_ACTUAL_SIZE_IN_BYTES,
						XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_GET_DPC_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_DevProvisionCertParams) /
					XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				   (u32)(sizeof(XAsu_DevProvisionCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to delete the calling subsystem's
 *		provisioning key pairs backing the Device Provisioning Certificate (SAS section
 *		12.16.3).
 *
 *		The provisioning authentication and wrapping key pairs are removed from the internal
 *		exportable key vault so that the next XAsu_GetDevProvisionCert call regenerates them.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_DeleteDevProvisionCert(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_DELETE_DPC_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID, 0U,
				   ClientParamPtr->SecureFlag);

	/** Send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
