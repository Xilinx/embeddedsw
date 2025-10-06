/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_dme.c
*
* This file contains the implementation of the interface functions for OCP DME functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   yog  08/21/25 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xocp_dme_server_apis OCP DME server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xasufw_hw.h"
#include "xasufw_status.h"
#include "xasufw_trnghandler.h"
#include "xasufw_util.h"
#include "xecc.h"
#include "xocp_dme.h"
#include "xocp.h"
#include "xaes.h"
#include "xkdf.h"
#include "xsha.h"
#include "xsha_hw.h"

#ifdef XASU_OCP_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XOCP_DME_KEK_SIZE_IN_BYTES	(32U)	/**< DME KEK size in bytes */
#define XOCP_DME_CONTEXT		"DME_ENCRYPTION_KEY" /**< DME context */
#define XOCP_DME_CONTEXT_LEN		(18U)		/**< DME context length */
#define XOCP_DME0_IV_INC_VAL		(0x02U)	/**< DME0 IV increment value */
#define XOCP_DME1_IV_INC_VAL		(0x03U)	/**< DME1 IV increment value */
#define XOCP_DME2_IV_INC_VAL		(0x04U)	/**< DME2 IV increment value */
#define XOCP_DEC_BLACK_KEY_IV_INC_VAL	(0x10U)	/**< DME decryption black key IV increment value */
#define XASU_OCP_DME_TAG_SIZE_IN_BYTES	(16U)	/**< DME tag size in bytes */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
static u8 XOcp_DmeKek[XOCP_DME_KEK_SIZE_IN_BYTES] = {0U};	/**< DME KEK */
static u8 DmeKekFlag = XASU_FALSE;

/************************************ Function Prototypes ****************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr);
static void XOcp_IncrementIv(u8* Iv, u8 IncVal);
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 DmeUserKeyAddr, u8* Iv, u8* DmeDecPvtKey);

/*************************************************************************************************/
/**
 * @brief	This function generates the DME KEK.
 *
 * @return
 *		- XASUFW_SUCCESS, if all three keys are received successfully.
 *		- XASUFW_DME_DECRYPT_BLACK_KEY_0_FAIL, if black key decryption fails.
 *		- XASUFW_DME_CMAC_KDF_FAIL, if CMAC KDF operation fails.
 *
 *************************************************************************************************/
s32 XOcp_GenerateDmeKek(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAsufw_Dma *DmaPtr = XAsufw_GetDmaInstance(ASUDMA_0_DEVICE_ID);
	XAsu_KdfParams KdfParams = {0U};
	const char *CtxStr = XOCP_DME_CONTEXT;
	const u8 *Context = (const u8 *)CtxStr;
	u8 KekIv[XASU_OCP_DME_IV_SIZE_IN_BYTES] = {0U};
	const u8 *IvPtr = (u8*)(UINTPTR)XASU_RTCA_BH_IV_ADDR;
	XFih_Var FihStatus = XFih_VolatileAssignS32(XASUFW_FAILURE);

	Status = Xil_SMemCpy(KekIv, XASU_OCP_DME_IV_SIZE_IN_BYTES, IvPtr,
			XASU_OCP_DME_IV_SIZE_IN_BYTES, XASU_OCP_DME_IV_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}
	XOcp_IncrementIv(KekIv, XOCP_DEC_BLACK_KEY_IV_INC_VAL);

	/** Decrypt efuse black key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_DecryptEfuseBlackKey(AesInstancePtr, DmaPtr,
			XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_0_VALUE, XASU_AES_KEY_SIZE_256_BITS,
			(u64)(UINTPTR)KekIv, XASU_OCP_DME_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_DME_DECRYPT_BLACK_KEY_0_FAIL);
		goto END;
	}

	/** Update KDF parameters. */
	KdfParams.KeyInAddr = 0U;
	KdfParams.ContextAddr = (u64)(UINTPTR)Context;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)XOcp_DmeKek;
	KdfParams.KeyInLen = XASU_AES_KEY_SIZE_256_BITS;
	KdfParams.ContextLen = XOCP_DME_CONTEXT_LEN;
	KdfParams.KeyOutLen = XOCP_DME_KEK_SIZE_IN_BYTES;

	/** Generate DME KEK using CMAC-KDF. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_CmacGenerate(DmaPtr, &KdfParams, XASU_AES_EFUSE_KEY_RED_0);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_DME_CMAC_KDF_FAIL);
	}

END:
	FihStatus = XFih_VolatileAssignS32(Status);
	XFIH_IF_FAILOUT (FihStatus, ==, XFIH_SUCCESS) {
		DmeKekFlag = XASU_TRUE;
	}

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function encrypts the provided DME private key of provided key ID, copies the
 * 		received IV to the ASU memory based on key ID and returns the encrypted key.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OcpDmeKeyEnc	Pointer to the XAsu_OcpDmeKeyEncrypt structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if DME key encryption is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_OCP_DME_IV_COPY_FAIL, if DME IV copy operation fails.
 *	- XASUFW_OCP_DME_AES_COMPUTE_FAIL, if DME key encryption operation fails.
 *
 *************************************************************************************************/
