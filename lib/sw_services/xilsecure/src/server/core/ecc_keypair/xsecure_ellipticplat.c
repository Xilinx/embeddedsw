/******************************************************************************
* Copyright (c) 2022 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticplat.c
*
* This file contains the implementation of the interface functions for ECC
* engine specific to Versal Net platform.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   dc   07/10/22 Initial release
*       am   07/23/22 Removed Ecdsa_ModEccOrder function definition, as it is
*                     declared in Ecdsa.h file
*       dc   09/04/22 set TRNG to HRNG mode after Private key ECC mod order
* 5.1   har  01/06/23 Add support to generate ephemeral key
* 5.2   yog  05/18/23 Updated the flow for Big Endian ECC Mode setting
*       yog  08/07/23 Replaced trng API calls using trngpsx driver
*       dd   10/11/23 MISRA-C violation Rule 10.3 fixed
* 5.3   har  11/01/23 Updated core API for ECDH
*       kpt  11/24/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       kpt  01/09/24 Updated option for non-blocking trng reseed
*	ss   04/05/24 Fixed doxygen warnings
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_ellipticplat.h"
#include "xsecure_plat_kat.h"
#include "xsecure_error.h"
#include "xsecure_elliptic.h"
#include "xsecure_utils.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_error.h"
#include "xil_sutil.h"
#include "xsecure_init.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/
#define XSECURE_ECC_TRNG_DF_LENGTH			(2U) /**< Default length of xilsecure ecc true random number generator*/
#define XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN		(60U) /**< Length of xilsecure ecc true random number generator*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
extern EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function generates 48-byte key for P-384 curve using TRNG.
 *
 * @param	CrvType	specifies the type of the ECC curve.
 * @param	PrivateKey	is the pointer to XSecure_ElliptcPrivateKeyGen
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 * @note
 *	This API expects TRNG HW to be in HEALTHY state, This can
 *	be achieved by running preoperational health tests.
 *
 *****************************************************************************/
