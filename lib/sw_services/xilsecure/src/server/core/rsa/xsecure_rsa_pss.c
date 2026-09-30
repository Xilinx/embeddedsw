/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/rsa/xsecure_rsa_pss.c
*
* This file contains the implementation of the RSA-PSS (SHA3-384) signature
* verification server APIs.
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis XilSecure RSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_RSA_EXCLUDE

#include "xsecure_rsa_pss.h"
#include "xsecure_rsa.h"
#include "xsecure_mgf.h"
#include "xil_sutil.h"

/************************** Constant Definitions *****************************/
#define XSECURE_RSA_PSS_MSB_PAD_MASK	((u8)0x80U) /**< RSA PSS MSB padding mask */
#define XSECURE_RSA_PSS_SIG_TRAIL_BYTE	((u8)0xBCU) /**< RSA PSS signature trailer byte */
#define XSECURE_RSA_PSS_SALT_SEP_BYTE	((u8)0x01U) /**< DB padding / salt separator */
#define XSECURE_RSA_PSS_SALT_LEN	(XSECURE_SHA_384_HASH_SIZE_IN_BYTES) /**< Salt length */
#define XSECURE_RSA_PSS_TRAILER_LEN	(1U) /**< Length of the 0xBC trailer in EM */
#define XSECURE_RSA_PSS_SALT_SEP_LEN	(1U) /**< Length of the 0x01 separator in DB */
#define XSECURE_RSA_PSS_PADDING1_LEN	(8U) /**< I2OSP padding length */
#define XSECURE_RSA_PSS_MIN_EM_LEN	(XSECURE_SHA_384_HASH_SIZE_IN_BYTES + \
					 XSECURE_RSA_PSS_SALT_LEN + \
					 XSECURE_RSA_PSS_TRAILER_LEN + \
					 XSECURE_RSA_PSS_SALT_SEP_LEN)
	/**< Minimum EM length: hLen + sLen + trailer + salt separator */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static const u8 XSecure_RsaPssPadding1[XSECURE_RSA_PSS_PADDING1_LEN]
	__attribute__((aligned(32U))) = {0U};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function verifies an RSA-PSS signature (SHA3-384) for the
 *		given message hash.
 *
 * @param	Input	Pointer to the XSecure_RsaPssSignVerifyInput structure
 *
 * @return
 *		 - XST_SUCCESS - If verification was successful
 *		 - XSECURE_RSA_PSS_ERR_INVALID - On invalid arguments
 *		 - XSECURE_RSA_PSS_ERR_MEMSET_ENC - If signed input data memset fails
 *		 - XSECURE_RSA_PSS_ERR_RSA_ENC - If RSA encryption fails
 *		 - XSECURE_RSA_PSS_ERR_MGF - If mask generation fails
 *		 - XSECURE_RSA_PSS_ERR_SHA - If SHA operation fails
 *		 - XSECURE_RSA_PSS_ERR_BC - If signature trailer byte mismatch
 *		 - XSECURE_RSA_PSS_ERR_MSDB - If masked DB MSB mismatch
 *		 - XSECURE_RSA_PSS_ERR_DB - If DB pattern mismatch
 *		 - XSECURE_RSA_PSS_ERR_HASH_CMP - If hash comparison fails
 *		 - XSECURE_RSA_PSS_ERR_CLEAR - If clearing the local hash buffer fails
 *
 ******************************************************************************/
