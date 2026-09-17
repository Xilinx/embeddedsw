/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xprovision.c
*
* This file implements provisioning of the USER eFUSE keys. The key comes from one of three sources
* selected via KeySource: ASU-generated (TRNG), AKV vault (KeyId), or user buffer (KeyAddr).
* USER_KEY_0 allows only the ASU-generated source; USER_KEY_1 allows all three. The key is always
* PUF KEK encrypted (black form) in the ASU, then sent to the PLM (XilNVM) over IPI for the eFUSE
* write.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kp   06/27/26 Initial release
*       vm   09/15/26 Dropped the UDE public key cache after a UDE slot write
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xprovision_server_apis Provisioning Server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xprovision.h"
#include "xasu_provision_common.h"
#include "xasufw_hw.h"
#include "xasufw_memory.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_debug.h"
#include "xasufw_init.h"
#include "xasufw_keytransfer.h"
#include "xasufw_ipi.h"
#include "xaes.h"
#include "xtrng.h"
#include "xrsa_ecc.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xkeymanager.h"
#include "xasu_sharedmem.h"
#include "xasu_aesinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_shainfo.h"
#include "xasu_def.h"
#include "xasu_generic.h"
#include "xocp_ude.h"
#include "xfih.h"
#include "xil_sutil.h"

#ifdef XASU_PROVISION_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XPROVISION_EFUSE_KEY_SIZE_IN_BYTES	(32U)	/**< USER eFUSE key size in bytes
							(256-bit) */
#define XPROVISION_EFUSE_IV_SIZE_IN_BYTES	(12U)	/**< IV size in bytes for the black key
							AES-GCM operation (96-bit) */
#define XPROVISION_EFUSE_TAG_SIZE_IN_BYTES	(16U)	/**< GCM tag size in bytes */

#define XPROVISION_UDE_PVT_KEY_LEN_IN_WORDS	(XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES >> 2U)
							/**< UDE private key length in words */

/*
 * UDE key type codes recorded in the key vault header (two bits per UDE slot; the KeyManager packs
 * the code into the slot's position).
 */
#define XPROVISION_UDE_KEY_TYPE_CODE_P384	(0x0U)	/**< 2-bit UDE key type code: ECC P-384 */
#define XPROVISION_UDE_KEY_TYPE_CODE_MLDSA	(0x1U)	/**< 2-bit UDE key type code: ML-DSA */
#define XPROVISION_UDE_KEY_TYPE_CODE_UNPROGRAMMED	(0x3U)	/**< 2-bit UDE key type code:
								slot unprogrammed (matches the
								all-slots-unprogrammed default) */

#define XPROVISION_UDE_CLEAR_CHUNK_SIZE_IN_BYTES	(64U)	/**< Chunk size used to clear the
								caller certificate buffer during
								provisioning rollback */

/*
 * Device Provisioning Certificate (DPC) container layout (SAS Table 18): Device DNA, followed by
 * the length-prefixed UDE-signed authentication key certificate, the length-prefixed UDE-signed
 * wrapping key certificate, and a UDE signature over all of the preceding bytes.
 */
#define XPROVISION_DPC_DNA_SIZE_IN_BYTES	(EFUSE_CACHE_DNA_SIZE)	/**< Device DNA size in
									bytes. */
#define XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES	(4U)	/**< Size of each certificate length prefix
								(one u32) in bytes. */
#define XPROVISION_DPC_SIG_SIZE_IN_BYTES	(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES * 2U)
							/**< UDE signature size in bytes (R || S over
							NIST P-384). */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf);
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey);
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData);
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId);
static s32 XProvision_IsUdeSlotFree(u8 UdeSlot);
static s32 XProvision_WriteUdeKeyToEfuse(u32 UdeSlot, const u32 *KeyData);
static s32 XProvision_GetKeySlot(u32 SubSystemId, u16 *SlotPtr, u8 KeyUseCase);
static s32 XProvision_GetKeyId(u16 KeySlot, u32 *KeyIdPtr, u8 KeyUseCase);
static s32 XProvision_GetPubKey(u32 SubSystemId, u8 KeyUseCase, u8 *PubKeyBuf);

/*************************************************************************************************/
/**
 * @brief	This function programs a USER eFUSE key (USER_KEY_0 / USER_KEY_1) with a key
 *		from the source selected in ParamsPtr->KeySource (ASU-generated via TRNG, AKV
 *		vault, or user buffer). USER_KEY_0 is restricted to the ASU-generated source.
 *		The key is always stored in black form (PUF KEK encrypted) for both user keys.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance (used for red key encryption).
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is provisioned successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_KEY_FETCH_FAIL, if obtaining the key fails.
 *	- Error code from XAsufw_PmcKeyTransfer, if the PMC key transfer fails.
 *	- XASUFW_PUF_KEK_GEN_FAIL, if PUF KEK generation fails.
 *	- XASUFW_PROVISION_RED_ENCRYPT_FAIL, if red key encryption fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the target key is already programmed, or the PLM
 *		reports a failure for the blank-key check or the eFUSE write.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- Error code from XAes_KeyClear or Xil_SecureZeroize, if clearing the key vault or zeroizing
 *		the local key buffers fails, even when provisioning itself succeeded.
 *
 * @note	Each eFUSE key is checked to be unprogrammed before any key material is produced, so
 *		a given key is encrypted under its RTCA base IV at most once per device.
 *
 *************************************************************************************************/