int XSecure_EllipticPrvtKeyGenerate(XSecure_EllipticCrvTyp CrvType,
	XSecure_ElliptcPrivateKeyGen *PrivateKey)
{
	volatile int Status = (int)XSECURE_ECC_PRVT_KEY_GEN_ERR;
	volatile int ClearStatus = XST_FAILURE;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();
	XTrngpsx_UserConfig TrngUserCfg;
	/* The random ephimeral/Private key should be between 1 to ecc order
	 * of the curve. EK can be obtained by using mod operation over
	 * 384-bits random number. However to reduce bias associated with mod
	 * operation, standard recommends of using additional 64-bits i.e.
	 * 384 + 64 = 448-bits. IPCores library has option to perform 448-bit
	 * mod operation however it treats MSB as sign bit. So to overcome the
	 * issue, new function added to IPCore library uses next available
	 * large number operation that hardware can handle is selected,
	 * 480-bits (60 bytes).
	 * Below code takes 448 bits from random number generated remaining
	 * MSB bits are set to 0x00.
	 */
	u8 RandBuf[XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN] = {0x00};
	u8 RandBufEndianChange[XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN] = {0x00};
	u32 Index = 0U;
	s32 RIndex = 0U;
	u32 PrvtKey[XSECURE_ECC_P384_SIZE_IN_BYTES];
	EcdsaCrvInfo *Crv = NULL;

	if ((CrvType == XSECURE_ECC_NIST_P521) || (PrivateKey == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto RET;
	}

	Status = Xil_SMemSet(&TrngUserCfg, sizeof(XTrngpsx_UserConfig), 0U,
					sizeof(XTrngpsx_UserConfig));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (TrngInstance->State != XTRNGPSX_UNINITIALIZED_STATE) {
		Status = XTrngpsx_Uninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	TrngUserCfg.Mode = XTRNGPSX_DRNG_MODE;
	TrngUserCfg.DFLength = XSECURE_ECC_TRNG_DF_LENGTH;
	TrngUserCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE;
	TrngUserCfg.IsBlocking = FALSE;
	Status = XTrngpsx_Instantiate(TrngInstance,
			(u8 *)(UINTPTR)PrivateKey->SeedAddr, PrivateKey->SeedLength,
			(u8 *)(UINTPTR)PrivateKey->PerStringAddr, &TrngUserCfg);
	if (Status  != XST_SUCCESS) {
		goto END;
	}

	Status = XTrngpsx_Generate(TrngInstance, RandBuf,
			XTRNGPSX_SEC_STRENGTH_IN_BYTES, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XTrngpsx_Generate(TrngInstance,
			RandBuf + XTRNGPSX_SEC_STRENGTH_IN_BYTES,
			XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN -
			XTRNGPSX_SEC_STRENGTH_IN_BYTES, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Take ECDSA core out if reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(XSECURE_ECC_NIST_P384);
	if (Crv == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	RIndex = ((s32)XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN - 1);
	for (Index = 0U; (Index < XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN) && (RIndex >= 0); Index++, RIndex--) {
		XSecure_OutByte64((u64)(UINTPTR)(RandBufEndianChange + Index), (RandBuf[RIndex]));
	}
	/* IPCores library expects MSB bit to be 0 always */
	XSECURE_TEMPORAL_CHECK(END, Status, Ecdsa_ModEccOrder, Crv, RandBufEndianChange, (u8 *)(UINTPTR)PrvtKey);
	XSecure_GetData(XSECURE_ECC_P384_SIZE_IN_BYTES, (u8*)PrvtKey, PrivateKey->KeyOutPutAddr);

END:
	ClearStatus = XTrngpsx_Uninstantiate(TrngInstance);
	XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	ClearStatus |= Xil_SecureZeroize(RandBuf,
				 XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}
RET:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates 48-byte ephemeral key for P-384 curve using TRNG.
 *
 * @param	CrvType			Specifies the type of the ECC curve.
 * @param	EphemeralKeyAddr	Address of ephemeral key
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 * @note
 *	This API expects TRNG HW to be in HEALTHY state, This can
 *	be achieved by running preoperational health tests.
 *
 *****************************************************************************/
int XSecure_EllipticGenerateEphemeralKey(XSecure_EllipticCrvTyp CrvType,
	u32 EphemeralKeyAddr)
{
	volatile int Status = (int)XSECURE_ECC_PRVT_KEY_GEN_ERR;
	volatile int ClearStatus = XST_FAILURE;

	u8 RandBuf[XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN] = {0x00};
	u32 EphimeralKey[XSECURE_ECC_P384_SIZE_IN_BYTES];
	EcdsaCrvInfo *Crv = NULL;

	if (CrvType == XSECURE_ECC_NIST_P521) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto RET;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_GetRandomNum, RandBuf, XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);

	/* Take ECDSA core out if reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(XSECURE_ECC_NIST_P384);
	if (Crv == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	/* IPCores library expects MSB bit to be 0 always */
	XSECURE_TEMPORAL_CHECK(END, Status, Ecdsa_ModEccOrder, Crv, RandBuf, (u8 *)(UINTPTR)EphimeralKey);
	XSecure_GetData(XSECURE_ECC_P384_SIZE_IN_BYTES, (u8*)EphimeralKey, EphemeralKeyAddr);

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	ClearStatus = Xil_SecureZeroize(RandBuf,
				 XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}
RET:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the signature on provided hash using ecc.
 *
 * @param	CrvType		specifies the type of the ECC curve.
 * @param	Hash		is the pointer to the hash of the data to be signed
 * @param	HashLen		is the length of the hash.
 * @param	PrvtKey		is the pointer to ECC private key.
 * @param	Signature	is the pointer to the buffer where the ECC signature
 *				shall be stored.
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_EllipticGenEphemeralNSign(XSecure_EllipticCrvTyp CrvType,
		const u8* Hash, u32 HashLen, u8 *PrvtKey, u8* Signature)
{
	volatile int Status = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	u8 EphemeralKey[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 *SigR = Signature;
	u8 *SigS = &Signature[XSECURE_ECC_P384_SIZE_IN_BYTES];
	XSecure_EllipticSign Sign = {SigR, SigS};

	if ((CrvType == XSECURE_ECC_NIST_P521) ||
			(HashLen != XSECURE_ECC_P384_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	/**
	 * Generate Ephemeral Key using TRNG for generating ECDSA signature
	 */
	Status = XSecure_EllipticGenerateEphemeralKey(CrvType,
							(UINTPTR)&EphemeralKey);
	if (Status != XST_SUCCESS) {
		goto CLEAR;
	}

	/**
	 * Generate Signature using Private Key to the provided hash
	 */
	Status = XSecure_EllipticGenerateSignature(CrvType, Hash,
		XSECURE_ECC_P384_SIZE_IN_BYTES, PrvtKey, EphemeralKey, &Sign);

CLEAR:
	/**
	 * Clear ephemeral key and private key
	 */
	ClearStatus = Xil_SecureZeroize(EphemeralKey, XSECURE_ECC_P384_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize(EphemeralKey, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if ((ClearStatus != XST_SUCCESS) || (ClearStatusTmp != XST_SUCCESS)) {
		Status = (ClearStatus | ClearStatusTmp);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is a wrapper function which calls the IPCores API to perform ECDH and
 * 		generate shared secret.
 *
 * @param	CrvType			Curve Type of the keys used to generate shared secret
 * @param	PrvtKeyAddr		64-bit address of the private key
 * @param	PubKeyAddr		64-bit address of public key
 * @param	SharedSecretAddr	64-bit address of buffer for storing shared secret
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  If any input parameter is invalid
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  If elliptic curve data received is NULL
 *		 - XST_FAILURE  On failure
 *
 * @note	Shared secret is calculated by performing scalar multiplication
 * 		on public key and private key provided as input.

 ******************************************************************************/
int XSecure_EcdhGetSecret(XSecure_EllipticCrvTyp CrvType, u64 PrvtKeyAddr, u64 PubKeyAddr,
	u64 SharedSecretAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	u8 SharedSecret[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 PrivKey[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];
	EcdsaCrvInfo *Crv = NULL;
	XSecure_EllipticKeyAddr KeyAddr;
	EcdsaKey Key;
	u32 Size = 0U;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521) &&
		(CrvType != XSECURE_ECC_NIST_P256)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (CrvType == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
		OffSet = Size;
	}
	else if(CrvType == XSECURE_ECC_NIST_P521){
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
		OffSet = Size + XSECURE_ECDSA_P521_ALIGN_BYTES;
	}
	else{
		Size = XSECURE_ECC_P256_SIZE_IN_BYTES;
		OffSet = Size;
	}

	Status = XSecure_ECCRandInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	KeyAddr.Qx = PubKeyAddr;
	KeyAddr.Qy = PubKeyAddr + (u64)OffSet;

	XSecure_PutData(Size, PrivKey, PrvtKeyAddr);
	XSecure_PutData(Size, (u8 *)PubKey, KeyAddr.Qx);
	XSecure_PutData(Size, (u8 *)(PubKey + OffSet), KeyAddr.Qy);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR, XSECURE_ECDSA_RSA_RESET_OFFSET);

	Status = XST_FAILURE;
	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		XSECURE_TEMPORAL_CHECK(END, Status, Ecdsa_CDH_Q, Crv, PrivKey, &Key, SharedSecret);
		XSecure_GetData(Size, SharedSecret, SharedSecretAddr);
	}
	else{
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	}

END:
	/**
	 * Zeroize local copy of key
	 */
	ClearStatus = Xil_SecureZeroize((u8*)PrivKey, XSECURE_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)PrivKey, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ClearStatusTmp | ClearStatus;
	}

	/**
	 * Zeroize local copy of shared secret
	 */
	ClearStatus = Xil_SecureZeroize((u8*)SharedSecret, XSECURE_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)SharedSecret, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ClearStatusTmp | ClearStatus;
	}

	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR, XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}
#endif
/** @} */
