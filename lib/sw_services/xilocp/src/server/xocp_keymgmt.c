/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.1   vns  12/27/22 Skips the device key generation if CDI is not valid
*       vns  01/10/23 Adds logic to generate the DEVAK on subsystem based.
* 1.2   har  02/24/23 Added logic to get index of usr cfg for requested subsystem ID
*       vns  07/06/23 Added DEVAK regenerate support and Data clear before shutdown
*       am   07/20/23 Cleared DICE_CDI seed
*       kpt  07/25/23 Add redundancy for key generation APIs
*       yog  08/07/23 Replaced trng API calls using trngpsx driver
*       am   09/04/23 Added XOcp_ValidateDiceCdi function
* 1.3   kpt  11/06/23 Add support to run ECC KAT before attestation
*       kpt  11/24/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       ng   02/12/24 optimised u8 vars to u32 for size reduction
*       am   02/14/24 Fixed internal security review comments
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MNGMT
#include "xocp_keymgmt.h"
#include "xocp_hw.h"
#include "xocp_def.h"
#include "xocp_common.h"
#include "xplmi_dma.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_hmac.h"
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_ellipticplat.h"
#include "xil_util.h"
#include "xsecure_init.h"
#include "xsecure_plat_kat.h"
#include "xcert_genx509cert.h"
#include "xsecure_defs.h"
#include "xplmi_update.h"

/************************** Constant Definitions *****************************/

#define XOCP_PMC_SUBSYSTEM_ID				(0x1C000001U)	/**< PMC Subsystem ID */
#define XOCP_DEVAK_SUBSYS_HASH_VERSION 			(1U) /**< DEVAK subsys hash version list */
#define XOCP_DEVAK_SUBSYS_HASH_LCVERSION 		(1U) /**< DEVAK subsys lowest compatible
							      * version list */
#define XOCP_SUBSYSTEM_ID_0				(0U) /**< SubSystem Id zero */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XOcp_KeyGenDevAkSeed(u32 CdiAddr, u32 CdiLen, u32 DataAddr,
	u32 DataLen, XSecure_HmacRes *Out);
