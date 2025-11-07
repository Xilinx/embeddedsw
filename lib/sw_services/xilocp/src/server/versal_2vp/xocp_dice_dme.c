/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dice_dme.c
*
* This file contains the implementation of DME challenge signature and DICE CDI generation for
* versal_2vp.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.5   tvp  06/05/25 Initial release
*       sd   11/07/25 Update condition to reflect the revised function return value
*
* </pre>
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/

#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_hw.h"
#include "xocp_sha.h"
#include "xocp_keymgmt.h"
#include "xocp_dice_dme.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_trng.h"
#include "xsecure_elliptic.h"
#include "xsecure_ellipticplat.h"
#include "xsecure_kat.h"
#include "xsecure_aes.h"
#include "xsecure_core.h"
#include "xsecure_kdf.h"
#include "xsecure_plat_kat.h"
#include "xilpdi_plat.h"
#include "xpuf.h"
#include "xpuf_hw.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
static int XOcp_DigestDiceSecCfgDigest(void);
static int XOcp_DigestTapConf(void);
static int XOcp_GetRomPlmHash(void);
static int XOcp_DigestHashCom(XSecure_Sha3Hash *HashAllPtr);
static int XOcp_GenerateCdi(void);
static int XOcp_DmeKeySel(u32 *KeySel);
static int XOcp_PufRegeneration(void);
static u32 XOcp_DmeHashGen(XOcp_Dme *DmePtr, u8 *Hash);
static void XOcp_GetIncIv(u8 *Iv, u8 IncVal, u8 *UpdatedIv);
static void XOcp_GetIvData(u8 *IvAddr);
static int XOcp_DmeChallengeSignature(XOcp_Dme *DmePtr);

/************************************ Variable Definitions ****************************************/
/**< Secure state hash */
static XOcp_SecureStateHash SecureStateHash;
/**< Secure efuse configuration */
static XOcp_SecureConfig SecureConfig;
/**< Secure tap configuration */
static XOcp_SecureTapConfig SecureTapConfig;

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function provides Kek IV.
 *
 * @param	IvAddr is pointer to store Kek IV.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static void XOcp_GetIvData(u8 *IvAddr)
{
	const XilPdi_BootHdr *BootHdr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;
	volatile u32 eFUSEEncStatus = Xil_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_0) &
					       XOCP_DEC_ONLY_MEASURED_MASK;
	volatile u32 eFUSEEncStatusTmp = Xil_In32(XOCP_EFUSE_CACHE_SECURITY_MISC_0) &
						  XOCP_DEC_ONLY_MEASURED_MASK;
	if ((eFUSEEncStatus != 0U) || (eFUSEEncStatusTmp != 0U)) {
		/* Copy the Black Key IV from eFUSE */
		Xil_SMemCpy((void *)(UINTPTR)IvAddr, XOCP_SECURE_IV_LEN_IN_BYTES,
			    (void *)(UINTPTR)XOCP_EFUSE_CACHE_BLACK_IV_0,
			    XOCP_SECURE_IV_LEN_IN_BYTES, XOCP_SECURE_IV_LEN_IN_BYTES);
	} else {
		/* Copy the Black Key IV from BH */
		Xil_SMemCpy((void *)(UINTPTR)IvAddr, XOCP_SECURE_IV_LEN_IN_BYTES, BootHdr->KekIv,
			    XOCP_SECURE_IV_LEN_IN_BYTES, XOCP_SECURE_IV_LEN_IN_BYTES);
	}
}