s32 XProvision_UserEfuse(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			 const XAsu_ProvisionUserEfuseParams *ParamsPtr, u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 KeyBuf[XPROVISION_EFUSE_KEY_SIZE_IN_BYTES] = {0U};
	/* Black key passed to the PLM by address; word-aligned u32 array by type. */
	u32 BlackKeyBuf[XASUFW_NVM_EFUSE_AES_KEY_LEN_IN_WORDS] = {0U};
	u32 PufKekGenStatus = XASUFW_FAILURE;

	/** Validate the server-side resource pointers. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate all command parameters up front: EfuseKeyId, KeyLen, the per-source
	 * KeyId/KeyAddr rules, and the key source against the destination eFUSE.
	 */
	Status = XAsu_ProvisionValidateUserEfuseParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** User key 0 accepts benign key only from ASU-generated source. */
	if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) &&
	    (ParamsPtr->KeySource != XASU_PROVISION_KEY_SOURCE_ASU_GENERATED)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Reject an already programmed key before any key material is produced, so a doomed
	 * request cannot consume a vault key usage count or re-encrypt under the same KEK and IV.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_CheckEfuseKeyBlank((u32)ParamsPtr->EfuseKeyId);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Obtain the plaintext key from the requested source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_AcquireKey(DmaPtr, ParamsPtr, SubsystemId, KeyBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/** Reject an all-zero key before it is burned, for every key source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_KEY_FETCH_FAIL;
		goto END_CLR;
	}

	/**
	 * Re-transfer the eFUSE keys and PUF KEK from the PMC for this encryption and validate the
	 * PUF KEK generation status.
	 */
	XFIH_CALL_GOTO(XAsufw_PmcKeyTransfer, XFihVar, Status, END_CLR, &PufKekGenStatus);

	if (PufKekGenStatus != (u32)XASUFW_SUCCESS) {
		Status = XASUFW_PUF_KEK_GEN_FAIL;
		goto END_CLR;
	}

	/** Both USER eFUSE keys are programmed in black form: PUF KEK encrypt inside the ASU. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_EncryptRedKey(DmaPtr, AesInstancePtr, ParamsPtr->EfuseKeyId, KeyBuf,
					 (u8 *)BlackKeyBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_RED_ENCRYPT_FAIL);
		goto END_CLR;
	}

	/** Request the PLM (XilNVM) to program the selected USER eFUSE key with the black key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_WriteUserKeyToEfuse((u32)ParamsPtr->EfuseKeyId,
						(const u32 *)BlackKeyBuf);

END_CLR:
	/**
	 * Clear eFUSE key 0, eFUSE key 1, eFUSE red key 0, eFUSE red key 1 and the PUF key
	 * in a single call, covering every path after XAsufw_PmcKeyTransfer.
	 */
	XFIH_CALL(XAes_KeyClear, XFihVar, ClearStatus, AesInstancePtr,
		  XAES_KEY_CLEAR_MULTI_KEY_FLAG |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_PUF_KEY));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Zeroize the local key buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, KeyBuf,
		  XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize((u8 *)BlackKeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function obtains the 256-bit plaintext key to be programmed, based on the
 *		explicitly selected key source (ASU-generated via TRNG, AKV vault, or user buffer).
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 * @param	KeyBuf		Pointer to the buffer that receives the 256-bit key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key is obtained successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the key source is invalid or the vault key is not
 *		256-bit.
 *	- Error code from XKeyManager_UpdateKeyObjFromVault, if the AKV key fetch fails, including
 *		XASUFW_KEYMANAGER_KEY_TYPE_MISMATCH when the KeyId is not an AES sub-vault key.
 *	- XASUFW_MEM_COPY_FAIL, if copying the AKV key (vault path) fails.
 *	- XASUFW_DMA_COPY_FAIL, if the DMA transfer of the user key (buffer path) fails.
 *	- Error code from XTrng_GetRandomNumbers, if TRNG key generation (ASU-generated path) fails.
 *
 *************************************************************************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_AesKeyObject KeyObject = {0U};

	/** Obtain the key based on the explicitly selected key source. */
	switch (ParamsPtr->KeySource) {
		case XASU_PROVISION_KEY_SOURCE_ASU_GENERATED:
			/** ASU-generated path: generate a 256-bit AES key from the TRNG. */
			Status = XTrng_GetRandomNumbers(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			break;
		case XASU_PROVISION_KEY_SOURCE_VAULT:
			/** Vault path: fetch the AES key from the AKV by KeyId. */
			KeyObject.KeyId = ParamsPtr->KeyId;
			Status = XKeyManager_UpdateKeyObjFromVault(DmaPtr, ParamsPtr->KeyId,
				(u64)(UINTPTR)&KeyObject, SubsystemId,
				(u8)(XASU_KEYMANAGER_AES_ENC_USE_CASE |
				XASU_KEYMANAGER_AES_DEC_USE_CASE),
				XKEYMANAGER_RSA_OP_NONE, XASU_AES_SUBVAULT_ID);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			/** Only a 256-bit key can be programmed. */
			if (KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS) {
				Status = XASUFW_PROVISION_INVALID_PARAM;
				goto END;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     (const void *)(UINTPTR)KeyObject.KeyAddress,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_USER_BUFFER:
			/** Buffer path: copy the user supplied key into the local buffer. */
			Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)KeyBuf,
					       XPROVISION_EFUSE_KEY_SIZE_IN_BYTES, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
			}
			break;
		default:
			Status = XASUFW_PROVISION_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the given plaintext key with the PUF KEK to produce the black
 *		key to be programmed into the eFUSE. The caller transfers the PUF KEK before this
 *		call and clears the key vault after it.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EfuseKeyId	Target USER eFUSE key (selects the KEK IV: eFUSE0 or eFUSE1).
 * @param	PlainKey	Pointer to the 256-bit plaintext key.
 * @param	BlackKey	Pointer to the buffer that receives the 256-bit black
 *				(encrypted) key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the black key is generated successfully.
 *	- XASUFW_MEM_COPY_FAIL, if copying the eFUSE KEK IV fails.
 *	- XASUFW_KEK_IV_IS_ZERO, if the eFUSE KEK IV is zero.
 *	- Error code from XAes_EncryptRedKey, if the AES encryption operation fails.
 *
 * @note	Each eFUSE key has its own RTCA base IV, which is used as-is.
 *
 *************************************************************************************************/
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	const u8 *IvPtr = (EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_1) ?
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_1_IV_ADDR :
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	u8 IvBuf[XPROVISION_EFUSE_IV_SIZE_IN_BYTES] = {0U};
	u8 TagBuf[XPROVISION_EFUSE_TAG_SIZE_IN_BYTES] = {0U};
	XAsu_AesParams AesParams = {0U};

	/** Copy the eFUSE KEK IV (eFUSE0 or eFUSE1) from PLM RTCA and ensure it is non-zero. */
	Status = Xil_SMemCpy(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES, IvPtr,
			     XPROVISION_EFUSE_IV_SIZE_IN_BYTES, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEK_IV_IS_ZERO;
		goto END;
	}

	/** AES-GCM encrypt the red key with the PUF KEK, selected by XAes_EncryptRedKey. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = (u8)XASU_AES_ENCRYPT_OPERATION;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)IvBuf;
	AesParams.IvObj.IvLen = XPROVISION_EFUSE_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)PlainKey;
	AesParams.OutputDataAddr = (u64)(UINTPTR)BlackKey;
	AesParams.DataLen = XPROVISION_EFUSE_KEY_SIZE_IN_BYTES;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XPROVISION_EFUSE_TAG_SIZE_IN_BYTES;

	/** Encrypt the red key with the PUF KEK and return the raw AES status. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_EncryptRedKey(AesInstancePtr, DmaPtr, &AesParams);

	/** Zeroize the local tag buffer. */
	ClearStatus = Xil_SecureZeroize(TagBuf, XPROVISION_EFUSE_TAG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to the PLM to program a USER eFUSE key
 *		(USER_KEY_0 / USER_KEY_1) with the provided black (PUF KEK encrypted) key.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 * @param	KeyData		Pointer to the 256-bit (8-word) black key data to be programmed.
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is programmed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the PLM reports a failure for the eFUSE write.
 *
 *************************************************************************************************/
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the AES-key-write IPI payload: {hdr, EnvDis|KeyType, KeyAddrLow,
	 * KeyAddrHigh}. The PLM reads the key by address; KeyData is a u32 array (the black key),
	 * so it is naturally word (4-byte) aligned as the XilNVM AES key write requires, and it
	 * stays valid for the duration of this blocking IPI exchange.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_WRITE_AES_KEY_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX] =
		(EfuseKeyId << XASUFW_PLM_XILNVM_KEYTYPE_SHIFT) |
		XASUFW_PLM_XILNVM_ENV_MON_CHECK;
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX] = (u32)(UINTPTR)KeyData;
	/* ASU is a 32-bit address space; the high address word is always zero. */
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX] = 0U;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * The IPI read succeeded but the PLM may still report a failure in the response.
	 * Print the PLM response and return the provisioning error code to the client.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE write failed: 0x%08x\r\n", Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function asks the PLM to verify the target USER eFUSE key against the CRC of
 * 		an all-zero key, which matches only while the key is unprogrammed.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 *
 * @return
 *	- XASUFW_SUCCESS, if the target USER eFUSE key is unprogrammed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the key is already programmed or the PLM reports any
 *		other failure.
 *
 *************************************************************************************************/
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the CRC check payload: {hdr, Crc, KeyType}.
	 * KeyType occupies a whole word here.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_CRC_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_CHECK_CRC_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_CRC_INDEX] = XASUFW_NVM_EFUSE_CRC_AES_ZEROS;
	Payload[XASUFW_PLM_XILNVM_CRC_KEYTYPE_INDEX] = EfuseKeyId;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A CRC mismatch means the key is already programmed; any other failure is a controller or
	 * command error. Print the PLM response and return a single error code for both.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE CRC check failed: 0x%08x\r\n",
			      Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provisions a Unique Device Endorsement (UDE) key pair.
 *
 *		The ECC NIST P-384 key pair is generated inside the ASU so the private key is never
 *		exposed outside the ASU. The private key is encrypted with the PUF KEK (reusing the OCP
 *		UDE encryption scheme), the encrypted key is programmed into the requested UDE eFUSE
 *		slot via the PLM (XilNVM), and a self-signed X.509 certificate over the generated
 *		public key is returned to the caller along with its actual size.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr			Pointer to the SHA3 (XSHA_1) instance used to hash the
 *					self-signed certificate.
 * @param	ParamsPtr		Pointer to the XAsu_ProvisionUdeParams structure.
 * @param	CertActualSizePtr	Pointer to the response buffer location that receives the
 *					actual size in bytes of the generated certificate. The client
 *					copies this value to ParamsPtr->CertActualSizeAddr during the
 *					callback.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE key pair is provisioned successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_UDE_SLOT_PROGRAMMED, if the target UDE slot is already programmed.
 *	- XASUFW_PROVISION_UDE_ENCRYPT_FAIL, if UDE key pair generation, private key encryption or
 *	  certificate generation fails.
 *	- XASUFW_PROVISION_UDE_EFUSE_WRITE_FAIL, if the eFUSE write request to the PLM fails.
 *	- XASUFW_PROVISION_UDE_KEY_TYPE_UPDATE_FAIL, if updating the key vault header fails.
 *	- XASUFW_OCP_DISABLED, if OCP is disabled.
 *
 *************************************************************************************************/
