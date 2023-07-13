/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.c
* This file contains versalnet specific code for xilsecure rsa server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/23/23 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_plat_rsa.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"
#include "xsecure_plat.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES  (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - \
						(2U * XSECURE_SHA3_HASH_LENGTH_IN_BYTES) - 2U) /**< RSA maximum message size in bytes */

#define XSECURE_RSA_MAX_PS_SIZE_IN_BYTES   (XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES) /**< RSA maximum PS size in bytes */
#define XSECURE_RSA_MAX_DB_SIZE_IN_BYTES   (XSECURE_RSA_MAX_PS_SIZE_IN_BYTES  + \
						XSECURE_SHA3_HASH_LENGTH_IN_BYTES + 1U) /**< RSA maximum DB size in bytes */

/************************** Function Prototypes ******************************/

static int XSecure_RsaOaepEncode(XSecure_RsaOaepParam *OaepParam, u64 OutputAddr);
static int XSecure_RsaOaepDecode(XSecure_RsaOaepParam *OaepParam, u64 InputDataAddr);

/************************** Variable Definitions *****************************/

#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
static u8 Modulus[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {
	0xB0U, 0x54U, 0xA4U, 0xFEU, 0x8BU, 0x16U, 0xA3U, 0x79U,
	0x7BU, 0x5DU, 0x1CU, 0x7EU, 0x09U, 0xEFU, 0x1AU, 0xABU,
	0x8BU, 0xBBU, 0xA5U, 0x1AU, 0x88U, 0x8BU, 0x1BU, 0x2DU,
	0xE1U, 0x8CU, 0x61U, 0x87U, 0xFAU, 0x08U, 0xFBU, 0x74U,
	0xBCU, 0x4EU, 0xD7U, 0xBDU, 0xEDU, 0xC5U, 0xCBU, 0x07U,
	0x53U, 0x90U, 0x1CU, 0x06U, 0x77U, 0xCFU, 0x74U, 0x99U,
	0x08U, 0x94U, 0x71U, 0xE0U, 0x26U, 0xBBU, 0xFCU, 0xD6U,
	0x52U, 0x07U, 0xCCU, 0xD7U, 0x02U, 0x3EU, 0x54U, 0x68U,
	0x7AU, 0x19U, 0x4FU, 0xD5U, 0xC3U, 0x31U, 0xA3U, 0x5CU,
	0xCBU, 0x7AU, 0x4CU, 0x6CU, 0xA1U, 0x47U, 0x4AU, 0xD1U,
	0xA7U, 0x28U, 0xF2U, 0x23U, 0xCAU, 0x70U, 0xF6U, 0x57U,
	0x0DU, 0x6AU, 0x6BU, 0xB5U, 0x4FU, 0xF7U, 0xDEU, 0xC7U,
	0xA0U, 0xA1U, 0x90U, 0xB8U, 0x11U, 0x0AU, 0x10U, 0xE5U,
	0xB1U, 0xC7U, 0x13U, 0x65U, 0x5CU, 0x8FU, 0x0AU, 0x7FU,
	0x26U, 0xA9U, 0xF8U, 0x3DU, 0xBFU, 0x2CU, 0x78U, 0xF8U,
	0x65U, 0x3FU, 0xA4U, 0x5BU, 0x5DU, 0x4BU, 0x8DU, 0x2AU,
	0x7FU, 0x61U, 0x50U, 0x10U, 0xCFU, 0x8CU, 0x6AU, 0xE4U,
	0xA1U, 0x8FU, 0xAEU, 0x96U, 0x73U, 0xA1U, 0x71U, 0xC2U,
	0xE4U, 0x36U, 0x3FU, 0xF5U, 0x4DU, 0x9BU, 0x38U, 0x23U,
	0x49U, 0x49U, 0x70U, 0xFEU, 0x10U, 0x66U, 0xCFU, 0x41U,
	0xADU, 0x98U, 0xB5U, 0x3DU, 0x78U, 0x93U, 0xD1U, 0xC1U,
	0x1BU, 0x84U, 0x69U, 0x17U, 0xEFU, 0x30U, 0x06U, 0x56U,
	0xF6U, 0xC1U, 0x87U, 0xBBU, 0xADU, 0x17U, 0xE9U, 0x48U,
	0xD5U, 0xC8U, 0xF0U, 0x57U, 0xF9U, 0x78U, 0x45U, 0x27U,
	0x30U, 0x14U, 0x15U, 0x9BU, 0xBBU, 0xAAU, 0x99U, 0x15U,
	0x0CU, 0xDDU, 0x90U, 0x84U, 0xEDU, 0xAAU, 0x21U, 0x52U,
	0x0BU, 0x3AU, 0x56U, 0x2BU, 0x1DU, 0x6AU, 0x85U, 0x75U,
	0xA3U, 0x98U, 0x14U, 0x05U, 0xEFU, 0xFEU, 0x7EU, 0x1FU,
	0x2AU, 0x52U, 0x8EU, 0xE8U, 0x57U, 0x30U, 0xF5U, 0x2AU,
	0xD1U, 0x90U, 0xC4U, 0x85U, 0x86U, 0xA5U, 0xB6U, 0xF8U,
	0xC4U, 0x8DU, 0xF2U, 0xDFU, 0xA0U, 0x97U, 0x00U, 0xABU,
	0xEEU, 0xA5U, 0xF7U, 0x3EU, 0x3EU, 0xDEU, 0xD7U, 0x37U,
	0xB3U, 0x5CU, 0x05U, 0xEDU, 0x2BU, 0x44U, 0x03U, 0x57U,
	0x30U, 0x75U, 0x56U, 0xEBU, 0x47U, 0x2AU, 0xFDU, 0x34U,
	0x94U, 0x34U, 0x3FU, 0x55U, 0x1AU, 0x9EU, 0x15U, 0x49U,
	0x5AU, 0x48U, 0x33U, 0xFEU, 0xE3U, 0xC1U, 0xFCU, 0x45U,
	0x3BU, 0xE4U, 0x8BU, 0x0BU, 0x39U, 0xCAU, 0x59U, 0x36U,
	0x74U, 0xA2U, 0x05U, 0xA1U, 0xA1U, 0x48U, 0xB7U, 0xE4U,
	0x45U, 0xB7U, 0xB4U, 0xE9U, 0xBBU, 0xDFU, 0x9AU, 0x4EU,
	0x18U, 0x71U, 0xBFU, 0x2AU, 0x4FU, 0xDBU, 0x48U, 0xA7U,
	0x5BU, 0x8FU, 0x91U, 0x5AU, 0x9AU, 0x7AU, 0xD1U, 0x45U,
	0x25U, 0x43U, 0x14U, 0x58U, 0xBEU, 0x7CU, 0x43U, 0x44U,
	0x6BU, 0x05U, 0xFEU, 0xAAU, 0xC2U, 0xC7U, 0x2FU, 0x55U,
	0x6CU, 0xA8U, 0x16U, 0xBEU, 0x38U, 0xDDU, 0xB9U, 0xABU,
	0x60U, 0x78U, 0xCBU, 0xF1U, 0x2BU, 0x19U, 0xB8U, 0xE1U,
	0x0FU, 0xD5U, 0x9FU, 0xC6U, 0x61U, 0xF2U, 0x8FU, 0xC4U,
	0x7DU, 0x43U, 0xC3U, 0x56U, 0x65U, 0xDEU, 0x10U, 0x4EU,
	0x31U, 0xD7U, 0xC6U, 0xECU, 0x57U, 0x8FU, 0xA7U, 0x2FU
};
#endif

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function encodes the given message using PKCS #1 v2.0
 *          RSA Optimal Asymmetric Encryption Padding scheme.
 *              EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 * @param	OutputAddr is address where the encoded data is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
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
	XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

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

	Status = HashPtr->ShaDigest(OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
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
 *              RSA Optimal Asymmetric Encryption Padding scheme i.e.
 *              EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 * @param	InputDataAddr is the address where decrypted output data is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
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
	XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

	if (HashPtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if (XSecure_InByte64(InputDataAddr) != 0x00U) {
		Status = (int)XSECURE_RSA_OAEP_BYTE_MISMATCH_ERROR;
		goto END;
	}

	Status = HashPtr->ShaDigest(OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
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
	if (ActualMsgLen > (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - 2U * HashPtr->HashLen - 2U)) {
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
 * @param	InstancePtr is pointer to the XSecure_Rsa instance.
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];

	if ((InstancePtr == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if (OaepParam->ShaInstancePtr == NULL) {
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
 * @param	InstancePtr is pointer to the XSecure_Rsa instance.
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepDecrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];

	if ((InstancePtr == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if (OaepParam->ShaInstancePtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_RsaPrivateDecrypt_64Bit(InstancePtr, OaepParam->InputDataAddr, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES,
			(u64)(UINTPTR)Output);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	Status = XSecure_RsaOaepDecode(OaepParam, (u64)(UINTPTR)Output);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets RSA private key.
 *
 * @return
 *        - Pointer to RSA private key or NULL otherwise.
 *
 ******************************************************************************/
XSecure_RsaKey *XSecure_GetRsaPrivateKey(void)
{
	static XSecure_RsaKey RsaPrivKey = {0U};
#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
	static u8 PrivateExp[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {
		0x70U, 0x1EU, 0xE2U, 0x6BU, 0x18U, 0x75U, 0xE4U, 0xACU,
		0xB2U, 0x4CU, 0x9AU, 0x79U, 0x6AU, 0x47U, 0xA7U, 0x65U,
		0xBDU, 0x0CU, 0x2CU, 0x07U, 0x9BU, 0x1BU, 0x18U, 0xC5U,
		0x2BU, 0xCDU, 0xDFU, 0x96U, 0x5EU, 0xDEU, 0xA7U, 0x45U,
		0xABU, 0x17U, 0x57U, 0x07U, 0x65U, 0xD1U, 0x87U, 0x2FU,
		0xB9U, 0x6AU, 0xC0U, 0xE7U, 0x3BU, 0xF7U, 0xA5U, 0xE4U,
		0x49U, 0x18U, 0x5AU, 0xF0U, 0x74U, 0xB0U, 0xC4U, 0x2CU,
		0x63U, 0x2CU, 0xA2U, 0x8BU, 0x74U, 0xD9U, 0xBDU, 0x42U,
		0x51U, 0xA1U, 0x16U, 0xAAU, 0x8DU, 0xDBU, 0x5AU, 0x4CU,
		0xFEU, 0xF5U, 0x5CU, 0xF3U, 0x15U, 0xFAU, 0x07U, 0x11U,
		0x18U, 0x81U, 0x68U, 0xB6U, 0x69U, 0x5BU, 0x3AU, 0xC8U,
		0x4FU, 0xB7U, 0x83U, 0x95U, 0x5EU, 0xABU, 0xF5U, 0xF4U,
		0x69U, 0xA7U, 0x52U, 0x26U, 0x38U, 0x6FU, 0x76U, 0x1FU,
		0x17U, 0xD3U, 0xC5U, 0xF9U, 0x85U, 0xC9U, 0x4EU, 0x9BU,
		0x5BU, 0x32U, 0x68U, 0xAEU, 0x9AU, 0x88U, 0xE4U, 0xC2U,
		0xCDU, 0x5EU, 0x92U, 0x5BU, 0xA6U, 0x0FU, 0x0CU, 0x4CU,
		0x21U, 0x82U, 0xF7U, 0x2EU, 0x39U, 0x4AU, 0xC8U, 0x0DU,
		0x68U, 0xCDU, 0xBBU, 0xEDU, 0xDEU, 0xC8U, 0xA1U, 0x55U,
		0x93U, 0x57U, 0x0BU, 0x84U, 0x27U, 0x3CU, 0xB3U, 0x8DU,
		0x7CU, 0x64U, 0x3EU, 0x7CU, 0xCEU, 0x61U, 0x11U, 0x18U,
		0x6AU, 0x11U, 0xD7U, 0xECU, 0xA6U, 0x67U, 0x17U, 0xD0U,
		0xFAU, 0x76U, 0x4FU, 0x7CU, 0xF0U, 0x92U, 0x50U, 0xF5U,
		0xABU, 0x21U, 0x93U, 0xF5U, 0x3AU, 0x33U, 0x22U, 0x1BU,
		0x36U, 0x14U, 0xF6U, 0xFEU, 0x79U, 0x9AU, 0x88U, 0x1DU,
		0x3CU, 0x99U, 0x4DU, 0xA1U, 0x01U, 0x7FU, 0x07U, 0xA5U,
		0x0AU, 0xB9U, 0xC3U, 0xF1U, 0x8CU, 0x80U, 0x94U, 0x04U,
		0x44U, 0xF7U, 0x40U, 0x5DU, 0x4AU, 0xCFU, 0xF8U, 0xA5U,
		0x9FU, 0x7FU, 0x03U, 0xD6U, 0x3EU, 0x46U, 0x76U, 0x69U,
		0x67U, 0xF6U, 0x20U, 0x76U, 0xE1U, 0xB1U, 0x9DU, 0x9CU,
		0x52U, 0xB1U, 0xF0U, 0xCFU, 0x6BU, 0x01U, 0x75U, 0x45U,
		0xFCU, 0x35U, 0x1FU, 0xF2U, 0x89U, 0x15U, 0x19U, 0xB3U,
		0x8FU, 0x3DU, 0x4FU, 0x59U, 0x45U, 0x75U, 0x22U, 0x72U,
		0xA2U, 0xD1U, 0x1EU, 0x5DU, 0x58U, 0xEFU, 0x74U, 0x41U,
		0xA6U, 0x8EU, 0x66U, 0x9BU, 0x81U, 0x46U, 0x4FU, 0x58U,
		0xADU, 0xD9U, 0xCCU, 0x18U, 0x2BU, 0x7FU, 0x4DU, 0xB9U,
		0xA0U, 0xD0U, 0x13U, 0xE2U, 0x74U, 0x86U, 0x30U, 0xE4U,
		0x04U, 0x22U, 0x18U, 0xDAU, 0x37U, 0x4CU, 0x8FU, 0xE1U,
		0xF0U, 0x75U, 0x95U, 0x60U, 0xF7U, 0x71U, 0xABU, 0x75U,
		0x31U, 0x90U, 0xF7U, 0x10U, 0xB9U, 0x7BU, 0x0FU, 0xDFU,
		0x19U, 0x0CU, 0xE6U, 0x89U, 0xD9U, 0xB8U, 0x1EU, 0xD9U,
		0x12U, 0xBEU, 0xA2U, 0x25U, 0x36U, 0x14U, 0xE0U, 0x68U,
		0x96U, 0xA2U, 0x02U, 0xECU, 0x15U, 0xACU, 0x25U, 0xA2U,
		0x49U, 0x74U, 0x38U, 0xA3U, 0xBBU, 0x00U, 0x63U, 0xC7U,
		0xDEU, 0x3BU, 0xF7U, 0x68U, 0x92U, 0xA3U, 0xACU, 0x0DU,
		0xACU, 0x52U, 0x7AU, 0x1DU, 0x2BU, 0xC3U, 0x24U, 0x06U,
		0xDDU, 0x8CU, 0x1AU, 0x4CU, 0x07U, 0x4DU, 0x2FU, 0x74U,
		0x87U, 0x7EU, 0x3BU, 0x75U, 0x30U, 0x8BU, 0xF6U, 0xB0U,
		0xBDU, 0xD3U, 0x09U, 0xF1U, 0x92U, 0x20U, 0x8AU, 0xC1U
	};

	RsaPrivKey.Modulus = Modulus;
	RsaPrivKey.ModExt = NULL;
	RsaPrivKey.Exponent = PrivateExp;
#endif

	return &RsaPrivKey;
}

/*****************************************************************************/
/**
 * @brief	This function gets RSA public key.
 *
 * @return
 *		- Pointer to RSA public key or NULL otherwise
 *
 ******************************************************************************/
XSecure_RsaKey *XSecure_GetRsaPublicKey(void)
{
	static u32 PublicExp = 0x1000100U;
	static XSecure_RsaKey RsaPubKey = {0U};

#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
	RsaPubKey.Modulus = Modulus;
	RsaPubKey.ModExt = NULL;
	RsaPubKey.Exponent = (u8 *)&PublicExp;
#endif

	return &RsaPubKey;
}

#endif