/**************************************************************************************************/
/**
 * @brief	This function performs PUF re-generation on demand.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_PufRegeneration(void)
{
	XPuf_Data PufData __attribute__((aligned(32U))) = {0U};
	int Status = XST_FAILURE;
	u32 PufIpStatus;
	const XilPdi_BootHdr *BootHdr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;
	volatile u32 PufHdLocation;

	PufHdLocation = (BootHdr->ImgAttrb & XOCP_BH_IMG_ATTRB_PUF_HD_MASK) >>
			XOCP_BH_IMG_ATTRB_PUF_HD_SHIFT;
	PufData.ShutterValue = BootHdr->PufShutterVal;
	PufData.GlobalVarFilter = (u8)(PufData.ShutterValue >> XOCP_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT);

	if (PufHdLocation == XOCP_PUF_HD_BHDR) {
		PufData.ReadOption = XPUF_READ_FROM_RAM;
		PufData.SyndromeAddr = XIH_BH_PRAM_ADDR + XIH_BH_PUF_HD_OFFSET;
		PufData.Chash = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_CHASH_OFFSET);
		PufData.Aux = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_AUX_OFFSET);
		XPlmi_Printf(DEBUG_INFO, "BHDR PUF HELPER DATA with CHASH: %0x and AUX:%0x\n\r",
			     PufData.Chash, PufData.Aux);
	} else {
		/**
		 * Skip PUF regeneration, if CHASH in eFuse cache is zero.
		 */
		if (Xil_In32(XOCP_PUF_CHASH_EFUSE_CACHE_ADDR) == 0U) {
			Status = XST_SUCCESS;
			goto END;
		}
		XPlmi_Printf(DEBUG_INFO, "PUF helper data is in efuse cache\n\r");
		PufData.ReadOption = XPUF_READ_FROM_EFUSE_CACHE;
	}

	/* If PUF is disabled then raise an error */
	PufIpStatus = Xil_In32(XOCP_EFUSE_CACHE_SECURITY_CONTROL) & XPUF_PUF_DIS;
	if (PufIpStatus == XPUF_PUF_DIS) {
		Status = XPLM_ERR_PUF_DISABLED;
		goto END;
	}

	PufData.PufOperation = XPUF_REGEN_ON_DEMAND;

	Status = XPuf_Regeneration(&PufData);
