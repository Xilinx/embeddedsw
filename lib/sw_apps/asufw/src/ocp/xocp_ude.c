/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_ude.c
*
* This file contains the implementation of the interface functions for OCP UDE(Unique Device
* Endorsement) functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   yog  08/21/25 Initial release
*       yog  04/17/26 Updated code to zeroize UDE KEK immediately after use to avoid exposure in memory.
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xocp_ude_server_apis OCP UDE server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xasufw_hw.h"
#include "xasufw_status.h"
#include "xasufw_trnghandler.h"
#include "xasufw_util.h"
#include "xecc.h"
#include "xocp_ude.h"
#include "xocp.h"
#include "xaes.h"
#include "xkdf.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasu_generic.h"
#include "xasu_def.h"
#include "xil_sutil.h"
#include "xasufw_memory.h"
#include "xrsa_ecc.h"
#include "xasufw_perf.h"

#ifdef XASU_OCP_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XOCP_UDE_KEK_SIZE_IN_BYTES	(32U)	/**< UDE KEK size in bytes */
#define XOCP_UDE_CONTEXT		"UDE_ENCRYPTION_KEY" /**< UDE context */
#define XOCP_UDE_CONTEXT_LEN		(18U)		/**< UDE context length */
#define XOCP_UDE0_IV_INC_VAL		(0x02U)	/**< UDE0 IV increment value */
#define XOCP_UDE1_IV_INC_VAL		(0x03U)	/**< UDE1 IV increment value */
#define XOCP_UDE2_IV_INC_VAL		(0x04U)	/**< UDE2 IV increment value */
#define XOCP_DEC_BLACK_KEY_IV_INC_VAL	(0x10U)	/**< UDE decryption black key IV increment value */
#define XASU_OCP_UDE_TAG_SIZE_IN_BYTES	(16U)	/**< UDE tag size in bytes */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr);
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 UdeUserKeyAddr, u8* Iv, u8* UdeDecPvtKey);
static s32 XOcp_GetActiveUdeKeyInfo(u32 *UdeUserKeyPtr, u8 *IvIncValPtr);
static s32 XOcp_BuildUdeResponseData(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey,
				     XAsu_OcpUdeResponse *OcpUdeResp,
				     const XAsu_OcpUdeParams *OcpUdeParamsPtr);
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey);
static s32 XOcp_ProcessUdeResponse(XAsufw_Dma *DmaPtr, XAsu_OcpUdeResponse *OcpUdeResp,
				   const XAsu_OcpUdeParams *OcpUdeParamsPtr);
static s32 XOcp_GenerateUdeKek(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *OutputKek);

/*************************************************************************************************/
/**
 * @brief	This function generates the UDE KEK by decrypting the eFUSE black key using the PUF KEK
 * 		and applying a Key Derivation Function (KDF) to the result.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	OutputKek	Pointer to output buffer where KEK will be written.
 * 				Caller is responsible for zeroizing this buffer after use.
 *
 * @return
 *		- XASUFW_SUCCESS, if UDE KEK generation is successful.
 *		- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateUdeKek(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *OutputKek)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const char *CtxStr = XOCP_UDE_CONTEXT;
	const u8 *Context = (const u8 *)CtxStr;

	/** Generate UDE KEK from eFUSE black key using PUF KEK and KDF. */
	Status = XKdf_GenerateKekFromEfuse0(DmaPtr, AesInstancePtr, Context,
					    XOCP_UDE_CONTEXT_LEN,
					    XOCP_DEC_BLACK_KEY_IV_INC_VAL,
					    XOCP_UDE_KEK_SIZE_IN_BYTES,
					    OutputKek);

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function generates a UDE private key, encrypts it with the UDE KEK,
 * 		and returns the encrypted key based on the provided key ID.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeKeyEnc	Pointer to the XAsu_OcpUdeKeyEncrypt structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE key encryption is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_OCP_UDE_IV_COPY_FAIL, if UDE IV copy operation fails.
 *	- XASUFW_OCP_UDE_AES_COMPUTE_FAIL, if UDE key encryption operation fails.
 *	- XASUFW_OCP_UDE_PVT_KEY_GEN_FAIL, if UDE private key generation fails.
 *
 *************************************************************************************************/
