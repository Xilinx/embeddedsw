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
* 1.8   skg  12/07/22 Added Additional PPKs non zero check
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xloader_auth_enc.h"
#include "xilpdi.h"
#include "xplmi.h"
#include "xplmi_status.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef PLM_EN_ADD_PPKS
static int XLoader_IsAdditionalPpkFeatureEnabled(void);
static int XLoader_CheckNonZeroAdditionalPpk(void);
#endif

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from
 * boot header.
 *
 * @param	PdiPtr is pointer to the XilPdi instance.
 *
 * @return	None.
 *
 ******************************************************************************/
void XLoader_UpdateKekSrc(XilPdi *PdiPtr)
{
	PdiPtr->KekStatus = 0x0U;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED "
			"key availability status\n\r");
	switch(PdiPtr->MetaHdr.BootHdrPtr->EncStatus) {
	case XLOADER_BH_BLK_KEY:
		PdiPtr->KekStatus = XLOADER_BHDR_RED_KEY;
		break;
	case XLOADER_BBRAM_BLK_KEY:
		PdiPtr->KekStatus = XLOADER_BBRAM_RED_KEY;
		break;
	case XLOADER_EFUSE_BLK_KEY:
		PdiPtr->KekStatus = XLOADER_EFUSE_RED_KEY;
		break;
	default:
		/* No KEK is available for PLM */
		break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after "
			"for PLM %x\n\r", PdiPtr->KekStatus);
}

/*****************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source
 *
 * @param	PdiKeySrc is the Key source given in Pdi
 * @param	KekStatus is the current KekStatus
 * @param	KeySrcPtr is the pointer to the calculated KeySrc
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 KekStatus, void *KeySrcPtr)
{
	(void)PdiKeySrc;
	(void)KeySrcPtr;
	(void)KekStatus;

	/* Obfuscated Key is not supported in Versal */
	return XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, 0U);
}

/*****************************************************************************/
/**
* @brief    This function runs the KAT for RSA
*
* @param    PmcDmaPtr - Pointer to DMA instance
*
* @return   XST_SUCCESS on success and error code on failure
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
* @brief	This function checks Secure State for Authentication
*
* @param	AHWRoT - Buffer to store Secure state for authentication
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_CheckSecureStateAuth(volatile u32* AHWRoT)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u8 IsBhdrAuth;
	volatile u8 IsBhdrAuthTmp;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroPpk);
#ifdef PLM_EN_ADD_PPKS
	if((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)){
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroAdditionalPpk);
	}
#endif
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
		 * If PPK hash is programmed in eFUSEs, then Secure State of boot is A-HWRoT
		 */
		*AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Asymmetric HWRoT\r\n");
	}
	else {
		if ((IsBhdrAuth == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE) ||
			(IsBhdrAuthTmp == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE)) {
			/**
			 * If BHDR authentication is enabled, then Secure State of boot is emulated A-HWRoT
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

/*****************************************************************************/
/**
* @brief	This function verifies whether the additional PPK is valid.
*
* @param	PpkHash is pointer to the PPK hash.
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_IsAdditionalPpkValid(const u8 *PpkHash) {
	int Status = XST_FAILURE;
#ifdef PLM_EN_ADD_PPKS
	int StatusTmp = XST_FAILURE;

	/**< Read Additional PPks enable bits*/
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
* @return	XST_SUCCESS on success and error code on failure
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

#ifdef PLM_EN_ADD_PPKS
/*****************************************************************************/
/**
* @brief	This function checks if Additional PPK is programmed.
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_CheckNonZeroAdditionalPpk(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/**< Read Additional PPks enable bits*/
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
* @return	XST_SUCCESS on success and error code on failure
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
#endif /* END OF PLM_SECURE_EXCLUDE */
