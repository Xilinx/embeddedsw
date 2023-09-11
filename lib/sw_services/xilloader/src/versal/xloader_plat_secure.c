/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xloader_plat_secure.c
*
* This file contains the versal specific secure code related to PDI image
* loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       kpt  07/05/2022 Added support to update KAT status
*       kpt  07/05/2022 Added XLoader_RsaKat
* 1.01  har  11/17/2022 Added XLoader_CheckSecureStateAuth
*       ng   11/23/2022 Fixed doxygen file name error
*       skg  12/07/2022 Added Additional PPKs non zero check
*       sk   03/17/2023 Renamed Kekstatus to DecKeySrc in xilpdi structure
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   06/12/2023 Renamed XLoader_UpdateKekSrc to XLoader_GetKekSrc
* 1.9   kpt  07/12/2023 Added mask generation function
*       sk   07/26/2023 Made status redundant variable volatile in
*                       XLoader_IsAdditionalPpkValid
*       sk   07/31/2023 Removed Additional PPK check in XLoader_CheckSecureStateAuth
*       dd   08/11/2023 Updated doxygen comments
*	kpt  08/20/2023 Fix compilation warning by placing XLoader_I2Osp prototype under
*			PLM_SECURE_EXCLUDE
*       dd   09/11/2023 MISRA-C violation Rule 10.3 fixed
*       dd	 09/11/2023 MISRA-C violation Rule 12.1 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi.h"
#include "xplmi_status.h"
#include "xloader_plat_secure.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef PLM_EN_ADD_PPKS
#define XLOADER_EFUSE_ADDITIONAL_PPK_ENABLE_BITS_MASK       (0X00030000U)
                    /**< PPK 3&4 Enable bits mask*/
#define XLOADER_EFUSE_PPK3_START_OFFSET                 (0xF12502C0U)
                    /**< PPK3 start register address */
#define XLOADER_EFUSE_PPK4_END_OFFSET                   (0xF12502FCU)
                    /**< PPK4 start register address */
#endif /**< END OF PLM_EN_ADD_PPKS*/

/************************** Function Prototypes ******************************/

#ifdef PLM_EN_ADD_PPKS
static int XLoader_IsAdditionalPpkFeatureEnabled(void);
static int XLoader_CheckNonZeroAdditionalPpk(void);
#endif

#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert);
#endif

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from
 * 			boot header.
 *
 * @return
 *			 - Decryption Key source status.
 *
 ******************************************************************************/
u32 XLoader_GetKekSrc(void)
{
	volatile u32 DecKeySrc = 0x0U;
	const XilPdi_BootHdr *BootHdrPtr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED "
			"key availability status\n\r");
	switch(BootHdrPtr->EncStatus) {
	case XLOADER_BH_BLK_KEY:
		DecKeySrc = XLOADER_BHDR_RED_KEY;
		break;
	case XLOADER_BBRAM_BLK_KEY:
		DecKeySrc = XLOADER_BBRAM_RED_KEY;
		break;
	case XLOADER_EFUSE_BLK_KEY:
		DecKeySrc = XLOADER_EFUSE_RED_KEY;
		break;
	default:
		/* No KEK is available for PLM */
		break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after "
			"for PLM %x\n\r", DecKeySrc);

	return DecKeySrc;
}