s32 XOcp_EncryptUdeKeys(XAsufw_Dma *DmaPtr, const XAsu_OcpUdeKeyEncrypt *OcpUdeKeyEnc)
{
	/**
	 * Capture the start time of the OCP UDE key encryption operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_OCP_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	const u8 *IvPtr = (const u8*)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	u8 UdeKekIv[XASU_OCP_UDE_IV_SIZE_IN_BYTES] = {0U};
	u8 PvtKey[XASU_OCP_UDE_KEY_SIZE_IN_BYTES] = {0U};
	u8 IvIncVal = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (OcpUdeKeyEnc == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Validate key address is non-zero. */
	if (OcpUdeKeyEnc->UdeEncPvtKeyAddr == 0U) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Copy IV from RTCA to local buffer. */
	Status = Xil_SMemCpy(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvPtr,
			XASU_OCP_UDE_IV_SIZE_IN_BYTES, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Check if IV is non-zero. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_IV_IS_ZERO;
		goto END;
	}
	/**
	 * Based on UDE key ID, update the KEK IV with the offset to the local buffer.
	 */
	/*
	 * | UDE key ID | Offset  |
	 * |     0      |    2    |
	 * |     1      |    3    |
	 * |     2      |    4    |
	 */
	switch (OcpUdeKeyEnc->UdeKeyId) {
		case XASU_OCP_UDE_USER_KEY_0_ID:
			IvIncVal = XOCP_UDE0_IV_INC_VAL;
			break;
		case XASU_OCP_UDE_USER_KEY_1_ID:
			IvIncVal = XOCP_UDE1_IV_INC_VAL;
			break;
		case XASU_OCP_UDE_USER_KEY_2_ID:
			IvIncVal = XOCP_UDE2_IV_INC_VAL;
			break;
		default:
			Status = XASUFW_OCP_INVALID_PARAM;
			goto END;
			break;
	}
	Xil_IncrementBuffer(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvIncVal);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePvtKey(XASU_ECC_NIST_P384, XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
				PvtKey, NULL, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_PVT_KEY_GEN_FAIL;
		goto END_CLR;
	}

	/** Encrypt UDE private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)UdeKekIv, (u64)(UINTPTR)PvtKey,
			OcpUdeKeyEnc->UdeEncPvtKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_AES_COMPUTE_FAIL);
	}

	/**
	 * Measure and print the performance time for the OCP UDE key encryption operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

END_CLR:
	/** Zeroize local buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, PvtKey,
					XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function generates the UDE response
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeParamsPtr	Pointer to XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if subsystem hash address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED, if all UDE private keys are revoked.
 *	- XASUFW_OCP_DEVICE_ID_CALC_FAIL, if device ID calculation fails.
 *	- XASUFW_OCP_MEASUREMENT_UPDATE_FAIL, if measurement update fails.
 *	- XASUFW_MEM_COPY_FAIL, if memcpy operation fails.
 *	- XASUFW_OCP_SHA_DIGEST_FAIL, if SHA digest generation fails.
 *	- XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, if UDE signature generation fails.
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, if UDE private key decryption fails.
 *	- XASUFW_OCP_NONCE_UPDATE_FAIL, if nonce update fails.
 *
 *************************************************************************************************/
s32 XOcp_GenerateUdeResponse(XAsufw_Dma *DmaPtr, const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	/**
	 * Capture the start time of the OCP UDE response generation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_OCP_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_OcpUdeResponse *OcpUdeResp = NULL;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (OcpUdeParamsPtr == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	if ((OcpUdeParamsPtr->OcpUdeResponseAddr == 0U) || (OcpUdeParamsPtr->NonceAddr == 0U)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	OcpUdeResp = (XAsu_OcpUdeResponse *)(UINTPTR)OcpUdeParamsPtr->OcpUdeResponseAddr;

	/** Perform core UDE response generation. */
	Status = XOcp_ProcessUdeResponse(DmaPtr, OcpUdeResp, OcpUdeParamsPtr);

	/**
	 * Measure and print the performance time for the OCP UDE response generation operation,
	 * if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts the UDE encrypted private key using UDE KEK.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeUserKeyAddr	Address of UDE user key
 * @param	Iv		Pointer to the IV (Initialization Vector).
 * @param	UdeDecPvtKey	Pointer to store the decrypted key.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES operation is successful.
 * 	- XASUFW_MEM_COPY_FAIL, if mem copy fails.
 * 	- XASUFW_OCP_UDE_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 * 	- XASUFW_ZEROIZE_MEMSET_FAIL, if mem set fails.
 * 	- XASUFW_OCP_UDE_AES_COMPUTE_FAIL, if AES compute fails.
 * 	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, if key decryption fails.
 * 	- Errors codes from AES, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 UdeUserKeyAddr, u8* Iv, u8* UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 ZeroData[XASU_OCP_UDE_KEY_SIZE_IN_BYTES];
	u8 UdeEncPvtKey[XASU_OCP_UDE_KEY_SIZE_IN_BYTES] = {0U};
	volatile u32 Index = 0U;

	/** Copy the UDE user key data to the local buffer. */
	Status = Xil_SMemCpy(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES,
			(u8*)(UINTPTR)UdeUserKeyAddr, XASU_OCP_UDE_KEY_SIZE_IN_BYTES,
			XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Change endianness of UDE encrypted private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/* Zeroize the zero data buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureZeroize(ZeroData, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * Decrypt the UDE encrypted private key.
	 *  - Encrypt the zero data with the UDE KEK and updated IV.
	 *  - XOR the resultant data with the UDE user key data (UDE encrypted private key).
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)Iv, (u64)(UINTPTR)ZeroData,
			(u64)(UINTPTR)UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_AES_COMPUTE_FAIL);
		goto END;
	}
	for (Index = 0U; Index < XASU_OCP_UDE_KEY_SIZE_IN_BYTES; Index++) {
		UdeDecPvtKey[Index] ^= UdeEncPvtKey[Index];
	}

	if (Index != XASU_OCP_UDE_KEY_SIZE_IN_BYTES) {
		Status = XASUFW_OCP_UDE_KEY_DECRYPT_FAIL;
	}

END:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the AES encrypt/decrypt operation as part of OCP.
 * 		- Regenerates UDE KEK on-demand for each operation.
 * 		- Immediately zeroizes KEK after use so that it's lifetime is scoped to a
 * 		  single AES operation.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	IvAddr		Pointer to the IV (Initialization Vector) address.
 * @param	InAddr		Pointer to the input data address.
 * @param	OutAddr		Pointer to the output data address.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES operation is successful.
 * 	- XASUFW_OCP_UDE_KEK_GEN_FAIL, if UDE KEK generation fails.
 * 	- XASUFW_OCP_UDE_PUF_KEK_GEN_FAIL, if PUF KEK generation was not successful.
 * 	- XASUFW_OCP_UDE_KEK_NOT_PRESENT, if generated UDE KEK buffer is all zeros.
 * 	- XASUFW_OCP_AES_WRITE_KEY_FAILURE, if AES write key fails.
 * 	- Error codes from AES, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XAsu_AesParams AesParams;
	XAsu_AesKeyObject KeyObject;
	u8 TagBuf[XASU_OCP_UDE_TAG_SIZE_IN_BYTES];
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdeKek[XOCP_UDE_KEK_SIZE_IN_BYTES] = {0U};
	u32 PufKekStatus = XAsufw_GetPufKekGenStatus();

	/** Generate UDE KEK on-demand if PUF KEK generation is successful. */
	if (PufKekStatus == XASUFW_PUF_KEK_GEN_SUCCESS) {
		Status = XOcp_GenerateUdeKek(DmaPtr, AesInstancePtr, UdeKek);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_KEK_GEN_FAIL);
			goto END;
		}
	} else {
		Status = XASUFW_OCP_UDE_PUF_KEK_GEN_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Check if the UDE KEK buffer is non-zero. */
	Status = XAsu_IsBufferNonZero(UdeKek, XOCP_UDE_KEK_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_KEK_NOT_PRESENT;
		goto END;
	}

	KeyObject.KeySize = (u32)XASU_AES_KEY_SIZE_256_BITS;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeyAddress = (u64)(UINTPTR)(UdeKek);

	/** Write AES key - UDE KEK for encryption and decryption of UDE private keys. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_WriteKey(AesInstancePtr, DmaPtr, &KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_AES_WRITE_KEY_FAILURE);
		goto END_CLR;
	}

	/**
	 * Initialize AES parameters structure. AES compute is always performed in encrypt mode
	 * for both encryption and decryption of UDE private keys.
	 * Decryption of the UDE encrypted private key is as follows:
	 *  - Encrypt the zero data with the UDE KEK and updated IV.
	 *  - XOR the resultant data with the UDE user key data (UDE encrypted private key).
	 */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObject;
	AesParams.IvAddr = (u64)(UINTPTR)IvAddr;
	AesParams.IvLen = XASU_OCP_UDE_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)InAddr;
	AesParams.OutputDataAddr = (u64)(UINTPTR)OutAddr;
	AesParams.DataLen = XASU_OCP_UDE_KEY_SIZE_IN_BYTES;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XASU_OCP_UDE_TAG_SIZE_IN_BYTES;

	/** Perform AES operation based on the operation type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_Compute(AesInstancePtr, DmaPtr, &AesParams);

END_CLR:

	/** Clear the key written to the XASU_AES_USER_KEY_7 key source. */
	XFIH_CALL(XAes_KeyClear, XFihUde, ClearStatus, AesInstancePtr, KeyObject.KeySrc);
	Status = XAsufw_UpdateErrorStatus(Status, ClearStatus);

	/** Zeroize the key object. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	/**
	 * Immediately zeroize the UDE KEK from memory.
	 * This ensures the KEK only existed for the duration of this single operation.
	 */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, UdeKek, XOCP_UDE_KEK_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function checks the revocation status of UDE private keys and gets the active
 * 		UDE key information (Key address and IV increment value).
 *
 * @param	UdeUserKeyPtr	Pointer to the variable that will hold active UDE user key address.
 * @param	IvIncValPtr	Pointer to the variable that will hold IV increment value.
 *
 * @return
 *	- XASUFW_SUCCESS, if active UDE key information is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XOcp_GetActiveUdeKeyInfo(u32 *UdeUserKeyPtr, u8 *IvIncValPtr)
{
	s32 Status = XASUFW_FAILURE;
	XFih_Var UdeFipsRegValue = XFih_VolatileAssignU32(EFUSE_CACHE_UDE_REVOKE_ALL_MASK);

	/** Get the UDE revocation bits from the FIPS register with FIH protection. */
	UdeFipsRegValue = XFih_VolatileAssignU32(Xil_In32(EFUSE_CACHE_UDE_FIPS_ADDRESS));

	/** If all keys are revoked, return error. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_ALL_MASK, ==,
	                          EFUSE_CACHE_UDE_REVOKE_ALL_MASK) {
		Status = XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED;
		goto END;
	}

	/** Check UDE key 0 revocation status and get the UDE key information if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_0_MASK, ==, 0U) {
		*UdeUserKeyPtr = EFUSE_CACHE_USERKEY_0_ADDR;
		*IvIncValPtr = XOCP_UDE0_IV_INC_VAL;
		Status = XASUFW_SUCCESS;
		XFIH_GOTO(END);
	}

	/** If UDE key 0 is revoked, get the UDE key information for the UDE key 1 if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_1_MASK, ==, 0U) {
		*UdeUserKeyPtr = EFUSE_CACHE_USERKEY_1_ADDR;
		*IvIncValPtr = XOCP_UDE1_IV_INC_VAL;
		Status = XASUFW_SUCCESS;
		XFIH_GOTO(END);
	}

	/** If UDE key 0 and 1 are revoked, get the UDE key information for the UDE key 2 if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_2_MASK, ==, 0U) {
		*UdeUserKeyPtr = EFUSE_CACHE_USERKEY_2_ADDR;
		*IvIncValPtr = XOCP_UDE2_IV_INC_VAL;
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function populates the UDE response fields (Device ID, Nonce, Measurement),
 *		calculates the hash over them, and generates the UDE signature.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	UdeDecPvtKey		Pointer to the decrypted UDE private key.
 * @param	OcpUdeResp		Pointer to the UDE response structure to populate.
 * @param	OcpUdeParamsPtr		Pointer to the XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE response data population and signing is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_DEVICE_ID_CALC_FAIL, if device ID calculation fails.
 *	- XASUFW_OCP_NONCE_UPDATE_FAIL, if nonce update fails.
 *	- XASUFW_OCP_MEASUREMENT_UPDATE_FAIL, if measurement update fails.
 *	- XASUFW_OCP_SHA_DIGEST_FAIL, if SHA digest generation fails.
 *	- XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, if UDE signature generation fails.
 *
 *************************************************************************************************/
static s32 XOcp_BuildUdeResponseData(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey,
				     XAsu_OcpUdeResponse *OcpUdeResp,
				     const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	XSha *ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd ShaCmd;
	const XOcp_DeviceKeys *DevIkDataPtr = NULL;
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};

	/** Get the DevIk structure pointer. */
	DevIkDataPtr = XOcp_GetDevIk();

	/** Device ID update. */
	if (DevIkDataPtr->IsDevIkKeyReady == XASU_TRUE) {
		/**
		 * - If DevIk public key is generated, DeviceId is SHA2-384(DevIk_public_key).
		 */
		ShaCmd.ShaMode = XASU_SHA_MODE_384;
		ShaCmd.DataAddr = (u64)(UINTPTR)(DevIkDataPtr->EccX);
		ShaCmd.DataSize = XASUFW_DOUBLE_VALUE(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
		ShaCmd.HashAddr = (u64)(UINTPTR)OcpUdeResp->Ude.DeviceId;
		ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	} else {
		/** - Else, DeviceId shall be all 0's. */
		Status = Xil_SMemSet((u8 *)OcpUdeResp->Ude.DeviceId,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, 0U,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DEVICE_ID_CALC_FAIL);
		goto END;
	}

	/** Nonce update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OcpUdeParamsPtr->NonceAddr,
			(u64)(UINTPTR)OcpUdeResp->Ude.Nonce, XASU_OCP_UDE_NONCE_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_NONCE_UPDATE_FAIL;
		goto END;
	}

	/** Measurement update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PMC_GLOBAL_HW_PCR_0_ADDR,
			(u64)(UINTPTR)OcpUdeResp->Ude.Measurement,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_MEASUREMENT_UPDATE_FAIL;
		goto END;
	}

	/** Calculate hash of the fields DeviceId, Nonce and Measurement using SHA2-384. */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = (u64)(UINTPTR)OcpUdeResp->Ude.DeviceId;
	ShaCmd.DataSize = sizeof(XAsu_OcpUde);
	ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
	ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SHA_DIGEST_FAIL);
		goto END;
	}

	/** Generate signature for UDE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenerateSignature(EccInstancePtr, DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdeDecPvtKey,
			NULL, (u64)(UINTPTR)HashBuf, XASU_SHA_384_HASH_LEN,
			(u64)(UINTPTR)OcpUdeResp->UdeSignatureR);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL);
	}

END:
	/** Zeroize the local hash buffer. */
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function prepares the decrypted UDE private key by copying the IV, checking
 *		key revocation status, validating eFUSE programming, and decrypting the key.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeKekIv	Pointer to the IV buffer (pre-populated with BH IV).
 * @param	UdeDecPvtKey	Pointer to the buffer to store the decrypted private key.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE key preparation is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED, if all UDE private keys are revoked.
 *	- XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED, if UDE key is not programmed into eFUSEs.
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, if UDE private key decryption fails.
 *
 *************************************************************************************************/
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	/** Check UDE key revocation status and get the key address and Iv increment value. */
	Status = XOcp_GetActiveUdeKeyInfo(&UdeUserKeyAddr, &IvIncVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Return error if UDE encrypted key is not programmed into eFUSEs. */
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)UdeUserKeyAddr, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Increment IV based on the UDE revoke bits. */
	Xil_IncrementBuffer(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvIncVal);

	/** Decrypt the UDE encrypted private key from eFUSEs. */
	Status = XOcp_DecryptPvtKey(DmaPtr, UdeUserKeyAddr, UdeKekIv, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_KEY_DECRYPT_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function performs the core UDE response generation operations including
 *		IV copy, key preparation, public key generation, and response data building.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeResp		Pointer to the UDE response structure to populate.
 * @param	OcpUdeParamsPtr		Pointer to the XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE response generation is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_MEM_COPY_FAIL, if IV copy fails.
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, if UDE private key decryption fails.
 *	- XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, if UDE public key generation fails.
 *
 *************************************************************************************************/
static s32 XOcp_ProcessUdeResponse(XAsufw_Dma *DmaPtr, XAsu_OcpUdeResponse *OcpUdeResp,
				   const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 UdeDecPvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 UdeKekIv[XASU_OCP_UDE_IV_SIZE_IN_BYTES] = {0U};
	const u8 *IvPtr = (const u8*)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** Copy IV from RTCA to local buffer. */
	Status = Xil_SMemCpy(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvPtr,
			XASU_OCP_UDE_IV_SIZE_IN_BYTES, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Check if IV is non-zero. */
	Status = XAsu_IsBufferNonZero(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_IV_IS_ZERO;
		goto END;
	}

	/** Prepare the decrypted UDE private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_PrepareUdeDecKey(DmaPtr, UdeKekIv, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL);
		goto END;
	}

	/** Generate the UDE public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GeneratePublicKey(EccInstancePtr, DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdeDecPvtKey,
			(u64)(UINTPTR)OcpUdeResp->UdePublicKeyX);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL);
		goto END;
	}

	/** Populate UDE response fields and generate signature. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_BuildUdeResponseData(DmaPtr, UdeDecPvtKey, OcpUdeResp, OcpUdeParamsPtr);

END:
	/** Zeroize local buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, UdeDecPvtKey,
					XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

#endif /* XASU_OCP_ENABLE */
/** @} */
