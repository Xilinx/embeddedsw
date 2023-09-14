/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xocp_keymgmt.h"
#include "xocp_hw.h"
#include "xocp_def.h"
#include "xocp_common.h"
#include "xplmi_dma.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_hmac.h"
#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_ellipticplat.h"
#endif
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

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XOcp_KeyGenDevAkSeed(u32 KeyAddr, u32 KeyLen, u32 DataAddr,
			 u32 DataLen, XSecure_HmacRes *Out);
static int XOcp_KeyZeroize(u32 CtrlReg, UINTPTR StatusReg);
static int XOcp_KeyGenerateDevIk(void);
static XOcp_KeyMgmt *XOcp_GetKeyMgmtInstance(void);
static XOcp_SubSysHash *XOcp_GetSubSysHash(void);
static int XOcp_ValidateDiceCdi(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function provides the pointer to the common XOcp_DevAkData
 * instance which has to be used across the project to store the data.
 *
 * @return
 *      -   Pointer to the XOcp_DevAkData instance
 *
 ******************************************************************************/
XOcp_DevAkData *XOcp_GetDevAkData(void)
{
	static XOcp_DevAkData DevAkData[XOCP_MAX_DEVAK_SUPPORT] = {0U};

	return &DevAkData[0];
}

/*****************************************************************************/
/**
 * @brief       This function gets the hash(s) of the corresponding subsystem(s)
 * which will be updated during DEVAK generation and stored during an
 * in place PLM update to regenerate the DEVAK.
 *
 * @return
 *      -   Pointer to the XOcp_ instance
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
 * @brief       This function returns whether Device identity key is ready or
 * not.
 *
 * @return
 *		- FALSE if DEV IK is not ready
 *		- TRUE if DEV IK is ready
 *
 ******************************************************************************/
int XOcp_IsDevIkReady(void)
{
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	return KeyInstPtr->IsDevKeyReady;
}

/*****************************************************************************/
/**
 * @brief	This function generates the DEVIK ECC private and public
 *		key pair and generates the self-signed X.509 certificate to share
 *		DEVIK public key.
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
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();
	XOcp_KeyMgmt *KeyInstPtr = XOcp_GetKeyMgmtInstance();

	KeyInstPtr->IsDevKeyReady = FALSE;

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
			StatusTmp, XTrngpsx_PreOperationalSelfTests, TrngInstance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
	}

	/* Generate private and public key pair for ECC */
	XSECURE_TEMPORAL_CHECK(END, Status, XOcp_KeyGenerateDevIk);

	XOcp_Printf(DEBUG_INFO, "Generated DEV IK\n\r");

	KeyInstPtr->IsDevKeyReady = TRUE;

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

	if (KeyMgmtInstance->IsDevKeyReady != TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}
	do {
		for (Index = 0U; Index < XOCP_MAX_DEVAK_SUPPORT; Index++, SubSysHashDs++) {
			if ((DevAkData->SubSystemId == SubSysHashDs->SubSystemId) &&
				(SubSysHashDs->ValidData == (u32)TRUE)) {
				Status = Xil_SMemCpy((void *)DevAkData->SubSysHash,
						XSECURE_HASH_SIZE_IN_BYTES,
						(const void *)SubSysHashDs->SubSysHash,
						 XSECURE_HASH_SIZE_IN_BYTES,
						XSECURE_HASH_SIZE_IN_BYTES);
				if (Status != XST_SUCCESS) {
					goto END;
				}

				XSECURE_TEMPORAL_CHECK(END, Status, XOcp_GenerateDevAk, DevAkData->SubSystemId);
				break;
			}
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

	if (KeyMgmtInstance->IsDevKeyReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "No Device keys are supported\n\r");
		Status = XST_SUCCESS;
		goto END;
	}
	if (KeyMgmtInstance->DevAkInputIndex >= XOCP_MAX_DEVAK_SUPPORT) {
		XOcp_Printf(DEBUG_GENERAL,
			"Maximum count of DEVAK supported is %d\r\n",
			XOCP_MAX_DEVAK_SUPPORT);
		Status = XOCP_DEVAK_MAX_COUNT_EXCEED;
		goto END;
	}

	DevAkData[KeyMgmtInstance->DevAkInputIndex].SubSystemId = SubSystemId;

	Status = XPlmi_MemCpy64(
			(UINTPTR)DevAkData[KeyMgmtInstance->DevAkInputIndex].PerString,
			(UINTPTR)PerString, XTRNGPSX_PERS_STRING_LEN_IN_BYTES);
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
 * @param	SubSystemId holds the sub system ID of PDI of whose DEVAK
 *		generation is requested.
 * @return
 *		- XST_SUCCESS - If key generation is success
 *		- Error code - Upon any failure
 *
 ******************************************************************************/
int XOcp_GenerateDevAk(u32 SubSystemId)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 DevAkIndex = XOcp_GetSubSysReqDevAkIndex(SubSystemId);
	u8 Seed[XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES];
#ifndef PLM_ECDSA_EXCLUDE
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];
#endif
	int ClrStatus = XST_FAILURE;
	volatile u8 CryptoKatEn = (u8)TRUE;
	volatile u8 CryptoKatEnTmp = (u8)TRUE;
	XOcp_SubSysHash *SubSysHashDs = XOcp_GetSubSysHash();
	SubSysHashDs = SubSysHashDs + DevAkIndex;

	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = (int)XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}
	XOcp_Printf(DEBUG_INFO, "Generating DEV AK of subsystem ID %x\n\r", SubSystemId);

	DevAkData = DevAkData + DevAkIndex;
	DevAkData->IsDevAkKeyReady = (u32)FALSE;

	XSECURE_TEMPORAL_CHECK(END, Status, XOcp_KeyGenDevAkSeed, XOCP_PMC_GLOBAL_DICE_CDI_SEED_0,
						   XOCP_CDI_SIZE_IN_BYTES, (u32)DevAkData->SubSysHash, XSECURE_HASH_SIZE_IN_BYTES,
						   (XSecure_HmacRes *)Seed);