s32 XProvision_Ude(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XAsu_ProvisionUdeParams *ParamsPtr,
		   u32 *CertActualSizePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	u32 CertSize = 0U;
	XFih_Var XFihProvision = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 EncPvtKey[XPROVISION_UDE_PVT_KEY_LEN_IN_WORDS];
	u8 UdeKeyTypeCode;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (CertActualSizePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_ProvisionValidateUdeParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** Clear the encrypted key buffer before use. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureZeroize((u8 *)EncPvtKey, XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Ensure the target UDE eFUSE slot is not already programmed. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_IsUdeSlotFree(ParamsPtr->UdeSlot);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_SLOT_PROGRAMMED);
		XFIH_GOTO(END);
	}

	/**
	 * Generate the UDE key pair, encrypt the private key with the PUF KEK into the local buffer,
	 * and generate a self-signed X.509 certificate over the generated public key into the
	 * caller's certificate buffer. The OCP UDE key encryption scheme is reused so the encrypted
	 * key stays byte-compatible with the UDE challenge response decryption path, and the private
	 * key is never exposed outside the ASU.
	 */
#ifdef XASU_OCP_ENABLE
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_EncryptUdeKeys(DmaPtr, ShaPtr, (u32)ParamsPtr->UdeSlot, (u8 *)EncPvtKey,
						ParamsPtr->CertBufAddr, ParamsPtr->CertBufSize,
						&CertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_ENCRYPT_FAIL);
		XFIH_GOTO(END_CLR);
	}
#else
	ASSIGN_VOLATILE(Status, XASUFW_OCP_DISABLED);
	XFIH_GOTO(END_CLR);
