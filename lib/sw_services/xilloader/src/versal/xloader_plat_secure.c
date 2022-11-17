/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_plat_secure.c
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

#endif /* END OF PLM_SECURE_EXCLUDE */