#ifndef PLM_ECDSA_EXCLUDE
	/* Generate the DEV AK public and private keys */
	KeyGenParams.SeedAddr = (u32)Seed;
	KeyGenParams.SeedLength = XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)DevAkData->PerString;
	KeyGenParams.KeyOutPutAddr = (u32)DevAkData->EccPrvtKey;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticPrvtKeyGenerate, XSECURE_ECC_NIST_P384,
						   &KeyGenParams);

	PubKeyAddr.Qx = (UINTPTR)(u8*)EccX;
	PubKeyAddr.Qy = (UINTPTR)(u8*)EccY;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticGenerateKey_64Bit, XSECURE_ECC_NIST_P384,
						   (u64)(UINTPTR)DevAkData->EccPrvtKey, &PubKeyAddr);

	CryptoKatEn = XPlmi_IsCryptoKatEn();
	CryptoKatEnTmp = CryptoKatEn;
	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_EllipticPwct,
			XSECURE_ECC_NIST_P384, (u64)(UINTPTR)DevAkData->EccPrvtKey, &PubKeyAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK);
	}
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)DevAkData->EccX,
						(u64)(UINTPTR)EccX);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)DevAkData->EccY,
						(u64)(UINTPTR)EccY);

	DevAkData->IsDevAkKeyReady = (u32)TRUE;
	/* Store hash of the sub-system */
	SubSysHashDs->ValidData = (u32)FALSE;
	SubSysHashDs->SubSystemId = DevAkData->SubSystemId;
	Status = Xil_SMemCpy((void *)SubSysHashDs->SubSysHash, XSECURE_HASH_SIZE_IN_BYTES,
				(const void *)DevAkData->SubSysHash,
				XSECURE_HASH_SIZE_IN_BYTES,
				XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		SubSysHashDs->ValidData = (u32)TRUE;
	}

	XPlmi_PrintArray(DEBUG_DETAILED, (u64)(UINTPTR)DevAkData->EccPrvtKey,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PRVT KEY");
	XPlmi_PrintArray(DEBUG_INFO, (u64)(UINTPTR)DevAkData->EccX,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PUB KEY X");
	XPlmi_PrintArray(DEBUG_INFO, (u64)(UINTPTR)DevAkData->EccY,
		XSECURE_HASH_SIZE_IN_BYTES/XPLMI_WORD_LEN, "ECC PUB KEY Y");

#else
	(void)CryptoKatEn;
    (void)CryptoKatEnTmp;
	(void)StatusTmp;
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
#endif

END:
	ClrStatus = Xil_SMemSet(Seed, XOCP_CDI_SIZE_IN_BYTES,
				0U, XOCP_CDI_SIZE_IN_BYTES);
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
u32 XOcp_GetSubSysReqDevAkIndex(u32 SubSystemId)
{
	u32 DevAkIndex = XOCP_INVALID_DEVAK_INDEX;
	XOcp_KeyMgmt *KeyMgmtInstance = XOcp_GetKeyMgmtInstance();
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 Index = 0U;

	/* If no device key is supported returns invalid */
	if (KeyMgmtInstance->IsDevKeyReady != TRUE) {
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
 * @param   XOcp_GetX509CertPtr points to the address of XOcp_X509Cert structure.
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
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (KeyInstPtr->IsDevKeyReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "No Device keys are supported\n\r");
		Status = XOCP_ERR_DEVIK_NOT_READY;
		goto END;
	}

	if (XOcp_GetX509CertPtr->DevKeySel == XOCP_DEVIK) {
		CertConfig.SubSystemId = XOCP_PMC_SUBSYSTEM_ID;
		CertConfig.AppCfg.IsSelfSigned = TRUE;
		CertConfig.AppCfg.SubjectPublicKey =
				(u8 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
		CertConfig.AppCfg.IssuerPrvtKey = (u8 *)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0;
		CertConfig.AppCfg.IssuerPublicKey = (u8 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
		if (XOcp_GetX509CertPtr->IsCsr != (u8)TRUE) {
			CertConfig.AppCfg.IsCsr = (u8)FALSE;
			CertConfig.AppCfg.FwHash = (u8 *)XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0;
		}
		else {
			CertConfig.AppCfg.IsCsr = (u8)TRUE;
		}
	}
	else {
		DevAkIndex = XOcp_GetSubSysReqDevAkIndex(SubSystemId);
		if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
			Status = XOCP_ERR_INVALID_DEVAK_REQ;
			goto END;
		}

		DevAkData = XOcp_GetDevAkData();
		DevAkData = DevAkData + DevAkIndex;
		if (DevAkData->IsDevAkKeyReady != TRUE) {
			XOcp_Printf(DEBUG_DETAILED, "Device Attestation key is not generated\n\r");
			Status = XOCP_ERR_DEVAK_NOT_READY;
			goto END;
		}
		CertConfig.SubSystemId = SubSystemId;
		CertConfig.AppCfg.IsSelfSigned = FALSE;
		CertConfig.AppCfg.SubjectPublicKey = (u8 *)DevAkData->EccX;
		CertConfig.AppCfg.IssuerPrvtKey = (u8 *)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0;
		CertConfig.AppCfg.IssuerPublicKey = (u8 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
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
 *	attestation key. The PLM signs the hash of the data (SHA384/SHA3-384)
 *	supplied by the user as input. The PLM uses ECC P-384 DevAK private key to
 * sign the input hash
 *
 * @param   AttestWithDevAkPtr - Address of XOcp_AttestWithDevAk structure.
 * @param	SubSystemId holds the image ID.
 *
 * @return
 *		- XST_SUCCESS - Upon successful attestation
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_AttestWithDevAk(XOcp_Attest *AttestWithDevAkPtr, u32 SubSystemId)
{
	int Status = XST_FAILURE;
	u32 DevAkIndex;
	XOcp_DevAkData *DevAkData = NULL;
#ifndef PLM_ECDSA_EXCLUDE
	u8 Hash[XSECURE_ECC_P384_SIZE_IN_BYTES];
	u8 Signature[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];
#endif

	if ((SubSystemId == 0x0U) || (AttestWithDevAkPtr->HashAddr == 0x0U) ||
				(AttestWithDevAkPtr->SignatureAddr == 0x0U)) {
		goto END;
	}

	DevAkIndex = XOcp_GetSubSysReqDevAkIndex(SubSystemId);
	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}

	DevAkData = XOcp_GetDevAkData();
	DevAkData = DevAkData + DevAkIndex;
	if (DevAkData->IsDevAkKeyReady != TRUE) {
		XOcp_Printf(DEBUG_DETAILED, "Device Attestation key is not generated\n\r");
		Status = XOCP_ERR_DEVAK_NOT_READY;
		goto END;
	}

#ifndef PLM_ECDSA_EXCLUDE
	/* Covert hash to little endian */
	XSecure_FixEndiannessNCopy(AttestWithDevAkPtr->HashLen,
		(u64)(UINTPTR)Hash, AttestWithDevAkPtr->HashAddr);
	/* Generate the signature using DEVAK */
	Status = XSecure_EllipticGenEphemeralNSign(XSECURE_ECC_NIST_P384, Hash,
			AttestWithDevAkPtr->HashLen,
			(u8 *)(UINTPTR)DevAkData->EccPrvtKey,
			(u8 *)Signature);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			AttestWithDevAkPtr->SignatureAddr,
				(u64)(UINTPTR)Signature);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
		AttestWithDevAkPtr->SignatureAddr + XSECURE_ECC_P384_SIZE_IN_BYTES,
		(u64)(UINTPTR)(Signature + XSECURE_ECC_P384_SIZE_IN_BYTES));
#endif
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes DEVIK and CDI
 *
 * @return
 *		- XST_SUCCESS - If zeroization status is pass
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_DataZeroize(XPlmi_ModuleOp Op)
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
		if (KeyInstPtr->IsDevKeyReady == TRUE) {
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
		Status = XST_INVALID_PARAM;
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

	XPlmi_Out32(CtrlReg, XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK);

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
	volatile int StatusTmp = XST_FAILURE;
	u8 Seed[XOCP_CDI_SIZE_IN_BYTES];
	u8 PersString[XTRNGPSX_PERS_STRING_LEN_IN_BYTES];
#ifndef PLM_ECDSA_EXCLUDE
	XSecure_ElliptcPrivateKeyGen KeyGenParams;
	XSecure_EllipticKeyAddr PubKeyAddr;
	u8 EccPrvtKey[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];
#endif
	u32 ClrStatus = XST_FAILURE;
	volatile u8 CryptoKatEn = TRUE;
	volatile u8 CryptoKatEnTmp = TRUE;

	/* Copy CDI from PMC global registers to Seed buffer */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (void *)Seed, XOCP_CDI_SIZE_IN_BYTES,
						   (const void *)XOCP_PMC_GLOBAL_DICE_CDI_SEED_0, XOCP_CDI_SIZE_IN_BYTES,
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

	Status = Xil_SMemCpy((void *)PersString,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			(const void *)XOCP_EFUSE_DEVICE_DNA_CACHE,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES,
			XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef PLM_ECDSA_EXCLUDE
	/* Generate the DEV IK public and private keys */
	KeyGenParams.SeedAddr = (u32)Seed;
	KeyGenParams.SeedLength = XOCP_CDI_SIZE_IN_BYTES;
	KeyGenParams.PerStringAddr = (u32)PersString;
	KeyGenParams.KeyOutPutAddr = (u32)EccPrvtKey;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticPrvtKeyGenerate, XSECURE_ECC_NIST_P384,
						   &KeyGenParams);

	PubKeyAddr.Qx = (UINTPTR)(u8*)EccX;
	PubKeyAddr.Qy = (UINTPTR)(u8*)EccY;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_EllipticGenerateKey_64Bit, XSECURE_ECC_NIST_P384,
						   (u64)(UINTPTR)EccPrvtKey, &PubKeyAddr);

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
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (void *)XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0,
						   XOCP_ECC_P384_SIZE_BYTES, (const void *)EccPrvtKey,
						   XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);

	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				(u64)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0,
						(u64)(UINTPTR)EccX);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				 (u64)(UINTPTR)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0,
						(u64)(UINTPTR)EccY);

#else
	(void)CryptoKatEn;
	(void)CryptoKatEnTmp;
	(void)StatusTmp;
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
#endif

END:
#ifndef PLM_ECDSA_EXCLUDE
	ClrStatus = Xil_SMemSet(EccPrvtKey, XOCP_ECC_P384_SIZE_BYTES,
				0U, XOCP_ECC_P384_SIZE_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
#endif
	ClrStatus = Xil_SMemSet(Seed, XOCP_CDI_SIZE_IN_BYTES,
				0U, XOCP_CDI_SIZE_IN_BYTES);
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
	volatile int RetStatus = XST_GLITCH_ERROR;
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

	Status = XST_FAILURE;
	Status = XSecure_HmacInit(&HmacInstance, &Sha3Instance, KeyAddr, KeyLen);
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
 * @brief       This function provides the pointer to the common XOcp_KeyMgmt
*	instance which has to be used across the project to store the data.
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
	volatile int Status = XOCP_DICE_CDI_SEED_ZERO;
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
				Status = XOCP_DICE_CDI_PARITY_ERROR;
				goto END;
			}
			else {
				Status = XST_SUCCESS;
				break;
			}
		}
	}
	if (Index > XOCP_CDI_SIZE_IN_WORDS) {
		Status = XOCP_ERR_GLITCH_DETECTED;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