#endif /* XASU_OCP_ENABLE */

	/**
	 * Record the provisioned UDE key type in the key vault header for the programmed slot only
	 * (00b = ECC P-384, 01b = ML-DSA). The KeyManager packs the two-bit code into the slot's
	 * position (UDE_0 in the lowest two bits, UDE_3 in the highest two bits) without disturbing
	 * the other slots.
	 */
	if (ParamsPtr->UdeKeyType == XASU_PROVISION_UDE_KEY_TYPE_ECC_P384) {
		UdeKeyTypeCode = XPROVISION_UDE_KEY_TYPE_CODE_P384;
	} else {
		UdeKeyTypeCode = XPROVISION_UDE_KEY_TYPE_CODE_MLDSA;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UpdateUdeKeyType(ParamsPtr->UdeSlot, UdeKeyTypeCode);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_KEY_TYPE_UPDATE_FAIL);
		goto END_CLR;
	}

	/**
	 * Return the actual size of the generated UDE certificate to the caller through the
	 * response buffer. The client copies this value from the response buffer to
	 * ParamsPtr->CertActualSizeAddr during the callback. The certificate itself was already
	 * copied to the caller's buffer by XOcp_EncryptUdeKeys.
	 */
	*CertActualSizePtr = CertSize;

	/** Program the encrypted UDE private key into the requested eFUSE slot via the PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_WriteUdeKeyToEfuse((u32)ParamsPtr->UdeSlot, (const u32 *)EncPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_EFUSE_WRITE_FAIL);
	}

#ifdef XASU_OCP_ENABLE
	/**
	 * The UDE public key cache names the slot by the address of its encrypted key, and the
	 * write leaves that address unchanged, so drop the cache to keep a key derived earlier in
	 * this boot from being served for the key just programmed.
	 */
	XOcp_InvalidateUdePubKeyCache();
#endif /* XASU_OCP_ENABLE */

END_CLR:
	/** Zeroize the local encrypted key buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihProvision, ClearStatus, (u8 *)EncPvtKey,
		  XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the requested UDE eFUSE slot is free (not yet
 *		programmed) by reading its eFUSE cache and ensuring it is all zeros.
 *
 * @param	UdeSlot		UDE eFUSE slot (XASU_OCP_UDE_USER_KEY_0/1/2_ID).
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE slot is free.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the UDE slot is invalid.
 *	- XASUFW_PROVISION_UDE_SLOT_PROGRAMMED, if the UDE slot is already programmed.
 *	- XASUFW_OCP_DISABLED, if OCP is disabled.
 *
 *************************************************************************************************/
static s32 XProvision_IsUdeSlotFree(u8 UdeSlot)
{
	s32 Status = XASUFW_FAILURE;
#ifdef XASU_OCP_ENABLE
	u32 UdeUserKeyAddr = 0U;

	/** Map the UDE slot to its eFUSE cache address. */
	Status = XOcp_GetUdeSlotInfo((u32)UdeSlot, &UdeUserKeyAddr, NULL);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INVALID_PARAM);
		goto END;
	}

	/**
	 * If the eFUSE cache for the slot is non-zero, the slot is already programmed. Note the PLM
	 * (XilNVM) also enforces this during the key write.
	 */
	Status = XASUFW_FAILURE;
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)UdeUserKeyAddr,
				      XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_UDE_SLOT_PROGRAMMED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
#else
	/** UDE slot mapping lives in the OCP module, so the check is unsupported without OCP. */
	(void)UdeSlot;
	Status = XASUFW_OCP_DISABLED;
#endif /* XASU_OCP_ENABLE */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to the PLM to program a UDE (DME) private key
 *		into the requested eFUSE slot with the provided encrypted key, and reloads the eFUSE
 *		cache so the newly programmed key is visible.
 *
 * @param	UdeSlot		UDE eFUSE (DME user key) slot to program.
 * @param	KeyData		Pointer to the encrypted UDE private key data to be programmed.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE eFUSE key is programmed.
 *	- XASUFW_PROVISION_INVALID_PARAM, if input parameters are invalid.
 *	- XASUFW_PROVISION_PLM_IPI_FAIL, if the IPI request/response to the PLM fails.
 *	- XASUFW_PROVISION_PLM_ERROR, if the PLM reports a failure for the eFUSE write.
 *
 *************************************************************************************************/
static s32 XProvision_WriteUdeKeyToEfuse(u32 UdeSlot, const u32 *KeyData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	if ((KeyData == NULL) || ((UdeSlot != XASU_OCP_UDE_USER_KEY_0_ID) &&
	    (UdeSlot != XASU_OCP_UDE_USER_KEY_1_ID) && (UdeSlot != XASU_OCP_UDE_USER_KEY_2_ID))) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Build the DME-key-write IPI payload: {hdr, EnvDis | (DmeKeyType << 16), KeyAddrLow,
	 * KeyAddrHigh}.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_WRITE_DME_KEY_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX] =
		(UdeSlot << XASUFW_PLM_XILNVM_KEYTYPE_SHIFT) |
		XASUFW_PLM_XILNVM_ENV_MON_DIS;
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX] = (u32)(UINTPTR)KeyData;
	/* ASU is a 32-bit address space; the high address word is always zero. */
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX] = 0U;

	/** Send an IPI request to the PLM to write the UDE eFUSE key. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	}

	/** Read the IPI response and check for a PLM-reported failure. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	} else if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM UDE eFUSE write failed: 0x%08x\r\n", Response);
		Status = XASUFW_PROVISION_PLM_ERROR;
		goto END;
	} else {
		/* Do nothing. */
	}

	/**
	 * Request the PLM to reload the eFUSE cache and program the protection bits so the newly
	 * programmed UDE key is reflected in the eFUSE cache.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(0U, XASUFW_PLM_XILNVM_RELOAD_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_RELOAD_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	}

	Response = (u32)XASUFW_FAILURE;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
	} else if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE reload failed: 0x%08x\r\n", Response);
		Status = XASUFW_PROVISION_PLM_ERROR;
	} else {
		/* Do nothing. */
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECC key pair for the wrapped-provisioning
 *		feature and stores it in the ASU-owned exportable key vault. The private key is
 *		restricted to the requested provisioning use case only, and neither
 *		half of the pair nor either key ID is returned to the caller: both halves stay in
 *		the vault and the unwrap path locates the private key internally. Each subsystem
 *		may hold one provisioning key pair per use case, so a request whose reserved slot is
 *		already populated is rejected instead of replacing the key: a repeated request
 *		cannot invalidate a public half which has already been handed out. Replacing a pair
 *		remains possible, but only as a deliberate act of the subsystem which owns it,
 *		which has to delete the pair first with XProvision_DeleteKeyPair(). No
 *		other subsystem can do it on its behalf, because the slot is derived from the
 *		requesting subsystem rather than taken from the request.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	SubSystemId	Subsystem requesting the provisioning key pair. Together with the
 *				use case it selects the reserved key slot within the exportable
 *				vault.
 * @param	KeyUseCase	Provisioning use case the key pair is generated for, either the
 *				provisioning-material unwrap use case (ECIES) or the provisioning
 *				authenticated-encryption use case. It selects the reserved slot
 *				range and is stored as the private key's use case. Nothing else is
 *				caller-supplied: the vault id (ASU-owned exportable vault), curve
 *				(P-384), key length, epoch time and non-depleting usage count are
 *				all set internally.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key pair is generated and both halves are stored.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if DmaPtr is NULL.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, which is the case when the use case is not a supported
 *		provisioning use case or the subsystem is not assigned to any channel. The
 *		underlying cause is packed into the status. The key pair delete path reports a
 *		slot lookup failure with the same code.
 *	- XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS, if the reserved slot already holds a
 *		provisioning private key for that use case, or the slot lookup did not confirm the
 *		slot is free.
 *	- Any other status comes through unchanged from XKeyManager_GenerateEccKeyPair(), which
 *		performs the generation and the vault store; see that function for the codes it
 *		returns. XASUFW_KEYMANAGER_KEY_SLOT_IN_USE arrives this way when the reserved slot
 *		is occupied, because the already-exists guard above inspects only the private half
 *		and a slot left holding a stranded public half reaches the store.
 *
 *************************************************************************************************/
