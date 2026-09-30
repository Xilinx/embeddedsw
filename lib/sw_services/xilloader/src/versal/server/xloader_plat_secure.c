/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 2.0   kpt  10/09/2023 Fixed compilation warning when PLM_EN_ADD_PPKS macro is enabled
* 2.1   pre  01/11/2024 Removed unused macro
*       ng   02/01/2024 u8 variables optimization
*       sk   03/13/24 Fixed doxygen comments format
* 2.4   vss  02/01/2026 Updated PPK revoke error logic.
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

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

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifndef PLM_SECURE_EXCLUDE
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
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_RsaKat(void) {
	int Status = XST_FAILURE;

	Status = XSecure_RsaPublicEncryptKat();

	return Status;
}
#endif
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
	volatile u32 IsBhdrAuth;
	volatile u32 IsBhdrAuthTmp;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroPpk);
	IsBhdrAuth = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
			XIH_BH_IMG_ATTRB_BH_AUTH_MASK) >>
			XIH_BH_IMG_ATTRB_BH_AUTH_SHIFT);
	IsBhdrAuthTmp = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_BH_AUTH_MASK) >>
		XIH_BH_IMG_ATTRB_BH_AUTH_SHIFT);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		if ((IsBhdrAuth == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE) ||
		(IsBhdrAuthTmp == XIH_BH_IMG_ATTRB_BH_AUTH_VALUE)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED, XIL_SIGNED_ZERO);
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


/** @} end of xloader_server_apis group */