END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function calculates the hash of secure efuse configuration.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_DigestDiceSecCfgDigest(void)
{
	int Status = XST_FAILURE;

	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/* Read secure efuse configuration */
	XOcp_ReadSecureConfig(&SecureConfig);

	/* Calculate secure efuse configuration hash */
	Status = XSecure_ShaDigest(Sha3InstPtr, XSECURE_SHA3_384, (UINTPTR)&SecureConfig,
				sizeof(XOcp_SecureConfig),
				(u64)(UINTPTR)(XSecure_Sha3Hash*)&SecureStateHash.SecureConfigHash,
				sizeof(SecureStateHash.SecureConfigHash));

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function calculates the hash of secure tap configuration.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_DigestTapConf(void)
{
	int Status = XST_FAILURE;

	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/* Read secure tap configuration */
	XOcp_ReadTapConfig(&SecureTapConfig);

	/* Calculate secure tap configuration hash */
	Status = XSecure_ShaDigest(Sha3InstPtr, XSECURE_SHA3_384, (UINTPTR)&SecureTapConfig,
				   sizeof(XOcp_SecureTapConfig),
				   (u64)(UINTPTR)(XSecure_Sha3Hash*)&SecureStateHash.TapConfigHash,
				   sizeof(SecureStateHash.TapConfigHash));

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function copy the ROM and PLM hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_GetRomPlmHash(void)
{
	int Status = XST_FAILURE;

	/* Copy ROM hash */
	Status = Xil_SMemCpy(&SecureStateHash.RomHash, XOCP_PCR_HASH_SIZE_IN_BYTES,
			     (u8*)(UINTPTR)XOCP_PMC_ROM_HASH_ADDR, XOCP_PCR_HASH_SIZE_IN_BYTES,
			     XOCP_PCR_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy PLM hash */
	Status = Xil_SMemCpy(&SecureStateHash.PlmHash, XOCP_PCR_HASH_SIZE_IN_BYTES,
			     (u8*)(UINTPTR)XOCP_PMC_PLM_HASH_ADDR, XOCP_PCR_HASH_SIZE_IN_BYTES,
			     XOCP_PCR_HASH_SIZE_IN_BYTES);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function calculates common hash of ROM, PLM and secure configurations hash.
 *
 * @param	HashAllPtr pointer to common calculated hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_DigestHashCom(XSecure_Sha3Hash *HashAllPtr)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XOcp_SecureStateHash *SecMeasureHashPtr = &SecureStateHash;

	XSecure_Sha3Start(Sha3InstPtr);
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecMeasureHashPtr->RomHash,
				    XSECURE_SHA3_HASH_LENGTH_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecMeasureHashPtr->PlmHash,
				    XSECURE_SHA3_HASH_LENGTH_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecMeasureHashPtr->SecureConfigHash,
				    XSECURE_SHA3_HASH_LENGTH_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecMeasureHashPtr->TapConfigHash,
				    XSECURE_SHA3_HASH_LENGTH_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Finish(Sha3InstPtr, HashAllPtr);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function increments the IV with the value passed.
 *
 * @param	Iv is the pointer to the init vector to be incremented.
 * @param	IncVal is the value with which IV needs to be incremented.
 * @param	UpdatedIv is the pointer to store the updated IV.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static void XOcp_GetIncIv(u8 *Iv, u8 IncVal, u8 *UpdatedIv)
{
	u8 Carry = 0;
	u8 Byte;
	u16 ByteSum = 0;
	int Idx;

	for (Idx = XOCP_SECURE_IV_LEN_IN_BYTES - 1; Idx >= 0; Idx--) {
		Byte = Iv[Idx];
		ByteSum = Byte + IncVal + Carry;
		Carry = ByteSum >> 8U;
		UpdatedIv[Idx] = ByteSum & 0xFF;
		if (Carry == 0) {
			break;
		}
	}
}

/**************************************************************************************************/
/**
 * @brief	This function calculate CDI and store in XOcp_RegSpace.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XOcp_GenerateCdi(void)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	volatile u32 CryptoKAT;
	volatile u32 CryptoKATTmp;
	XSecure_KdfParams KdfInput;
	u8 KdfOut[XOCP_CDI_SIZE_IN_BYTES] = {0U};
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XSecure_Sha3Hash HashAll;
	XSecure_Aes *AesInstance = XSecure_GetAesInstance();
	u8 Iv[XOCP_SECURE_IV_LEN_IN_BYTES] = {0U};
	u8 UpdatedIv[XOCP_SECURE_IV_LEN_IN_BYTES] = {0U};
	u8 DecUds[XOCP_CDI_SIZE_IN_BYTES] = {0U};
	XOcp_RegSpace* XOcp_Reg = XOcp_GetRegSpace();
	u8 AesBuffer[XOCP_AES_OCP_DATA_LEN_IN_BYTES] = {0};
	u32 EncryptedZeros[XOCP_AES_OCP_DATA_LEN_IN_WORDS] = {0};
	u8 GcmTag[XOCP_AES_GCM_TAG_SIZE] = {0};
	u32 Index;

	CryptoKAT = Xil_In32(EFUSE_CACHE_MISC_CTRL) & XOCP_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK;
	CryptoKATTmp = Xil_In32(EFUSE_CACHE_MISC_CTRL) &
				XOCP_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK;

	/* KAT for HMAC */
	if ((CryptoKAT != 0U) || (CryptoKATTmp != 0U)) {
		Status = XSecure_HmacKat(Sha3InstPtr);
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_ERR_KAT_FAILED;
			goto RET;
		}
	}

	/*
	 * Calculate hash of (ROM hash || PLM CDO hash || Efuse secure configurations hash ||
	 * tapConfig hash)
	 */
	Status = XOcp_DigestHashCom(&HashAll);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_DIGEST_CALC;
		goto RET;
	}

	/* Get KekIv to decrypt UDS */
	XOcp_GetIvData((u8 *)&Iv[0]);

	/* Increment Iv */
	XOcp_GetIncIv((u8 *)Iv, XOCP_UDS_IV_INC, UpdatedIv);

	Status = XSecure_AesKeyZero(AesInstance, XSECURE_AES_PUF_KEY);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_AES_KEY_ZEROIZE;
		goto RET;
	}

	Status = XOcp_PufRegeneration();
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_PUF_REGENERATION;
		goto RET;
	}

	/* Encrypt all zero data with the key */
	Status = XSecure_AesEncryptInit(AesInstance, XSECURE_AES_PUF_KEY, XSECURE_AES_KEY_SIZE_256,
					(u64)(UINTPTR)UpdatedIv);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_AES_ENC_INIT;
		goto RET;
	}
	Status = XSecure_AesEncryptData(AesInstance, (u64)(UINTPTR)AesBuffer,
					(u64)(UINTPTR)EncryptedZeros, XOCP_AES_OCP_DATA_LEN_IN_BYTES,
					(u64)(UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_AES_ENC_DATA;
		goto RET;
	}

	/* Decrypt the encrypted UDS */
	for (Index = 0; Index < XOCP_AES_OCP_DATA_LEN_IN_WORDS; Index++) {
		Xil_Out32(((u32)(UINTPTR)DecUds + (u32)(Index * XOCP_WORD_SIZE)),
				(EncryptedZeros[Index] ^ Xil_In32((u32)XOCP_UDS_EFUSE_CACHE_ADDR +
								  (u32)(Index * XOCP_WORD_SIZE))));
	}

	/* Generate KDF with UDS as key and Hash as context */
	KdfInput.Context = (u8 *)&HashAll;
	KdfInput.ContextLen = XSECURE_SHA3_HASH_LENGTH_IN_WORDS;
	KdfInput.Key = (u8 *)&DecUds;
	KdfInput.KeyLen = XOCP_CDI_SIZE_IN_BYTES;
	Status = XSecure_Hkdf(&KdfInput, (u8 *)&KdfOut, XOCP_CDI_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_KDF_FAILED;
		goto RET;
	}

	/* Copy CDI to XOcp_RegSpace */
	Status = Xil_SMemCpy((void *)XOcp_Reg->DiceCdiSeedAddr, XOCP_ECC_P384_SIZE_BYTES, KdfOut,
			     XOCP_CDI_SIZE_IN_BYTES, XOCP_CDI_SIZE_IN_BYTES);
	XPlmi_Out32(XOcp_Reg->DiceCdiSeedValidAddr, XOCP_CDI_SEED_VALID);

RET:
	/** Zeroize all local buffers */
	ClrStatus = Xil_SecureZeroize((u8 *)KdfOut, XOCP_CDI_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)&HashAll, sizeof(XSecure_Sha3Hash));
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)Iv, XOCP_SECURE_IV_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)UpdatedIv, XOCP_SECURE_IV_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)DecUds, XOCP_CDI_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)AesBuffer, XOCP_AES_OCP_DATA_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)EncryptedZeros, XOCP_AES_OCP_DATA_LEN_IN_WORDS);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)GcmTag, XOCP_AES_GCM_TAG_SIZE);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generate the CDI for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
int XOcp_GenerateDiceCdi(void)
{
	int Status = XST_FAILURE;

	/* Get plm and rom hash */
	Status = XOcp_GetRomPlmHash();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Secure efuse measurement */
	Status = XOcp_DigestDiceSecCfgDigest();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Secure tap measurement */
	Status = XOcp_DigestTapConf();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate CDI and store in XOcp_RegSpace */
	Status = XOcp_GenerateCdi();

END:
	/** Even if DICE CDI generation gets failed, PLM should boot successfully */
	Status = XST_SUCCESS;

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function calculates the SHA3-384 hash on DME structure.
 *
 * @param	DmePtr is the pointer to the DME challenge structure.
 * @param	Hash is the pointer of 48 bytes array in which calculated hash will be updated.
 *
 * @return
 *		- XST_SUCCESS on success.
 * 		- ErrorCode on failure.
 *
 **************************************************************************************************/
static u32 XOcp_DmeHashGen(XOcp_Dme *DmePtr, u8 *Hash)
{
	volatile u32 Status = XST_FAILURE;
	u32 DmeStructSize = sizeof(XOcp_Dme);
	XSecure_Sha3Hash DmeMeasurementHash, DmeHash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	Status = Xil_SMemSet((void *)(UINTPTR)DmePtr->Measurement, XSECURE_HASH_SIZE_IN_BYTES, 0U,
			     XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		goto RET;
	}

	Status = XOcp_DigestHashCom(&DmeMeasurementHash);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DIGEST_CALC;
		goto RET;
	}

	Status = Xil_SMemCpy((void *)(UINTPTR)DmePtr->Measurement, XSECURE_HASH_SIZE_IN_BYTES,
			     (void *)(UINTPTR)&DmeMeasurementHash, XSECURE_HASH_SIZE_IN_BYTES,
			     XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_MEAS_COPY_FAILED;
		goto RET;
	}

	Status = XSecure_Sha3Digest(ShaInstPtr, (UINTPTR)DmePtr, DmeStructSize, &DmeHash);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DIGEST_CALC;
		goto RET;
	}

	Status = Xil_SChangeEndiannessAndCpy((void *)(UINTPTR)Hash, XSECURE_HASH_SIZE_IN_BYTES,
					     (void *)(UINTPTR)DmeHash.Hash,
					     XSECURE_HASH_SIZE_IN_BYTES,
					     XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		goto RET;
	}

	Status = Xil_SMemSet((void *)(UINTPTR)DmePtr->Measurement, XSECURE_HASH_SIZE_IN_BYTES, 0U,
			     XSECURE_HASH_SIZE_IN_BYTES);