static int XOcp_KeyZeroize(u32 CtrlReg, UINTPTR StatusReg);
static int XOcp_KeyGenerateDevIk(void);
static XOcp_KeyMgmt *XOcp_GetKeyMgmtInstance(void);
static XOcp_SubSysHash *XOcp_GetSubSysHash(void);
static int XOcp_ValidateDiceCdi(void);
static XOcp_DevAkData *XOcp_GetDevAkData(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function provides the pointer to the common XOcp_DevAkData
 *              instance which has to be used across the project to store the data.
 *
 * @return
 *          -   Pointer to the XOcp_DevAkData instance
 *
 ******************************************************************************/
static XOcp_DevAkData *XOcp_GetDevAkData(void)
{
	static XOcp_DevAkData DevAkData[XOCP_MAX_DEVAK_SUPPORT] = {0U};

	return &DevAkData[0];
}

/*****************************************************************************/
/**
 * @brief       This function gets the hash(s) of the corresponding subsystem(s)
 *              which will be updated during DEVAK generation and stored during an
 *              in place PLM update to regenerate the DEVAK.
 *
 * @return
 *          -   Pointer to the XOcp_ instance
 *
 ******************************************************************************/
static XOcp_SubSysHash *XOcp_GetSubSysHash(void)
{
	static XOcp_SubSysHash SubSysHash[XOCP_MAX_DEVAK_SUPPORT]
				__attribute__ ((aligned(4U))) = {0U};

	EXPORT_OCP_DS(SubSysHash, XOCP_DEVAK_SUBSYS_HASH_DS_ID,
		XOCP_DEVAK_SUBSYS_HASH_VERSION, XOCP_DEVAK_SUBSYS_HASH_LCVERSION,
		sizeof(SubSysHash), (u32)(UINTPTR)&SubSysHash[0]);

	return &SubSysHash[0];
}

/*****************************************************************************/
/**
 * @brief       This function returns whether Device identity key is ready or not.
 *
 * @return
 *		- FALSE if DEV IK is not ready
 *		- TRUE if DEV IK is ready
 *
 ******************************************************************************/
u32 XOcp_IsDevIkReady(void)
{
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	return KeyInstPtr->KeyMgmtReady;
}

/*****************************************************************************/
/**
 * @brief   This function generates the DEVIK ECC private and public
 *          key pair and generates the self-signed X.509 certificate to share
 *          DEVIK public key.
 *
 * @return
 *	-	XST_SUCCESS - If whole operation is success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GenerateDevIKKeyPair(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatusTmp = XST_FAILURE;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	KeyInstPtr->KeyMgmtReady = FALSE;

	/* If CDI is not valid device key generation is skipped */
	if (XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID) == 0x0U) {
		XOcp_Printf(DEBUG_GENERAL, "Device key init is skipped"
			" as no valid CDI is found\n\r");
		Status = XST_SUCCESS;
		goto RET;
	}

	/* Read and validate whether, DICE CDI SEED is valid or not */
	Status = XOcp_ValidateDiceCdi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((XPlmi_IsKatRan(XPLMI_SECURE_TRNG_KAT_MASK) != TRUE) ||
		(TrngInstance->ErrorState != XTRNGPSX_HEALTHY)) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status,
			SStatusTmp, XTrngpsx_PreOperationalSelfTests, TrngInstance);
		if ((Status != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
	}

	/* Generate private and public key pair for ECC */
	XSECURE_TEMPORAL_CHECK(END, Status, XOcp_KeyGenerateDevIk);

	XOcp_Printf(DEBUG_INFO, "Generated DEV IK\n\r");

	KeyInstPtr->KeyMgmtReady = TRUE;

END:
	if (Status != XST_SUCCESS) {
		/* Zeroize private keys */
		Status = XOcp_KeyZeroize(XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_CTRL,
				(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_STATUS);
	}
RET:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function regenerates the DEVAK ECC private and public
 *		key pair of the susbsytems after in place PLM update.
 *
 * @return
 *	-	XST_SUCCESS - If whole operation is success
 *	-	XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_RegenSubSysDevAk(void)
{
	volatile int Status = XST_FAILURE;
	XOcp_SubSysHash *SubSysHashDs = XOcp_GetSubSysHash();
	XOcp_DevAkData	*DevAkData = XOcp_GetDevAkData();
	XOcp_KeyMgmt *KeyMgmtInstance = XOcp_GetKeyMgmtInstance();
	volatile u32 Index = 0U;
	u32 DevAkIndex = 0U;

	if (KeyMgmtInstance->KeyMgmtReady != TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}
	do {
		for (Index = 0U; Index < XOCP_MAX_DEVAK_SUPPORT; Index++) {
			if ((DevAkData->SubSystemId == SubSysHashDs->SubSystemId) &&
				(SubSysHashDs->ValidData == TRUE)) {
				Status = Xil_SMemCpy((void *)(UINTPTR)DevAkData->SubSysHash,
						XSECURE_HASH_SIZE_IN_BYTES,
						(const void *)(UINTPTR)SubSysHashDs->SubSysHash,
						 XSECURE_HASH_SIZE_IN_BYTES,
						XSECURE_HASH_SIZE_IN_BYTES);
				if (Status != XST_SUCCESS) {
					goto END;
				}

				XSECURE_TEMPORAL_CHECK(END, Status, XOcp_GenerateDevAk, DevAkData->SubSystemId);
				break;
			}
			SubSysHashDs++;
		}
		DevAkIndex++;
		DevAkData++;
	} while ((DevAkIndex < XOCP_MAX_DEVAK_SUPPORT) &&
			(DevAkIndex < KeyMgmtInstance->DevAkInputIndex));

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the DEVAK input personalized string along with
 *		corresponding subsystem ID.
 *
 * @param	SubSystemId of which DEVAK shall be generated.
 * @param	PerString holds 48 byte personalised string to the corresponding
 *		subsystem ID.
 *
 * @return
 *		- XST_SUCCESS - If whole operation is success
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_DevAkInputStore(u32 SubSystemId, u8 *PerString)
{
	int Status = XST_FAILURE;
	XOcp_KeyMgmt *KeyMgmtInstance = XOcp_GetKeyMgmtInstance();
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();

	if (KeyMgmtInstance->KeyMgmtReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "No Device keys are supported\n\r");
		Status = XST_SUCCESS;
		goto END;
	}
	if (KeyMgmtInstance->DevAkInputIndex >= XOCP_MAX_DEVAK_SUPPORT) {
		XOcp_Printf(DEBUG_GENERAL,
			"Maximum count of DEVAK supported is %d\r\n",
			XOCP_MAX_DEVAK_SUPPORT);
		Status = (int)XOCP_DEVAK_MAX_COUNT_EXCEED;
		goto END;
	}

	DevAkData[KeyMgmtInstance->DevAkInputIndex].SubSystemId = SubSystemId;

	Status = XPlmi_MemCpy64(
			(u64)(UINTPTR)DevAkData[KeyMgmtInstance->DevAkInputIndex].PerString,
			(u64)(UINTPTR)PerString, XTRNGPSX_PERS_STRING_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Each time this index points to the next empty DEVAK input array */
	KeyMgmtInstance->DevAkInputIndex++;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the DEVAK ECC private and public key pair.
 *
 * @param	SubSystemId of the PDI for which DEVAK generation is requested.
 *
 * @return
 *		- XST_SUCCESS - If key generation is success
 *		- Error code - Upon any failure
 *
 ******************************************************************************/
int XOcp_GenerateDevAk(u32 SubSystemId)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatusTmp = XST_FAILURE;
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 DevAkIndex = XOcp_GetSubSysDevAkIndex(SubSystemId);
	u8 Seed[XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES];
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];
	int ClrStatus = XST_FAILURE;
	volatile u32 CryptoKatEn = TRUE;
	volatile u32 CryptoKatEnTmp = TRUE;
	XOcp_SubSysHash *SubSysHashDs = XOcp_GetSubSysHash();
	SubSysHashDs = SubSysHashDs + DevAkIndex;

	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = (int)XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}
	XOcp_Printf(DEBUG_INFO, "Generating DEV AK of subsystem ID %x\n\r", SubSystemId);

	DevAkData = DevAkData + DevAkIndex;
	DevAkData->IsDevAkKeyReady = FALSE;

	XSECURE_TEMPORAL_CHECK(END, Status, XOcp_KeyGenDevAkSeed, XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
		XOCP_CDI_SIZE_IN_BYTES, (u32)(UINTPTR)DevAkData->SubSysHash, XSECURE_HASH_SIZE_IN_BYTES,
		(XSecure_HmacRes *)(UINTPTR)Seed);

	/* Generate the DEV AK public and private keys */
	KeyGenParams.SeedAddr = (u32)(UINTPTR)Seed;
	KeyGenParams.SeedLength = XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)(UINTPTR)DevAkData->PerString;
	KeyGenParams.KeyOutPutAddr = (u32)(UINTPTR)DevAkData->EccPrvtKey;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticPrvtKeyGenerate, XSECURE_ECC_NIST_P384,
						   &KeyGenParams);

	PubKeyAddr.Qx = (u64)(UINTPTR)EccX;
	PubKeyAddr.Qy = (u64)(UINTPTR)EccY;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticGenerateKey_64Bit, XSECURE_ECC_NIST_P384,
						   (u64)(UINTPTR)DevAkData->EccPrvtKey, &PubKeyAddr);

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = CryptoKatEn;
	if ((CryptoKatEn == (u32)TRUE) || (CryptoKatEnTmp == (u32)TRUE)) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, SStatusTmp, XSecure_EllipticPwct,
			XSECURE_ECC_NIST_P384, (u64)(UINTPTR)DevAkData->EccPrvtKey, &PubKeyAddr);
		if ((Status != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
	}
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)DevAkData->EccX,
						(u64)(UINTPTR)EccX);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)DevAkData->EccY,
						(u64)(UINTPTR)EccY);

	DevAkData->IsDevAkKeyReady = TRUE;
	/* Store hash of the sub-system */
	SubSysHashDs->ValidData = FALSE;
	SubSysHashDs->SubSystemId = DevAkData->SubSystemId;
	Status = Xil_SMemCpy((void *)(UINTPTR)SubSysHashDs->SubSysHash, XSECURE_HASH_SIZE_IN_BYTES,
				(const void *)(UINTPTR)DevAkData->SubSysHash,
				XSECURE_HASH_SIZE_IN_BYTES,
				XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		SubSysHashDs->ValidData = TRUE;
	}

	XPlmi_PrintArray(DEBUG_DETAILED, (u64)(UINTPTR)DevAkData->EccPrvtKey,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PRVT KEY");
	XPlmi_PrintArray(DEBUG_INFO, (u64)(UINTPTR)DevAkData->EccX,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PUB KEY X");
	XPlmi_PrintArray(DEBUG_INFO, (u64)(UINTPTR)DevAkData->EccY,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PUB KEY Y");

END:
	ClrStatus = Xil_SecureZeroize(Seed, XOCP_CDI_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function returns the DEVAK index of the corresponding subsystem,
 *		if it doesn't match with data base, this function returns invalid index.
 *
 * @param	SubSystemId holds the sub system ID of whose corresponding DEVAK
 *		index is requested.
 *
 * @return
 *	-	Index of XOcp_DevAkData array
 *	-	XOCP_INVALID_DEVAK_INDEX
 *
 ******************************************************************************/
u32 XOcp_GetSubSysDevAkIndex(u32 SubSystemId)
{
	u32 DevAkIndex = XOCP_INVALID_DEVAK_INDEX;
	XOcp_KeyMgmt *KeyMgmtInstance = XOcp_GetKeyMgmtInstance();
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 Index = 0U;

	/* Returns invalid DEVAK index if no device key is supported */
	if (KeyMgmtInstance->KeyMgmtReady != TRUE) {
		goto END;
	}
	while (Index < KeyMgmtInstance->DevAkInputIndex) {
		if (SubSystemId == DevAkData->SubSystemId) {
			DevAkIndex = Index;
			break;
		}
		Index++;
		DevAkData++;
	}

END:
	return DevAkIndex;
}

/*****************************************************************************/
/**
 * @brief	This function generates the X.509 Certificate for device keys
 *
 * @param	XOcp_GetX509CertPtr points to the address of XOcp_X509Cert structure.
 * @param	SubSystemId is the ID of the subsystem from which certificate is
 *		requested.
 *
 * @return
 *		- XST_SUCCESS - If X.509 certificate generation is success
 *		- Error code - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetX509Certificate(XOcp_X509Cert *XOcp_GetX509CertPtr, u32 SubSystemId)
{
	volatile int Status = XST_FAILURE;
	u32 DevAkIndex;
	XOcp_DevAkData *DevAkData = NULL;
	XCert_Config CertConfig;
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	if ((XOcp_GetX509CertPtr->DevKeySel != XOCP_DEVIK) &&
			(XOcp_GetX509CertPtr->DevKeySel != XOCP_DEVAK)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (KeyInstPtr->KeyMgmtReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "No Device keys are supported\n\r");
		Status = (int)XOCP_ERR_DEVIK_NOT_READY;
		goto END;
	}

	if (XOcp_GetX509CertPtr->DevKeySel == XOCP_DEVIK) {
		CertConfig.SubSystemId = XOCP_PMC_SUBSYSTEM_ID;
		CertConfig.AppCfg.IsSelfSigned = TRUE;
		CertConfig.AppCfg.SubjectPublicKey =
				(u8 *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
		CertConfig.AppCfg.IssuerPrvtKey = (u8 *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0;
		CertConfig.AppCfg.IssuerPublicKey = (u8 *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
		if (XOcp_GetX509CertPtr->IsCsr != TRUE) {
			CertConfig.AppCfg.IsCsr = FALSE;
			CertConfig.AppCfg.FwHash = (u8 *)(UINTPTR)XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0;
		}
		else {
			CertConfig.AppCfg.IsCsr = TRUE;
		}
	}
	else {
		DevAkIndex = XOcp_GetSubSysDevAkIndex(SubSystemId);
		if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
			Status = (int)XOCP_ERR_INVALID_DEVAK_REQ;
			goto END;
		}

		DevAkData = XOcp_GetDevAkData();
		DevAkData = DevAkData + DevAkIndex;
		if (DevAkData->IsDevAkKeyReady != TRUE) {
			XOcp_Printf(DEBUG_DETAILED, "Device Attestation key is not generated\n\r");
			Status = (int)XOCP_ERR_DEVAK_NOT_READY;
			goto END;
		}
		CertConfig.SubSystemId = SubSystemId;
		CertConfig.AppCfg.IsSelfSigned = FALSE;
		CertConfig.AppCfg.SubjectPublicKey = (u8 *)(UINTPTR)DevAkData->EccX;
		CertConfig.AppCfg.IssuerPrvtKey = (u8 *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0;
		CertConfig.AppCfg.IssuerPublicKey = (u8 *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
		CertConfig.AppCfg.FwHash = DevAkData->SubSysHash;
	}

	Status = XCert_GenerateX509Cert(XOcp_GetX509CertPtr->CertAddr,
			(u32)(UINTPTR)XOcp_GetX509CertPtr->CertSize,
			(u32 *)(UINTPTR)XOcp_GetX509CertPtr->ActualLenAddr, &CertConfig);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function allows user to sign his own data using device
 *		attestation key. The PLM signs the hash of the data supplied by
 *		the user as input. The PLM uses ECC P-384 DevAK private
 *		key to sign the input hash.
 *
 * @param   	AttestWithDevAkPtr - Address of XOcp_AttestWithDevAk structure.
 * @param	SubSystemId holds the image ID.
 *
 * @return
 *		- XST_SUCCESS - Upon successful attestation
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_AttestWithDevAk(XOcp_Attest *AttestWithDevAkPtr, u32 SubSystemId)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 DevAkIndex;
	XOcp_DevAkData *DevAkData = NULL;
	u8 Hash[XSECURE_ECC_P384_SIZE_IN_BYTES];
	u8 Signature[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];

	if ((SubSystemId == XOCP_SUBSYSTEM_ID_0) ||
		(AttestWithDevAkPtr->HashAddr == 0x0U) ||
		(AttestWithDevAkPtr->SignatureAddr == 0x0U)) {
		goto END;
	}

	DevAkIndex = XOcp_GetSubSysDevAkIndex(SubSystemId);
	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = (int)XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}

	DevAkData = XOcp_GetDevAkData();
	DevAkData = DevAkData + DevAkIndex;
	if (DevAkData->IsDevAkKeyReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "Device Attestation key is not generated\n\r");
		Status = (int)XOCP_ERR_DEVAK_NOT_READY;
		goto END;
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XSECURE_KAT_MAJOR_ERROR, Status, StatusTmp,
			XSecure_EllipticSignGenerateKat, (XSecure_EllipticCrvClass)XSECURE_ECC_PRIME);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK);
	}

	/* Covert hash to little endian */
	XSecure_FixEndiannessNCopy(AttestWithDevAkPtr->HashLen,
		(u64)(UINTPTR)Hash, AttestWithDevAkPtr->HashAddr);
	/* Generate the signature using DEVAK */
	Status = XSecure_EllipticGenEphemeralNSign(XSECURE_ECC_NIST_P384, Hash,
			AttestWithDevAkPtr->HashLen,
			(u8 *)(UINTPTR)DevAkData->EccPrvtKey,
			Signature);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			AttestWithDevAkPtr->SignatureAddr,
				(u64)(UINTPTR)Signature);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
		AttestWithDevAkPtr->SignatureAddr + XSECURE_ECC_P384_SIZE_IN_BYTES,
		(u64)(UINTPTR)(Signature + XSECURE_ECC_P384_SIZE_IN_BYTES));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes DEVIK and CDI SEED during OCP module shutdown.
 *
 * @param	Op - OCP module operation
 *
 * @return
 *		- XST_SUCCESS - If zeroization status is pass
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ShutdownHandler(XPlmi_ModuleOp Op)
{
	volatile int Status = XST_FAILURE;
	static u32 OcpHandlerState = XPLMI_MODULE_NORMAL_STATE;
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	if (Op.Mode == XPLMI_MODULE_SHUTDOWN_INITIATE) {
		if (OcpHandlerState == XPLMI_MODULE_NORMAL_STATE) {
			OcpHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;
			Status = XST_SUCCESS;
		}
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_COMPLETE) {
		if (OcpHandlerState == XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE) {
			Status = XST_SUCCESS;
			goto END;
		}
		if (OcpHandlerState != XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			goto END;
		}
		if (KeyInstPtr->KeyMgmtReady == TRUE) {
			/* Zeroize DEVIK */
			Status = XOcp_KeyZeroize(XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_CTRL,
				(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_STATUS);
				/* Zeroize DICE CDI SEED irrespective of DEVIK Zeroize status */
			Status |= XOcp_KeyZeroize(XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_CTRL,
				(UINTPTR)XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_STATUS);
		}
		else {
			Status = XST_SUCCESS;
		}
		OcpHandlerState = XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE;
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_ABORT) {
		if (OcpHandlerState == XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			OcpHandlerState = XPLMI_MODULE_NORMAL_STATE;
			Status = XST_SUCCESS;
		}
	}
	else {
		Status = (int)XST_INVALID_PARAM;
	}

END:
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
static int XOcp_KeyZeroize(u32 CtrlReg, UINTPTR StatusReg)
{
	int Status = XST_FAILURE;
	u32 ReadReg;

	/* Writes data to 32-bit address and checks for blind writes */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureOut32, CtrlReg,
		XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK);

	Status = (int)Xil_WaitForEvent(StatusReg,
			XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK,
			XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK,
			XOCP_TIMEOUT_MAX);
	if (Status == XST_SUCCESS) {
		ReadReg = XPlmi_In32(StatusReg) &
				XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK;
		if (ReadReg != XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK) {
			Status = XST_FAILURE;
		}
	}

END:
	/* Clearing Zeroize Control register */
	XPlmi_Out32(CtrlReg, XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_CLEAR_MASK);

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
	volatile int SStatusTmp = XST_FAILURE;
	u8 Seed[XOCP_CDI_SIZE_IN_BYTES];
	u8 PersString[XTRNGPSX_PERS_STRING_LEN_IN_BYTES];
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccPvtKey[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];
	int ClrStatus = XST_FAILURE;
	volatile u32 CryptoKatEn = TRUE;
	volatile u32 CryptoKatEnTmp = TRUE;

	/* Copy CDI from PMC global registers to Seed buffer */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (void *)(UINTPTR)Seed, XOCP_CDI_SIZE_IN_BYTES,
		(const void *)(UINTPTR)XOCP_PMC_GLOBAL_DICE_CDI_SEED_0, XOCP_CDI_SIZE_IN_BYTES,
		XOCP_CDI_SIZE_IN_BYTES);

	/*
	 * Copy Personalized string to buffer, here the input string is DNA
	 * which is of size 16 bytes where as TRNG requires 48 bytes of data as
	 * personalized so the remaining bytes are set to zero.
	 */
	 Status = Xil_SMemSet(PersString, XTRNGPSX_PERS_STRING_LEN_IN_BYTES,
			0U, XTRNGPSX_PERS_STRING_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy((void *)(UINTPTR)PersString,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			(const void *)(UINTPTR)XOCP_EFUSE_DEVICE_DNA_CACHE,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate the DEV IK public and private keys */
	KeyGenParams.SeedAddr = (u32)(UINTPTR)Seed;
	KeyGenParams.SeedLength = XOCP_CDI_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)(UINTPTR)PersString;
	KeyGenParams.KeyOutPutAddr = (u32)(UINTPTR)EccPvtKey;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticPrvtKeyGenerate, XSECURE_ECC_NIST_P384,
						   &KeyGenParams);

	PubKeyAddr.Qx = (u64)(UINTPTR)EccX;
	PubKeyAddr.Qy = (u64)(UINTPTR)EccY;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticGenerateKey_64Bit, XSECURE_ECC_NIST_P384,
						   (u64)(UINTPTR)EccPvtKey, &PubKeyAddr);

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = CryptoKatEn;
	if ((CryptoKatEn == (u32)TRUE) || (CryptoKatEnTmp == (u32)TRUE)) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, SStatusTmp, XSecure_EllipticPwct,
			XSECURE_ECC_NIST_P384, (u64)(UINTPTR)EccPvtKey, &PubKeyAddr);
		if ((Status != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK);
	}

	/* Copy Private key to PMC global registers */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (void *)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
						   XOCP_ECC_P384_SIZE_BYTES, (const void *)(UINTPTR)EccPvtKey,
						   XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);

	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				(u64)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0,
						(u64)(UINTPTR)EccX);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				 (u64)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0,
						(u64)(UINTPTR)EccY);

