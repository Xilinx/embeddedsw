/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.c
* This file contains Versal Net specific code for Xilsecure rsa server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/23/23 Initial release
* 5.3   am      09/28/23 Added wrapper functions for IPCore's RSA APIs
*       dd      10/11/23 MISRA-C violation Rule 10.3 fixed
*       dd      10/11/23 MISRA-C violation Rule 12.1 fixed
*       dd      10/11/23 MISRA-C violation Rule 8.13 fixed
*       kpt     12/13/23 Added RSA CRT support for RSA keyunwrap
* 5.3   ng      01/28/24 Added SDT support
*       ng      03/26/24 Fixed header include in SDT flow
*       kpt     03/22/24 Fix MISRA C violation of Rule 10.3
* 5.4   yog     04/29/24 Fixed doxygen warnings.
*       kpt     05/26/24 Added RSA Expopt API.
*       kpt     06/13/24 Add support for RSA key generation.
*       kpt     07/04/24 Add major error code for RSA key pair generation.
*       kal     07/24/24 Code refactoring changes for versal_2ve_2vm
*       mb      10/14/24 Removed duplicate definitions
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis Xilsecure RSA Server APIs
* @{
*/
/***************************** Include Files *********************************/

#ifdef SDT
#include "xsecure_config.h"
#endif

#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_plat_rsa.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"
#include "xsecure_plat.h"
#include "xsecure_error.h"
#include "xsecure_plat_kat.h"
#include "xsecure_defs.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xplmi_scheduler.h"

/************************** Constant Definitions *****************************/

#define XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES  (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - \
						(2U * XSECURE_SHA3_HASH_LENGTH_IN_BYTES) - 2U) /**< RSA maximum message size in bytes */

#define XSECURE_RSA_MAX_PS_SIZE_IN_BYTES   (XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES) /**< RSA maximum PS size in bytes */
#define XSECURE_RSA_MAX_DB_SIZE_IN_BYTES   (XSECURE_RSA_MAX_PS_SIZE_IN_BYTES  + \
						XSECURE_SHA3_HASH_LENGTH_IN_BYTES + 1U) /**< RSA maximum DB size in bytes */

/************************** Function Prototypes ******************************/