RET:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function select the valid DME private key.
 *
 * @param	DmeKeyAddr is a pointer to DME private key.
 *
 * @return
 *		- XST_SUCCESS if DME private key is available to use.
 * 		- ErrorCode if valid DME private key not present.
 *
 **************************************************************************************************/
static int XOcp_DmeKeySel(u32 *DmeKeyAddr)
{
	int Status = XST_FAILURE;
	volatile u32 eFuseMisCCtrl = Xil_In32(XOCP_EFUSE_CACHE_MISC_CTRL);

	if ((eFuseMisCCtrl & XOCP_DME_KEY_0_REVOKE_MASK) == 0U) {
		*DmeKeyAddr = XOCP_DME_USER0_EFUSE_CACHE_ADDR;
		Status = XST_SUCCESS;
	} else if ((eFuseMisCCtrl & XOCP_DME_KEY_1_REVOKE_MASK) == 0U) {
		*DmeKeyAddr = XOCP_DME_USER1_EFUSE_CACHE_ADDR;
		Status = XST_SUCCESS;
	} else {
		Status = XOCP_DME_INVALID_PRIVATE_KEY;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function decrypts the DME private key.
 *
 * @param	DecDmeKeyAddr is the address of where decrypted DME private key will be stored.
 *
 * @return
 *		- XST_SUCCESS on success.
 * 		- ErrorCode on failure.
 *
 **************************************************************************************************/
static int XOcp_DmeDecryptPrivKey(u32 DecDmeKeyAddr)
{
	XSecure_Aes *AesInstance = XSecure_GetAesInstance();
	int Status = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	u32 InitVector[XOCP_SECURE_IV_LEN_IN_WORDS] = {0};
	u32 IncInitVector[XOCP_SECURE_IV_LEN_IN_WORDS] = {0};
	u32 EncDmePrivKey = 0U;
	u8 AesBuffer[XOCP_AES_OCP_DATA_LEN_IN_BYTES] = {0};
	u32 EncryptedZeros[XOCP_AES_OCP_DATA_LEN_IN_WORDS] = {0};
	u8 GcmTag[XOCP_AES_GCM_TAG_SIZE] = {0};
	u32 Index;

	Status = XOcp_DmeKeySel(&EncDmePrivKey);
	if ((Status != XST_SUCCESS) || (!EncDmePrivKey)) {
		goto END;
	}

	/* Get KekIv to decrypt DME private key */
	XOcp_GetIvData((u8 *)&InitVector[0]);

	/* Increment Iv */
	XOcp_GetIncIv((u8 *)(UINTPTR)InitVector, (u8)XOCP_DME_IV_INC, (u8 *)(UINTPTR)IncInitVector);

	Status = XSecure_AesKeyZero(AesInstance, XSECURE_AES_PUF_KEY);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_AES_KEY_ZEROIZE;
		goto END;
	}

	/* Regenerating the PUF */
	Status = XOcp_PufRegeneration();
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_PUF_REGENERATION;
		goto END;
	}

	/* Encrypt all zero data with the key */
	Status = XSecure_AesEncryptInit(AesInstance, XSECURE_AES_PUF_KEY, XSECURE_AES_KEY_SIZE_256,
					(u64)(UINTPTR)IncInitVector);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_AES_ENC_INIT;
		goto END;
	}
	Status = XSecure_AesEncryptData(AesInstance, (u64)(UINTPTR)AesBuffer,
					(u64)(UINTPTR)EncryptedZeros, XOCP_AES_OCP_DATA_LEN_IN_BYTES,
					(u64)(UINTPTR)GcmTag);

	/* Decrypt DME private key */
	for (Index = 0; Index < XOCP_AES_OCP_DATA_LEN_IN_WORDS; Index++) {
		Xil_Out32((DecDmeKeyAddr + (u32)(Index * XOCP_WORD_SIZE)),
				(EncryptedZeros[Index] ^ Xil_In32(EncDmePrivKey +
								  (u32)(Index * XOCP_WORD_SIZE))));
	}

