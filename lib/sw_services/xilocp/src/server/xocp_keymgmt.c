/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_keymgmt.c
*
* This file contains the implementation of the interface functions for key
* management related to DEVIK and DEVAK.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  07/08/22 Initial release
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_keymgmt.h"
#include "xocp_hw.h"
#include "xplmi_dma.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_sha.h"
#include "xsecure_hmac.h"
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_ellipticplat.h"
#include "xil_util.h"
#include "xsecure_init.h"
#include "xsecure_plat_kat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XOcp_KeyGenDevAkSeed(u32 KeyAddr, u32 KeyLen, u32 DataAddr,
			 u32 DataLen, XSecure_HmacRes *Out);
static int XOcp_KeyZeroize(u32 CtrlReg, u32 StatusReg);
static int XOcp_KeyGenerateDevIk(void);
static int XOcp_KeyGenerateDevAk(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function generates the DEVIK DEVAK ECC private and public
 *		key pair and generates the signatures for both the keys.
 *
 * @return
 *	-	XST_SUCCESS - If whole operation is success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_KeyInit(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	if ((XPlmi_IsKatRan(XPLMI_SECURE_TRNG_KAT_MASK) != TRUE) ||
		(TrngInstance->ErrorState != XSECURE_TRNG_HEALTHY)) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status,
			StatusTmp, XSecure_TrngPreOperationalSelfTests, TrngInstance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
	}

	/* Generate private and public key pair for ECC */
	Status = XOcp_KeyGenerateDevIk();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_KeyGenerateDevAk();

END:
	/* Zeroize private keys */
	ClearStatus = XOcp_KeyZeroize(XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_CTRL,
			XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_STATUS);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}
	ClearStatus = XOcp_KeyZeroize(XOCP_PMC_GLOBAL_DEV_AK_PRIVATE_ZEROIZE_CTRL,
			XOCP_PMC_GLOBAL_DEV_AK_PRIVATE_ZEROIZE_STATUS);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes the data using specified control registers.
 *
 * @param	CtrlReg holds the address of zeroize control register.
 * @param	StatusReg holds the address of status register.
 *
 * @return
 *		- XST_SUCCESS - If zeroization status is pass
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
static int XOcp_KeyZeroize(u32 CtrlReg, u32 StatusReg)
{
	int Status = XST_FAILURE;
	u32 ReadReg;

	XPlmi_Out32(CtrlReg, XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK);

	Status = (int)Xil_WaitForEvent(StatusReg,
			XOCP_PMC_GLOBAL_ZEROIZE_STATUS_DONE_MASK,
			XOCP_PMC_GLOBAL_ZEROIZE_STATUS_DONE_MASK,
			XOCP_TIMEOUT_MAX);
	if (Status == XST_SUCCESS) {
		ReadReg = XPlmi_In32(StatusReg) &
				XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK;
		if (ReadReg != XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK) {
			Status = XST_FAILURE;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the DEVIK ECC private and public key pair.
 *
 * @return
 *	-	XST_SUCCESS - If key generation is success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
static int XOcp_KeyGenerateDevIk(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u8 Seed[XOCP_CDI_SIZE_IN_BYTES];
	u8 PersString[XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES];
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccPrvtKey[XOCP_ECC_P384_SIZE_BYTES];
	u32 ClrStatus = XST_FAILURE;
	u8 CryptoKatEn = TRUE;
	u8 CryptoKatEnTmp = TRUE;

	/* Copy CDI from PMC global registers to Seed buffer */
	Status = Xil_SMemCpy((void *)Seed, XOCP_CDI_SIZE_IN_BYTES,
				(const void *)XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
				XOCP_CDI_SIZE_IN_BYTES,
				XOCP_CDI_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Copy Personalized string to buffer, here the input string is DNA
	 * which is of size 16 bytes where as TRNG requires 48 bytes of data as
	 * personalized so the remaining bytes are set to zero.
	 */
	 Status = Xil_SMemSet(PersString, XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES,
			0U, XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy((void *)PersString,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			(const void *)XOCP_EFUSE_DEVICE_DNA_CACHE,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate the DEV IK public and private keys */
	KeyGenParams.SeedAddr = (u32)Seed;
	KeyGenParams.SeedLength = XOCP_CDI_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)PersString;
	KeyGenParams.KeyOutPutAddr = (u32)EccPrvtKey;
	Status = XSecure_EllipticPrvtKeyGenerate(XSECURE_ECC_NIST_P384,
				&KeyGenParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PubKeyAddr.Qx = (UINTPTR)(u8*)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
	PubKeyAddr.Qy = (UINTPTR)(u8*)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0;
	Status = XSecure_EllipticGenerateKey_64Bit(XSECURE_ECC_NIST_P384,
			(u64)(UINTPTR)EccPrvtKey, &PubKeyAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = CryptoKatEn;
	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_EllipticPwct,
			XSECURE_ECC_NIST_P384, (u64)(UINTPTR)EccPrvtKey, &PubKeyAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK);
	}

	/* Copy Private key to PMC global registers */
	Status = Xil_SMemCpy((void *)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
				XOCP_ECC_P384_SIZE_BYTES, (const void *)EccPrvtKey,
				XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	ClrStatus = Xil_SMemSet(EccPrvtKey, XOCP_ECC_P384_SIZE_BYTES,
				0U, XOCP_ECC_P384_SIZE_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SMemSet(Seed, XOCP_CDI_SIZE_IN_BYTES,
				0U, XOCP_CDI_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SMemSet(PersString,
				XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES,
				0U, XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the DEVAK ECC private and public key pair.
 *
 * @return
 *	-	XST_SUCCESS - If key generation is success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
static int XOcp_KeyGenerateDevAk(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u8 CerberusHash[XSECURE_HASH_SIZE_IN_BYTES] = {0U};
	u8 Seed[XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES];
	u8 PersString[XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES];
	u8 EccPrvtKey[XOCP_ECC_P384_SIZE_BYTES];
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];
	int ClrStatus = XST_FAILURE;
	u8 CryptoKatEn = TRUE;
	u8 CryptoKatEnTmp = TRUE;

	Status = XOcp_KeyGenDevAkSeed(XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
				XOCP_CDI_SIZE_IN_BYTES, (u32)CerberusHash,
				XSECURE_HASH_SIZE_IN_BYTES,
				(XSecure_HmacRes *)Seed);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * For time being the personalized string is taken as DNA and this will be
	 * an input in future and is open item for now.
	 * copy Personalized string to buffer, here the input string is DNA
	 * which is of size 16 bytes where as TRNG requires 48 bytes of data as
	 * personalized string, so remaining bytes are set to zero.
	 */
	 Status = Xil_SMemSet(PersString, XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES, 0U,
				XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy((void *)PersString,
				XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
				(const void *)XOCP_EFUSE_DEVICE_DNA_CACHE,
				XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
				XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate the DEV AK public and private keys */
	KeyGenParams.SeedAddr = (u32)Seed;
	KeyGenParams.SeedLength = XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)PersString;
	KeyGenParams.KeyOutPutAddr = (u32)EccPrvtKey;
	Status = XSecure_EllipticPrvtKeyGenerate(XSECURE_ECC_NIST_P384, &KeyGenParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PubKeyAddr.Qx = (UINTPTR)(u8*)EccX;
	PubKeyAddr.Qy = (UINTPTR)(u8*)EccY;
	Status = XSecure_EllipticGenerateKey_64Bit(XSECURE_ECC_NIST_P384,
			(u64)(UINTPTR)EccPrvtKey, &PubKeyAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = CryptoKatEn;
	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_EllipticPwct,
			XSECURE_ECC_NIST_P384, (u64)(UINTPTR)EccPrvtKey, &PubKeyAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
	}

	/* Copy Public key to PMC global registers */
	Status = Xil_SMemCpy((void *)XOCP_PMC_GLOBAL_DEV_AK_PUBLIC_X_0,
			XOCP_ECC_P384_SIZE_BYTES, (const void *)(UINTPTR)PubKeyAddr.Qx,
			XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy((void *)XOCP_PMC_GLOBAL_DEV_AK_PUBLIC_Y_0,
			XOCP_ECC_P384_SIZE_BYTES, (const void *)(UINTPTR)PubKeyAddr.Qy,
			XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy Private key to PMC global registers */
	Status = Xil_SMemCpy((void *)XOCP_PMC_GLOBAL_DEV_AK_PRIVATE_0,
			XOCP_ECC_P384_SIZE_BYTES, (const void *)EccPrvtKey,
			XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
END:
	ClrStatus = Xil_SMemSet(EccPrvtKey, XOCP_ECC_P384_SIZE_BYTES,
				0U, XOCP_ECC_P384_SIZE_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SMemSet(Seed, XOCP_CDI_SIZE_IN_BYTES,
				0U, XOCP_CDI_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SMemSet(PersString,
				XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES,
				0U, XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SMemSet(CerberusHash, XSECURE_HASH_SIZE_IN_BYTES,
				0U, XSECURE_HASH_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the seed for DEVAK key generation using
 *		HMAC
 *
 * @param	KeyAddr holds the address of key to be used for HMAC.
 * @param	KeyLen specifies the length of the key.
 * @param	DataAddr holds the data address to be updated to HMAC.
 * @param	DataLen specifies the length of the data to be updated to HMAC.
 * @param	Out is a pointer of type XSecure_HmacRes where the resultant gets
 *		updated.
 *
 * @return
 *	-	XST_SUCCESS - upon success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
static int XOcp_KeyGenDevAkSeed(u32 KeyAddr, u32 KeyLen, u32 DataAddr,
			 u32 DataLen, XSecure_HmacRes *Out)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(0U);
	XSecure_Sha3 Sha3Instance = {0U};
	XSecure_Hmac HmacInstance;

	Status = XSecure_Sha3Initialize(&Sha3Instance, PmcDmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_Sha3Kat,
				&Sha3Instance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_HMAC_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_HmacKat,
				&Sha3Instance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_HMAC_KAT_MASK);
	}

	Status = XSecure_HmacInit(&HmacInstance, &Sha3Instance, KeyAddr, KeyLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_HmacUpdate(&HmacInstance, (u64)DataAddr, DataLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_HmacFinal(&HmacInstance, Out);

END:
	return Status;
}