s32 XOcp_EncryptDmeKeys(XAsufw_Dma *DmaPtr, const XAsu_OcpDmeKeyEncrypt *OcpDmeKeyEnc)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 *IvPtr = (u8*)(UINTPTR)XASU_RTCA_BH_IV_ADDR;
	u8 DmeKekIv[XASU_OCP_DME_IV_SIZE_IN_BYTES] = {0U};
	u8 IvIncVal = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (OcpDmeKeyEnc == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Check if DME KEK is present. */
	if (DmeKekFlag == XASU_FALSE) {
		Status = XASUFW_OCP_DME_KEK_NOT_PRESENT;
		goto END;
	}

	Status = Xil_SMemCpy(DmeKekIv, XASU_OCP_DME_IV_SIZE_IN_BYTES,
			IvPtr, XASU_OCP_DME_IV_SIZE_IN_BYTES, XASU_OCP_DME_IV_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/**
	 * Based on DME key ID, update the KEK IV with the offset to the local buffer.
	 * | DME key ID | Offset  |
	 * |     0      |    2    |
	 * |     1      |    3    |
	 * |     2      |    4    |
	 */
	switch (OcpDmeKeyEnc->DmeKeyId) {
		case XASU_OCP_DME_USER_KEY_0_ID:
			IvIncVal = XOCP_DME0_IV_INC_VAL;
			break;
		case XASU_OCP_DME_USER_KEY_1_ID:
			IvIncVal = XOCP_DME1_IV_INC_VAL;
			break;
		case XASU_OCP_DME_USER_KEY_2_ID:
			IvIncVal = XOCP_DME2_IV_INC_VAL;
			break;
		default:
			Status = XASUFW_OCP_INVALID_PARAM;
			goto END;
			break;
	}
	XOcp_IncrementIv(DmeKekIv, IvIncVal);

	/** Encrypt DME private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)DmeKekIv, OcpDmeKeyEnc->DmePvtKeyAddr,
			OcpDmeKeyEnc->DmeEncPvtKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_AES_COMPUTE_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function generates the DME response
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	OcpDmeParam	Pointer to XAsu_OcpDme structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if subsystem hash address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_OCP_DME_ALL_PVT_KEYS_REVOKED, if all DME private keys are revoked.
 *	- XASUFW_OCP_DEVICE_ID_CALC_FAIL, if device ID calculation fails.
 *	- XASUFW_OCP_MEASUREMENT_UPDATE_FAIL, if measurement update fails.
 *	- XASUFW_MEM_COPY_FAIL, if memcpy operation fails.
 *	- XASUFW_OCP_SHA_DIGEST_FAIL, if SHA digest generation fails.
 *	- XASUFW_OCP_DME_SIGNATURE_GEN_FAIL, if DME signature generation fails.
 *	- XASUFW_OCP_DME_KEY_DECRYPT_FAIL, if DME private key decryption fails.
 *	- XASUFW_OCP_NONCE_UPDATE_FAIL, if nonce update fails.
 *
 *************************************************************************************************/
s32 XOcp_GenerateDmeResponse(XAsufw_Dma *DmaPtr, const XAsu_OcpDmeParams *OcpDmeParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihDme = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 DmeFipsRegValue = 0U;
	u32 DmeUserKeyAddr = 0U;
	u8 DmeDecPvtKey[XASU_ECC_P384_SIZE_IN_BYTES] = {0U};
	const u8 *IvPtr = (u8*)(UINTPTR)XASU_RTCA_BH_IV_ADDR;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	XSha *ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd ShaCmd;
	u8 DmeKekIv[XASU_OCP_DME_IV_SIZE_IN_BYTES] = {0U};
	const XOcp_DeviceKeys *DevIkDataPtr = NULL;
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};
	u8 IvIncVal = 0U;
	XAsu_OcpDmeResponse *OcpDmeResp =
		(XAsu_OcpDmeResponse *)(UINTPTR)OcpDmeParamsPtr->OcpDmeResponseAddr;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL) || (OcpDmeParamsPtr == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Copy IV to the local buffer. */
	Status = Xil_SMemCpy(DmeKekIv, XASU_OCP_DME_IV_SIZE_IN_BYTES,
			IvPtr, XASU_OCP_DME_IV_SIZE_IN_BYTES, XASU_OCP_DME_IV_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/**
	 * Get the DME encrypted private key address and update the IV with the offset based on
	 * DME revoke bits.
	 */
	DmeFipsRegValue = Xil_In32(EFUSE_CACHE_DME_FIPS_ADDRESS);

	if ((DmeFipsRegValue & EFUSE_CACHE_DME_REVOKE_0_MASK) == 0U) {
		DmeUserKeyAddr = EFUSE_CACHE_USERKEY_0_ADDR;
		IvIncVal = XOCP_DME0_IV_INC_VAL;
	} else if ((DmeFipsRegValue & EFUSE_CACHE_DME_REVOKE_1_MASK) == 0U) {
		DmeUserKeyAddr = EFUSE_CACHE_USERKEY_1_ADDR;
		IvIncVal = XOCP_DME1_IV_INC_VAL;
	} else if ((DmeFipsRegValue & EFUSE_CACHE_DME_REVOKE_2_MASK) == 0U) {
		DmeUserKeyAddr = EFUSE_CACHE_USERKEY_2_ADDR;
		IvIncVal = XOCP_DME2_IV_INC_VAL;
	} else {
		Status = XASUFW_OCP_DME_ALL_PVT_KEYS_REVOKED;
		goto END;
	}
	XOcp_IncrementIv(DmeKekIv, IvIncVal);

	Status = XOcp_DecryptPvtKey(DmaPtr, DmeUserKeyAddr, DmeKekIv, DmeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_KEY_DECRYPT_FAIL);
		goto END_CLR;
	}

	/** Get the DevIk structure pointer. */
	DevIkDataPtr = XOcp_GetDevIk();

	/** Device ID update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (DevIkDataPtr->IsDevIkKeyReady == XASU_TRUE) {
		/**
		 * - If DevIk public key is generated, DeviceId is SHA2-384(DevIk_public_key).
		 */
		ShaCmd.ShaMode = XASU_SHA_MODE_384;
		ShaCmd.DataAddr = (u64)(UINTPTR)(DevIkDataPtr->EccX);
		ShaCmd.DataSize = XASUFW_DOUBLE_VALUE(XASU_ECC_P384_SIZE_IN_BYTES);
		ShaCmd.HashAddr = (u64)(UINTPTR)OcpDmeResp->Dme.DeviceId;
		ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	} else {
		/** - Else, DeviceId shall be all 0's. */
		Status = Xil_SMemSet((u8 *)OcpDmeResp->Dme.DeviceId, XASU_ECC_P384_SIZE_IN_BYTES, 0U,
			XASU_ECC_P384_SIZE_IN_BYTES);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DEVICE_ID_CALC_FAIL);
		goto END_CLR;
	}

	/** Nonce update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OcpDmeParamsPtr->NonceAddr,
			(u64)(UINTPTR)OcpDmeResp->Dme.Nonce, XASU_OCP_DME_NONCE_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_NONCE_UPDATE_FAIL;
		goto END_CLR;
	}

	/** Measurement update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PMC_GLOBAL_HW_PCR_0_ADDR,
			(u64)(UINTPTR)OcpDmeResp->Dme.Measurement, XASU_ECC_P384_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_MEASUREMENT_UPDATE_FAIL;
		goto END_CLR;
	}

	/** Calculate hash of the fields DeviceId, Nonce and Measurement using SHA2-384. */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = (u64)(UINTPTR)OcpDmeResp->Dme.DeviceId;
	ShaCmd.DataSize = sizeof(XAsu_OcpDme);
	ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
	ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SHA_DIGEST_FAIL);
		goto END_CLR;
	}

	/** Generate signature for DME. */
	Status = XEcc_GenerateSignature(EccInstancePtr, DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)DmeDecPvtKey,
			NULL, (u64)(UINTPTR)HashBuf, XASU_SHA_384_HASH_LEN,
			(u64)(UINTPTR)OcpDmeResp->DmeSignatureR);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_SIGNATURE_GEN_FAIL);
	}