/*****************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source
 *
 * @param	PdiKeySrc is the Key source given in Pdi
 * @param	DecKeyMask is the current DecKeyMask
 * @param	KeySrcPtr is the pointer to the calculated KeySrc
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 DecKeyMask, void *KeySrcPtr)
{
	(void)PdiKeySrc;
	(void)KeySrcPtr;
	(void)DecKeyMask;

	/* Obfuscated Key is not supported in Versal */
	return XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, 0);
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
* @brief    This function runs the KAT for RSA
*
* @param    PmcDmaPtr - Pointer to DMA instance
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_RsaKat(XPmcDma *PmcDmaPtr) {
	int Status = XST_FAILURE;

	(void)PmcDmaPtr;

	Status = XSecure_RsaPublicEncryptKat();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a
 * 			specified length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 * @return
 * 			- None
 *
 ******************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XLOADER_I2OSP_INT_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/*****************************************************************************/
/**
 * @brief	Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr is pointer to the XSecure_Sha3 instance.
 * @param	Out is pointer in which output of this function will be stored.
 * @param	OutLen specifies the required length.
 * @param	Input is pointer which holds the input data for	which mask should
 * 			be calculated which should be 48 bytes length.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
 *
 ******************************************************************************/
int XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr,
	u8 * Out, u32 OutLen, u8 *Input)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = XLOADER_SHA3_LEN;
	XSecure_Sha3Hash HashStore;
	u8 Convert[XIH_PRTN_WORD_LEN] = {0U};
	u32 Size = XLOADER_SHA3_LEN;
	u8 *OutTmp;

	if ((Sha3InstancePtr == NULL) || (Out == NULL) ||
		(Input == NULL)) {
		goto END;
	}

	if (OutLen == 0U) {
		goto END;
	}

	OutTmp = Out;
	while (Counter <= (OutLen / HashLen)) {
		XLoader_I2Osp(Counter, XIH_PRTN_WORD_LEN, Convert);

		Status = XSecure_Sha3Start(Sha3InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha3Update(Sha3InstancePtr, (UINTPTR)Input, HashLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha3Update(Sha3InstancePtr, (UINTPTR)Convert,
					XIH_PRTN_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha3Finish(Sha3InstancePtr, &HashStore);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (Counter == (OutLen / HashLen)) {
			/*
			 * Only 463 bytes are required but the chunklen is 48 bytes.
			 * The extra bytes are discarded by the modulus operation below.
			 */
			 Size = (OutLen % HashLen);
		}
		Status = Xil_SMemCpy(OutTmp, Size, HashStore.Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		OutTmp = &OutTmp[XLOADER_SHA3_LEN];
		Counter = Counter + 1U;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(Convert, sizeof(Convert), 0U,
                        sizeof(Convert));
	ClearStatus |= XPlmi_MemSetBytes(&HashStore, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}
#endif

/*****************************************************************************/
/**
* @brief	This function verifies whether the additional PPK is valid.
*
* @param	PpkHash is pointer to the PPK hash.
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_IsAdditionalPpkValid(const u8 *PpkHash) {
#ifdef PLM_EN_ADD_PPKS
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	/** - Read Additional PPks enable bits*/
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsAdditionalPpkFeatureEnabled);
	if((Status == XST_SUCCESS) && (StatusTmp == XST_SUCCESS)){
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid,
				XLOADER_PPK_SEL_3, PpkHash);
		if((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid,
				XLOADER_PPK_SEL_4, PpkHash);
			if ((Status == XST_SUCCESS) &&
			(	StatusTmp == XST_SUCCESS)) {
				/* Selection matched with PPK4 HASH */
				XPlmi_Printf(DEBUG_INFO, "PPK4 is valid\n\r");
			}
		}
		else {
			/* Selection matched with PPK3 HASH */
			XPlmi_Printf(DEBUG_INFO, "PPK3 is valid\n\r");
		}
	}
	Status |= StatusTmp;
#else
	int Status = XST_FAILURE;
	(void)PpkHash;
#endif

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks for the additional PPK select and returns the
*           PPK invalid mask and PPK efuse cache start offset if PPK is valid.
*
* @param	PpkSelect	PPK selection of eFUSE.
* @param    InvalidMask Pointer to the PPK invalid mask
* @param    PpkOffset   Pointer to the efuse cache PPK start offset
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_AdditionalPpkSelect(XLoader_PpkSel PpkSelect, u32 *InvalidMask, u32 *PpkOffset)
{
	int Status = XST_FAILURE;

#ifdef PLM_EN_ADD_PPKS
	switch ((u32)PpkSelect) {
		case XLOADER_PPK_SEL_3:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK3_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK3_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_4:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK4_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK4_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
			break;
	}
#else
	(void)PpkSelect;
	(void)InvalidMask;
	(void)PpkOffset;
#endif

	return Status;
}

#endif/* END OF PLM_SECURE_EXCLUDE */
/*****************************************************************************/
/**
 * @brief	This function checks Secure State for Authentication
 *
 * @param	AHWRoT - Buffer to store Secure state for authentication
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED if PPK is programmed and
 * 			boot header authentication is enabled.
 *
 ******************************************************************************/
int XLoader_CheckSecureStateAuth(volatile u32* AHWRoT)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u8 IsBhdrAuth;
	volatile u8 IsBhdrAuthTmp;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroPpk);
	IsBhdrAuth = (u8)((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
			XIH_BH_IMG_ATTRB_BH_AUTH_MASK) >>
			XIH_BH_IMG_ATTRB_BH_AUTH_SHIFT);
	IsBhdrAuthTmp = (u8)((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_BH_AUTH_MASK) >>
		XIH_BH_IMG_ATTRB_BH_AUTH_SHIFT);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		if ((IsBhdrAuth == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE) ||
		(IsBhdrAuthTmp == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED, 0);
			goto END;
		}
		/**
		 * - If PPK hash is programmed in eFUSEs, then Secure State of
		 * boot is A-HWRoT.
		 */
		*AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Asymmetric HWRoT\r\n");
	}
	else {
		if ((IsBhdrAuth == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE) ||
			(IsBhdrAuthTmp == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE)) {
			/**
			 * - If BHDR authentication is enabled, then Secure State of boot
			 * is emulated A-HWRoT.
			 */
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT;
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Emulated Asymmetric HWRoT\r\n");
		}
		else {
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_NONSECURE;
		}
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

#ifdef PLM_EN_ADD_PPKS
/*****************************************************************************/
/**
 * @brief	This function checks if Additional PPK is programmed.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 *
 ******************************************************************************/
static int XLoader_CheckNonZeroAdditionalPpk(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/** - Read Additional PPks enable bits*/
	Status = XLoader_IsAdditionalPpkFeatureEnabled();
	if(Status != XST_SUCCESS){
		goto END;
	}

	for (Index = XLOADER_EFUSE_PPK3_START_OFFSET;
		Index <= XLOADER_EFUSE_PPK4_END_OFFSET;
		Index = Index + XIH_PRTN_WORD_LEN) {
		/* Any bit of PPK hash are non-zero break and return success */
		if (XPlmi_In32(Index) != 0x0U) {
			Status = XST_SUCCESS;
			break;
		}
	}
	if (Index > (XLOADER_EFUSE_PPK4_END_OFFSET + XIH_PRTN_WORD_LEN)) {
		Status = (int)XLOADER_ERR_GLITCH_DETECTED;
	}
	else if (Index < XLOADER_EFUSE_PPK3_START_OFFSET) {
		Status = (int)XLOADER_ERR_GLITCH_DETECTED;
	}
	else if (Index <= XLOADER_EFUSE_PPK4_END_OFFSET) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function Reads Additional PPKs Enable bits.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_EFUSE_5_PPK_FEATURE_NOT_SUPPORTED if additional PPKs are
 * 			not enabled.
 *
 ******************************************************************************/
static int XLoader_IsAdditionalPpkFeatureEnabled(void)
{
	int Status = XST_FAILURE;
		volatile u32 ReadReg;
		volatile u32 ReadRegTmp;

		/* Read PPK3 and PPK4 enable set bits */
		ReadReg = Xil_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
		ReadRegTmp = Xil_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);

		ReadReg = ReadReg & XLOADER_EFUSE_ADDITIONAL_PPK_ENABLE_BITS_MASK;
		ReadRegTmp = ReadRegTmp & XLOADER_EFUSE_ADDITIONAL_PPK_ENABLE_BITS_MASK;

		if ((ReadReg == 0x0U) || (ReadRegTmp == 0x0U)) {
			Status = (int)XLOADER_EFUSE_5_PPK_FEATURE_NOT_SUPPORTED;
			goto END;
		}
		Status = XST_SUCCESS;
END:
	return Status;
}

#endif /* END OF PLM_EN_ADD_PPKS */
