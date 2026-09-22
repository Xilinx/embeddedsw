/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision_common.c
 *
 * This file contains the Provisioning code which is common across the client and server.
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
 * @addtogroup xasu_provision_common_apis Provisioning Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_provision_common.h"
#include "xasu_keymanager_common.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_ocpinfo.h"

/********************************* Constant Definitions ******************************************/
#define XASU_PROVISION_LE_BYTE_0_OFFSET		(0U) /**< Least-significant byte offset. */
#define XASU_PROVISION_LE_BYTE_1_OFFSET		(1U) /**< Second byte offset. */
#define XASU_PROVISION_LE_BYTE_2_OFFSET		(2U) /**< Third byte offset. */
#define XASU_PROVISION_LE_BYTE_3_OFFSET		(3U) /**< Most-significant u32 byte offset. */
#define XASU_PROVISION_BYTE_SHIFT		(8U) /**< Shift for byte 1 of a little-endian word. */
#define XASU_PROVISION_TWO_BYTE_SHIFT		(16U) /**< Shift for byte 2 of a little-endian word. */
#define XASU_PROVISION_THREE_BYTE_SHIFT		(24U) /**< Shift for byte 3 of a little-endian word. */

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the USER eFUSE key provisioning parameters.
 *
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure which
 *				holds the USER eFUSE key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are validated successfully.
 *	- XST_FAILURE, if the parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUserEfuseParams(const XAsu_ProvisionUserEfuseParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate the target eFUSE key identifier. */
	if ((ParamsPtr->EfuseKeyId != XASU_PROVISION_EFUSE_USER_KEY_0) &&
	    (ParamsPtr->EfuseKeyId != XASU_PROVISION_EFUSE_USER_KEY_1)) {
		goto END;
	}

	/** Validate the key length (only 256-bit keys are supported by USER eFUSE keys). */
	if (ParamsPtr->KeyLen != XASU_PROVISION_USER_EFUSE_KEY_SIZE_IN_BYTES) {
		goto END;
	}

	/**
	 * Validate the key source, its corresponding KeyId/KeyAddr fields, and the key
	 * source against the destination eFUSE.
	 */
	switch (ParamsPtr->KeySource) {
		case XASU_PROVISION_KEY_SOURCE_ASU_GENERATED:
			/** ASU generates the key; no KeyId/KeyAddr must be supplied. */
			if ((ParamsPtr->KeyId != 0U) || (ParamsPtr->KeyAddr != 0U)) {
				goto END;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_VAULT:
			/** Vault source: rejected for eFUSE0; KeyId non-zero, KeyAddr zero. */
			if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) ||
			    (ParamsPtr->KeyId == 0U) || (ParamsPtr->KeyAddr != 0U)) {
				goto END;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_USER_BUFFER:
			/** Buffer source: rejected for eFUSE0; KeyAddr non-zero, KeyId zero. */
			if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) ||
			    (ParamsPtr->KeyAddr == 0U) || (ParamsPtr->KeyId != 0U)) {
				goto END;
			}
			break;
		default:
			goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the Unique Device Endorsement (UDE) key provisioning
 *		parameters.
 *
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUdeParams structure which holds the UDE
 *				key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are validated successfully.
 *	- XST_FAILURE, if the parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUdeParams(const XAsu_ProvisionUdeParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate the certificate output buffer address and its actual-size output address. */
	if ((ParamsPtr->CertBufAddr == 0U) || (ParamsPtr->CertActualSizeAddr == 0U)) {
		goto END;
	}

	/** Validate the certificate output buffer size. */
	if (ParamsPtr->CertBufSize == 0U) {
		goto END;
	}

	/**
	 * TODO: Accept SLH-DSA after the UDE API, OCP key generation/encryption and DPC format
	 * support an SLH-DSA key type.
	 */
	/** Validate the UDE key type (only ECC NIST P-384 is supported currently). */
	if (ParamsPtr->UdeKeyType != XASU_PROVISION_UDE_KEY_TYPE_ECC_P384) {
		goto END;
	}

	/** Validate the target UDE eFUSE slot. */
	if ((ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_0_ID) &&
	    (ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_1_ID) &&
	    (ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_2_ID)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the Get Device Provisioning Certificate (DPC) parameters.
 *
 * @param	ParamsPtr	Pointer to the XAsu_DevProvisionCertParams structure which holds the
 *				DPC generation input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are validated successfully.
 *	- XST_FAILURE, if the parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateDpcParams(const XAsu_DevProvisionCertParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate the DPC output buffer address and its actual-size output address. */
	if ((ParamsPtr->DpcBufAddr == 0U) || (ParamsPtr->DpcActualSizeAddr == 0U)) {
		goto END;
	}

	/** Validate the DPC output buffer size. */
	if (ParamsPtr->DpcBufSize == 0U) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the AEPI image size required for a provisioning-instruction
 *		stream.
 *
 * @param	RecordBlobLen	Size of the plaintext instruction-record stream in bytes.
 * @param	ImageSizePtr	Pointer that receives the required image size in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the image size is returned successfully.
 *	- XST_FAILURE, if RecordBlobLen is outside the supported range or ImageSizePtr is NULL.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionGetImageSize(u32 RecordBlobLen, u32 *ImageSizePtr)
{
	volatile s32 Status = XST_FAILURE;
	u32 NumChunks = 0U;

	if ((ImageSizePtr == NULL) || (RecordBlobLen == 0U) ||
	    (RecordBlobLen > XASU_PROVISION_IMAGE_MAX_RECORD_BLOB_LEN)) {
		Status = XST_FAILURE;
		goto END;
	}

	NumChunks = RecordBlobLen / XASU_PROVISION_IMAGE_CHUNK_DATA_SIZE;
	if ((RecordBlobLen % XASU_PROVISION_IMAGE_CHUNK_DATA_SIZE) != 0U) {
		NumChunks++;
	}
	*ImageSizePtr = XASU_PROVISION_IMAGE_HEADER_SIZE +
			(NumChunks * XASU_PROVISION_IMAGE_CHUNK_SIZE);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the atomic AEPI wrap-command parameters.
 *
 * @param	ParamsPtr	Pointer to the wrap-command parameters.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are valid and the output buffer is large enough.
 *	- XST_FAILURE, if a mandatory address is zero, the record length is unsupported or the
 *	  output buffer is too small.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateWrapParams(const XAsu_WrapProvisionInstructionsParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;
	u32 ImageSize = 0U;

	if ((ParamsPtr == NULL) || (ParamsPtr->RecordBlobAddr == 0U) ||
	    (ParamsPtr->ImageAddr == 0U)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_ProvisionGetImageSize(ParamsPtr->RecordBlobLen, &ImageSize);
	if ((Status != XST_SUCCESS) || (ParamsPtr->ImageBufSize < ImageSize)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate the atomic AEPI unwrap-command parameters.
 *
 * @param	ParamsPtr	Pointer to the unwrap-command parameters.
 *
 * @return
 *	- XST_SUCCESS, if the parameters identify a complete AEPI image buffer.
 *	- XST_FAILURE, if the pointer or image address is NULL, the image is shorter than its
 *	  fixed header, its address range overflows, or a reserved field is non-zero.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUnwrapParams(
	const XAsu_UnwrapProvisionInstructionsParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;

	if ((ParamsPtr == NULL) || (ParamsPtr->ImageAddr == 0U) ||
	    (ParamsPtr->TotalLen < XASU_PROVISION_IMAGE_HEADER_SIZE) ||
	    ((u64)ParamsPtr->TotalLen > (~(u64)0U - ParamsPtr->ImageAddr)) ||
	    (ParamsPtr->Reserved != 0U)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate CREATE_KEY_VAULT parameters used by an AEPI record.
 *
 * @param	ParamsPtr	Pointer to the key-vault creation parameters.
 *
 * @return
 *	- XST_SUCCESS, if the request explicitly selects a valid user vault and its key-manager
 *	  creation parameters are valid.
 *	- XST_FAILURE, if any parameter is invalid or requests output-ID auto-assignment.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateCreateKeyVaultParams(
	const XAsu_KeyManagerSubVaultParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;

	if ((ParamsPtr == NULL) || (ParamsPtr->VaultIdAddr != 0U) ||
	    (XAsu_ProvisionValidateUserVaultId((u32)ParamsPtr->VaultId) != XST_SUCCESS)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_KmValidateVaultCreateParams(ParamsPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate STORE_KEY metadata used by an AEPI record and resolve its wire-data length.
 *
 * @param	MetadataPtr	Pointer to the key metadata.
 * @param	DataLenPtr	Pointer that receives the serialized key-object length.
 *
 * @return
 *	- XST_SUCCESS, if the metadata is valid for an unwrapped key in an explicit user slot.
 *	- XST_FAILURE, if a pointer, key ID, key type, metadata field, attribute, use case or
 *	  serialized key-object length is invalid.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateStoreKeyMetadata(
	const XAsu_KeyManagerKeyMetadata *MetadataPtr, u32 *DataLenPtr)
{
	s32 Status = XST_FAILURE;
	u32 DataLen = 0U;
	u8 KeyType = 0U;

	if ((MetadataPtr == NULL) || (DataLenPtr == NULL) ||
	    (XAsu_ProvisionValidateUserKeyId(MetadataPtr->CompositeKeyId) != XST_SUCCESS)) {
		Status = XST_FAILURE;
		goto END;
	}

	KeyType = XASU_KM_GET_KEY_TYPE(MetadataPtr->CompositeKeyId);
	/*
	 * TODO: Accept and size SLH-DSA private/public objects after KeyManager defines their
	 * sub-vault identifiers, stored-object formats and key-length validation.
	 */
	if (((KeyType & XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) != 0U) ||
	    (KeyType >= (u8)XASU_INVALID_SUBVAULT_ID)) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Validate the metadata fields for the selected key type. */
	Status = XAsu_KmValidateKeyMetadata(MetadataPtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_KmValidateKeyAttributes(KeyType, MetadataPtr->KeyAttributes);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_KmValidateAesKeySplitUseCase(KeyType, MetadataPtr->KeyUseCase);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_KmValidateKeyLength((u32)MetadataPtr->Length, KeyType);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_ProvisionGetStoreKeyDataLen(MetadataPtr->CompositeKeyId,
			MetadataPtr->Length, &DataLen);
	if ((Status != XST_SUCCESS) ||
	    (DataLen > XASU_PROVISION_MAX_STORE_KEY_DATA_SIZE)) {
		Status = XST_FAILURE;
		goto END;
	}

	*DataLenPtr = DataLen;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate XOR-key parameters used by an AEPI record.
 *
 * @param	ParamsPtr	Pointer to the XOR-key parameters.
 *
 * @return
 *	- XST_SUCCESS, if all three key IDs select explicit user slots and the key-manager XOR
 *	  parameters are valid.
 *	- XST_FAILURE, if any parameter is invalid or requests output-ID auto-assignment.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateXorParams(const XAsu_KmXorKeyParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;

	if ((ParamsPtr == NULL) || (ParamsPtr->OutKeyIdAddr != 0U) ||
	    (XAsu_ProvisionValidateUserKeyId(ParamsPtr->SplitKeyId1) != XST_SUCCESS) ||
	    (XAsu_ProvisionValidateUserKeyId(ParamsPtr->SplitKeyId2) != XST_SUCCESS) ||
	    (XASU_KM_GET_KEY_TYPE(ParamsPtr->SplitKeyId1) != (u8)XASU_AES_SUBVAULT_ID) ||
	    (XASU_KM_GET_KEY_TYPE(ParamsPtr->SplitKeyId2) != (u8)XASU_AES_SUBVAULT_ID) ||
	    (XAsu_ProvisionValidateUserKeyId(
		    ParamsPtr->OutKeyMetadata.CompositeKeyId) != XST_SUCCESS)) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XAsu_KmValidateXorKeyParams(ParamsPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Read a 16-bit little-endian value from an unaligned byte buffer.
 *
 * @param	Ptr	Pointer to at least two input bytes.
 *
 * @return	The decoded 16-bit value.
 *
 *************************************************************************************************/
u16 XAsu_ProvisionReadU16Le(const u8 *Ptr)
{
	return (u16)((u16)Ptr[XASU_PROVISION_LE_BYTE_0_OFFSET] |
		((u16)Ptr[XASU_PROVISION_LE_BYTE_1_OFFSET] << XASU_PROVISION_BYTE_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief	Read a 32-bit little-endian value from an unaligned byte buffer.
 *
 * @param	Ptr	Pointer to at least four input bytes.
 *
 * @return	The decoded 32-bit value.
 *
 *************************************************************************************************/
u32 XAsu_ProvisionReadU32Le(const u8 *Ptr)
{
	return ((u32)Ptr[XASU_PROVISION_LE_BYTE_0_OFFSET] |
		((u32)Ptr[XASU_PROVISION_LE_BYTE_1_OFFSET] << XASU_PROVISION_BYTE_SHIFT) |
		((u32)Ptr[XASU_PROVISION_LE_BYTE_2_OFFSET] << XASU_PROVISION_TWO_BYTE_SHIFT) |
		((u32)Ptr[XASU_PROVISION_LE_BYTE_3_OFFSET] << XASU_PROVISION_THREE_BYTE_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief	Write a 16-bit value to an unaligned byte buffer in little-endian order.
 *
 * @param	Ptr	Pointer to at least two output bytes.
 * @param	Value	Value to encode.
 *
 *************************************************************************************************/
void XAsu_ProvisionWriteU16Le(u8 *Ptr, u16 Value)
{
	Ptr[XASU_PROVISION_LE_BYTE_0_OFFSET] = (u8)Value;
	Ptr[XASU_PROVISION_LE_BYTE_1_OFFSET] = (u8)(Value >> XASU_PROVISION_BYTE_SHIFT);
}

/*************************************************************************************************/
/**
 * @brief	Write a 32-bit value to an unaligned byte buffer in little-endian order.
 *
 * @param	Ptr	Pointer to at least four output bytes.
 * @param	Value	Value to encode.
 *
 *************************************************************************************************/
void XAsu_ProvisionWriteU32Le(u8 *Ptr, u32 Value)
{
	Ptr[XASU_PROVISION_LE_BYTE_0_OFFSET] = (u8)Value;
	Ptr[XASU_PROVISION_LE_BYTE_1_OFFSET] = (u8)(Value >> XASU_PROVISION_BYTE_SHIFT);
	Ptr[XASU_PROVISION_LE_BYTE_2_OFFSET] = (u8)(Value >> XASU_PROVISION_TWO_BYTE_SHIFT);
	Ptr[XASU_PROVISION_LE_BYTE_3_OFFSET] = (u8)(Value >> XASU_PROVISION_THREE_BYTE_SHIFT);
}

/*************************************************************************************************/
/**
 * @brief	Validate that a vault ID explicitly selects a user-owned vault.
 *
 * @param	VaultId	Vault ID to validate.
 *
 * @return
 *	- XST_SUCCESS, if VaultId selects a user-owned vault.
 *	- XST_FAILURE, if VaultId requests auto-assignment, an ASU-owned vault or an
 *	  out-of-range vault.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUserVaultId(u32 VaultId)
{
	s32 Status = XST_FAILURE;

	if ((VaultId >= XASU_KM_USER_VAULT_START_ID) && (VaultId < XASU_KM_MAX_VAULTS)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate that a composite key ID explicitly selects a slot in a user-owned vault.
 *
 * @param	KeyId	Composite key ID to validate.
 *
 * @return
 *	- XST_SUCCESS, if KeyId selects a non-zero slot in a user-owned vault.
 *	- XST_FAILURE, if KeyId requests slot auto-assignment or selects a reserved or
 *	  out-of-range vault.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUserKeyId(u32 KeyId)
{
	s32 Status = XST_FAILURE;

	if ((XAsu_ProvisionValidateUserVaultId((u32)XASU_KM_GET_VAULT_ID(KeyId)) ==
			XST_SUCCESS) && (XASU_KM_GET_KEY_INDEX(KeyId) != 0U)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Return the serialized key-object length for an AEPI STORE_KEY record.
 *
 * @param	CompositeKeyId	Composite key ID containing the destination sub-vault type.
 * @param	KeyLen		Key length stored in the key metadata.
 * @param	DataLenPtr	Pointer that receives the serialized key-object length.
 *
 * @return
 *	- XST_SUCCESS, if the serialized length is returned.
 *	- XST_FAILURE, if KeyLen is zero, DataLenPtr is NULL, or an ECC public key length
 *	  exceeds the largest supported coordinate length.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionGetStoreKeyDataLen(u32 CompositeKeyId, u16 KeyLen, u32 *DataLenPtr)
{
	s32 Status = XST_FAILURE;
	u32 DataLen;
	u8 KeyType;

	if ((DataLenPtr == NULL) || (KeyLen == 0U)) {
		Status = XST_FAILURE;
		goto END;
	}

	KeyType = XASU_KM_GET_KEY_TYPE(CompositeKeyId) &
			(u8)(~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK);
	switch (KeyType) {
	case XASU_RSA_PVT_SUBVAULT_ID:
		DataLen = (u32)sizeof(XAsu_RsaKeyPairObject);
		break;
	case XASU_RSA_PUB_SUBVAULT_ID:
		DataLen = (u32)sizeof(XAsu_RsaPubKeyComp);
		break;
	case XASU_ECC_PUB_SUBVAULT_ID:
		if (KeyLen > XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES) {
			Status = XST_FAILURE;
			goto END;
		}
		DataLen = XAsu_DoubleCurveLength((u32)KeyLen);
		break;
	default:
		DataLen = (u32)KeyLen;
		break;
	}

	*DataLenPtr = DataLen;
	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