END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihDme, ClearStatus, (u8 *)(UINTPTR)DmeDecPvtKey,
					XASU_ECC_P384_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)DmeKekIv, XASU_OCP_DME_IV_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the AES encrypt/decrypt operation as part of OCP.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	DmeUserKeyAddr	Address of DME user key
 * @param	Iv		Pointer to the IV (Initialization Vector).
 * @param	DmeDecPvtKey	Pointer to store the decrypted key.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES operation is successful.
 * 	- XASUFW_MEM_COPY_FAIL, if mem copy fails.
 * 	- XASUFW_OCP_DME_KEK_NOT_PRESENT, if DME KEK is not present.
 * 	- XASUFW_OCP_DME_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 * 	- XASUFW_ZEROIZE_MEMSET_FAIL, if mem set fails.
 * 	- XASUFW_OCP_DME_AES_COMPUTE_FAIL, if AES compute fails.
 * 	- XASUFW_OCP_DME_KEY_DECRYPT_FAIL, if key decryption fails.
 * 	- Errors codes from AES, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 DmeUserKeyAddr, u8* Iv, u8* DmeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 ZeroData[XASU_OCP_DME_KEY_SIZE_IN_BYTES] = {0U};
	u8 DmeEncPvtKey[XASU_OCP_DME_KEY_SIZE_IN_BYTES] = {0U};
	volatile u32 Index = 0U;

	/** Check if DME KEK is present. */
	if (DmeKekFlag == XASU_FALSE) {
		Status = XASUFW_OCP_DME_KEK_NOT_PRESENT;
		goto END;
	}

	/** Copy the DME user key data to the local buffer. */
	Status = Xil_SMemCpy(DmeEncPvtKey, XASU_OCP_DME_KEY_SIZE_IN_BYTES,
			(u8*)(UINTPTR)DmeUserKeyAddr, XASU_OCP_DME_KEY_SIZE_IN_BYTES,
			XASU_OCP_DME_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Change endianness of DME encrypted private key. */
	Status = XAsufw_ChangeEndianness(DmeEncPvtKey, XASU_OCP_DME_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_DME_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/* Zeroize the zero data buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureZeroize(ZeroData, XASU_OCP_DME_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * Decrypt the DME encrypted private key.
	 *  - Encrypt the zero data with the DME KEK and updated IV.
	 *  - XOR the resultant data with the DME user key data (DME encrypted private key).
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)Iv, (u64)(UINTPTR)ZeroData,
			(u64)(UINTPTR)DmeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_AES_COMPUTE_FAIL);
		goto END;
	}
	for (Index = 0U; Index < XASU_OCP_DME_KEY_SIZE_IN_BYTES; Index++) {
		DmeDecPvtKey[Index] ^= DmeEncPvtKey[Index];
	}

	if (Index != XASU_OCP_DME_KEY_SIZE_IN_BYTES) {
		Status = XASUFW_OCP_DME_KEY_DECRYPT_FAIL;
	}

END:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)DmeEncPvtKey, XASU_OCP_DME_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the AES encrypt/decrypt operation as part of OCP.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	IvAddr		Pointer to the IV (Initialization Vector) address.
 * @param	InAddr		Pointer to the input data address.
 * @param	OutAddr		Pointer to the output data address.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES operation is successful.
 * 	- XASUFW_OCP_AES_WRITE_KEY_FAILURE, if AES write key fails.
 * 	- Errors codes from AES, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XAsu_AesParams AesParams;
	XAsu_AesKeyObject KeyObject;
	u8 TagBuf[XASU_OCP_DME_TAG_SIZE_IN_BYTES];
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	KeyObject.KeySize = (u32)XASU_AES_KEY_SIZE_256_BITS;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeyAddress = (u64)(UINTPTR)(XOcp_DmeKek);

	/** Write AES key - DME KEK for encryption and decryption of DME private keys. */
	Status = XAes_WriteKey(AesInstancePtr, DmaPtr, (u64)(UINTPTR)&KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_AES_WRITE_KEY_FAILURE);
		goto END;
	}

	/* Initialize AES parameters structure for encryption/decryption operation. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObject;
	AesParams.IvAddr = (u64)(UINTPTR)IvAddr;
	AesParams.IvLen = XASU_OCP_DME_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)InAddr;
	AesParams.OutputDataAddr = (u64)(UINTPTR)OutAddr;
	AesParams.DataLen = XASU_OCP_DME_KEY_SIZE_IN_BYTES;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XASU_OCP_DME_TAG_SIZE_IN_BYTES;

	/** Perform AES operation based on the operation type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_Compute(AesInstancePtr, DmaPtr, &AesParams);

END:
	/** Clear the key written to the XASU_AES_USER_KEY_7 key source. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(AesInstancePtr, KeyObject.KeySrc));

	/** Zeroize the key object. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Zeroize the local tag buffer. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)TagBuf, XASU_OCP_DME_TAG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the IV for encryption of DME keys
 * 		after incrementing.
 *
 * @param	Iv		- Iv to be incremented.
 * @param	IncVal		- Increment value.
 *
 *************************************************************************************************/
static void XOcp_IncrementIv(u8* Iv, u8 IncVal)
{
	u8 *IvPtr = Iv;
	u32 Carry = IncVal;
	u32 Result;
	s32 Index;

	/**
	 * IV increment is done as below:
	 * Repeat I = 0 to 11 OR till Carry becomes zero.
	 * Get (Iv[I], carry) by performing Iv[I] + carry.
	 */
	for (Index = (s32)(XASU_OCP_DME_IV_SIZE_IN_BYTES - 1); Index >= 0; Index--) {
		Result = IvPtr[Index] + Carry;
		IvPtr[Index] = (u8)(Result & XASUFW_LSB_MASK_VALUE);
		Carry = Result >> XASUFW_ONE_BYTE_SHIFT_VALUE;
		/** If carry is non zero continue else break. */
		if (Carry == 0U) {
			break;
		}
	}
}
#endif /* XASU_OCP_ENABLE */
/** @} */