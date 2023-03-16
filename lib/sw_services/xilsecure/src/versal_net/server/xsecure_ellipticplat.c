/******************************************************************************
* Copyright (c) 2022 - 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticplat.c
*
* This file contains the implementation of the interface functions for ECC
* engine specific to versal net platform.
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
*
*
* </pre>
*
* @note
*
******************************************************************************/

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
#include "xil_util.h"

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
 * This function generates 48-byte key for P-384 curve using TRNG.
 *
 * @param	CrvType specifies the type of the ECC curve.
 * @param	PrivateKey is the pointer to XSecure_ElliptcPrivateKeyGen
 *
 * @return	XST_SUCCESS on success
 *		Error code otherwise
 *
 * @note
 *      This API expects TRNG HW to be in HEALTHY state, This can
 *      be achieved by running preoperational health tests.
 *
 *****************************************************************************/
int XSecure_EllipticPrvtKeyGenerate(XSecure_EllipticCrvTyp CrvType,
	XSecure_ElliptcPrivateKeyGen *PrivateKey)
{
	volatile int Status = XSECURE_ECC_PRVT_KEY_GEN_ERR;
	volatile int ClearStatus = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	XSecure_TrngUserConfig TrngUserCfg;
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
	EcdsaCrvInfo *Crv = NULL;

	if ((CrvType == XSECURE_ECC_NIST_P521) || (PrivateKey == NULL)) {
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto RET;
	}

	Status = Xil_SMemSet(&TrngUserCfg, sizeof(XSecure_TrngUserConfig), 0U,
					sizeof(XSecure_TrngUserConfig));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (TrngInstance->State != XSECURE_TRNG_UNINITIALIZED_STATE) {
		Status = XSecure_TrngUninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	TrngUserCfg.Mode = XSECURE_TRNG_DRNG_MODE;
	TrngUserCfg.DFLength = XSECURE_ECC_TRNG_DF_LENGTH;
	TrngUserCfg.SeedLife = XSECURE_TRNG_DEFAULT_SEED_LIFE;
	Status = XSecure_TrngInstantiate(TrngInstance,
			(u8 *)(UINTPTR)PrivateKey->SeedAddr, PrivateKey->SeedLength,
			(u8 *)(UINTPTR)PrivateKey->PerStringAddr, &TrngUserCfg);
	if (Status  != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_TrngGenerate(TrngInstance, RandBuf,
			XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_TrngGenerate(TrngInstance,
			RandBuf + XSECURE_TRNG_SEC_STRENGTH_IN_BYTES,
			XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN -
			XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
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

	/* IPCores library expects MSB bit to be 0 always */
	Status = Ecdsa_ModEccOrder(Crv, RandBuf,
			(u8 *)(UINTPTR)PrivateKey->KeyOutPutAddr);

END:
	ClearStatus = XSecure_TrngUninstantiate(TrngInstance);
	ClearStatus |= XSecure_TrngInitNCfgHrngMode();
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}

	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	ClearStatus = Xil_SMemSet((void *)RandBuf,
				 XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN,
				0U, XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}
RET:
	return Status;
}

/*****************************************************************************/
/**
 * This function generates 48-byte ephemeral key for P-384 curve using TRNG.
 *
 * @param	CrvType specifies the type of the ECC curve.
 * @param	EphemeralKeyAddr Address of ephemeral key
 *
 * @return	XST_SUCCESS on success
 *		Error code otherwise
 *
 * @note
 *      This API expects TRNG HW to be in HEALTHY state, This can
 *      be achieved by running preoperational health tests.
 *
 *****************************************************************************/
int XSecure_EllipticGenerateEphemeralKey(XSecure_EllipticCrvTyp CrvType,
	u32 EphemeralKeyAddr)
{
	volatile int Status = XSECURE_ECC_PRVT_KEY_GEN_ERR;
	volatile int ClearStatus = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	XSecure_TrngUserConfig TrngUserCfg;

	u8 RandBuf[XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN] = {0x00};
	EcdsaCrvInfo *Crv = NULL;

	if (CrvType == XSECURE_ECC_NIST_P521) {
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto RET;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, &TrngUserCfg, sizeof(XSecure_TrngUserConfig), 0U,
		sizeof(XSecure_TrngUserConfig));

	if (TrngInstance->State != XSECURE_TRNG_UNINITIALIZED_STATE) {
		XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngUninstantiate, TrngInstance);
	}

	TrngUserCfg.Mode = XSECURE_TRNG_HRNG_MODE;
	TrngUserCfg.AdaptPropTestCutoff = XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	TrngUserCfg.RepCountTestCutoff = XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF;
	TrngUserCfg.DFLength = XSECURE_TRNG_USER_CFG_DF_LENGTH;
	TrngUserCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE;

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngInstantiate, TrngInstance, NULL, 0U, NULL, &TrngUserCfg);

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngGenerate, TrngInstance, RandBuf, XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngGenerate, TrngInstance, RandBuf + XSECURE_TRNG_SEC_STRENGTH_IN_BYTES,
		XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN - XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);

	/* Take ECDSA core out if reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(XSECURE_ECC_NIST_P384);
	if (Crv == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	/* IPCores library expects MSB bit to be 0 always */
	XSECURE_TEMPORAL_CHECK(END, Status, Ecdsa_ModEccOrder, Crv, RandBuf, (u8 *)(UINTPTR)EphemeralKeyAddr);

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	ClearStatus = Xil_SMemSet((void *)RandBuf,
				 XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN,
				0U, XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);
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
 * @param	Hash is the pointer to the hash of the data to be signed
 * @param	HashLen is the length of the hash.
 * @param	PrvtKey is the pointer to ECC private key.
 * @param	Signature is the pointer to the buffer where the ECC signature
 *		shall be stored.
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
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}
	/**
	 * Initialize TRNG to generate Ephemeral Key
	 */
	Status = XSecure_TrngInitNCfgHrngMode();
	if (Status != XST_SUCCESS) {
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

#endif