static int XSecure_RsaOaepEncode(XSecure_RsaOaepParam *OaepParam, u64 OutputAddr);
static int XSecure_RsaOaepDecode(XSecure_RsaOaepParam *OaepParam, u64 InputDataAddr);
static XSecure_RsaKeyMgmt* XSecure_GetRsaKeyMgmtInstance(void);
static int XSecure_GetRsaKeySlotIdx(u32 *RsaKeyIdx, u32 RsaKeyStatus);
static XSecure_RsaKeyGenParam* XSecure_GetRsaKeyGenParam(void);
static int XSecure_RsaKeyGenInit(XSecure_RsaKeyGenParam* RsaParam, XSecure_RsaKeyPtr* KeyPairState, u32 RsaKeyLen);
static int XSecure_RsaKeyGenerate(XSecure_RsaKeyPtr *KeyPairState, u32 QuantSize);
static int XSecure_RemoveRsaKeyPairGenerationFromScheduler(void);
static int XSecure_GenerateRsaKeyPair(void* arg);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function encodes the given message using PKCS #1 v2.0
 *		RSA Optimal Asymmetric Encryption Padding scheme.
 *		EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam	is pointer to the XSecure_RsaOaepParam instance.
 * @param	OutputAddr	is address where the encoded data is stored.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_OAEP_INVALID_PARAM  On invalid parameter.
 *		 - XSECURE_RSA_OAEP_INVALID_MSG_LEN  On invalid message length.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_RsaOaepEncode(XSecure_RsaOaepParam *OaepParam, u64 OutputAddr)
{
	volatile int Status = XST_FAILURE;
	u8 Seed[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 DB[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 DBMask[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 SeedMask[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u32 Index = 0U;
	u32 DBLen = 0U;
	u32 DiffHashLen = 0U;
	u32 ActualMsgLen = 0U;
	XSecure_MgfInput MgfParam;
	const XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

	if (HashPtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if ((OaepParam->InputDataAddr == 0x00U) || (OaepParam->OutputDataAddr == 0x00U)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	/* Get the actual message length based on the hash algorithm */
	if (HashPtr->HashLen > XSECURE_SHA3_HASH_LENGTH_IN_BYTES) {
		DiffHashLen = (HashPtr->HashLen - XSECURE_SHA3_HASH_LENGTH_IN_BYTES);
	} else {
		DiffHashLen = (XSECURE_SHA3_HASH_LENGTH_IN_BYTES - HashPtr->HashLen);
	}

	ActualMsgLen = (XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES - (DiffHashLen * 2U));
	if  (OaepParam->InputDataSize > ActualMsgLen) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_MSG_LEN;
		goto END;
	}

	Status = HashPtr->ShaDigest(OaepParam->ShaType, OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
				       (u64)(UINTPTR)DB);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Index = ActualMsgLen - OaepParam->InputDataSize;
	DB[HashPtr->HashLen + Index] = 0x01U;

	XSecure_MemCpy64((u64)(UINTPTR)&DB[HashPtr->HashLen + Index + 1U], OaepParam->InputDataAddr, OaepParam->InputDataSize);

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_GetRandomNum, Seed, HashPtr->HashLen);

	DBLen = Index + OaepParam->InputDataSize + HashPtr->HashLen + 1U;

	MgfParam.Seed = Seed;
	MgfParam.SeedLen = HashPtr->HashLen;
	MgfParam.Output = DBMask;
	MgfParam.OutputLen = DBLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < DBLen;  Index++) {
		DB[Index] ^= DBMask[Index];
	}

	MgfParam.Seed = DB;
	MgfParam.SeedLen = DBLen;
	MgfParam.Output = SeedMask;
	MgfParam.OutputLen = HashPtr->HashLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < HashPtr->HashLen; Index++) {
		Seed[Index] ^= 	SeedMask[Index];
	}

	/* Encode the message in to OutputAddr */
	XSecure_OutByte64(OutputAddr, 0x00U);
	XSecure_MemCpy64((OutputAddr + 1U), (u64)(UINTPTR)Seed, HashPtr->HashLen);
	XSecure_MemCpy64((OutputAddr + HashPtr->HashLen + 1U), (u64)(UINTPTR)DB, DBLen);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decodes the given message which is encoded with
 *		RSA Optimal Asymmetric Encryption Padding scheme i.e.
 *		EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam	is pointer to the XSecure_RsaOaepParam instance.
 * @param	InputDataAddr	is the address where decrypted output data is stored.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_OAEP_INVALID_PARAM  On invalid parameter.
 *		 - XSECURE_RSA_OAEP_BYTE_MISMATCH_ERROR  If byte mismatch.
 *		 - XSECURE_RSA_OAEP_DATA_CPY_ERROR  If data copy fails.
 *		 - XSECURE_RSA_OAEP_DATA_CMP_ERROR  If data compare fails.
 *		 - XSECURE_RSA_OAEP_DB_MISMATCH_ERROR  If DB ia mismatched.
 *		 - XSECURE_RSA_OAEP_INVALID_MSG_LEN  On invalid message length.
 *		 - XST_FAILURE On failure.
 *
 ******************************************************************************/
static int XSecure_RsaOaepDecode(XSecure_RsaOaepParam *OaepParam, u64 InputDataAddr)
{
	volatile int Status = XST_FAILURE;
	u8 Hash[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 DB[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 DBMask[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 SeedMask[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 Seed[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	volatile u8 DBTmp = 0xFFU;
	u8 IsPsZero = 0x00U;
	u32 Index = 0U;
	u32 ActualMsgLen = 0U;
	XSecure_MgfInput MgfParam;
	const XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

	if (HashPtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if (XSecure_InByte64(InputDataAddr) != 0x00U) {
		Status = (int)XSECURE_RSA_OAEP_BYTE_MISMATCH_ERROR;
		goto END;
	}

	Status = HashPtr->ShaDigest(OaepParam->ShaType, OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
				       (u64)(UINTPTR)Hash);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_OAEP_DATA_CPY_ERROR;
		goto END;
	}

	/**< Split the message in to EM = 0X00 || SeedMask || DBMask */
	XSecure_MemCpy64((u64)(UINTPTR)SeedMask, (InputDataAddr + 1U), HashPtr->HashLen);
	XSecure_MemCpy64((u64)(UINTPTR)DBMask, (InputDataAddr + 1U + HashPtr->HashLen),
			 (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U));

	/**< Extract seed from SeedMask */
	MgfParam.Seed = DBMask;
	MgfParam.SeedLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U);
	MgfParam.Output = Seed;
	MgfParam.OutputLen = HashPtr->HashLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < HashPtr->HashLen; Index++) {
		Seed[Index] ^= SeedMask[Index];
	}

	/**< Extract DB from DBMask */
	MgfParam.Seed = Seed;
	MgfParam.SeedLen = HashPtr->HashLen;
	MgfParam.Output = DB;
	MgfParam.OutputLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U);
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U); Index++) {
		DB[Index] ^= DBMask[Index];
	}

	Status = Xil_SMemCmp(DB, HashPtr->HashLen, Hash, HashPtr->HashLen, HashPtr->HashLen);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_OAEP_DATA_CMP_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	Index = HashPtr->HashLen;
	for (; Index < (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U); Index++) {
		DBTmp = DB[Index];
		if ((DB[Index] == 0x0U) && (DBTmp == 0x00U)) {
			IsPsZero |= DB[Index];
		}
		else {
			break;
		}
	}

	if ((DB[Index] != 0x01U) || (IsPsZero != 0x00U)) {
		Status = (int)XSECURE_RSA_OAEP_DB_MISMATCH_ERROR;
		goto END;
	}

	Index = Index + 1U;
	ActualMsgLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U) - Index;
	if (ActualMsgLen > (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - (2U * HashPtr->HashLen) - 2U)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_MSG_LEN;
		goto END;
	}

	OaepParam->OutputDataSize = ActualMsgLen;
	XSecure_MemCpy64(OaepParam->OutputDataAddr, (u64)(UINTPTR)&DB[Index], ActualMsgLen);
	Status = XST_SUCCESS;
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function encodes the given message using RSA OAEP and encrypts it.
 *
 * @param	InstancePtr	is pointer to the XSecure_Rsa instance.
 * @param	OaepParam	is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_OAEP_INVALID_PARAM  On invalid parameter.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {0U};

	if ((InstancePtr == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_RsaOaepEncode(OaepParam, (u64)(UINTPTR)Output);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaPublicEncrypt_64Bit(InstancePtr, (u64)(UINTPTR)Output, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES,
						OaepParam->OutputDataAddr);

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function decodes the given message and decrypts it using RSA OAEP.
 *
 * @param	PrivKey	is pointer to the XSecure_RsaPrivKey instance.
 * @param	OaepParam	is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_OAEP_INVALID_PARAM  On invalid parameter.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepDecrypt(XSecure_RsaPrivKey *PrivKey, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {0U};

	if ((PrivKey == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_RsaExpCRT((u8*)(UINTPTR)OaepParam->InputDataAddr, PrivKey->P, PrivKey->Q, PrivKey->DP, PrivKey->DQ, PrivKey->QInv, NULL,
					PrivKey->Mod, (int)(XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES * XSECURE_BYTE_IN_BITS), (u8*)(UINTPTR)Output);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/* Reverse Output of CRT function */
	Status = Xil_SReverseData(Output, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XSecure_RsaOaepDecode(OaepParam, (u64)(UINTPTR)Output);

END_RST:
	SStatus = Xil_SecureZeroize(Output, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function adds periodic task of generation RSA key pair to scheduler.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_ERR_ADD_TASK_SCHEDULER  If failed to add task to scheduler.
 *
 ******************************************************************************/
int XSecure_AddRsaKeyPairGenerationToScheduler(void)
{
	volatile int Status = XST_FAILURE;

	Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_XILSECURE_ID,
					XSecure_GenerateRsaKeyPair, NULL,
					XSECURE_KEY_PAIR_GEN_POLL_INTERVAL,
					XPLM_TASK_PRIORITY_1, NULL, XPLMI_PERIODIC_TASK);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XSECURE_ERR_ADD_TASK_SCHEDULER, 0);
	}
	else {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,"RSA key pair generation task added successfully\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function removes keypair generation task from scheduler.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_ERR_REMOVE_TASK_SCHEDULER  If failed to remove task from scheduler.
 *
 ******************************************************************************/
static int XSecure_RemoveRsaKeyPairGenerationFromScheduler(void)
{
	volatile int Status = XST_FAILURE;

	/* Remove key pair generation task from scheduler */
	Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_XILSECURE_ID,
			XSecure_GenerateRsaKeyPair, 0U, NULL);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XSECURE_ERR_REMOVE_TASK_SCHEDULER, 0);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs the RSA key initialization.
 *
 * @param	RsaParam	is the pointer to XSecure_RsaKeyGenParam.
 * @param	KeyPairState	is the pointer to XSecure_RsaKeyPtr which holds the state of
 *				key that needs to be generated.
 * @param	RsaKeyLen	Length of RSA key length in bytes.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_INVALID_PARAM  If input parameter is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_RsaKeyGenInit(XSecure_RsaKeyGenParam* RsaParam, XSecure_RsaKeyPtr* KeyPairState, u32 RsaKeyLen)
{
	volatile int Status = XST_FAILURE;

	if ((KeyPairState == NULL) || (RsaParam == NULL)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	if (RsaKeyLen == XSECURE_RSA_2048_KEY_SIZE) {
		RsaParam->QuantSize = XSECURE_RSA_2048_QUANT_SIZE;
	}
	else if (RsaKeyLen == XSECURE_RSA_3072_KEY_SIZE) {
		RsaParam->QuantSize = XSECURE_RSA_3072_QUANT_SIZE;
	}
	else if (RsaKeyLen == XSECURE_RSA_4096_KEY_SIZE) {
		RsaParam->QuantSize = XSECURE_RSA_4096_QUANT_SIZE;
	}
	else {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	/**< Initialize private key components to KeyPairState*/
	KeyPairState->D = RsaParam->KeyPair.PrivKey->Exp;
	KeyPairState->DP = RsaParam->KeyPair.PrivKey->DP;
	KeyPairState->DQ = RsaParam->KeyPair.PrivKey->DQ;
	KeyPairState->P = RsaParam->KeyPair.PrivKey->P;
	KeyPairState->Q = RsaParam->KeyPair.PrivKey->Q;
	KeyPairState->iQ = RsaParam->KeyPair.PrivKey->QInv;
	KeyPairState->M = RsaParam->KeyPair.PrivKey->Mod;

	/**< Initialize Public key components to KeyPairState */
	Status = Xil_SMemSet(RsaParam->KeyPair.PubKey->PubExp, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES, 0U,
				XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	RsaParam->KeyPair.PubKey->PubExp[0U] = XSECURE_RSA_PUBLIC_EXPONENT;

	KeyPairState->E = (u8*)(UINTPTR)RsaParam->KeyPair.PubKey->PubExp;

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	Status = rsaprvkeyinit_Q(RsaKeyLen * XSECURE_BYTE_IN_BITS, NULL, KeyPairState);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs the RSA key generation in steps.
 *
 * @param	KeyPairState	is pointer to the XSecure_RsaKeyPtr.
 * @param	QuantSize	Size of the key generation steps in terms of quant size
 *				i.e. Half-size RSA key.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_RSA_INVALID_PARAM  If input parameter is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_RsaKeyGenerate(XSecure_RsaKeyPtr *KeyPairState, u32 QuantSize)
{
	volatile int Status = XSECURE_RSA_INVALID_PARAM;

	if ((KeyPairState == NULL) || (QuantSize == 0U)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	Status = rsaprvkeystep_Q(QuantSize, KeyPairState);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns RSA key inuse index.
 *
 * @return
 *		 - KeyInUse  To indicate the key that is in use
 *
 ******************************************************************************/
u32 XSecure_GetRsaKeyInUseIdx(void)
{
	XSecure_RsaKeyMgmt* RsaKeyMgmt = XSecure_GetRsaKeyMgmtInstance();

	if (RsaKeyMgmt->KeyInUse == 0xFFFFFFFFU) {
		(void)XSecure_GetRsaKeySlotIdx(&RsaKeyMgmt->KeyInUse, XSECURE_RSA_KEY_AVAIL);
	}

	return RsaKeyMgmt->KeyInUse;
}

/*****************************************************************************/
/**
 * @brief	This function destroys the RSA key in use.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_RsaDestroyKeyInUse(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaKeyMgmt* RsaKeyMgmt = XSecure_GetRsaKeyMgmtInstance();

	if (RsaKeyMgmt->KeyInUse < XSECURE_RSA_MAX_KEY_GEN_SUPPORT) {
		/*< Clear RSA private and public keys */
		Status = Xil_SMemSet(RsaKeyMgmt->Key[RsaKeyMgmt->KeyInUse].KeyPair.PrivKey, sizeof(XSecure_RsaPrivKey),
			0U, sizeof(XSecure_RsaPrivKey));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemSet(RsaKeyMgmt->Key[RsaKeyMgmt->KeyInUse].KeyPair.PubKey, sizeof(XSecure_RsaPubKey),
			0U, sizeof(XSecure_RsaPubKey));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemSet(&RsaKeyMgmt->Key[RsaKeyMgmt->KeyInUse], sizeof(XSecure_RsaKeyGenParam),
			0U,  sizeof(XSecure_RsaKeyGenParam));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		RsaKeyMgmt->KeyInUse = 0xFFFFFFFFU;
		Status = XSecure_AddRsaKeyPairGenerationToScheduler();
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs the RSA key generation.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_ERR_RSA_KEY_PAIR_GEN_SCHEDULER  If RSA key pair generation fails.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_GenerateRsaKeyPair(void* arg)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	static XSecure_RsaKeyOpState RsaKeyGenState = XSECURE_RSA_KEY_DEFAULT_STATE;
	static XSecure_RsaKeyGenParam* RsaKeyParam = NULL;
	static 	XSecure_RsaKeyPtr RsaKeyPairState = {0U};

	(void)arg;

	/* State machine for RSA key generation
	 * Note: RSA key generation operation is non-reentrant
	 * hence state is maintained for whole operation
	 */
	if (RsaKeyGenState == XSECURE_RSA_KEY_DEFAULT_STATE) {
		/* Get free RSA key param */
		RsaKeyParam = XSecure_GetRsaKeyGenParam();
		if (RsaKeyParam == NULL) {
			Status = XSecure_RemoveRsaKeyPairGenerationFromScheduler();
			goto END;
		}
		RsaKeyGenState = XSECURE_RSA_KEY_INIT_STATE;
	}

	if (RsaKeyGenState == XSECURE_RSA_KEY_INIT_STATE) {
		/*
		 * Key generation init internally uses TRNG to generate
		 * random numbers for prime number generation i.e. P and Q
		 */
		Status = XSecure_RsaKeyGenInit(RsaKeyParam, &RsaKeyPairState, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			RsaKeyGenState = XSECURE_RSA_KEY_INIT_STATE;
			goto END;
		}
		RsaKeyGenState = XSECURE_RSA_KEY_GEN_STATE;
	} else if (RsaKeyGenState == XSECURE_RSA_KEY_GEN_STATE) {
		/*
		 * Key generate process uses Rabin miller primality test
		 * to determine the prime number that is closer to the
		 * random number.
		 */
		Status = XSecure_RsaKeyGenerate(&RsaKeyPairState, RsaKeyParam->QuantSize);
		if (Status != XSECURE_RSA_KEY_STATUS_WAIT) {
			if (Status != XST_SUCCESS) {
				RsaKeyGenState = XSECURE_RSA_KEY_INIT_STATE;
				goto END;
			}
			RsaKeyGenState = XSECURE_RSA_KEY_READY_STATE;
		}
		Status = XST_SUCCESS;
	} else if (RsaKeyGenState == XSECURE_RSA_KEY_READY_STATE) {
		Status = Xil_SChangeEndiannessAndCpy((u8*)RsaKeyParam->KeyPair.PubKey->Mod, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES,
				(u8*)RsaKeyParam->KeyPair.PrivKey->Mod ,XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* RSA pair wise consistency test */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XSECURE_KAT_MAJOR_ERROR, Status, StatusTmp,
			XSecure_RsaPwct, RsaKeyParam->KeyPair.PrivKey, RsaKeyParam->KeyPair.PubKey,
			NULL, XSECURE_SHA2_384);
		if (Status != XST_SUCCESS || StatusTmp != XST_SUCCESS) {
			RsaKeyGenState = XSECURE_RSA_KEY_INIT_STATE;
			goto END;
		}
		RsaKeyParam->IsRsaKeyAvail = XSECURE_RSA_KEY_AVAIL;
		RsaKeyGenState = XSECURE_RSA_KEY_DEFAULT_STATE;
	} else {
		/* for MISRA-C violation */
	}

END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XSECURE_ERR_RSA_KEY_PAIR_GEN_SCHEDULER, Status);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns pointer to XSecure_RsaKeyMgmt instance.
 *
 * @return
 *		 - RsaKeyMgmt  is pointer to the XSecure_RsaKeyMgmt instances.
 *
 ******************************************************************************/
static XSecure_RsaKeyMgmt* XSecure_GetRsaKeyMgmtInstance(void) {
	static XSecure_RsaKeyMgmt RsaKeyMgmt = {.KeyInUse = 0xFFFFFFFFU};

	return &RsaKeyMgmt;
}

/*****************************************************************************/
/**
 * @brief	This function returns RSA private key.
 *
 * @return
 * 		 - Pointer to XSecure_RsaPrivKey or NULL otherwise.
 *
 ******************************************************************************/
XSecure_RsaPrivKey* XSecure_GetRsaPrivateKey(u32 RsaIdx)
{
	static XSecure_RsaPrivKey RsaPrivateKey[XSECURE_RSA_MAX_KEY_GEN_SUPPORT];
	XSecure_RsaPrivKey* PrivKey = NULL;

	if (RsaIdx < XSECURE_RSA_MAX_KEY_GEN_SUPPORT) {
		PrivKey =  &RsaPrivateKey[RsaIdx];
	}

	return PrivKey;
}

/*****************************************************************************/
/**
 * @brief	This function returns RSA public key.
 *
 * @return
 *		 - Pointer to XSecure_RsaPubKey or NULL otherwise.
 *
 ******************************************************************************/
XSecure_RsaPubKey* XSecure_GetRsaPublicKey(u32 RsaIdx)
{
	static XSecure_RsaPubKey RsaPublicKey[XSECURE_RSA_MAX_KEY_GEN_SUPPORT];
	XSecure_RsaPubKey* PubKey = NULL;

	if (RsaIdx < XSECURE_RSA_MAX_KEY_GEN_SUPPORT) {
		PubKey =  &RsaPublicKey[RsaIdx];
	}

	return PubKey;
}

/*****************************************************************************/
/**
 * @brief	This function returns RSA key index based on RsaKeyStatus.
 *
 * @param	RsaKeyIdx	is pointer to the variable containing RSA free index.
 * @param	RsaKeyStatus	Key status to be checked.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_GetRsaKeySlotIdx(u32 *RsaKeyIdx, u32 RsaKeyStatus)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaKeyMgmt* RsaKeyMgmt = XSecure_GetRsaKeyMgmtInstance();
	u32 Idx = 0U;

	while (Idx < XSECURE_RSA_MAX_KEY_GEN_SUPPORT) {
		if (RsaKeyMgmt->Key[Idx].IsRsaKeyAvail == RsaKeyStatus) {
			*RsaKeyIdx = Idx;
			Status = XST_SUCCESS;
			break;
		}
		Idx++;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns RSA free key generation param.
 *
 * @return
 *		 - Pointer to XSecure_RsaKeyGenParam or NULL otherwise.
 *
 ******************************************************************************/
static XSecure_RsaKeyGenParam* XSecure_GetRsaKeyGenParam(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaKeyMgmt* RsaKeyMgmt = XSecure_GetRsaKeyMgmtInstance();
	XSecure_RsaKeyGenParam* RsaKeyParam = NULL;
	u32 RsaKeyIndex = 0U;

	Status = XSecure_GetRsaKeySlotIdx(&RsaKeyIndex, XSECURE_RSA_KEY_FREE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaKeyIndex < XSECURE_RSA_MAX_KEY_GEN_SUPPORT) {
		RsaKeyParam = &RsaKeyMgmt->Key[RsaKeyIndex];
		RsaKeyParam->KeyPair.PrivKey = XSecure_GetRsaPrivateKey(RsaKeyIndex);
		RsaKeyParam->KeyPair.PubKey = XSecure_GetRsaPublicKey(RsaKeyIndex);
	}

END:
	return RsaKeyParam;
}

#endif
/** @} */