END:
	/** Zeroize all local buffers */
	ClrStatus = Xil_SecureZeroize((u8 *)InitVector, XOCP_SECURE_IV_LEN_IN_WORDS);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)IncInitVector, XOCP_SECURE_IV_LEN_IN_WORDS);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)AesBuffer, XOCP_AES_OCP_DATA_LEN_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)EncryptedZeros, XOCP_AES_OCP_DATA_LEN_IN_WORDS);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)GcmTag, XOCP_AES_GCM_TAG_SIZE);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}


	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generates the signature for DME challenge structure.
 *
 * @param	DmePtr is the pointer to the DME challenge structure.
 *
 * @return
 *		- XST_SUCCESS if DME challenge signature generation is successful.
 * 		- ErrorCode if there is a failure.
 *
 **************************************************************************************************/
static int XOcp_DmeChallengeSignature(XOcp_Dme *DmePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;

	u8 Qx[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	XSecure_EllipticKey DmePubKeyXY = { .Qx = Qx,
					    .Qy = Qy };
	XSecure_EllipticKeyAddr DmePubKeyXYAddr = { .Qx = (u64)(UINTPTR)Qx,
						    .Qy = (u64)(UINTPTR)Qy };
	XSecure_EllipticSign DmeSignature = { .SignR = R,
					      .SignS = S };
	XSecure_TrngInstance *TrngInstance;
	u8 DecDmePrivKey[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0};
	XOcp_RegSpace* XOcp_Reg = XOcp_GetRegSpace();
	u8 Hash[XSECURE_HASH_SIZE_IN_BYTES];
	u8 EphimeralKey[XSECURE_ECC_P384_SIZE_IN_BYTES];

	/* Start the TRNG */
	Status = XSecure_TrngInitNCfgMode(XSECURE_TRNG_HRNG_MODE, NULL, 0, NULL);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XLOADER_TRNG_INIT_FAIL;
		goto END;
	}

	/* Calculate the hash on DME challenge structure */
	Status = XOcp_DmeHashGen(DmePtr, Hash);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DIGEST_CALC;
		goto END;
	}

	/* Decrypt the DME private key with PUF key */
	Status = XOcp_DmeDecryptPrivKey((u32)(UINTPTR)&DecDmePrivKey[0]);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_DEC_PRIV_KEY;
		goto END;
	}

	/* Reseed and wait */
	Status = XSecure_TrngInitNCfgMode(XSECURE_TRNG_PTRNG_MODE, NULL, 0, NULL);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XLOADER_TRNG_INIT_FAIL;
		goto END;
	}

	/* Generate the DME public key */
	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P384, DecDmePrivKey, &DmePubKeyXY);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_GEN_DME_PUB_KEY;
		goto END;
	}

	/* Pair Wise Consistency Test check */
	Status = XSecure_EllipticPwct(XSECURE_ECC_NIST_P384, (u64)(UINTPTR)DecDmePrivKey,
				      &DmePubKeyXYAddr);
	if (Status != (u32)XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"ERROR: DME Pwct failed!\r\n");
		Status = Status | XOCP_ERR_DME_PWCT;
		goto END;
	}

	/* Generate the ephemeral key using TRNG */
	Status = XSecure_EllipticGenerateEphemeralKey(XSECURE_ECC_NIST_P384,
						      (u32)(UINTPTR)EphimeralKey);
	if (Status != (u32)XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"ERROR: Ephemeral Key generation failed!\r\n");
		Status = Status | XOCP_ERR_GEN_ECC_KEY;
		goto END;
	}

	/* Generate the signature */
	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P384, Hash,
			(u32)XSECURE_HASH_SIZE_IN_BYTES, DecDmePrivKey, EphimeralKey,
			&DmeSignature);
	if (Status != (u32)XST_SUCCESS) {
		/* Clear the Ephimeral key if signature generation is failed */
		Xil_SMemSet((void *)(UINTPTR)EphimeralKey, XSECURE_HASH_SIZE_IN_BYTES,
			    0U, XSECURE_HASH_SIZE_IN_BYTES);
		XPlmi_Printf(DEBUG_INFO,"ERROR: DME Signature generation failed!\r\n");
		Status = Status | XOCP_ERR_GEN_DME_SIGN;
		goto END;
	}

	/* Clear the Ephimeral key */
	ClrStatus = Xil_SMemSet((void *)(UINTPTR)EphimeralKey, XSECURE_HASH_SIZE_IN_BYTES,
			     0U, XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		ClrStatus = Status | ClrStatus;
		goto END;
	}

	/* Copy DME signature from local buffer to register */
	Status = Xil_SMemCpy((void *)(UINTPTR)XOcp_Reg->DmeSignRAddr, XSECURE_HASH_SIZE_IN_BYTES,
			     (void *)(UINTPTR)DmeSignature.SignR, XSECURE_HASH_SIZE_IN_BYTES,
			     XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_SIGN_COPY_FAILED;
		goto END;
	}
	Status = Xil_SMemCpy((void *)(UINTPTR)XOcp_Reg->DmeSignSAddr, XSECURE_HASH_SIZE_IN_BYTES,
			     (void *)(UINTPTR)DmeSignature.SignS, XSECURE_HASH_SIZE_IN_BYTES,
			     XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != (u32)XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_SIGN_COPY_FAILED;
		goto END;
	}

	/* Verify the signature generated */
	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P384, Hash,
			(u32)XSECURE_HASH_SIZE_IN_BYTES, &DmePubKeyXY, &DmeSignature);
	if (Status != (u32)XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"ERROR: Signature verification failed!\r\n");
		Status = Status | XOCP_ERR_DME_SIGN_VERIF_FAILED;
		goto END;
	}