s32 XProvision_GenerateKeyPair(XAsufw_Dma *DmaPtr, u32 SubSystemId, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 PvtKeyId = 0U;
	u16 KeySlot = 0U;
	u32 KeyIds[XKEYMANAGER_KEY_PAIR_ID_COUNT] = {0U};
	XAsu_KeyManagerParams KmParams = {0};

	/** Validate input parameters. */
	if (DmaPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. Status holds the cause from the lookup, an unsupported use case or a subsystem
	 * with no channel, so it is retained rather than replaced.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	/**
	 * A subsystem gets one provisioning key pair per use case. The slot is fixed for that
	 * pairing, so a populated slot means a key pair whose public half may already be held
	 * elsewhere: refuse, so that a repeated request cannot invalidate that copy by accident.
	 * This is not a guarantee that the pair cannot be replaced. The owning subsystem can
	 * delete it and generate a new one, which is a deliberate act of the party which handed
	 * the public half out, and no other subsystem can do it on its behalf.
	 * Only an explicit XASUFW_KEYMANAGER_KEY_NOT_FOUND lets the request through, so a lookup
	 * which is skipped or whose result is disturbed refuses the request instead of overwriting
	 * the key.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetKeyId(KeySlot, &PvtKeyId, KeyUseCase);
	if (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND) {
		Status = XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS;
		goto END;
	}

	/**
	 * The provisioning key pair profile is fixed and never taken from the caller: it is always
	 * generated on the NIST P-384 curve, stored only in the ASU-owned exportable vault, and
	 * restricted to the provisioning (ECIES) use cases, one per half. The private key is kept as a
	 * non-depleting key so it can service repeated unwrap requests. The slot is reserved per
	 * subsystem so the KeyId stays derivable after an export/import cycle. Populate the metadata
	 * internally so the caller cannot alter the profile.
	 */
	KmParams.KeyMetadata.CompositeKeyId = PvtKeyId;
	KmParams.KeyMetadata.KeyUseCase = KeyUseCase;
	KmParams.KeyMetadata.KeyAttributes = (u8)XASU_ECC_NIST_P384;
	KmParams.KeyMetadata.Length = (u16)XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.UsageCount = XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE;
	/**
	 * The key pair goes to the vault, not to a caller buffer, so KeyObjectAddr stays zero.
	 * The vault path requires exactly one of KeyObjectAddr and KeyIdAddr to be set, so
	 * KeyIdAddr points at the local Key IDs; the assigned IDs are written through the KeyIds
	 * argument and never travel outside the ASU.
	 */
	KmParams.KeyObjectAddr = 0U;
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KeyIds[0U];

	/**
	 * Generate and store the key pair through the common ECC key pair path, as the ASU: the
	 * exportable vault is ASU-owned, so its vault access check also rejects the case where an
	 * imported blob has handed vault 1 to another subsystem. Both halves land in the reserved
	 * slot of their sub-vault and the assigned Key IDs stay ASU-internal (not returned to the
	 * caller) so they cannot be reused as handles for other operations.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GenerateEccKeyPair(DmaPtr, &KmParams, KeyIds,
						XKEYMANAGER_ASU_SUBSYSTEM_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the wrapped-provisioning ECC key pair held in a
 *		subsystem's reserved slot of the ASU-owned exportable key vault, so that the
 *		subsystem can generate a fresh pair. Both halves are deleted as the ASU, because
 *		the exportable vault is ASU-owned and a client cannot delete from it directly.
 *		A slot holding only one half still has that half cleaned up, but the operation
 *		reports XASUFW_KEYMANAGER_KEY_NOT_FOUND because there was no complete key pair to
 *		delete; success is reported only when both halves are deleted. Each provisioning
 *		use case owns its own reserved slot, so only the pair held for the requested use
 *		case is deleted and the other use case's pair is left untouched.
 *
 * @param	SubSystemId	Subsystem whose provisioning key pair is deleted. Together with the
 *				use case it selects the reserved key slot within the exportable
 *				vault.
 * @param	KeyUseCase	Provisioning use case whose key pair is deleted, either the
 *				provisioning-material unwrap use case (ECIES) or the provisioning
 *				authenticated-encryption use case. It must be the use case the pair
 *				was generated for, because it selects the reserved slot range.
 *				Nothing else is caller-supplied: the vault id and the sub-vaults are
 *				fixed internally.
 *
 * @return
 *	- XASUFW_SUCCESS, only if both halves of the key pair are deleted.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, which is the case when the use case is not a supported
 *		provisioning use case or the subsystem is not assigned to any channel. The
 *		underlying cause is packed into the status. The key pair generation path reports a
 *		slot lookup failure with the same code.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the reserved slot did not hold both halves (neither
 *		half, or only one), so there was no complete key pair to delete.
 *	- The delete failure reported by XKeyManager_DeleteKey() packed with
 *		XASUFW_KEYMANAGER_KEY_DELETE_ERROR, if deleting a half which is present fails. The
 *		cause, such as a missing vault, a vault no longer owned by the ASU or a failed
 *		zeroization, is kept in the first error field rather than discarded, so the value
 *		does not compare equal to XASUFW_KEYMANAGER_KEY_DELETE_ERROR on its own.
 *
 *************************************************************************************************/
s32 XProvision_DeleteKeyPair(u32 SubSystemId, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static const XAsu_KeyManagerSubVaultType SubVaults[XKEYMANAGER_ASYM_KEY_TYPE_COUNT] = {
		XASU_ECC_PUB_SUBVAULT_ID, XASU_ECC_PVT_SUBVAULT_ID
	};
	u16 KeySlot = 0U;
	u32 KeyId = 0U;
	u32 TypeIdx = 0U;
	volatile u32 DeletedCount = 0U;

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. The lookup failure is reported with the same code as the key pair generation path,
	 * so one cause does not reach the caller as two different codes depending on which command
	 * was issued.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	/**
	 * Delete both halves from the reserved slot as the ASU, public half first. The private half
	 * is what XProvision_GetKeyId() looks at, so it is the marker the key pair
	 * generation guard tests. Removing it last keeps that marker truthful: if
	 * the second delete fails, the pair is still usable and the generation guard still refuses
	 * cleanly, and re-issuing this operation clears the slot. Deleting it first would leave a
	 * public half that no path reports, yet which still occupies the reserved slot and makes the
	 * next generation fail inside the store.
	 * A half which is already absent is not an error: the goal is an empty slot, and stopping
	 * there would leave the other half behind and keep blocking a fresh key pair. Only an
	 * absent key is tolerated, not an unusable sub-vault: a vault 1 whose ECC public sub-vault
	 * has no capacity for the reserved slot, as an imported pre-public-half blob would give,
	 * is reported rather than silently half-deleted.
	 */
	for (TypeIdx = 0U; TypeIdx < XKEYMANAGER_ASYM_KEY_TYPE_COUNT; TypeIdx++) {
		KeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
						SubVaults[TypeIdx], KeySlot);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_DeleteKey(KeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID);
		if (Status == XASUFW_SUCCESS) {
			DeletedCount++;
		} else if (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND) {
			/* Status holds the cause from XKeyManager_DeleteKey, retain it. */
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_KEYMANAGER_KEY_DELETE_ERROR);
			goto END;
		} else {
			/* This sub-vault holds nothing for the subsystem, continue. */
		}
	}

	/**
	 * Report success only when both halves of the key pair were deleted. If the slot held
	 * neither half or only one of them (an incomplete pair), there was no complete key pair to
	 * delete, so report key not found.
	 */
	if (DeletedCount == XKEYMANAGER_ASYM_KEY_TYPE_COUNT) {
		Status = XASUFW_SUCCESS;
	} else {
		Status = XASUFW_KEYMANAGER_KEY_NOT_FOUND;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Map a subsystem and a provisioning use case to the ECC key slot reserved for that
 *		pairing in the ASU-owned exportable vault. Each use case owns a disjoint range of
 *		XASU_MAX_CHANNELS_SUPPORTED slots and the slot within a range is the subsystem's
 *		channel index plus one, so the unwrap use case occupies slots
 *		1..XASU_MAX_CHANNELS_SUPPORTED and the authenticated-encryption use case occupies
 *		the range above it. A subsystem can therefore hold one key pair per use case
 *		without the two ever resolving to the same slot.
 *
 * @param	SubSystemId	Subsystem whose reserved slot is requested.
 * @param	SlotPtr		Pointer that receives the 1-based slot on success.
 * @param	KeyUseCase	Provisioning use case the slot is requested for. Selects the range.
 *
 * @return
 *	- XASUFW_SUCCESS, if the use case is supported and the subsystem maps to a channel.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if SlotPtr is NULL or the subsystem is not assigned to
 *		any channel.
 *	- XASUFW_KEYMANAGER_INVALID_PROVISIONING_USE_CASE, if the use case is not exactly one of
 *		the supported provisioning use cases.
 *
 *************************************************************************************************/
static s32 XProvision_GetKeySlot(u32 SubSystemId, u16 *SlotPtr, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 Index = 0U;
	u16 SlotBase = 0U;

	if (SlotPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Select the reserved range for the use case. The comparison is an exact match rather than
	 * a bit test, so a value carrying more than one use case is refused: a provisioning key
	 * pair holds exactly one. An unrecognised use case is refused rather than defaulted to a
	 * range, so a caller can never land on the slot of another use case.
	 */
	if (KeyUseCase == XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE) {
		SlotBase = 0U;
	} else if (KeyUseCase == XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE) {
		SlotBase = (u16)XASU_MAX_CHANNELS_SUPPORTED;
	} else {
		Status = XASUFW_KEYMANAGER_INVALID_PROVISIONING_USE_CASE;
		goto END;
	}

	for (Index = 0U; Index < XASU_MAX_CHANNELS_SUPPORTED; Index++) {
		if (SubSystemId == XAsu_GetSubsysIdFromIpiMask(XAsu_GetIpiMask(Index))) {
			*SlotPtr = (u16)(Index + XKEYMANAGER_KEY_INDEX_OFFSET + SlotBase);
			Status = XASUFW_SUCCESS;
			break;
		}
	}

	/** The subsystem holds no channel, so no slot is reserved for it. */
	if (Index == XASU_MAX_CHANNELS_SUPPORTED) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Derive the provisioning private key ID for a reserved slot in the ASU-owned
 *		exportable vault and confirm the slot is occupied by a key that carries the
 *		requested provisioning use case. Lets the key pair generation guard test the slot
 *		without the caller ever holding the KeyId. Because the ID is derived rather than
 *		remembered, it also resolves after the vault has been exported and imported again.
 *		The caller resolves the slot first with XProvision_GetKeySlot(), so the
 *		mapping is not repeated here.
 *
 * @param	KeySlot		Reserved slot of the subsystem whose private key ID is requested.
 * @param	KeyIdPtr	Pointer that receives the private key ID on success.
 * @param	KeyUseCase	Provisioning use case the slot must carry. Must be the same use case
 *				the slot was resolved for, so the guard tests the key pair which
 *				would actually be replaced.
 *
 * @return
 *	- XASUFW_SUCCESS, if the slot holds a key carrying the requested use case.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if KeyIdPtr is NULL.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the vault or the slot cannot be resolved, or the slot
 *	  does not carry that use case. Reported only from the branch which observed the slot to
 *	  be empty, because it is the value the key pair generation guard treats as permissive.
 *	- XASUFW_FAILURE, if the lookup did not run to a decision.
 *
 *************************************************************************************************/
static s32 XProvision_GetKeyId(u16 KeySlot, u32 *KeyIdPtr, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 KeyId = 0U;

	if (KeyIdPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	KeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					XASU_ECC_PVT_SUBVAULT_ID, KeySlot);

	/**
	 * Validate the slot without consuming a use of the key: the caller fetches the key object
	 * itself when it is actually used.
	 * XASUFW_KEYMANAGER_KEY_NOT_FOUND is the one value which lets the key pair generation
	 * guard replace the slot contents, so it is reported only from the branch which observed
	 * the slot to be empty. It is deliberately not the initial value: a lookup whose result is
	 * disturbed leaves the initial failure and the guard refuses.
	 */
	if (XKeyManager_GetKeyObjectPtr(KeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID, KeyUseCase,
			(u8)FALSE) != NULL) {
		Status = XASUFW_SUCCESS;
	} else {
		Status = XASUFW_KEYMANAGER_KEY_NOT_FOUND;
	}

	*KeyIdPtr = KeyId;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the public key of a subsystem's provisioning ECC NIST P-384
 *		key pair held in the ASU-owned exportable key vault, without generating it. The use
 *		case selects which of the two reserved slot bands the pair lives in: the
 *		provisioning-material unwrap use case (ECIES) resolves to the wrapping pair and the
 *		provisioning authenticated-encryption use case resolves to the authentication pair.
 *		Both halves of a provisioning pair are stored with the same use-case value, so the
 *		private use case also matches the stored public half. Neither half nor either key ID
 *		is otherwise exposed: only the public key bytes are copied out for the Device
 *		Provisioning Certificate.
 *
 * @param	SubSystemId	Subsystem whose provisioning public key is requested. Together with
 *				the use case it selects the reserved slot within the exportable vault.
 * @param	KeyUseCase	Provisioning use case whose public key is requested, either
 *				XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE (wrapping/ECIES) or
 *				XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE (authentication). It
 *				must be the use case the pair was generated for, because it selects
 *				the reserved slot range.
 * @param	PubKeyBuf	Buffer that receives the uncompressed (X || Y) public key
 *				(XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES bytes).
 *
 * @return
 *	- XASUFW_SUCCESS, if the public key is returned successfully.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if PubKeyBuf is NULL.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, with the underlying cause packed into the status.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the reserved slot holds no public key for that use
 *		case, i.e. the key pair does not exist in the exportable vault.
 *	- Error code from Xil_SMemCpy, if copying the public key fails.
 *
 *************************************************************************************************/
static s32 XProvision_GetPubKey(u32 SubSystemId, u8 KeyUseCase, u8 *PubKeyBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_EccPubKeyObject *PubKeyObj = NULL;
	u16 KeySlot = 0U;
	u32 PubKeyId = 0U;

	/** Validate input parameters. */
	if (PubKeyBuf == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. The unwrap use case resolves to the wrapping band and the authenticated-encryption
	 * use case to the authentication band, so the same lookup serves both provisioning pairs.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	PubKeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					  XASU_ECC_PUB_SUBVAULT_ID, KeySlot);

	/**
	 * Resolve the public half in the reserved slot without consuming a use of the key. Both
	 * halves of a provisioning pair carry the same use-case value, so the private use case
	 * requested here is a subset of the stored public half's use case and the lookup matches.
	 */
	PubKeyObj = (const XKeyManager_EccPubKeyObject *)(void *)XKeyManager_GetKeyObjectPtr(
			PubKeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID, KeyUseCase, (u8)FALSE);
	if (PubKeyObj == NULL) {
		Status = XASUFW_KEYMANAGER_KEY_NOT_FOUND;
		goto END;
	}

	/** Copy the uncompressed public key to the caller buffer. */
	Status = Xil_SMemCpy(PubKeyBuf, XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES,
			     PubKeyObj->PublicKey, XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES,
			     XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates and returns the Device Provisioning Certificate (DPC) for the
 *		requesting subsystem (SAS section 12.16.2).
 *
 *		The provisioning authentication key pair (ECC NIST P-384) and provisioning wrapping
 *		key pair (ECIES over NIST P-384) are looked up in the ASU-owned exportable key vault
 *		and generated with the TRNG if absent. A UDE-signed X.509 certificate is generated
 *		over each public key, and the DPC container is assembled directly in the caller
 *		buffer as: Device DNA, the length-prefixed authentication certificate, the
 *		length-prefixed wrapping certificate, and a UDE signature (ECC P-384) over all the
 *		preceding bytes. The active UDE key pair signs both the certificates and the
 *		container; its private key is decrypted inside the ASU and zeroized before returning.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA instance used to hash the DPC container. The Get
 *				DPC command reserves the SHA3 engine, so this is the SHA3 (XSHA_1)
 *				instance allocated by the resource handler.
 * @param	ParamsPtr	Pointer to the XAsu_DevProvisionCertParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (selects the exportable
 *				vault slots that hold its provisioning key pairs).
 * @param	DpcActualSizePtr	Pointer to the response buffer location that receives the
 *					actual DPC size in bytes. The client copies this value to
 *					ParamsPtr->DpcActualSizeAddr during the callback.
 *
 * @return
 *	- XASUFW_SUCCESS, if the DPC is generated successfully.
 *	- XASUFW_PROVISION_DPC_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_DPC_UDE_KEY_FAIL, if the active UDE key pair cannot be obtained.
 *	- XASUFW_PROVISION_DPC_KEY_FETCH_FAIL, if a provisioning key pair cannot be obtained or
 *		generated.
 *	- XASUFW_PROVISION_DPC_CERT_GEN_FAIL, if a UDE-signed certificate cannot be generated.
 *	- XASUFW_PROVISION_DPC_ASSEMBLE_FAIL, if a DPC field cannot be written to the caller buffer.
 *	- XASUFW_PROVISION_DPC_SIGN_FAIL, if the UDE signature over the container cannot be computed.
 *	- XASUFW_PROVISION_DPC_COPY_FAIL, if the signature cannot be copied to the caller.
 *	- XASUFW_OCP_DISABLED, if the OCP module is not enabled in the build.
 *
 *************************************************************************************************/
s32 XProvision_GetDevProvisionCert(XAsufw_Dma *DmaPtr, XSha *ShaPtr,
				   const XAsu_DevProvisionCertParams *ParamsPtr, u32 SubsystemId,
				   u32 *DpcActualSizePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihDpc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdePvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};
	u8 SignBuf[XPROVISION_DPC_SIG_SIZE_IN_BYTES] = {0U};
#ifdef XASU_OCP_ENABLE
	XAsu_ShaOperationCmd ShaCmd;
	u8 UdePubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u8 AuthPubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u8 WrapPubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u64 DpcBufAddr;
	u32 LenField;
	u32 Offset = 0U;
	u32 AuthCertSize = 0U;
	u32 WrapCertSize = 0U;
	u32 TotalSize = 0U;
#endif /* XASU_OCP_ENABLE */

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (DpcActualSizePtr == NULL)) {
		Status = XASUFW_PROVISION_DPC_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_ProvisionValidateDpcParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_DPC_INVALID_PARAM;
		goto END;
	}

#ifdef XASU_OCP_ENABLE
	DpcBufAddr = ParamsPtr->DpcBufAddr;

	/** Obtain the active (non-revoked) UDE key pair that signs the DPC. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetActiveUdeKeyPair(DmaPtr, UdePvtKey, UdePubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_UDE_KEY_FAIL);
		goto END_CLR;
	}

	/**
	 * Generate the provisioning authentication (ECC P-384) key pair if it does not already
	 * exist, then fetch its public key. An already-existing pair is reused rather than treated
	 * as an error, so a repeated DPC request rebuilds the certificate over the same key that
	 * was already handed out (SAS section 8.4). The pair lives in the authentication slot band,
	 * selected by the authenticated-encryption provisioning use case.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GenerateKeyPair(DmaPtr, SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE);
	if ((Status != XASUFW_SUCCESS) &&
	    (Status != XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetPubKey(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE, AuthPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/**
	 * Generate the provisioning wrapping (ECIES P-384) key pair if it does not already exist,
	 * reusing the common provisioning key pair generator, then fetch its public key. An
	 * already-existing pair is not an error here. The pair lives in the wrapping slot band,
	 * selected by the provisioning-material unwrap (ECIES) use case.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GenerateKeyPair(DmaPtr, SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if ((Status != XASUFW_SUCCESS) &&
	    (Status != XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetPubKey(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE, WrapPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/** DPC field 1: Device DNA. Ensure the caller buffer can hold the DNA field. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_DNA_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)EFUSE_CACHE_DNA_0, DpcBufAddr + Offset,
			       XPROVISION_DPC_DNA_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_DNA_SIZE_IN_BYTES;

	/**
	 * DPC field 2: length-prefixed UDE-signed authentication key certificate. The certificate
	 * is generated directly into the caller buffer, after its 4-byte length prefix, then the
	 * length is written back.
	 */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateUdeSignedKeyCert(DmaPtr, ShaPtr, AuthPubKey, (u8)XASU_TRUE, UdePubKey,
			UdePvtKey, DpcBufAddr + Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES,
			ParamsPtr->DpcBufSize - (Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES),
			&AuthCertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_CERT_GEN_FAIL);
		goto END_CLR;
	}
	/** The certificate length prefix is stored in big-endian (network) byte order. */
	LenField = Xil_Htonl(AuthCertSize);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&LenField, DpcBufAddr + Offset,
			       XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES + AuthCertSize;

	/** DPC field 3: length-prefixed UDE-signed wrapping key certificate. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateUdeSignedKeyCert(DmaPtr, ShaPtr, WrapPubKey, (u8)XASU_FALSE, UdePubKey,
			UdePvtKey, DpcBufAddr + Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES,
			ParamsPtr->DpcBufSize - (Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES),
			&WrapCertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_CERT_GEN_FAIL);
		goto END_CLR;
	}
	/** The certificate length prefix is stored in big-endian (network) byte order. */
	LenField = Xil_Htonl(WrapCertSize);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&LenField, DpcBufAddr + Offset,
			       XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES + WrapCertSize;

	/** Ensure the caller buffer can hold the UDE signature field before writing it. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_SIG_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	/**
	 * DPC field 4: UDE signature over all preceding bytes. Compute the SHA-384 digest of the
	 * assembled fields (on the SHA3 engine) and sign it with the active UDE private key
	 * (ECC NIST P-384).
	 */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = DpcBufAddr;
	ShaCmd.DataSize = Offset;
	ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
	ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaPtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_SIGN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGenerateSignature(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdePvtKey, NULL,
			(u64)(UINTPTR)HashBuf, XASU_SHA_384_HASH_LEN, (u64)(UINTPTR)SignBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_SIGN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SignBuf, DpcBufAddr + Offset,
			       XPROVISION_DPC_SIG_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_COPY_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_SIG_SIZE_IN_BYTES;
	TotalSize = Offset;

	/**
	 * Return the actual size of the assembled DPC to the caller through the response buffer.
	 * The client copies this value from the response buffer to ParamsPtr->DpcActualSizeAddr
	 * during the callback.
	 */
	*DpcActualSizePtr = TotalSize;

	/**
	 * Record the DPC-specific success sentinel in the global redundant ReturnStatus. The command
	 * layer copies it into the response AdditionalStatus, where the client redundantly confirms
	 * the DPC was generated successfully, hardening the result against a glitched Status.
	 */
	ReturnStatus = XASUFW_PROVISION_DPC_SUCCESS;
#else
	ASSIGN_VOLATILE(Status, XASUFW_OCP_DISABLED);
#endif /* XASU_OCP_ENABLE */

END_CLR:
	/** Zeroize the decrypted UDE private key and the local hash/signature working buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihDpc, ClearStatus, UdePvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize(SignBuf, XPROVISION_DPC_SIG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the requesting subsystem's provisioning authentication and
 *		wrapping key pairs from the ASU-owned exportable key vault (SAS section 12.16.3), so
 *		that the next Get Device Provisioning Certificate request regenerates them.
 *
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (selects the exportable
 *				vault slots that hold its provisioning key pairs).
 *
 * @return
 *	- XASUFW_SUCCESS, if the provisioning key pairs are deleted successfully.
 *	- XASUFW_PROVISION_DPC_DELETE_FAIL, with the KeyManager error embedded, if the deletion
 *		fails.
 *
 *************************************************************************************************/
s32 XProvision_DeleteDevProvisionCert(u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * Delete the provisioning authentication key pair from its reserved slot band. A slot that
	 * holds no key pair is not an error here: the goal is that the next Get Device Provisioning
	 * Certificate request regenerates the keys, so a missing pair already satisfies that.
	 */
	Status = XProvision_DeleteKeyPair(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE);
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_DELETE_FAIL);
		goto END;
	}

	/** Delete the provisioning wrapping (ECIES) key pair from its reserved slot band. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_DeleteKeyPair(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_DELETE_FAIL);
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

#endif /* XASU_PROVISION_ENABLE */
/** @} */