int XSecure_RsaPssSignVerify(const XSecure_RsaPssSignVerifyInput *Input)
{
	volatile int Status = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile u8 HashTmp;
	volatile u32 Index = 0U;
	volatile u32 IndexTmp;
	u32 KeySize;
	u32 MaskedDbLen;
	u32 DbLen;
	u8 *EmWorkspace;
	u8 *DbWorkspace;
	XSecure_Sha *ShaInstPtr;
	const u8 *EmHashPtr;
	u8 EmHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES] __attribute__ ((aligned(32U)));
	XSecure_MgfInput MgfInput;

	/** - Validate the input arguments */
	if ((Input == NULL) || (Input->MsgHashAddr == 0U) || (Input->SignAddr == 0U) ||
		(Input->RsaInstPtr == NULL) || (Input->ShaInstPtr == NULL) ||
		(Input->SignedInputData == NULL) || (Input->DataBlock == NULL)) {
		Status = (int)XSECURE_RSA_PSS_ERR_INVALID;
		goto END;
	}

	EmWorkspace = Input->SignedInputData;
	DbWorkspace = Input->DataBlock;
	ShaInstPtr = Input->ShaInstPtr;
	KeySize = Input->KeySize;

	/** - EM must fit hLen + sLen + 0x01 + 0xBC and the caller workspace. */
	if ((KeySize < XSECURE_RSA_PSS_MIN_EM_LEN) ||
		(KeySize > XSECURE_RSA_PSS_SIGNED_INPUT_DATA_LEN)) {
		Status = (int)XSECURE_RSA_PSS_ERR_INVALID;
		goto END;
	}

	/* EM = maskedDB || H || 0xBC. DB = PS || 0x01 || salt. */
	MaskedDbLen = KeySize - XSECURE_SHA_384_HASH_SIZE_IN_BYTES -
			XSECURE_RSA_PSS_TRAILER_LEN;
	DbLen = KeySize - XSECURE_RSA_PSS_MIN_EM_LEN;
	if (MaskedDbLen > XSECURE_RSA_PSS_DATA_BLOCK_LEN) {
		Status = (int)XSECURE_RSA_PSS_ERR_INVALID;
		goto END;
	}

	/** - Clear the EM workspace before writing the RSA output. */
	Status = Xil_SMemSet(EmWorkspace, KeySize, 0U, KeySize);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_PSS_ERR_MEMSET_ENC;
		goto END;
	}

	/**
	 * - Recover EM with RSA public-key exponentiation (s^e mod n).
	 *   The 64-bit API is used so a signature in client DDR is never
	 *   truncated on a 32-bit PLM.
	 */
	Status = XSecure_RsaPublicEncrypt_64Bit(Input->RsaInstPtr, Input->SignAddr,
		KeySize, (u64)(UINTPTR)EmWorkspace);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_PSS_ERR_RSA_ENC;
		goto END;
	}

	/** - Rightmost EM octet must be the PSS trailer 0xBC. */
	if (EmWorkspace[KeySize - 1U] != XSECURE_RSA_PSS_SIG_TRAIL_BYTE) {
		Status = (int)XSECURE_RSA_PSS_ERR_BC;
		goto END;
	}

	/** - Leftmost bits of maskedDB that are outside emBits must be zero. */
	if ((EmWorkspace[0U] & XSECURE_RSA_PSS_MSB_PAD_MASK) != 0U) {
		Status = (int)XSECURE_RSA_PSS_ERR_MSDB;
		goto END;
	}

	/** - Split EM: H is the hLen octets before the trailer. */
	EmHashPtr = &EmWorkspace[MaskedDbLen];

	Status = Xil_SMemCpy(EmHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, EmHashPtr,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

	/**
	 * - dbMask = MGF(H, maskedDBLen), then DB = maskedDB XOR dbMask.
	 */
	MgfInput.Seed = EmHash;
	MgfInput.SeedLen = XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
	MgfInput.Output = DbWorkspace;
	MgfInput.OutputLen = MaskedDbLen;
	Status = XSecure_MaskGenFunc(XSECURE_SHA3_384, ShaInstPtr, &MgfInput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_PSS_ERR_MGF;
		goto END;
	}

	for (Index = 0U; Index < MaskedDbLen; Index++) {
		DbWorkspace[Index] = (u8)(DbWorkspace[Index] ^ EmWorkspace[Index]);
	}
	if (Index != MaskedDbLen) {
		Status = (int)XSECURE_RSA_PSS_ERR_DB;
		goto END;
	}
	IndexTmp = Index;

	/** - Clear the leftmost bits of DB that are outside emBits. */
	if (DbLen != 0U) {
		DbWorkspace[0U] = (u8)(DbWorkspace[0U] & (u8)(~XSECURE_RSA_PSS_MSB_PAD_MASK));
	}

	/** - PS in DB must be zeros, followed by the 0x01 salt separator. */
	for (Index = 0U; Index < DbLen; Index++) {
		if (DbWorkspace[Index] != 0U) {
			Status = (int)XSECURE_RSA_PSS_ERR_DB;
			goto END;
		}
	}
	HashTmp = DbWorkspace[Index];
	if ((IndexTmp != MaskedDbLen) || (Index != DbLen) ||
		(DbWorkspace[Index] != XSECURE_RSA_PSS_SALT_SEP_BYTE) ||
		(HashTmp != XSECURE_RSA_PSS_SALT_SEP_BYTE)) {
		Status = (int)XSECURE_RSA_PSS_ERR_DB;
		goto END;
	}

	/**
	 * - Hash M' = Padding1 (8 zero bytes) || mHash || salt, then compare
	 *   H' with H recovered from EM.
	 */
	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)XSecure_RsaPssPadding1,
		XSECURE_RSA_PSS_PADDING1_LEN);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

	Status = XSecure_ShaUpdate(ShaInstPtr, Input->MsgHashAddr,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	/** - Mark the salt as the last SHA update on sha_pmxc. */
	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}
#endif

	/** - Salt is the last sLen octets of DB, after PS || 0x01. */
	Status = XSecure_ShaUpdate(ShaInstPtr,
		(UINTPTR)&DbWorkspace[DbLen + XSECURE_RSA_PSS_SALT_SEP_LEN],
		XSECURE_RSA_PSS_SALT_LEN);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

	/*
	 * EmHash still holds H. Overwrite it with H' so a skipped ShaFinish
	 * cannot compare H against itself.
	 */
	Status = XST_FAILURE;
	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)EmHash,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto ERR_SHA;
	}

	/** - Compare H' against H from EM (double-read against glitching). */
	Status = XST_FAILURE;
	IndexTmp = 0U;
	for (Index = 0U; Index < XSECURE_SHA_384_HASH_SIZE_IN_BYTES; Index++) {
		HashTmp = EmHash[Index];
		if ((EmHash[Index] != EmHashPtr[IndexTmp]) ||
			(HashTmp != EmHashPtr[IndexTmp])) {
			Status = (int)XSECURE_RSA_PSS_ERR_HASH_CMP;
			goto END;
		}
		IndexTmp++;
	}

	if ((Index != XSECURE_SHA_384_HASH_SIZE_IN_BYTES) ||
		(IndexTmp != XSECURE_SHA_384_HASH_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_RSA_PSS_ERR_HASH_CMP;
		goto END;
	}

	Status = XST_SUCCESS;
	goto END;

ERR_SHA:
	Status = (int)XSECURE_RSA_PSS_ERR_SHA;
END:
	/** - Clear H' so it does not remain on the stack. */
	ClearStatus = Xil_SMemSet(EmHash, sizeof(EmHash), 0U, sizeof(EmHash));
	if (ClearStatus != XST_SUCCESS) {
		Status = (Status == XST_SUCCESS) ? (int)XSECURE_RSA_PSS_ERR_CLEAR : Status;
	}

	return Status;
}

#endif /* PLM_RSA_EXCLUDE */

/** @} */