END:

	TrngInstance = XSecure_GetTrngInstance();
	Status = XSecure_Uninstantiate(TrngInstance);

	/** Zeroize all local buffers */
	ClrStatus = Xil_SecureZeroize((u8 *)Qx, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)Qy, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)R, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)S, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)DecDmePrivKey, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}
	ClrStatus = Xil_SecureZeroize((u8 *)Hash, XSECURE_HASH_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generates the response to DME challenge request from PLM.
 *
 * @param	NonceAddr holds the address of 32 bytes buffer Nonce, which shall be used to fill
 * 		one of the member of DME structure.
 *
 * @param	DmeStructResAddr is the address to the 224 bytes buffer, which is used to store the
 * 		response to DME challenge request of type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS - Upon success.
 *		- XST_FAILURE - Upon failure.
 *
 **************************************************************************************************/
int XOcp_GenerateDmeResponseImpl(u64 NonceAddr, u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XOcp_RegSpace* XOcp_Reg = XOcp_GetRegSpace();
#ifdef PLM_OCP_KEY_MNGMT
	u32 *DevIkPubKey = (u32 *)(UINTPTR)XOcp_Reg->DevIkPubXAddr;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XSecure_Sha3Hash Sha3Hash_Instance;
#endif
	XOcp_Dme DmeInput;
	XOcp_DmeResponse *DmeResponse = XOcp_GetDmeResponse();
	XOcp_Dme *DmePtr = &DmeInput;
	XSecure_TrngInstance *XSecureTrngInstance = NULL;

	/* Zeorizing the DME structure */
	Status = Xil_SMemSet((void *)(UINTPTR)DmePtr, sizeof(XOcp_Dme), 0U, sizeof(XOcp_Dme));
	if (Status != XST_SUCCESS) {
		Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		goto RET;
	}

#ifdef PLM_OCP_KEY_MNGMT
	/* Fill the DME structure's DEVICE ID field with hash of DEV IK Public key */
	if (XOcp_IsDevIkReady() != FALSE) {
		if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != TRUE) {
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, SStatus,
							   XSecure_Sha3Kat, ShaInstPtr);
			if ((SStatus != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
				Status |= SStatus;
				goto RET;
			}
			XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);
		}
		Status = XSecure_Sha3Digest(ShaInstPtr, (UINTPTR)DevIkPubKey,
					    XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES,
					    &Sha3Hash_Instance);
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_ERR_DIGEST_CALC;
			goto RET;
		}
		Status = Xil_SMemCpy((void *)(UINTPTR)DmePtr->DeviceID,
				     XSECURE_MAX_HASH_SIZE_IN_BYTES,
				     (const void *)(UINTPTR)&Sha3Hash_Instance.Hash,
				     XSECURE_MAX_HASH_SIZE_IN_BYTES,
				     XSECURE_MAX_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_ERR_HASH_COPY_FAILED;
			goto RET;
		}
	}