END:
	ClrStatus = Xil_SecureZeroize(EccPvtKey, XOCP_ECC_P384_SIZE_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	ClrStatus = Xil_SecureZeroize(Seed, XOCP_CDI_SIZE_IN_BYTES);
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
static int XOcp_KeyGenDevAkSeed(u32 CdiAddr, u32 CdiLen, u32 DataAddr,
	u32 DataLen, XSecure_HmacRes *Out)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
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
			Status |= StatusTmp;
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_HMAC_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_HmacKat,
				&Sha3Instance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_HMAC_KAT_MASK);
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacInit(&HmacInstance, &Sha3Instance, CdiAddr, CdiLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacUpdate(&HmacInstance, (u64)DataAddr, DataLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacFinal(&HmacInstance, Out);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common XOcp_KeyMgmt
 *		instance which has to be used across the project to store the data.
 *
 * @return
 *      -   Pointer to the XOcp_KeyMgmt instance
 *
 ******************************************************************************/
static XOcp_KeyMgmt *XOcp_GetKeyMgmtInstance(void)
{

	static XOcp_KeyMgmt KeyMgmtInstance = {0U};

	return &KeyMgmtInstance;
}

/*****************************************************************************/
/**
 * @brief       This function validates the DICE CDI stored in PMC global register.
 *
 * @return
 *      -   XST_SUCCESS - On Successful read and validation of CDI Seed
 *	-   Errorcode  - On failure
 *
 ******************************************************************************/
static int XOcp_ValidateDiceCdi(void)
{
	volatile int Status = (int)XOCP_DICE_CDI_SEED_ZERO;
	volatile u32 Index;
	volatile u32 CdiParity = 0U;
	volatile u32 CdiParityTmp = 0U;

	/** Upon DICE CDI SEED zeroize, if CDI valid bit is not cleared in Versal Net.
	 *  Check whether DICE CDI SEED is non zero or not.
	 */
	for (Index = 0U; Index < XOCP_CDI_SIZE_IN_WORDS; Index++) {
		if (XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_0 +
			(Index * XSECURE_WORD_LEN)) != 0x0U) {
			CdiParity = XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY) &
				XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK;
			CdiParityTmp = XPlmi_In32(XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY) &
				XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK;
			if ((CdiParity != 0x0U) || (CdiParityTmp != 0x0U)) {
				Status = (int)XOCP_DICE_CDI_PARITY_ERROR;
				goto END;
			}
			else {
				Status = XST_SUCCESS;
				break;
			}
		}
	}
	if (Index > XOCP_CDI_SIZE_IN_WORDS) {
		Status = (int)XOCP_ERR_GLITCH_DETECTED;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_EcdhGetSecret server API to
 *		generate the shared secret using ECDH.
 *
 * @param	SubSystemId - ID of the subsystem from where the command is
 * 		originating
 * @param	PubKeyAddrLow - 64 bit address of the Public Key buffer
 * @param	SharedSecretAddrLow - 64 bit address of the Shared Secret buffer
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - Errorcode  On failure
 *
 ******************************************************************************/
int XOcp_GenSharedSecretwithDevAk(u32 SubSystemId, u64 PubKeyAddr, u64 SharedSecretAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 DevAkIndex;
	u64 PrvtKeyAddr;
	u8 PubKeyTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];
	u8 SharedSecretTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];

	DevAkIndex = XOcp_GetSubSysDevAkIndex(SubSystemId);
	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = (int)XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}
	DevAkData = DevAkData + DevAkIndex;

	if (DevAkData->IsDevAkKeyReady == TRUE) {
		PrvtKeyAddr = (u64)(UINTPTR)DevAkData->EccPrvtKey;

		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)PubKeyTmp, PubKeyAddr);
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)(PubKeyTmp + XSECURE_ECC_P384_SIZE_IN_BYTES),
			PubKeyAddr + XSECURE_ECC_P384_SIZE_IN_BYTES);

		Status = XSecure_EcdhGetSecret(XSECURE_ECC_NIST_P384, PrvtKeyAddr,
			(u64)(UINTPTR)PubKeyTmp, (u64)(UINTPTR)SharedSecretTmp);

		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			SharedSecretAddr, (u64)(UINTPTR)SharedSecretTmp);
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			SharedSecretAddr + XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)(SharedSecretTmp + XSECURE_ECC_P384_SIZE_IN_BYTES));
	}
	else {
		Status = (int)XOCP_ERR_DEVAK_NOT_READY;
	}

END:
	ClrStatus = Xil_SecureZeroize(SharedSecretTmp, XSECURE_ECC_P384_SIZE_IN_BYTES * 2U);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the DEVAK for requested subsystem by user.
 *
 * @param	SubsystemID is the ID of image.
 * @param	InhHash 64-bit address of hash.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XOCP_GenSubSysDevAk(u32 SubsystemID, u64 InHash)
{
	int Status = XST_FAILURE;
	u32 DevAkIndex = XOcp_GetSubSysDevAkIndex(SubsystemID);
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();

	if (DevAkIndex != XOCP_INVALID_DEVAK_INDEX)  {
		DevAkData = DevAkData + DevAkIndex;
		Status = XPlmi_MemCpy64((u64)(UINTPTR)DevAkData->SubSysHash,
					InHash, XOCP_CDI_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XOcp_GenerateDevAk(SubsystemID);
		XOcp_Printf(DEBUG_DETAILED, "DEV AK of subsystem is generated %x\n\r",
					SubsystemID);
	}
	else {
		XOcp_Printf(DEBUG_DETAILED, "DEV AK of subsystem is not generated \n\r");
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
#endif /* PLM_OCP_KEY_MNGMT */