#endif

	/* Fill the DME structure with Nonce */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)DmePtr->Nonce, NonceAddr, XOCP_DME_NONCE_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_PARAM_COPY_FAILED;
		goto RET;
	}

	Status = XOcp_DmeChallengeSignature(DmePtr);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_CH_SIGN;
		goto RET;
	}

	/* Copy the contents to user DME response structure */
	Status = Xil_SChangeEndiannessAndCpy((u8*)(UINTPTR)DmeResponse->DmeSignatureR,
				XOCP_ECC_P384_SIZE_BYTES, (const u8 *)XOcp_Reg->DmeSignRAddr,
				XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		Status = Status | XOCP_ERR_DME_SIGN_COPY_FAILED;
		goto RET;
	}

	Status = Xil_SChangeEndiannessAndCpy((u8*)(UINTPTR)DmeResponse->DmeSignatureS,
				XOCP_ECC_P384_SIZE_BYTES, (const u8 *)XOcp_Reg->DmeSignSAddr,
				XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status == XST_SUCCESS) {
		Status = Xil_SMemCpy(&DmeResponse->Dme, sizeof(XOcp_Dme), DmePtr, sizeof(XOcp_Dme),
					sizeof(XOcp_Dme));
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_DME_ERR;
			goto RET;
		}
		Status = XPlmi_MemCpy64(DmeStructResAddr, (u64)(UINTPTR)DmeResponse,
				sizeof(XOcp_DmeResponse));
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_DME_ERR;
			goto RET;
		}
	}
RET:
	if (Status != XST_SUCCESS) {
		ClearStatus = XPlmi_MemSet((u64)(UINTPTR)&DmeInput, 0U, sizeof(XOcp_Dme) /
			XOCP_WORD_LEN);
		if (ClearStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		} else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	/**
	 * TRNG is used for DME service and resets the core after the usage
	 * in this case TRNG state should be set to uninitialized state
	 * so that it can be re-initialize during runtime requests.
	 */
	XSecureTrngInstance = XSecure_GetTrngInstance();
	if (!XSecure_TrngIsUninitialized(XSecureTrngInstance)){
		SStatus = XSecure_Uninstantiate(XSecureTrngInstance);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			if (SStatus != XST_SUCCESS) {
				Status = SStatus | XOCP_DME_ERR;
			}
		}
	}

	return Status;
}
#endif /* PLM OCP */
