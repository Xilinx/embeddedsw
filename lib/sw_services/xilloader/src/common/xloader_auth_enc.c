/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_auth_enc.c
*
* This file contains authentication and encryption related code
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   12/16/20 First release
*       kal  12/23/20 Initialize Status to XST_FAILURE in XLoader_AesKatTest
*       kpt  01/06/21 Added redundancy for the loop in XLoader_CheckNonZeroPpk
*       kpt  01/12/21 Added check to validate keysrc for partitions when
*                     DEC only efuse bits are set
*       kpt  01/18/21 Added check to validate the index of for loop with lower
*                     bounds of ppk offset in XLoader_CheckNonZeroPpk
*       har  01/19/21 Added support for P521 KAT
*       kpt  01/21/21 Added check to verify revoke id before enabling Auth Jtag
*       har  02/01/21 Added check for metaheader encryption source
*       bm   02/12/21 Updated logic to use BootHdr directly from PMC RAM
*       kpt  02/16/21 Corrected check to return valid error code in case of
*                     MetaHeader IV mismatch and fixed gcc warning
*       har  03/02/21 Added support to verify IHT as AAD for first secure header
*       har  03/17/21 Cleaned up code to use the secure state of boot
*       ma   03/24/21 Redirect XilPdi prints to XilLoader
*       ma   03/24/21 Minor updates to prints in XilLoader
*       bm   04/10/21 Updated scheduler function calls
*       kpt  04/14/21 Added check to verify whether the encrypted data is 128 bit
*                     aligned
*       bm   05/10/21 Updated chunking logic for hashes
*       bm   05/13/21 Updated code to use common crypto instances from xilsecure
*       ma   05/18/21 Minor code cleanup
*       har  05/19/21 Support decryption of partition even if Secure state of
*                     boot is A-HWRoT or Emulated A-HWRoT
*       ma   05/21/21 Read KAT Status from RTCA Secure Boot State location
* 1.01  kpt  06/23/21 Added check to compare DNA before enabling Auth Jtag
*            07/01/21 Added support to disable Jtag as per the timeout
*                     set by user
*       td   07/08/21 Fix doxygen warnings
*       ma   07/12/21 Register NULL error handler for
*                     XLoader_CheckAuthJtagIntStatus scheduler task
*       har  07/15/21 Fixed doxygen warnings
*       td   07/15/21 Fixed doxygen warnings
*       bsv  08/17/21 Code clean up
*       rb   08/11/21 Fix compilation warnings
*       bm   08/24/2021 Added Extract Metaheader support
*       bsv  08/31/21 Code clean up
*       kpt  09/02/21 Added support to update KAT status in RTC area
*       am   09/09/21 Fixed multiple SPK Authentication while authenticating
*                     MetaHeader
*       kpt  09/09/21 Fixed SW-BP-BLIND-WRITE in XLoader_AuthEncClear
*       kpt  09/15/21 Modified check for PUF HD in XLoader_SecureEncOnlyValidations
*       kpt  09/18/21 Fixed SW-BP-REDUNDANCY
*                     Added check in XLoader_CheckAuthJtagIntStatus to avoid access
*                     to auth jtag if there is a failure in single attempt
*                     Renamed BHSignature variable to IHTSignature
*       bsv  10/01/21 Addressed code review comments
* 1.02  kpt  10/01/21 Removed redundant code in XLoader_VerifyRevokeId
*       kpt  10/07/21 Decoupled checksum functionality from secure code
*       kpt  10/20/21 Modified temporal checks to use temporal variables from
*                     data section
*       kpt  10/28/21 Fixed PMCDMA1 hang issue in sbi checksum copy to memory
*                     mode
* 1.03  skd  11/18/21 Added time stamps in XLoader_ProcessAuthEncPrtn
*       bsv  12/04/21 Address security review comment
*       kpt  12/13/21 Replaced standard library utility functions with xilinx
*                     maintained functions
*       skd  01/11/22 Moved comments to its proper place
*       skd  01/12/22 Updated goto labels for better readability
*       bsv  02/09/22 Code clean up to reduce stack size
*       bsv  02/09/22 Code clean up
*       bsv  02/10/22 Code clean up by removing unwanted initializations
*       bsv  02/11/22 Code optimization to reduce text size
*       bsv  02/13/22 Reduce stack usage of functions
*       har  02/17/22 Updated code to limit number of attempts to enable JTAG
*                     when efuse bits are set
* 1.04  skg  06/20/22 Fixed MISRA C Rule 10.3 violation
*       bm   07/06/22 Refactor versal and versal_net code
*       kpt  07/07/22 Added support to update KAT status
*       bsv  07/08/22 Changes related to Optional data in Image header table
*       kpt  07/24/22 Added support to go into secure lockdown when KAT fails
*       kpt  08/03/22 Added volatile keyword to avoid compiler optimization
*                     of loop redundancy checks
* 1.05  har  10/11/22 Used temporal check macro for redundancy checks for Xil_SMemCpy
*       sk   10/19/22 Fix security review comments
*       har  11/17/22 Made XLoader_CheckNonZeroPpk as non-static and moved here from xloader_secure.c file
*       ng   11/23/22 Updated doxygen comments
* 1.8   skg  12/07/22 Added Additional PPKs support
*       kal  01/05/23 Moved XLoader_GetAuthPubAlgo function to header file
*       sk   02/08/23 Renamed XLoader_UpdateKatStatus to XLoader_ClearKatOnPPDI
*       sk   02/09/23 Fixed Sec Review comments in XLoader_RsaSignVerify function
* 1.9   kpt  02/21/23 Fixed bug in XLoader_AuthEncClear
*       sk   02/28/23 Removed using of pointer to string literal in XLoader_AuthKat
*       sk   03/10/23 Added redundancy for AES Key selection
*       sk   03/17/23 Renamed Kekstatus to DecKeySrc in xilpdi structure
*       dc   03/30/23 Updated ECDSA authentication logic to support both BE/LE
*       ng   03/30/23 Updated algorithm and return values in doxygen comments
*       sk   05/18/2023 Deprecate copy to memory feature
*       kal  06/18/23 Send device to SLD when 2nd AuthJTag message authentication
*	              fails, when AUTH_JTAG_LOCK_DIS eFuse is programmed
*       am   06/19/23 Added KAT error code for failure cases
*       sk   07/06/23 Added Jtag DAP config support for Non-Secure Debug
*       am   07/03/23 Added authentication optimization support
*       ng   07/13/23 Added support for system device tree flow
*       yog  08/18/23 Added a check to return error when metaheader secure state
*                     doesnot match with plm secure state
*	kpt  08/20/23 Updated check to place ECDSA in reset and clear RAM memory when
*			PLM_ECDSA_EXCLUDE is not defined
*       yog  08/25/23 Removed check to return error code when MH secure state doesn't
*			match with plm secure
*       dd   09/11/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xloader_auth_enc.h"
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi_dma.h"
#include "xsecure_error.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_elliptic.h"
#include "xsecure_aes_core_hw.h"
#include "xsecure_rsa_core.h"
#include "xsecure_utils.h"
#include "xplmi_modules.h"
#include "xplmi_scheduler.h"
#include "xsecure_init.h"
#include "xloader_plat_secure.h"
#include "xloader_plat.h"
#include "xplmi_config.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u32 JtagTimeOut;	/**< Timeout value set by user */
	u8 JtagTimerEnabled;	/**< Enable JTAG timer */
	volatile u8 AuthFailCounter;
		/**< Counter for failed attempts to authenticate JTAG */
	volatile u8 AuthFailCounterTmp;	/**< For temporal redundancy */
} XLoader_AuthJtagStatus;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef PLM_RSA_EXCLUDE
#define XLOADER_RSA_PSS_MSB_PADDING_MASK	(u8)(0x80U)
					/**< RSA PSS MSB padding mask */
#define XLOADER_RSA_EM_MSB_INDEX		(0x0U)
					/**< RSA EM MSB Index */
#endif
#define XLOADER_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT	(31U)
		/**< Shift for Global Variation Filter in PUF shutter value */
#define XLOADER_AES_RESET_VAL			(0x1U)
					/**< AES Reset value */
#define XLOADER_AES_RESET_REG			(0xF11E0010U)
					/**< AES Reset register address */
#if !defined(PLM_RSA_EXCLUDE) || !defined(PLM_ECDSA_EXCLUDE)
#define XLOADER_ECDSA_RSA_RESET_REG     (0xF1200040U)
                    /**< ECDSA RSA Reset register address */
#define XLOADER_ECDSA_RSA_RESET_VAL			(0x1U)
					/**< ECDSA RSA Reset value */
#endif

/************************** Function Prototypes ******************************/

static int XLoader_SpkAuthentication(const XLoader_SecureParams *SecurePtr);
static int XLoader_DataAuth(XLoader_SecureParams *SecurePtr, u8 *Hash,
	u8 *Signature);
#ifndef PLM_ECDSA_EXCLUDE
static int XLoader_EcdsaSignVerify(const XSecure_EllipticCrvTyp CrvType, const u8 *DataHash,
	const u8 *Key, const u32 KeySize, const u8 *Signature);
#endif
#ifndef PLM_RSA_EXCLUDE
static int XLoader_RsaSignVerify(const XLoader_SecureParams *SecurePtr,
	u8 *MsgHash, XLoader_RsaKey *Key, u8 *Signature);
#endif
static int XLoader_VerifySignature(const XLoader_SecureParams *SecurePtr,
	u8 *Hash, XLoader_RsaKey *Key, u8 *Signature);
static int XLoader_AesDecryption(XLoader_SecureParams *SecurePtr,
	u64 SrcAddr, u64 DestAddr, u32 Size);
static int XLoader_AesKeySelect(const XLoader_SecureParams *SecurePtr,
	XLoader_AesKekInfo *KeyDetails, XSecure_AesKeySrc *KeySrc);
static int XLoader_PpkVerify(const XLoader_SecureParams *SecurePtr);
static int XLoader_VerifyRevokeId(u32 RevokeId);
static int XLoader_PpkCompare(const u32 EfusePpkOffset, const u8 *PpkHash);
static int XLoader_AuthHdrs(const XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr);
static int XLoader_DecHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static int XLoader_AuthNDecHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static int XLoader_SetAesDpaCm(const XSecure_Aes *AesInstPtr, u8 DpaCmCfg);
static int XLoader_DecryptBlkKey(const XSecure_Aes *AesInstPtr,
	const XLoader_AesKekInfo *KeyDetails);
static int XLoader_AesKatTest(XLoader_SecureParams *SecurePtr);
static int XLoader_SecureEncOnlyValidations(const XLoader_SecureParams *SecurePtr);
static int XLoader_ValidateIV(const u32 *IHPtr, const u32 *EfusePtr);
static int XLoader_AuthJtag(u32 *TimeOut);
static int XLoader_VerifyAuthHashNUpdateNext(XLoader_SecureParams *SecurePtr,
	u32 Size, u8 Last);
static int XLoader_CheckSecureState(u32 RegVal, u32 Var, u32 ExpectedValue);
static void XLoader_ClearKatStatusOnCfg(XilPdi *PdiPtr, u32 PlmKatMask);
static int XLoader_AuthKat(XLoader_SecureParams *SecurePtr);
static int XLoader_Sha3Kat(XLoader_SecureParams *SecurePtr);

/************************** Variable Definitions *****************************/
static XLoader_AuthJtagStatus AuthJtagStatus = {0U};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief	This function initializes authentication parameters of
* 			XLoader_SecureParams's instance.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	PrtnHdr is pointer to XilPdi_PrtnHdr instance that has to be
* 			processed.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_INIT_AC_COPY_FAIL if failed to copy authentication
* 			certificate from flash device.
*
******************************************************************************/
int XLoader_SecureAuthInit(XLoader_SecureParams *SecurePtr,
			const XilPdi_PrtnHdr *PrtnHdr)
{
	int Status = XST_FAILURE;
	volatile u32 AuthCertificateOfstTmp = PrtnHdr->AuthCertificateOfst;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
	u64 AcOffset;
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
		XPLMI_PMCRAM_CHUNK_MEMORY_1;

	/**
	 * - Check if authentication is enabled and copy authentication
	 * certificate from PDI source to memory location provided in image header.
	 * Also initialize the required SecurePtr members.
	 */
	if ((PrtnHdr->AuthCertificateOfst != 0x00U) ||
		(AuthCertificateOfstTmp != 0x00U)) {
		 XPlmi_Printf(DEBUG_INFO, "Authentication is enabled\n\r");

		SecurePtr->IsAuthenticated = (u8)TRUE;
		SecureTempParams->IsAuthenticated = (u8)TRUE;
		SecurePtr->SecureEn = (u8)TRUE;
		SecureTempParams->SecureEn = (u8)TRUE;

		AcOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)SecurePtr->PrtnHdr->AuthCertificateOfst *
				XIH_PRTN_WORD_LEN);
		SecurePtr->AcPtr = AuthCert;

		/* Copy Authentication certificate */
		Status = SecurePtr->PdiPtr->MetaHdr.DeviceCopy(AcOffset,
			(UINTPTR)SecurePtr->AcPtr,
				XLOADER_AUTH_CERT_MIN_SIZE, SecurePtr->DmaFlags);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_INIT_AC_COPY_FAIL, Status);
			goto END;
		}
		SecurePtr->ProcessPrtn = XLoader_ProcessAuthEncPrtn;
		SecurePtr->SecureHdrLen += XLOADER_AUTH_CERT_MIN_SIZE;
		SecurePtr->ProcessedLen = XLOADER_AUTH_CERT_MIN_SIZE;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function initializes encryption parameters of
* 			XLoader_SecureParams's instance.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	PrtnHdr is pointer to XilPdi_PrtnHdr instance that has to be
* 			processed.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC if both checksum and
* 			authentication or encryption are enabled.
* 			- XLOADER_ERR_PRTN_DECRYPT_NOT_ALLOWED if Partition is not allowed to
* 			be encrypted if State of boot is non secure.
* 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
* 			- XLOADER_ERR_PRTN_ENC_ONLY_KEYSRC on invalid key source when
* 			only encryption is enabled.
*
******************************************************************************/
int XLoader_SecureEncInit(XLoader_SecureParams *SecurePtr,
			const XilPdi_PrtnHdr *PrtnHdr)
{
	volatile int Status = XST_FAILURE;
	u32 ReadReg = 0U;
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	/** - Check if encryption is enabled */
	if (PrtnHdr->EncStatus != 0x00U) {
		 XPlmi_Printf(DEBUG_INFO, "Encryption is enabled\n\r");
		SecurePtr->IsEncrypted = (u8)TRUE;
		SecureTempParams->IsEncrypted = (u8)TRUE;
		SecurePtr->SecureEn = (u8)TRUE;
		SecureTempParams->SecureEn = (u8)TRUE;
	}

    /**
     * - Error out if checksum is enabled along with authentication or
     * encryption.
	 */
	if ((SecurePtr->IsCheckSumEnabled == (u8)TRUE) &&
		((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		 (SecureTempParams->IsAuthenticated== (u8)TRUE) ||
		 (SecurePtr->IsEncrypted == (u8)TRUE) ||
		 (SecureTempParams->IsEncrypted == (u8)TRUE))) {
		XPlmi_Printf(DEBUG_INFO, "Error: Checksum should not be enabled with "
				"authentication or encryption\n\r");
		Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC, 0);
		goto END;
	}

	SecurePtr->AesInstPtr = XSecure_GetAesInstance();
	/**
	 * - Run AES Kat test if the image is encrypted
	 * and metaheader is not encrypted
	 */
	if ((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) {
		Status = XLoader_AesKatTest(SecurePtr);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "AES KAT test failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}

		Status = XST_FAILURE;
		/**
		 * - Check secure state of boot as partition is allowed to be encrypted
		 * only if Secure state of boot is S-HWRoT, Emul S-HWRoT,
		 * A-HWRoT or Emul A-HWRoT.
		 */
		ReadReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
		Status = XLoader_CheckSecureState(ReadReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_NONSECURE);
		if (Status == XST_SUCCESS) {
			Status = XST_FAILURE;
			ReadReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
			Status = XLoader_CheckSecureState(ReadReg,
				SecureStateAHWRoT, XPLMI_RTCFG_SECURESTATE_NONSECURE);
			if (Status == XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_PRTN_DECRYPT_NOT_ALLOWED, 0);
				goto END;
			}
			if (ReadReg != SecureStateAHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, 0);
				goto END;
			}

		}
		else {
			if (ReadReg != SecureStateSHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, 0);
				goto END;
			}
		}

		/**
		 * - Check Secure State of the device.
		 * If S-HWRoT is enabled, then validate keysrc
		 */
		ReadReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
		Status = XST_FAILURE;
		Status = XLoader_CheckSecureState(ReadReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_SHWROT);
		if (Status != XST_SUCCESS) {
			if (ReadReg != SecureStateSHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, 0);
				goto END;
			}
		}
		else {
			if ((SecurePtr->PrtnHdr->EncStatus == XLOADER_EFUSE_KEY) ||
				(SecurePtr->PrtnHdr->EncStatus == XLOADER_BBRAM_KEY)) {
				XPlmi_Printf(DEBUG_INFO, "Error: Invalid key source for "
						"decrypt only case\n\r");
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_PRTN_ENC_ONLY_KEYSRC, 0);
				goto END;
			}
		}
		SecurePtr->ProcessPrtn = XLoader_ProcessAuthEncPrtn;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if authentication/encryption is compulsory.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
* 			- XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO if PPK not programmed and
* 			authentication is enabled
* 			- XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY if PPK Programmed but
* 			eFuse authentication is disabled.
* 			- XLOADER_ERR_ENCONLY_ENC_COMPULSORY if encryption is disabled.
* 			- XLOADER_ERR_METAHDR_KEYSRC_MISMATCH if metaheader Key Source does
* 			not match PLM Key Source.
*
******************************************************************************/
int XLoader_SecureValidations(const XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	u32 ReadAuthReg;
	u32 ReadEncReg;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);
	u32 MetaHeaderKeySrc = SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.EncKeySrc;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	XPlmi_Printf(DEBUG_INFO, "Performing security checks\n\r");
	/**
	 * - Check Secure State of device if A-HWROT is enabled then authentication
	 * is mandatory for metaheader and BHDR authentication must be disabled.
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
				XPLMI_RTCFG_SECURESTATE_NONSECURE);
			if (Status != XST_SUCCESS) {
				if (ReadAuthReg != SecureStateAHWRoT) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_GLITCH_DETECTED, 0);
				}
				goto END;
			}

			if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
				(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO, 0);
				goto END;
			}
		}
		else {
			if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
				(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
				/*
				 * BHDR authentication is
				 * enabled and PPK hash is not programmed
				 */
				XPlmi_Printf(DEBUG_INFO,
					"Authentication with BH enabled\n\r");
			}
			else {
				/* Authentication is not compulsory */
				XPlmi_Printf(DEBUG_DETAILED,
					"Authentication is not enabled\n\r");
			}
			Status = XST_SUCCESS;
		}
	}
	else {
		/* Authentication is compulsory */
		if ((SecurePtr->IsAuthenticated == (u8)FALSE) &&
			(SecureTempParams->IsAuthenticated == (u8)FALSE)) {
			XPlmi_Printf(DEBUG_INFO,
				"HWROT is enabled, non authenticated PDI is"
				" not allowed\n\r");
			Status = XPlmi_UpdateStatus(
						XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY, 0);
			goto END;
		}
		Status = XST_SUCCESS;
		XPlmi_Printf(DEBUG_INFO, "HWROT- Authentication is enabled\n\r");
	}

	/**
	 * - Check Secure State of the device.
	 * If S-HWRoT is enabled, then metaheader must be encrypted.
	 */
	ReadEncReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
	Status = XST_FAILURE;
	Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
		XPLMI_RTCFG_SECURESTATE_SHWROT);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
				XPLMI_RTCFG_SECURESTATE_NONSECURE);
				if (Status != XST_SUCCESS) {
					if (ReadEncReg != SecureStateSHWRoT) {
						Status = XPlmi_UpdateStatus(
							XLOADER_ERR_GLITCH_DETECTED, 0);
					}
					goto END;
				}
		}
	}
	else {
		if ((SecurePtr->IsEncrypted == (u8)FALSE) &&
			(SecureTempParams->IsEncrypted == (u8)FALSE)) {
			XPlmi_Printf(DEBUG_INFO, "DEC_ONLY mode is set,"
			" non encrypted meta header is not allowed\n\r");
			Status = XPlmi_UpdateStatus(
						XLOADER_ERR_ENCONLY_ENC_COMPULSORY, 0);
			goto END;
		}
		XPlmi_Printf(DEBUG_INFO, "Encryption is enabled\n\r");
		/* Enc only validations */
		Status = XLoader_SecureEncOnlyValidations(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}


	/**
	 * - Verify if Metaheader encryption key source for FPDI/PPDI is same as
	 * PLM Key source in Bootheader.
	 */
	if (((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) &&
		(MetaHeaderKeySrc != XilPdi_GetPlmKeySrc())) {
			XPlmi_Printf(DEBUG_INFO, "Metaheader Key Source does not"
			" match PLM Key Source\n\r");
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_METAHDR_KEYSRC_MISMATCH, 0);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function validates the encryption keysrc, puf helper data
* 			location and eFUSE IV for ENC only case.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_ENC_ONLY_KEYSRC_ERR if key source is not from efuse
* 			black key when encrypt only is enabled.
* 			- XLOADER_SEC_ENC_ONLY_PUFHD_LOC_ERR if PUFHD location is from eFuse
* 			when encrypt only is enabled.
*
******************************************************************************/
static int XLoader_SecureEncOnlyValidations(const XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 IsEncKeySrc;
	volatile u32 IsEncKeySrcTmp;
	volatile u32 PufHdLocation;
	volatile u32 PufHdLocationTmp;

	/**
	 * - If encrypt only is enabled, verify that key source is from
	 * efuse black key.
	 * Otherwise return XLOADER_SEC_ENC_ONLY_KEYSRC_ERR.
	 */
	IsEncKeySrc = SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.EncKeySrc;
	IsEncKeySrcTmp = SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.EncKeySrc;
	if ((IsEncKeySrc != XLOADER_EFUSE_BLK_KEY) ||
		(IsEncKeySrcTmp != XLOADER_EFUSE_BLK_KEY)) {
		XPlmi_Printf(DEBUG_INFO, "DEC_ONLY mode is set, "
			"Key src should be eFUSE blk key\n\r");
		Status = (int)XLOADER_SEC_ENC_ONLY_KEYSRC_ERR;
		goto END;
	}

	/**
	 * - If encrypt only is enabled, verify that PUF HD is from efuse.
	 * Otherwise return XLOADER_SEC_ENC_ONLY_PUFHD_LOC_ERR.
	 */
	PufHdLocation =
		XilPdi_GetPufHdMetaHdr(&(SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl))
		>> XIH_PH_ATTRB_PUFHD_SHIFT;
	PufHdLocationTmp =
		XilPdi_GetPufHdMetaHdr(&(SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl))
		>> XIH_PH_ATTRB_PUFHD_SHIFT;
	if ((PufHdLocation != XLOADER_PUF_HD_EFUSE) ||
		(PufHdLocationTmp != XLOADER_PUF_HD_EFUSE)) {
		XPlmi_Printf(DEBUG_INFO, "DEC_ONLY mode is set, "
			"PUFHD should be from eFuse\n\r");
		Status = (int)XLOADER_SEC_ENC_ONLY_PUFHD_LOC_ERR;
		goto END;
	}

	/** - Validate MetaHdr IV range with eFUSE IV */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_ValidateIV,
		SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.IvMetaHdr,
		(u32*)XLOADER_EFUSE_IV_METAHDR_START_OFFSET);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "DEC_ONLY mode is set,"
			" eFuse Meta header IV range is not matched\n\r");
		Status |= StatusTmp;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function authenticates the image header table
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_IHT_GET_DMA if acquiring DMA is failed.
* 			- XLOADER_ERR_IHT_COPY_FAIL if IHT copy fails.
* 			- XLOADER_ERR_IHT_HASH_CALC_FAIL if IHT hash calculation fails.
* 			- XLOADER_ERR_IHT_AUTH_FAIL if IHT authentication fails.
*
******************************************************************************/
int XLoader_ImgHdrTblAuth(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	XSecure_Sha3Hash Sha3Hash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u64 AcOffset;
	XilPdi_ImgHdrTbl *ImgHdrTbl =
		&SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl;
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
		XPLMI_PMCRAM_CHUNK_MEMORY_1;

	XPlmi_Printf(DEBUG_INFO, "Authentication of"
			" Image header table\n\r");

	SecurePtr->AcPtr = AuthCert;

	/** - Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_GET_DMA, 0);
		goto END;
	}

	/** - Copy Authentication certificate from PDI to PMCRAM */
	AcOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)(ImgHdrTbl->AcOffset) << XPLMI_WORD_LEN_SHIFT);

	Status = SecurePtr->PdiPtr->MetaHdr.DeviceCopy(AcOffset,
		(UINTPTR)SecurePtr->AcPtr, XLOADER_AUTH_CERT_MIN_SIZE, 0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_COPY_FAIL,
				Status);
		goto END;
	}

	/** - Calculate SHA-3 hash of the image header table */
	Status = XST_FAILURE;
	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_HASH_CALC_FAIL,
					Status);
		goto END;
	}

	Status = XLoader_Sha3Kat(SecurePtr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "SHA3 KAT failed\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_Sha3Digest(Sha3InstPtr, (UINTPTR)XILPDI_PMCRAM_IHT_COPY_ADDR,
		(ImgHdrTbl->OptionalDataLen << XPLMI_WORD_LEN_SHIFT) + XIH_IHT_LEN,
		&Sha3Hash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_HASH_CALC_FAIL, Status);
		goto END;
	}


	/** - Authenticate Image header table */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DataAuth, SecurePtr,
		Sha3Hash.Hash, (u8 *)SecurePtr->AcPtr->IHTSignature);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_AUTH_FAIL, Status);
		XPlmi_Printf(DEBUG_INFO, "Authentication of image header table "
					"is failed\n\r");
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)Sha3Hash.Hash,
					XLOADER_SHA3_LEN >> XPLMI_WORD_LEN_SHIFT, "IHT Hash");
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Authentication of Image header table is "
				"successful\n\r");

END:
	if (Status != XST_SUCCESS) {
		/* On failure clear IHT structure which has invalid data */
		ClrStatus = XPlmi_InitNVerifyMem((UINTPTR)ImgHdrTbl, XIH_IHT_LEN);
		if (ClrStatus != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
		}
		else {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	ClrStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClrStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function authenticates and/or decrypts the image headers
* 			and partition headers and copies the contents to the corresponding
* 			structures.
*
* @param	SecurePtr	Pointer to the XLoader_SecureParams instance.
* @param	MetaHdr		Pointer to the Meta header table.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_HDR_GET_DMA if acquiring DMA fails.
* 			- XLOADER_ERR_HDR_AES_OP_FAIL if AES initialization fails.
* 			- XLOADER_ERR_METAHDR_LEN_OVERFLOW on meta header length overflow.
* 			- XLOADER_ERR_HDR_COPY_FAIL on IH/PH header copy fail.
* 			- XLOADER_ERR_SEC_IH_READ_FAIL on image header read fail.
* 			- XLOADER_ERR_SEC_IH_VERIFY_FAIL on image header verify fail.
* 			- XLOADER_ERR_HDR_NOT_SECURE if neither authentication nor
* 			encryption is enabled for IH/PH.
* 			- XLOADER_ERR_SEC_PH_READ_FAIL if partition header read fails.
* 			- XLOADER_ERR_SEC_PH_VERIFY_FAIL if partition header verification fails.
*
******************************************************************************/
int XLoader_ReadAndVerifySecureHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearIHs = XST_FAILURE;
	int ClearPHs = XST_FAILURE;
	int Clearchunk = XST_FAILURE;
	volatile u32 Ihs;
	volatile u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen *
								XIH_PRTN_WORD_LEN;
	volatile u32 TotalSizeTmp = XLOADER_CHUNK_SIZE + 1U;
	u32 ImgHdrAddr = MetaHdr->ImgHdrTbl.ImgHdrAddr * XIH_PRTN_WORD_LEN;
	u32 TotalImgHdrLen = MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN;
	u32 TotalPrtnHdrLen = MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	XPlmi_Printf(DEBUG_INFO,
		"Loading secure image headers and partition headers\n\r");
	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_GET_DMA, 0);
		goto ERR_END;
	}

	/*
	 * If headers are in encrypted format
	 * either authentication is enabled or not
	 */
	if ((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) {
		SecurePtr->AesInstPtr = XSecure_GetAesInstance();
		/** - Initialize AES driver */
		Status = XSecure_AesInitialize(SecurePtr->AesInstPtr, SecurePtr->PmcDmaInstPtr);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Failed at XSecure_AesInitialize\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AES_OP_FAIL,
					Status);
			goto ERR_END;
		}

		Status = XLoader_AesKatTest(SecurePtr);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "AES KAT test failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto ERR_END;
		}

		XPlmi_Printf(DEBUG_INFO, "Headers are in encrypted format\n\r");
		SecurePtr->ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;

		if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
			(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
			XPlmi_Printf(DEBUG_INFO, "Authentication is enabled\n\r");
			TotalSize -= XLOADER_AUTH_CERT_MIN_SIZE;
		}
		TotalSizeTmp = TotalSize;
		/** - Validate Meta header length */
		if ((TotalSize > XLOADER_SECURE_CHUNK_SIZE) ||
			(TotalSizeTmp > XLOADER_SECURE_CHUNK_SIZE)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_METAHDR_LEN_OVERFLOW, 0);
			goto ERR_END;
		}

		/** - Read headers to a buffer */
		/** - Read IHT and PHT to buffers along with encryption overhead */
		Status = MetaHdr->DeviceCopy((MetaHdr->FlashOfstAddr + ImgHdrAddr),
			SecurePtr->ChunkAddr, TotalSize, 0x0U);
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_COPY_FAIL, Status);
			goto ERR_END;
		}

		Status = XST_FAILURE;
		/** - Authenticate headers and decrypt the headers */
		if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
			(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthNDecHdrs,
				SecurePtr, MetaHdr, SecurePtr->ChunkAddr);
		}
		/** - Decrypt the headers */
		else {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecHdrs, SecurePtr,
				MetaHdr, SecurePtr->ChunkAddr);
		}
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Clearchunk = XPlmi_InitNVerifyMem((UINTPTR)SecurePtr->ChunkAddr,
							TotalSize);
			if (Clearchunk != XST_SUCCESS) {
				Status = (int)((u32)Status | XLOADER_SEC_CHUNK_CLEAR_ERR);
			}
			goto ERR_END;
		}
		/** - Read IHT and PHT to structures and verify checksum */
		XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Image Headers\n\r",
			MetaHdr->ImgHdrTbl.NoOfImgs);
		Status = Xil_SMemCpy((void *)MetaHdr->ImgHdr, TotalImgHdrLen,
			(void *)(UINTPTR)SecurePtr->ChunkAddr, TotalImgHdrLen,
			TotalImgHdrLen);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_IH_READ_FAIL, Status);
			goto ERR_END;
		}
		Status = XilPdi_VerifyImgHdrs(MetaHdr);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_IH_VERIFY_FAIL, Status);
			goto ERR_END;
		}
		/** - Verify Meta header is revoked or not */
		for (Ihs = 0U; Ihs < MetaHdr->ImgHdrTbl.NoOfImgs; Ihs++) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp,
				XLoader_VerifyRevokeId,
				MetaHdr->ImgHdr[Ihs].EncRevokeID);
			if ((Status != XST_SUCCESS) ||
				(StatusTmp != XST_SUCCESS)) {
				XPlmi_Printf(DEBUG_GENERAL, "Meta header is revoked\n\r");
				Status |= StatusTmp;
				goto ERR_END;
			}
		}
		if (Ihs != MetaHdr->ImgHdrTbl.NoOfImgs) {
			Status = XST_FAILURE;
			goto ERR_END;
		}

		/** - Update buffer address to point to PHs */
		XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Partition Headers\n\r",
			MetaHdr->ImgHdrTbl.NoOfPrtns);
		Status = Xil_SMemCpy((void *)MetaHdr->PrtnHdr, TotalPrtnHdrLen,
			(void *)(UINTPTR)(SecurePtr->ChunkAddr + TotalImgHdrLen),
			TotalPrtnHdrLen, TotalPrtnHdrLen);
	}
	/* If only authentication is enabled */
	else if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
			(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		XPlmi_Printf(DEBUG_INFO, "Headers are only authenticated\n\r");
		/** - Authenticate Image headers and partition headers */
		Status = XLoader_AuthHdrs(SecurePtr, MetaHdr);
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Headers are not secure\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_NOT_SECURE, 0);
		goto END;
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_PH_READ_FAIL, Status);
		goto ERR_END;
	}
	Status = XilPdi_VerifyPrtnHdrs(MetaHdr);
	if(Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_PH_VERIFY_FAIL, Status);
	}

ERR_END:
	if (Status != XST_SUCCESS) {
		ClearIHs = XPlmi_InitNVerifyMem((UINTPTR)&MetaHdr->ImgHdr[0U],
			TotalImgHdrLen);
		ClearPHs = XPlmi_InitNVerifyMem((UINTPTR)&MetaHdr->PrtnHdr[0U],
			TotalPrtnHdrLen);
		if ((ClearIHs != XST_SUCCESS) || (ClearPHs != XST_SUCCESS)) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
		}
		else {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function authenticates the data with SPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	Hash is a Pointer to the expected hash buffer.
* @param	Signature pointer points to the signature buffer.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_GLITCH_DETECTED_ERROR if glitch is detected.
*
******************************************************************************/
static int XLoader_DataAuth(XLoader_SecureParams *SecurePtr, u8 *Hash,
	u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	volatile u8 IsEfuseAuth = (u8)TRUE;
	volatile u8 IsEfuseAuthTmp = (u8)TRUE;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 ReadAuthReg = 0x0U;

	Status = XLoader_AuthKat(SecurePtr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Auth KAT failed\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	/* Check Secure state of device
	 * If A-HWRoT is disabled then BHDR authentication is allowed
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT);
		if (Status != XST_SUCCESS) {
			if (ReadAuthReg != SecureStateAHWRoT) {
				Status = XLoader_UpdateMinorErr(
					XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0);
			}
			goto END;
		}
		else {
			IsEfuseAuth = (u8)FALSE;
			IsEfuseAuthTmp = (u8)FALSE;
		}
	}
	else {
		Status = XST_FAILURE;
		IsEfuseAuth = (u8)TRUE;
		IsEfuseAuthTmp = (u8)TRUE;
		/* Validate PPK hash */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkVerify, SecurePtr);
	}

	/* Perform SPK Validation */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SpkAuthentication, SecurePtr);

	/* Check for SPK ID revocation */
	if ((IsEfuseAuth == (u8)TRUE) || (IsEfuseAuthTmp == (u8)TRUE)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyRevokeId,
			SecurePtr->AcPtr->SpkId);
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature, SecurePtr,
		Hash, &SecurePtr->AcPtr->Spk, Signature);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function encrypts the RSA/ECDSA signature provided and
* 			compares it with expected hash.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	Hash is pointer to the expected hash
* @param	Key is pointer to the RSA/ECDSA public key to be used
* @param	Signature is pointer to the Signature
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_INVALID_AUTH on invalid authentication type.
*
******************************************************************************/
static int XLoader_VerifySignature(const XLoader_SecureParams *SecurePtr,
		u8 *Hash, XLoader_RsaKey *Key, u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	u32 AuthType;

	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		AuthType = XLoader_GetAuthPubAlgo(&(SecurePtr->AuthJtagMessagePtr->AuthHdr));
	}
	else {
		AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	}

	/* RSA authentication */
	if (AuthType ==	XLOADER_PUB_STRENGTH_RSA_4096) {
#ifndef PLM_RSA_EXCLUDE
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_RsaSignVerify, SecurePtr,
			Hash, Key, Signature);
#else

		XPlmi_Printf(DEBUG_INFO, "RSA code is excluded\n\r");
		XPlmi_UpdateStatus(XLOADER_ERR_RSA_NOT_ENABLED, 0U);
		goto END;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
#ifndef PLM_ECDSA_EXCLUDE
		/* ECDSA P384 authentication */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_EcdsaSignVerify,
			XSECURE_ECC_NIST_P384, Hash, (u8 *)Key->PubModulus,
			XLOADER_ECDSA_P384_KEYSIZE, Signature);
#else
		XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
		XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, 0U);
		goto END;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
#ifndef PLM_ECDSA_EXCLUDE
		/* ECDSA P521 authentication */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_EcdsaSignVerify,
			XSECURE_ECC_NIST_P521, Hash, (u8 *)Key->PubModulus,
			XLOADER_ECDSA_P521_KEYSIZE, Signature);
#else
		XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
		XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, 0U);
		goto END;
#endif
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
	}
#if (defined(PLM_RSA_EXCLUDE) && defined(PLM_ECDSA_EXCLUDE))
	(void)Hash;
	(void)Key;
	(void)Signature;
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function verifies SPK with PPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_SPK_HASH_CALCULATION_FAIL if SPK hash calculation fails.
* 			- XLOADER_SEC_BUF_CLEAR_ERR if error in clearing buffer.
*
******************************************************************************/
static int XLoader_SpkAuthentication(const XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha3Hash SpkHash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	volatile int ClearStatus = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "Performing SPK verification\n\r");
	/** Initialize sha3 instance */
	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/**
	 * Hash the Authentication Header and SPK
	 */
	/* Update Authentication Header */
	Status = XST_FAILURE;
	Status = XSecure_Sha3Update(Sha3InstPtr,(UINTPTR)&SecurePtr->AcPtr->AuthHdr,
		XLOADER_AUTH_HEADER_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3LastUpdate(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/* Update SPK */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)&SecurePtr->AcPtr->Spk,
		XLOADER_SPK_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &SpkHash);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature, SecurePtr,
		SpkHash.Hash, &SecurePtr->AcPtr->Ppk,
		(u8 *)&SecurePtr->AcPtr->SPKSignature);
	XPlmi_Printf(DEBUG_INFO, "SPK verification is successful\n\r");

END:
	ClearStatus = XPlmi_MemSetBytes(&SpkHash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function validates SPK by verifying if the given SPK ID
* 			has been revoked or not.
*
* @param	RevokeId is ID of the SPK to be verified for revocation
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_REVOCATION_ID_OUTOFRANGE_ERR if revocation ID is out
* 			of range.
* 			- XLOADER_SEC_ID_REVOKED if revocation ID range not verified.
*
******************************************************************************/
static int XLoader_VerifyRevokeId(u32 RevokeId)
{
	int Status = XST_FAILURE;
	volatile u32 Quo;
	volatile u32 QuoTmp;
	volatile u32 Mod;
	volatile u32 ModTmp;
	volatile u32 Value;
	volatile u32 ValueTmp;

	XPlmi_Printf(DEBUG_INFO, "Validating SPKID\n\r");
	/** Verify if provided revocation ID is in range of 0 to 255 */
	if(RevokeId > XLOADER_REVOCATION_IDMAX) {
		XPlmi_Printf(DEBUG_INFO, "Revocation ID provided is out of range, "
			"valid range is 0 - 255\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_REVOCATION_ID_OUTOFRANGE_ERR,
			0x0);
		goto END;
	}

	Quo = RevokeId >> XLOADER_WORD_IN_BITS_SHIFT;
	QuoTmp = RevokeId >> XLOADER_WORD_IN_BITS_SHIFT;
	Mod = RevokeId & XLOADER_WORD_IN_BITS_MASK;
	ModTmp = RevokeId & XLOADER_WORD_IN_BITS_MASK;
	Quo <<= XPLMI_WORD_LEN_SHIFT;
	QuoTmp <<= XPLMI_WORD_LEN_SHIFT;
	Value = XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_0_OFFSET + Quo);
	Value &= ((u32)1U << Mod);
	ValueTmp = XPlmi_In32(XLOADER_EFUSE_REVOCATION_ID_0_OFFSET + QuoTmp);
	ValueTmp &= ((u32)1U << ModTmp);
	if((Value != 0x00U) || (ValueTmp != 0x00U)) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_ID_REVOKED, 0x0);
		goto END;
	}

	Status = XST_SUCCESS;
	XPlmi_Printf(DEBUG_INFO, "Revocation ID is valid\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function compares calculated PPK hash with the
* 			efuse PPK hash.
*
* @param	EfusePpkOffset is PPK hash address of efuse.
* @param	PpkHash is pointer to the PPK hash to be verified.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if hash comparision failed.
*
******************************************************************************/
static int XLoader_PpkCompare(const u32 EfusePpkOffset, const u8 *PpkHash)
{
	int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;

	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, PpkHash,
			XLOADER_EFUSE_PPK_HASH_LEN, (void *)EfusePpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN, XLOADER_EFUSE_PPK_HASH_LEN);
	if ((HashStatus != XST_SUCCESS) || (HashStatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Error: PPK Hash comparison failed\r\n");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_COMPARE_FAIL, 0x0);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	The function reads PPK invalid bits. If the bits are valid,
* 			it compares the provided hash value with the programed hash value.
* 			Efuse stores only 256 bits of hash.
*
* @param	PpkSelect	PPK selection of eFUSE.
* @param	PpkHash		Pointer to the PPK hash to be verified.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_PPK_INVALID_BIT_ERR if invalid PPK bit set.
* 			- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
*
******************************************************************************/
int XLoader_IsPpkValid(XLoader_PpkSel PpkSelect, const u8 *PpkHash)
{
	volatile int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;
	const u8 HashZeros[XLOADER_EFUSE_PPK_HASH_LEN] = {0U};
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;
	u32 PpkOffset;
	u32 InvalidMask;

	switch (PpkSelect) {
		case XLOADER_PPK_SEL_0:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK0_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_1:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK1_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_2:
			InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD;
			PpkOffset = XLOADER_EFUSE_PPK2_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XLoader_AdditionalPpkSelect(PpkSelect, &InvalidMask, &PpkOffset);
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Check if PPK bits are valid or not by reading the EFUSE bits */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & InvalidMask;
	ReadRegTmp = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & InvalidMask;
	if ((ReadReg != 0x0U) || (ReadRegTmp != 0x0U)) {
		Status = (int)XLOADER_SEC_PPK_INVALID_BIT_ERR;
		goto END;
	}
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkCompare, PpkOffset, PpkHash);

	Status = XST_FAILURE;
	/** - Check if valid PPK hash is all zeros */
	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, HashZeros,
			XLOADER_EFUSE_PPK_HASH_LEN, (void *)PpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN, XLOADER_EFUSE_PPK_HASH_LEN);
	if ((HashStatus == XST_SUCCESS) || (HashStatusTmp == XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0);
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function verifies PPK.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
* 			- XLOADER_SEC_PPK_HASH_CALCULATION_FAIL if PPK hash calculation fails
* 			- XLOADER_SEC_ALL_PPK_INVALID_ERR if all PPKs are invalid.
* 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
*
******************************************************************************/
static int XLoader_PpkVerify(const XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	XSecure_Sha3Hash Sha3Hash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u32 ReadReg;

	/** - Check if all PPKs are revoked */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD) ==
		(XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD)) {
		XPlmi_Printf(DEBUG_INFO, "All PPKs are invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_ALL_PPK_REVOKED_ERR, 0x0);
		goto END;
	}

	/** - Calculate PPK hash */
	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3LastUpdate(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/** - Update PPK  */
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		Status = XSecure_Sha3Update(Sha3InstPtr,
			(UINTPTR)&(SecurePtr->AuthJtagMessagePtr->PpkData), XLOADER_PPK_SIZE);
	}
	else {
		Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)&SecurePtr->AcPtr->Ppk,
			XLOADER_PPK_SIZE);
	}
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid,
			XLOADER_PPK_SEL_0, Sha3Hash.Hash);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid,
				XLOADER_PPK_SEL_1, Sha3Hash.Hash);
			if((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid,
					XLOADER_PPK_SEL_2, Sha3Hash.Hash);
				if((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
					XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsAdditionalPpkValid,
						Sha3Hash.Hash);
					if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
						XPlmi_Printf(DEBUG_INFO, "No PPK is valid\n\r");
						Status = XLoader_UpdateMinorErr(XLOADER_SEC_ALL_PPK_INVALID_ERR,
							0x0);
					}
				}
				else {
					/* Selection matched with PPK2 HASH */
					XPlmi_Printf(DEBUG_INFO, "PPK2 is valid\n\r");

				}
			}
			else {
					/* Selection matched with PPK1 HASH */
					XPlmi_Printf(DEBUG_INFO, "PPK1 is valid\n\r");

			}
		}
		else {
				/* Selection matched with PPK0 HASH */
				XPlmi_Printf(DEBUG_INFO, "PPK0 is valid\n\r");

	    }
END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#ifndef PLM_RSA_EXCLUDE

/*****************************************************************************/
/**
 *
 * @brief	This function encrypts the RSA signature provided and performs
 * 			required PSS operations to extract salt and calculates M prime hash and
 * 			compares with hash obtained from EM.
 *
 * @param   PmcDmaInstPtr Pointer to DMA instance
 * @param   MsgHash of the data to be authenticated.
 * @param   RsaInstPtr is pointer to the XSecure_Rsa instance.
 * @param   Signature is pointer to RSA signature for data to be authenticated.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_MEMSET_ERROR and XLOADER_SEC_RSA_MEMSET_SHA3_ARRAY_FAIL
 * 			if failed to create memory for RSA SHA3.
 * 			- XLOADER_SEC_MEMSET_ERROR and XLOADER_SEC_RSA_MEMSET_VARSCOM_FAIL
 * 			if failed to create memory for RSA varscom.
 * 			- XLOADER_SEC_RSA_PSS_ENC_BC_VALUE_NOT_MATCHED if failed to match
 * 			with RSA ENC 0xbc value.
 * 			- XLOADER_SEC_MASKED_DB_MSB_ERROR if error in RSA EM MSB.
 * 			- XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL if failed to verify RSA PSS
 * 			signature.
 * 			- XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR if failed to verify
 * 			database in efuse.
 * 			- XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE if RSA PSS hash is not
 * 			matched.
 * 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
 *
 ******************************************************************************/
int XLoader_RsaPssSignVerify(XPmcDma *PmcDmaInstPtr,
		u8 *MsgHash, XSecure_Rsa *RsaInstPtr, u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 DbTmp;
	XSecure_Sha3Hash MPrimeHash;
	volatile u8 HashTmp;
	/* To reduce stack usage, RsaSha3Array and Buffer are moved to a structure
	 * called XLoader_StoreSecureData which resides at XPLMI_PMC_CHUNK_MEMORY_1.
	 */
	XLoader_StoreSecureData *StoreSecureDataPtr = (XLoader_StoreSecureData *)
		(UINTPTR)(XPLMI_PMCRAM_CHUNK_MEMORY_1);
	u8 *XSecure_RsaSha3Array = StoreSecureDataPtr->RsaSha3Array;
	/**
	 * - Initialise Buffer variable to store HashMgf and DB.
	 */
	u8 *Buffer = StoreSecureDataPtr->Buffer;
	XLoader_Vars Xsecure_Varsocm __attribute__ ((aligned(32U)));
	volatile u32 Index;
	u32 IndexTmp;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u8 *DataHash = (u8 *)MsgHash;

	Status = XPlmi_MemSetBytes(XSecure_RsaSha3Array, XLOADER_PARTITION_SIG_SIZE,
				0U, XLOADER_PARTITION_SIG_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_MEMSET_ERROR,
				(int)XLOADER_SEC_RSA_MEMSET_SHA3_ARRAY_FAIL);
		goto END;
	}
	Status = XPlmi_MemSetBytes(&Xsecure_Varsocm, sizeof(Xsecure_Varsocm),
				0U, sizeof(Xsecure_Varsocm));
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_MEMSET_ERROR,
				(int)XLOADER_SEC_RSA_MEMSET_VARSCOM_FAIL);
		goto END;
	}

	/**
	 * - RSA signature encryption with public key components.
	 */
	Status = XSecure_RsaPublicEncrypt(RsaInstPtr, Signature,
					XSECURE_RSA_4096_KEY_SIZE,
					XSecure_RsaSha3Array);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
							 Status);
		goto END;
	}

	/**
	 * - Check for signature encrypted message.
	 */
	if (XSecure_RsaSha3Array[XSECURE_RSA_4096_KEY_SIZE - 1U] !=
			XLOADER_RSA_SIG_EXP_BYTE) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_ENC_BC_VALUE_NOT_MATCHED, 0);
		goto END;
	}

	if ((XSecure_RsaSha3Array[XLOADER_RSA_EM_MSB_INDEX] &
		XLOADER_RSA_PSS_MSB_PADDING_MASK) !=
		XLOADER_RSA_EM_MSB_EXP_BYTE) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_MASKED_DB_MSB_ERROR, 0);
		goto END;
	}

	/* As PMCDMA can't accept unaligned addresses */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, Xsecure_Varsocm.EmHash, XLOADER_SHA3_LEN,
		&XSecure_RsaSha3Array[XLOADER_RSA_PSS_MASKED_DB_LEN], XLOADER_SHA3_LEN, XLOADER_SHA3_LEN);

	Status = XSecure_Sha3Initialize(Sha3InstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
									 Status);
		goto END;
	}

	/**
	 * - Extract Salt and Generate DB from masked DB and Hash.
	 */
	Status = XLoader_MaskGenFunc(Sha3InstPtr, Buffer,
			XLOADER_RSA_PSS_MASKED_DB_LEN, Xsecure_Varsocm.EmHash);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/** - XOR MGF output with masked DB from EM to get DB. */
	for (Index = 0U; Index < XLOADER_RSA_PSS_MASKED_DB_LEN; Index++) {
		Buffer[Index] = Buffer[Index] ^ XSecure_RsaSha3Array[Index];
	}

	/** - Check DB = PS <414 zeros> || 0x01. */
	for (Index = 0U; Index < (XLOADER_RSA_PSS_DB_LEN - 1U); Index++) {
		if (Index == 0x0U) {
			Buffer[Index] = Buffer[Index] &
				(u8)(~XLOADER_RSA_PSS_MSB_PADDING_MASK);
		}

		if (Buffer[Index] != 0x0U) {
			Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR,
				Status);
			goto END;
		}
	}
	if (Index != (XLOADER_RSA_PSS_DB_LEN - 1U)) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR,
				Status);
		goto END;
	}

	DbTmp = Buffer[Index];
	if ((DbTmp != 0x01U) || (Buffer[Index] != 0x01U)) {
		Status = XLoader_UpdateMinorErr(
                                XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR,
                                Status);
		goto END;
	}

	/* As PMCDMA can't accept unaligned addresses */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, Xsecure_Varsocm.Salt, XLOADER_RSA_PSS_SALT_LEN,
		&Buffer[XLOADER_RSA_PSS_DB_LEN], XLOADER_RSA_PSS_SALT_LEN, XLOADER_RSA_PSS_SALT_LEN);

	/** - Hash on M prime */
	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/* Padding 1 */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)Xsecure_Varsocm.Padding1,
			XLOADER_RSA_PSS_PADDING1);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/* Hash the Message */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)DataHash, XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/* Salt */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)Xsecure_Varsocm.Salt,
			XLOADER_RSA_PSS_SALT_LEN);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &MPrimeHash);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	Status = XST_FAILURE;
	IndexTmp = XLOADER_RSA_PSS_MASKED_DB_LEN;
	/** - Compare MPrime Hash with Hash from EM */
	for (Index = 0U; Index < XLOADER_SHA3_LEN; Index++) {
		HashTmp = MPrimeHash.Hash[Index];
		if ((MPrimeHash.Hash[Index] != XSecure_RsaSha3Array[IndexTmp]) ||
			(HashTmp != XSecure_RsaSha3Array[IndexTmp])) {
			XPlmi_Printf(DEBUG_INFO, "Failed at RSA PSS signature "
				"verification\n\r");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)MPrimeHash.Hash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "M prime Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)XSecure_RsaSha3Array,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "RSA Encrypted Signature");
			Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE, 0);
			goto END;
		}
		IndexTmp++;
	}

	if (Index == XLOADER_SHA3_LEN) {
		Status = XST_SUCCESS;
	}
	XPlmi_Printf(DEBUG_INFO, "RSA PSS verification is successful\n\r");

END:
	ClearStatus = XPlmi_MemSetBytes(&MPrimeHash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function initializes the RSA module with provided key and
 *			verifies the signature
 *
 * @param   SecurePtr is pointer to the XLoader_SecureParams instance
 * @param   MsgHash of the data to be authenticated.
 * @param   Key is pointer to the XSecure_Rsa instance.
 * @param   Signature is pointer to RSA signature for data to be authenticated.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_RSA_AUTH_FAIL if failed to verify RSA signature.
 *
 ******************************************************************************/
static int XLoader_RsaSignVerify(const XLoader_SecureParams *SecurePtr,
		u8 *MsgHash, XLoader_RsaKey *Key, u8 *Signature) {

	volatile int Status = XST_FAILURE;
	XSecure_Rsa *RsaInstPtr = XSecure_GetRsaInstance();

	/* Initialize RSA instance */
	Status = XSecure_RsaInitialize(RsaInstPtr, (u8 *)Key->PubModulus,
			(u8 *)Key->PubModulusExt, (u8 *)&Key->PubExponent);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_RSA_AUTH_FAIL,
					 Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XLoader_RsaPssSignVerify(SecurePtr->PmcDmaInstPtr, MsgHash, RsaInstPtr,
				Signature);
END:
	return Status;

}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function encrypts the ECDSA signature provided with
 * 			key components.
 *
 * @param	CrvType  is the type of the ECDSA curve
 * @param	DataHash is pointer to the hash of the data to be authenticated
 * @param	Key is pointer to the ECDSA key.
 * @param	KeySize is the size of the public key component in bytes
 * @param	Signature is pointer to the ECDSA signature
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_ECDSA_INVLD_KEY_COORDINATES on invalid key
 * 			coordinated for ECDSA.
 * 			- XLOADER_SEC_ECDSA_AUTH_FAIL if failed to verify ECDSA
 * 			signature.
 *
 ******************************************************************************/
static int XLoader_EcdsaSignVerify(const XSecure_EllipticCrvTyp CrvType, const u8 *DataHash,
	const u8 *Key, const u32 KeySize, const u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	const u8 *XKey = Key;
	const u8 *YKey = &Key[KeySize];
	const u8 *RSign = Signature;
	const u8 *SSign = &Signature[KeySize];
	u8 Qx[XLOADER_ECDSA_MAX_KEYSIZE];
	u8 Qy[XLOADER_ECDSA_MAX_KEYSIZE];
	u8 SigR[XLOADER_ECDSA_MAX_KEYSIZE];
	u8 SigS[XLOADER_ECDSA_MAX_KEYSIZE];
	u8 Hash[XLOADER_SHA3_LEN];
	u32 Index;
	XSecure_EllipticKey PublicKey;
	XSecure_EllipticSign Sign;
	const u8 *HashPtr;

	/*
	 * Data generated by Bootgen is in BE, but Elliptic APIs are operated
	 * on LE, so the conversion is happened here.
	 * If User configured Elliptic APIs to be operated in BE
	 * This API is skipping the endianess change in else section.
	 */

	if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
		for (Index = 0U; Index < KeySize; Index++) {
			Qx[Index] = XKey[KeySize - Index - 1U];
			Qy[Index] = YKey[KeySize - Index - 1U];
			SigR[Index] = RSign[KeySize - Index - 1U];
			SigS[Index] = SSign[KeySize - Index - 1U];
		}

		for (Index = 0U; Index < XLOADER_SHA3_LEN; Index++) {
			Hash[Index] = DataHash[XLOADER_SHA3_LEN - Index - 1U];
		}
		PublicKey.Qx = Qx;
		PublicKey.Qy = Qy;
		Sign.SignR = SigR;
		Sign.SignS = SigS;
		HashPtr = (const u8 *)Hash;
	}
	else {
		PublicKey.Qx = (u8 *)XKey;
		PublicKey.Qy = (u8 *)YKey;
		Sign.SignR = (u8 *)RSign;
		Sign.SignS = (u8 *)SSign;
		HashPtr = DataHash;
	}

	/* Validate point on the curve */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_EllipticValidateKey,
		CrvType, &PublicKey);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Failed at "
			"ECDSA Key validation\n\r");
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_ECDSA_INVLD_KEY_COORDINATES, Status);
	}
	else {
		/* Verify ECDSA */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_EllipticVerifySign,
			CrvType, HashPtr, XLOADER_SHA3_LEN, &PublicKey, &Sign);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_ECDSA_AUTH_FAIL, Status);
			XPlmi_Printf(DEBUG_INFO, "Failed at ECDSA signature verification\n\r");
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Authentication of ECDSA is "
				"successful\n\r");
		}
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
 *
 * @brief	This function decrypts the secure header/footer.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	SrcAddr		Pointer to the buffer where header/footer present
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_AES_OPERATION_FAILED on AES operation failure.
 * 			- XLOADER_SEC_ENC_DATA_NOT_ALIGNED_ERROR if encrypted data is not
 * 			128-bit aligned.
 *
 ******************************************************************************/
static int XLoader_DecryptSecureBlk(XLoader_SecureParams *SecurePtr,
		u64 SrcAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	/** Configure AES engine to push Key and IV */
	XPlmi_Printf(DEBUG_INFO, "Decrypting Secure header\n\r");
	Status = XSecure_AesCfgKupKeyNIv(SecurePtr->AesInstPtr, (u8)TRUE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** Push secure header */
	Status = XSecure_AesDecryptUpdate(SecurePtr->AesInstPtr, SrcAddr,
		XSECURE_AES_NO_CFG_DST_DMA, XLOADER_SECURE_HDR_SIZE, (u8)TRUE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** Verify GCM Tag */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_AesDecryptFinal,
		SecurePtr->AesInstPtr, SrcAddr + XLOADER_SECURE_HDR_SIZE);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Decrypting Secure header failed in "
			"AesDecrypt Final\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** Check if the encrypted data is 128 bit aligned */
	if ((SecurePtr->AesInstPtr->NextBlkLen &
		XLOADER_128_BIT_ALIGNED_MASK) != 0x00U) {
		XPlmi_Printf(DEBUG_INFO, "Encrypted data is not 128 bit aligned\n\r");
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_ENC_DATA_NOT_ALIGNED_ERROR, 0);
		goto END;
	}

	SecurePtr->RemainingEncLen = SecurePtr->RemainingEncLen -
				XLOADER_SECURE_HDR_TOTAL_SIZE;
	XPlmi_Printf(DEBUG_DETAILED, "Decryption NextBlkLen is %0x\n\r",
		SecurePtr->AesInstPtr->NextBlkLen);

END:
	SStatus = XSecure_AesCfgKupKeyNIv(SecurePtr->AesInstPtr, (u8)FALSE);
	if ((Status == XST_SUCCESS) && (SStatus != XST_SUCCESS)) {
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			SStatus);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the data.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams
 * @param	SrcAddr is pointer to the buffer where header/footer present
 * @param	DestAddr is pointer to the buffer where header/footer should
 * 			be copied
 * @param	Size is the number of byte sto be copied
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_AES_OPERATION_FAILED on AES operation failure.
 * 			- XLOADER_SEC_DATA_LEFT_FOR_DECRYPT_ERR if data is still left for
 * 			decryption.
 * 			- XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH if there is a size
 * 			mismatch for the remaining data left for decryption.
 *
 ******************************************************************************/
static int XLoader_DataDecrypt(XLoader_SecureParams *SecurePtr,
		u64 SrcAddr, u64 DestAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u64 InAddr = SrcAddr;
	u64 OutAddr = DestAddr;
	u32 Iv[XLOADER_SECURE_IV_LEN];
	u32 ChunkSize = Size;
	u8 Index;
	u32 RegVal;

	do {
		for (Index = 0U; Index < XLOADER_SECURE_IV_LEN; Index++) {
			RegVal = XPlmi_In32(SecurePtr->AesInstPtr->BaseAddress +
					(XSECURE_AES_IV_0_OFFSET +
					((u32)Index * XIH_PRTN_WORD_LEN)));
			Iv[Index] = Xil_Htonl(RegVal);
		}

		Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr,
			XSECURE_AES_KUP_KEY, XSECURE_AES_KEY_SIZE_256,
			(UINTPTR)Iv);
		if (Status != XST_SUCCESS) {
			Status  = XLoader_UpdateMinorErr(
					XLOADER_SEC_AES_OPERATION_FAILED,
					Status);
			break;
		}

		/** Decrypt the data */
		Status = XSecure_AesDecryptUpdate(SecurePtr->AesInstPtr, InAddr,
			OutAddr, SecurePtr->AesInstPtr->NextBlkLen, 0U);
		if (Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
				Status);
			break;
		}

		InAddr = InAddr + SecurePtr->AesInstPtr->NextBlkLen;
		OutAddr = OutAddr + SecurePtr->AesInstPtr->NextBlkLen;
		SecurePtr->SecureDataLen = SecurePtr->SecureDataLen +
			SecurePtr->AesInstPtr->NextBlkLen;
		ChunkSize = ChunkSize - SecurePtr->AesInstPtr->NextBlkLen;
		SecurePtr->RemainingEncLen = SecurePtr->RemainingEncLen -
			SecurePtr->AesInstPtr->NextBlkLen;

		/** Decrypt Secure footer */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecryptSecureBlk,
			SecurePtr, InAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			break;
		}
		ChunkSize = ChunkSize - XLOADER_SECURE_HDR_TOTAL_SIZE;
		InAddr = InAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;

		if (ChunkSize == 0x00U) {
			break;
		}
		if (SecurePtr->AesInstPtr->NextBlkLen == 0x00U) {
			if (SecurePtr->RemainingEncLen != 0U) {
				/* Still remaining data is there for decryption */
				Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_DATA_LEFT_FOR_DECRYPT_ERR, 0x0);
			}
			break;
		}
		else {
			if (SecurePtr->RemainingEncLen < SecurePtr->AesInstPtr->NextBlkLen) {
				Status = XLoader_UpdateMinorErr(
					XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH, 0x0);
				break;
			}
			if (ChunkSize < SecurePtr->AesInstPtr->NextBlkLen) {
				Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH,
							0x0);
				break;
			}
		}
	} while (ChunkSize >= SecurePtr->AesInstPtr->NextBlkLen);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the data.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams
 * @param	SrcAddr is address to the buffer where header/footer present
 * @param	DestAddr is the address to which header / footer is copied
 * @param	Size is the number of bytes to be copied
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_AES_OPERATION_FAILED on AES operation failure.
 * 			- XLOADER_SEC_DPA_CM_ERR if failed to configure DPA CM.
 *
 ******************************************************************************/
static int XLoader_AesDecryption(XLoader_SecureParams *SecurePtr,
		 u64 SrcAddr, u64 DestAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_AesKeySrc KeySrc = XSECURE_AES_BBRAM_KEY;
	u32 ChunkSize = Size;
	volatile u8 DpaCmCfg;
	volatile u8 DpaCmCfgTmp;
	XLoader_AesKekInfo KeyDetails;
	u64 SrcOffset = 0U;

	SecurePtr->SecureDataLen = 0U;

	if (SecurePtr->BlockNum == 0x0U) {
		/** Initialize AES driver */
		Status = XSecure_AesInitialize(SecurePtr->AesInstPtr,
			SecurePtr->PmcDmaInstPtr);
		if (Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
				Status);
			goto END;
		}

		KeyDetails.PufHdLocation = XilPdi_GetPufHdPh(SecurePtr->PrtnHdr)
			>> XIH_PH_ATTRB_PUFHD_SHIFT;
		KeyDetails.PdiKeySrc = SecurePtr->PrtnHdr->EncStatus;
		KeyDetails.KekIvAddr = (UINTPTR)SecurePtr->PrtnHdr->KekIv;
		Status = XLoader_AesKeySelect(SecurePtr,&KeyDetails, &KeySrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/** Configure DPA counter measure */
		DpaCmCfg = XilPdi_IsDpaCmEnable(SecurePtr->PrtnHdr);
		DpaCmCfgTmp = XilPdi_IsDpaCmEnable(SecurePtr->PrtnHdr);
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SetAesDpaCm,
			SecurePtr->AesInstPtr, (DpaCmCfg | DpaCmCfgTmp));
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_DPA_CM_ERR, Status);
			goto END;
		}
		/* Decrypting SH */
		Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr, KeySrc,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)SecurePtr->PrtnHdr->PrtnIv);
		if (Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
				Status);
			goto END;
		}
		/** Decrypt Secure header */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecryptSecureBlk, SecurePtr,
				SrcAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		SrcOffset += XLOADER_SECURE_HDR_TOTAL_SIZE;
		ChunkSize = ChunkSize - XLOADER_SECURE_HDR_TOTAL_SIZE;
	}
	Status = XLoader_DataDecrypt(SecurePtr, SrcAddr + SrcOffset, DestAddr,
			ChunkSize);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "AES Decryption is successful\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function helps in key selection.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams
 * @param	KeyDetails is pointer to the key details.
 * @param	KeySrc is pointer to the key source to be updated as
 *			key source for decrypting. If key provided is KEK format, this API
 *			decrypts and provides the red key source.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_DEC_INVALID_KEYSRC_SEL on invalid key source selected
 * 			for decryption.
 * 			- XLOADER_SEC_AES_KEK_DEC if failed to decrypt KEK.
 *
 ******************************************************************************/
static int XLoader_AesKeySelect(const XLoader_SecureParams *SecurePtr,
		XLoader_AesKekInfo *KeyDetails, XSecure_AesKeySrc *KeySrc)
{
	volatile int Status = XLoader_UpdateMinorErr(XLOADER_SEC_DEC_INVALID_KEYSRC_SEL,
		0x0);
	u32 *DecKeyMask = &SecurePtr->PdiPtr->DecKeySrc;
	const XilPdi_BootHdr *BootHdr = SecurePtr->PdiPtr->MetaHdr.BootHdrPtr;
	u32 KekStat = 0U;
	volatile u32 PdiKeySrcTmp;
	u8 DecryptBlkKey = (u8)FALSE;

	*KeySrc = XSECURE_AES_INVALID_KEY;

	XPlmi_Printf(DEBUG_INFO, "Key source is %0x\n\r", KeyDetails->PdiKeySrc);

	switch (KeyDetails->PdiKeySrc) {
	case XLOADER_EFUSE_KEY:
		*KeySrc = XSECURE_AES_EFUSE_KEY;
		PdiKeySrcTmp = XLOADER_EFUSE_KEY;
		Status = XST_SUCCESS;
		break;
	case XLOADER_EFUSE_BLK_KEY:
		PdiKeySrcTmp = XLOADER_EFUSE_BLK_KEY;
		if (((*DecKeyMask) & XLOADER_EFUSE_RED_KEY) == 0x0U) {
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_KEY;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_RED_KEY;
			KekStat = XLOADER_EFUSE_RED_KEY;
			DecryptBlkKey = (u8)TRUE;
		}
		else {
			Status = XST_SUCCESS;
		}
		*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
		break;
	case XLOADER_BBRAM_KEY:
		PdiKeySrcTmp = XLOADER_BBRAM_KEY;
		*KeySrc = XSECURE_AES_BBRAM_KEY;
		Status = XST_SUCCESS;
		break;
	case XLOADER_BBRAM_BLK_KEY:
		PdiKeySrcTmp = XLOADER_BBRAM_BLK_KEY;
		if (((*DecKeyMask) & XLOADER_BBRAM_RED_KEY) == 0x0U) {
			KeyDetails->KeySrc = XSECURE_AES_BBRAM_KEY;
			KeyDetails->KeyDst = XSECURE_AES_BBRAM_RED_KEY;
			KekStat = XLOADER_BBRAM_RED_KEY;
			DecryptBlkKey = (u8)TRUE;
		}
		else {
			Status = XST_SUCCESS;
		}
		*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
		break;
	case XLOADER_BH_BLK_KEY:
		PdiKeySrcTmp = XLOADER_BH_BLK_KEY;
		if (((*DecKeyMask) & XLOADER_BHDR_RED_KEY) == 0x0U) {
			KeyDetails->KeySrc = XSECURE_AES_BH_KEY;
			KeyDetails->KeyDst = XSECURE_AES_BH_RED_KEY;
			KekStat = XLOADER_BHDR_RED_KEY;

			/* Write BH key into BH registers */
			Status = XSecure_AesWriteKey(SecurePtr->AesInstPtr,
				XSECURE_AES_BH_KEY, XSECURE_AES_KEY_SIZE_256,
					(UINTPTR)BootHdr->Kek);
			if (Status != XST_SUCCESS) {
				break;
			}
			DecryptBlkKey = (u8)TRUE;
		}
		else {
			Status = XST_SUCCESS;
		}
		*KeySrc = XSECURE_AES_BH_RED_KEY;
		break;
	case XLOADER_EFUSE_USR_KEY0:
		PdiKeySrcTmp = XLOADER_EFUSE_USR_KEY0;
		*KeySrc = XSECURE_AES_EFUSE_USER_KEY_0;
		Status = XST_SUCCESS;
		break;
	case XLOADER_EFUSE_USR_BLK_KEY0:
		PdiKeySrcTmp = XLOADER_EFUSE_USR_BLK_KEY0;
		if (((*DecKeyMask) & XLOADER_EFUSE_USR0_RED_KEY) == 0x0U) {
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_USER_KEY_0;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_USER_RED_KEY_0;
			KekStat = XLOADER_EFUSE_USR0_RED_KEY;
			DecryptBlkKey = (u8)TRUE;
		}
		else {
			Status = XST_SUCCESS;
		}
		*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_0;
		break;
	case XLOADER_EFUSE_USR_KEY1:
		PdiKeySrcTmp = XLOADER_EFUSE_USR_KEY1;
		*KeySrc = XSECURE_AES_EFUSE_USER_KEY_1;
		Status = XST_SUCCESS;
		break;
	case XLOADER_EFUSE_USR_BLK_KEY1:
		PdiKeySrcTmp = XLOADER_EFUSE_USR_BLK_KEY1;
		if (((*DecKeyMask) & XLOADER_EFUSE_USR1_RED_KEY) == 0x0U) {
			KeyDetails->KeySrc = XSECURE_AES_EFUSE_USER_KEY_1;
			KeyDetails->KeyDst = XSECURE_AES_EFUSE_USER_RED_KEY_1;
			KekStat = XLOADER_EFUSE_USR1_RED_KEY;
			DecryptBlkKey = (u8)TRUE;
		}
		else {
			Status = XST_SUCCESS;
		}
		*KeySrc = XSECURE_AES_EFUSE_USER_RED_KEY_1;
		break;
	case XLOADER_USR_KEY0:
		PdiKeySrcTmp = XLOADER_USR_KEY0;
		*KeySrc = XSECURE_AES_USER_KEY_0;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY1:
		PdiKeySrcTmp = XLOADER_USR_KEY1;
		*KeySrc = XSECURE_AES_USER_KEY_1;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY2:
		PdiKeySrcTmp = XLOADER_USR_KEY2;
		*KeySrc = XSECURE_AES_USER_KEY_2;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY3:
		PdiKeySrcTmp = XLOADER_USR_KEY3;
		*KeySrc = XSECURE_AES_USER_KEY_3;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY4:
		PdiKeySrcTmp = XLOADER_USR_KEY4;
		*KeySrc = XSECURE_AES_USER_KEY_4;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY5:
		PdiKeySrcTmp = XLOADER_USR_KEY5;
		*KeySrc = XSECURE_AES_USER_KEY_5;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY6:
		PdiKeySrcTmp = XLOADER_USR_KEY6;
		*KeySrc = XSECURE_AES_USER_KEY_6;
		Status = XST_SUCCESS;
		break;
	case XLOADER_USR_KEY7:
		PdiKeySrcTmp = XLOADER_USR_KEY7;
		*KeySrc = XSECURE_AES_USER_KEY_7;
		Status = XST_SUCCESS;
		break;
	default:
		PdiKeySrcTmp = KeyDetails->PdiKeySrc;
		/* Aes Obfuscated Key is applicable only for versal_net */
		Status = XLoader_AesObfusKeySelect(KeyDetails->PdiKeySrc,
				*DecKeyMask, KeySrc);
		break;
	}

	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	if ((KeyDetails->PdiKeySrc != PdiKeySrcTmp) || (*KeySrc == XSECURE_AES_INVALID_KEY)) {
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0);
		goto END;
	}

	if (DecryptBlkKey == (u8)TRUE) {
		Status = XLoader_DecryptBlkKey(SecurePtr->AesInstPtr, KeyDetails);
		if (Status == XST_SUCCESS) {
			*DecKeyMask = (*DecKeyMask) | KekStat;
		}
		else {
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, Status);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function authenticates image headers and partition headers
 * 			of image.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	MetaHdr		Pointer to the Meta header.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SEC_IH_READ_FAIL if failed to read image header.
 * 			- XLOADER_ERR_SEC_PH_READ_FAIL if failed to read partition header.
 * 			- XLOADER_ERR_HDR_HASH_CALC_FAIL if failed to calculate hash for
 * 			image header or partition header.
 * 			- XLOADER_ERR_HDR_AUTH_FAIL if failed to authenticate for image
 * 			header or partition header.
 * 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear the buffer.
 *
 ******************************************************************************/
static int XLoader_AuthHdrs(const XLoader_SecureParams *SecurePtr,
			XilPdi_MetaHdr *MetaHdr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XSecure_Sha3Hash Sha3Hash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();

	Status = XilPdi_ReadImgHdrs(MetaHdr);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_IH_READ_FAIL, Status);
		goto END;
	}

	Status = XilPdi_ReadPrtnHdrs(MetaHdr);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_PH_READ_FAIL, Status);
		goto END;
	}

	/**
	 * As SPK and PPK are validated during authentication of IHT,
	 * using the same valid SPK to authenticate IHs and PHs.
	 * Calculate hash on the data
	 */
	Status = XSecure_Sha3Initialize(Sha3InstPtr,
			SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecurePtr->AcPtr,
		XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/* Image headers */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)MetaHdr->ImgHdr,
				(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/* Partition headers */
	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)MetaHdr->PrtnHdr,
				(MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}
	/* Read hash */
	Status = XSecure_Sha3Finish(Sha3InstPtr, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/** Verify the signature */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
		SecurePtr, Sha3Hash.Hash, &SecurePtr->AcPtr->Spk,
		(u8 *)SecurePtr->AcPtr->ImgSignature);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AUTH_FAIL, Status);
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)Sha3Hash.Hash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "Headers Hash");
		goto END;
	}

	Status = XilPdi_VerifyImgHdrs(MetaHdr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Checksum validation of image headers "
			"failed\n\r");
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "Authentication of image headers is "
		"successful\n\r");

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function authenticates and decrypts the headers.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	MetaHdr		Pointer to the Meta header.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *			address.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_HDR_HASH_CALC_FAIL if failed to calculate hash for
 * 			image header or partition header.
 * 			- XLOADER_ERR_HDR_AUTH_FAIL if failed to authenticate for image
 * 			header or partition header.
 * 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear the buffer.
 * 			- XLOADER_SEC_BUF_CLEAR_SUCCESS if successfully cleared buffer.
 *
 ******************************************************************************/
static int XLoader_AuthNDecHdrs(XLoader_SecureParams *SecurePtr,
		XilPdi_MetaHdr *MetaHdr,
		u64 BufferAddr)
{

	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	XSecure_Sha3Hash CalHash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen << XPLMI_WORD_LEN_SHIFT;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	/** Authenticate the headers */
	Status = XSecure_Sha3Initialize(Sha3InstPtr,
				SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)SecurePtr->AcPtr,
		(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)BufferAddr, TotalSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &CalHash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	/** Verify the RSA PSS signature */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
		SecurePtr, CalHash.Hash, &SecurePtr->AcPtr->Spk,
		(u8 *)SecurePtr->AcPtr->ImgSignature);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AUTH_FAIL,
							 Status);
		goto END;
	}
	else {
		XPlmi_Printf(DEBUG_INFO,
			"Authentication of the headers is successful\n\r");
	}

	/** Decrypt the headers and copy to structures */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecHdrs, SecurePtr,
			MetaHdr, BufferAddr);
	Status |= StatusTmp;

END:
	if (Status != XST_SUCCESS) {
		/* Clear the buffer */
		ClrStatus = XPlmi_InitNVerifyMem(BufferAddr, TotalSize);
		if (ClrStatus != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
		}
		else {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
		XPlmi_Printf(DEBUG_INFO, "Authentication/Decryption of "
				"headers failed with error 0x%x\r\n", Status);
	}
	ClrStatus = XPlmi_MemSetBytes(&CalHash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClrStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts headers.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	MetaHdr		Pointer to the Meta header.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *			address
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_HDR_NOT_ENCRYPTED image header or partition header is
 * 			not encrypted.
 * 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 * 			- XLOADER_ERR_HDR_AES_OP_FAIL if failed due to AES init or Decrypt
 * 			init or key selection failure.
 * 			- XLOADER_SEC_EFUSE_DPA_CM_MISMATCH_ERROR Metahdr DpaCm & eFuse
 * 			DpaCm values are not matched.
 * 			- XLOADER_SEC_DPA_CM_ERR if failed to configure DPA CM.
 * 			- XLOADER_ERR_HDR_AES_OP_FAIL if failed to initialize AES or Decrypt
 * 			or key selection.
 * 			- XLOADER_ERR_HDR_AAD_UPDATE_FAIL if failed to update IHT as AAD
 * 			during secure header decryption.
 * 			- XLOADER_ERR_HDR_DEC_FAIL if failed to decrypt IH/PH.
 *
 ******************************************************************************/
static int XLoader_DecHdrs(XLoader_SecureParams *SecurePtr,
			XilPdi_MetaHdr *MetaHdr, u64 BufferAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 Iv[XLOADER_SECURE_IV_LEN];
	u8 Index;
	XSecure_AesKeySrc KeySrc = XSECURE_AES_BBRAM_KEY;
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen * XIH_PRTN_WORD_LEN;
	u64 SrcAddr = BufferAddr;
	u8 PdiDpaCmCfg = XilPdi_IsDpaCmEnableMetaHdr(&MetaHdr->ImgHdrTbl);
	u32 EfuseDpaCmCfg = XPlmi_In32(XLOADER_EFUSE_SEC_MISC1_OFFSET) &
		(XLOADER_EFUSE_SEC_DPA_DIS_MASK);
	XLoader_AesKekInfo KeyDetails;
	u32 Offset;
	u32 RegVal;
	u32 ReadEncReg;
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	if ((SecurePtr->IsEncrypted != (u8)TRUE) &&
		(SecureTempParams->IsEncrypted != (u8)TRUE)) {
		XPlmi_Printf(DEBUG_INFO, "Headers are not encrypted\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_NOT_ENCRYPTED, 0);
		goto END;
	}

	/**
	 * Check Secure State of device
	 * If S-HWRoT is enabled then it is mandatory to use Black IV
	 */
	ReadEncReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
		XPLMI_RTCFG_SECURESTATE_SHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT);
		if (Status != XST_SUCCESS) {
			if (ReadEncReg != SecureStateSHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, 0);
			}
			goto END;
		}
	}
	else {
		if ((SecurePtr->PdiPtr->DecKeySrc & XLOADER_EFUSE_RED_KEY) == 0x0U) {
			Status = Xil_SMemCpy(SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.KekIv,
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)),
				(u32*)XLOADER_EFUSE_IV_BLACK_OBFUS_START_OFFSET,
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)),
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)));
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	KeyDetails.PufHdLocation = XilPdi_GetPufHdMetaHdr(&MetaHdr->ImgHdrTbl) >>
		XIH_IHT_ATTR_PUFHD_SHIFT;
	KeyDetails.PdiKeySrc = MetaHdr->ImgHdrTbl.EncKeySrc;
	KeyDetails.KekIvAddr = (UINTPTR)SecurePtr->PdiPtr->MetaHdr.ImgHdrTbl.KekIv;

	/** Select the Key Source */
	Status = XLoader_AesKeySelect(SecurePtr, &KeyDetails, &KeySrc);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Failed at Key selection\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AES_OP_FAIL, Status);
		goto END;
	}

	if (((PdiDpaCmCfg == XLOADER_PDI_DPACM_ENABLED) && (EfuseDpaCmCfg == XLOADER_EFUSE_SEC_DPA_DIS_MASK)) ||
		((PdiDpaCmCfg == XLOADER_PDI_DPACM_DISABLED) && (EfuseDpaCmCfg != XLOADER_EFUSE_SEC_DPA_DIS_MASK))) {
		XPlmi_Printf(DEBUG_INFO, "MetaHdr DpaCmCfg not matching with DpaCm "
			"eFuses\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_EFUSE_DPA_CM_MISMATCH_ERROR,
			Status);
		goto END;
	}

	/** Configure DPA CM */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SetAesDpaCm,
		SecurePtr->AesInstPtr, PdiDpaCmCfg);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_DPA_CM_ERR, Status);
		goto END;
	}

	/* Decrypting SH */
	Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr, KeySrc,
		XSECURE_AES_KEY_SIZE_256, (UINTPTR)MetaHdr->ImgHdrTbl.IvMetaHdr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AES_OP_FAIL, Status);
		goto END;
	}

	Status = XSecure_AesUpdateAad(SecurePtr->AesInstPtr,
		(UINTPTR)XILPDI_PMCRAM_IHT_COPY_ADDR, XIH_IHT_LEN +
		(MetaHdr->ImgHdrTbl.OptionalDataLen << XPLMI_WORD_LEN_SHIFT));
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Updating Image header Table as AAD "
			"failed during secure header decryption\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AAD_UPDATE_FAIL,
			Status);
		goto END;
	}

	/** Decrypt Secure header */
	Status = XLoader_DecryptSecureBlk(SecurePtr, SrcAddr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "SH decryption failed during header "
			"decryption\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_DEC_FAIL, Status);
		goto END;
	}
	Offset = SecurePtr->AesInstPtr->BaseAddress + XSECURE_AES_IV_0_OFFSET;
	for (Index = 0U; Index < XLOADER_SECURE_IV_LEN; Index++) {
		RegVal = XPlmi_In32(Offset);
		Iv[Index] = Xil_Htonl(RegVal);
		Offset += XPLMI_WORD_LEN;
	}

	Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr,
		XSECURE_AES_KUP_KEY, XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Failed at header decryption in "
			"XSecure_AesDecryptInit\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AES_OP_FAIL, Status);
		goto END;
	}

	SrcAddr = SrcAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;
	TotalSize = TotalSize - XLOADER_SECURE_HDR_TOTAL_SIZE;
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DataDecrypt, SecurePtr,
		(UINTPTR)SrcAddr, (UINTPTR)SecurePtr->ChunkAddr, TotalSize);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_DEC_FAIL, Status);
		XPlmi_Printf(DEBUG_INFO, "Failed at headers decryption\n\r");
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "Headers decryption is successful\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables or disables DPA counter measures.
 *
 * @param	AesInstPtr	Pointer to the XSecure_Aes instance.
 * @param	DpaCmCfg
 *			- TRUE - to enable AES DPA counter measure
 *			- FALSE -to disable AES DPA counter measure
 *
 * @return
 *		- XST_SUCCESS if enable/disable of DPA was successful.
 *		- Error if device doesn't support DPA CM or configuration is
 *		not successful
 *
 ******************************************************************************/
static int XLoader_SetAesDpaCm(const XSecure_Aes *AesInstPtr, u8 DpaCmCfg)
{
	int Status = XST_FAILURE;

	/* Configure AES DPA CM */
	Status = XSecure_AesSetDpaCm(AesInstPtr, DpaCmCfg);

	/* If DPA CM request is to disable and device also not supports DPA CM */
	if ((Status == (int)XSECURE_AES_DPA_CM_NOT_SUPPORTED) &&
		(DpaCmCfg == (u8)FALSE)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the black key with PUF key and stores in
 * 			specified destination AES red key register.
 *
 * @param	AesInstPtr is pointer to the XSecure_Aes instance.
 * @param	KeyDetails is pointer to the XLoader_AesKekInfo instance.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 * 			- XLOADER_SEC_PUF_REGN_ERRR if failed to regenerate PUF.
 * 			- XLOADER_SEC_AES_KEK_DEC if failed to decrypt KEK.
 *
 ******************************************************************************/
static int XLoader_DecryptBlkKey(const XSecure_Aes *AesInstPtr,
					const XLoader_AesKekInfo *KeyDetails)
{
	int Status = XST_FAILURE;
	/* To reduce stack usage, XPufData is moved to a structure called
	 * XLoader_StoreSecureData which resides at XPLMI_PMC_CHUNK_MEMORY_1.
	 */
	XLoader_StoreSecureData *SecureDataStorePtr = (XLoader_StoreSecureData *)
		(UINTPTR)XPLMI_PMCRAM_CHUNK_MEMORY_1;
	XPuf_Data *PufData = &SecureDataStorePtr->PufData;

	Status = XPlmi_MemSetBytes(PufData, sizeof(XPuf_Data), 0U,
		sizeof(XPuf_Data));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Decrypting PUF KEK\n\r");
	PufData->ShutterValue = XPUF_SHUTTER_VALUE;
	PufData->PufOperation = XPUF_REGEN_ON_DEMAND;
	PufData->GlobalVarFilter = (u8)(PufData->ShutterValue >>
		XLOADER_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT);

	if (KeyDetails->PufHdLocation == XLOADER_PUF_HD_BHDR) {
		PufData->ReadOption = XPUF_READ_FROM_RAM;
		PufData->SyndromeAddr = XIH_BH_PRAM_ADDR + XIH_BH_PUF_HD_OFFSET;
		PufData->Chash = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_CHASH_OFFSET);
		PufData->Aux = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_AUX_OFFSET);
		XPlmi_Printf(DEBUG_INFO, "BHDR PUF HELPER DATA with CHASH:"
			"%0x and AUX:%0x\n\r", PufData->Chash, PufData->Aux);
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "EFUSE PUF HELPER DATA\n\r");
		PufData->ReadOption = XPUF_READ_FROM_EFUSE_CACHE;
	}

	Status = XPuf_Regeneration(PufData);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Failed at PUF regeneration with status "
			"%0x\n\r", Status);
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PUF_REGN_ERRR, Status);
		goto END;
	}

	Status = XSecure_AesKekDecrypt(AesInstPtr, KeyDetails->KeySrc,
		KeyDetails->KeyDst, (UINTPTR)KeyDetails->KekIvAddr,
		XSECURE_AES_KEY_SIZE_256);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Failed during AES KEK decrypt\n\r");
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "Black key decryption is successful\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief       This function performs KAT test on AES crypto Engine
*
* @param       SecurePtr       Pointer to the XLoader_SecureParams instance.
*
* @return      XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_AesKatTest(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 DpacmEfuseStatus;
	u32 PlmDpacmKatStatus;

	/* Update KAT status based on the user configuration */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SECURE_AES_CMKAT_MASK);

	/* Set the data context of previous AES operation */
	XSecure_AesSetDataContext(SecurePtr->AesInstPtr);

	/*
	 * Skip running the KAT for AES DPACM or AES if it is already run
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set
	 * If KAT fails device will go into a secure lockdown state
	 */
	DpacmEfuseStatus = XPlmi_In32(XLOADER_EFUSE_SEC_MISC1_OFFSET) &
		XLOADER_EFUSE_SEC_DPA_DIS_MASK;
	PlmDpacmKatStatus = SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SECURE_AES_CMKAT_MASK;

	if((DpacmEfuseStatus == 0U) && (PlmDpacmKatStatus == 0U)) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_AesDecryptCmKat, SecurePtr->AesInstPtr)
		if(Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_KAT_FAILED_ERROR, Status);
			XPlmi_Printf(DEBUG_GENERAL, "DPACM KAT failed\n\r");
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SECURE_AES_CMKAT_MASK;

		/* Update KAT status in RTC area */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}

	/* Update KAT status based on the user configuration */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SECURE_AES_DEC_KAT_MASK);

	if((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SECURE_AES_DEC_KAT_MASK) == 0U) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_AesDecryptKat, SecurePtr->AesInstPtr);
		if(Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_KAT_FAILED_ERROR, Status);
			XPlmi_Printf(DEBUG_GENERAL, "AES KAT failed\n\r");
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SECURE_AES_DEC_KAT_MASK;

		/* Update KAT status in RTC area */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	XPlmi_Printf(DEBUG_INFO, "KAT test on AES crypto engine is successful\r\n");

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief       IV Criteria check
*
* @param       IHPtr is the IV to be compared
* @param       EfusePtr is the eFUSE cache address
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_IV_METAHDR_RANGE_ERROR on Failure.
*
* Example: iv[95:0] - F7F8FDE0 8674A28D C6ED8E37
* Bootgen follows the big-endian format so the values stored will be
*
* IHPtr[0]=E0FDF8F7 -> IV[64:95]
* IHPtr[1]=8DA27486 -> IV[32:63]
* IhPtr[2]=378EEDC6 -> IV[0:31]
*
* Our xilnvm driver also follows the same format to store it in eFUSE
*
* EfusePtr[0]=E0FDF8F7 -> IV[64:95]
* EfusePtr[1]=8DA27486 -> IV[32:63]
* EfusePtr[2]=378EEDC6 -> IV[0:31]]
*
* Spec says:
* IV[95:32] defined by user in meta header should match with eFUSEIV[95:32]
* IV[31:0] defined by user in meta header should >= eFUSEIV[31:0]
*
******************************************************************************/
static int XLoader_ValidateIV(const u32 *IHPtr, const u32 *EfusePtr)
{
	int Status = (int)XLOADER_SEC_IV_METAHDR_RANGE_ERROR;
	volatile u32 IHPtr_0U = IHPtr[0U];
	volatile u32 IHPtrTmp_0U = IHPtr[0U];
	volatile u32 IHPtr_1U = IHPtr[1U];
	volatile u32 IHPtrTmp_1U = IHPtr[1U];
	volatile u32 IHPtr_2U = IHPtr[2U];
	volatile u32 IHPtrTmp_2U = IHPtr[2U];

	if ((IHPtr_0U != EfusePtr[0U]) || (IHPtrTmp_0U != EfusePtr[0U])) {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[95:64]\r\n");
	}
	else if ((IHPtr_1U != EfusePtr[1U]) || (IHPtrTmp_1U != EfusePtr[1U])) {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[63:32]\r\n");
	}
	else if ((IHPtr_2U >= EfusePtr[2U]) && (IHPtrTmp_2U >= EfusePtr[2U])) {
		Status = XST_SUCCESS;
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[31:0]\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief   This function adds periodic checks of the status of Auth
 * 			JTAG interrupt status to the scheduler.
 *
 * @return
 * 			- XST_SUCCESS otherwise error code is returned
 * 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 * 			- XLOADER_ERR_ADD_TASK_SCHEDULER if failed to add task to
 * 			scheduler.
 *
 ******************************************************************************/
int XLoader_AddAuthJtagToScheduler(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 AuthJtagDis = XLOADER_AUTH_JTAG_DIS_MASK;
	volatile u32 AuthJtagDisTmp = XLOADER_AUTH_JTAG_DIS_MASK;
	u32 ReadAuthReg;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);

	/**
	 * - Read auth_jtag_disable efuse bits(20:19) of Security Control register
	 * in Efuse Cache module.
	 */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
		XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
		XLOADER_AUTH_JTAG_DIS_MASK;

	/**
	 * - Read PPK hash registers in Efuse Cache module and
	 * check if they are non-zero.
	 *
	 * - Check the preconditions for adding task to the scheduler
	 *   - Auth jtag disable efuse bits should not be set.
	 *   - PPK hash registers should be non zero.
	 *
	 * - If the preconditions are correct then add task to the scheduler for
	 * calling the API to check Auth JTAG interrupt status.
	 */
	if ((AuthJtagDis != XLOADER_AUTH_JTAG_DIS_MASK) &&
		(AuthJtagDisTmp != XLOADER_AUTH_JTAG_DIS_MASK)) {
		ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_AHWROT);
		if (Status != XST_SUCCESS) {
			if (ReadAuthReg != SecureStateAHWRoT) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, 0);
			}
			else {
				Status = XST_SUCCESS;
			}
		}
		else {
			Status = XST_FAILURE;
			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_LOADER_ID,
				XLoader_CheckAuthJtagIntStatus, NULL,
				XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL,
				XPLM_TASK_PRIORITY_1, NULL, XPLMI_PERIODIC_TASK);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus( XLOADER_ERR_ADD_TASK_SCHEDULER, 0);
			}
			else {
				XPlmi_Printf(DEBUG_INFO, "Auth Jtag task added successfully\r\n");
			}
		}
	}
	else {
		/**
		 * - The task should not be added to the scheduler if Auth JTAG
		 * disable efuse bit is set or PPK hash is not programmed in
		 * efuse. Thus forcing the Status to be XST_SUCCESS.
		 */
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks the status of Auth JTAG interrupt status and
* 			it disables the Jtag as per the timeout set by user.
*
* @param	Arg Not used in the function currently
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_AUTH_JTAG_EXCEED_ATTEMPTS if Jtag Authentication
* 			failed more than once.
* 			- XLOADER_DAP_TIMEOUT_DISABLED if timeout is disabled for DAP.
*
* @note    	If Auth JTAG interrupt status is set, then XLoader_AuthJtag
* 			API will be called.
*****************************************************************************/
int XLoader_CheckAuthJtagIntStatus(void *Arg)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 InterruptStatus = 0U;
	volatile u32 InterruptStatusTmp = 0U;
	volatile u32 LockDisStatus = 0U;
	volatile u32 LockDisStatusTmp = 0U;

	(void)Arg;

	/**
	 * - Read Auth Jtag Interrupt Status register in PMC TAP module.
	 */
	InterruptStatus = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET) &
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK;
	InterruptStatusTmp = XPlmi_In32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET) &
		XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK;
	if ((InterruptStatus == XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK) &&
		(InterruptStatusTmp == XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK)) {
		XPlmi_Out32(XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET,
			XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK);
		/**
		  * - Check if Auth JTAG Lock disable efuse bits are set.
		  * If set then allow limited number of attempts to enable JTAG.
		  *
		  */
		LockDisStatus = XPlmi_In32(
			XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
			XLOADER_AUTH_JTAG_LOCK_DIS_MASK;
		LockDisStatusTmp = XPlmi_In32(
			XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
			XLOADER_AUTH_JTAG_LOCK_DIS_MASK;
		if ((LockDisStatus == XLOADER_AUTH_JTAG_LOCK_DIS_MASK) ||
			(LockDisStatusTmp == XLOADER_AUTH_JTAG_LOCK_DIS_MASK)) {
			if ((AuthJtagStatus.AuthFailCounter >= XLOADER_AUTH_JTAG_MAX_ATTEMPTS) ||
				(AuthJtagStatus.AuthFailCounterTmp >= XLOADER_AUTH_JTAG_MAX_ATTEMPTS)) {
				/**
				 * - When AUTH_JTAG_LOCK_DIS eFuse is programmed, allow only one
				 * failed attempt for AuthJTag message. For the second failure
				 * trigger secure lock down.
				 *
				 */
				XPlmi_TriggerTamperResponse(XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK,
				XPLMI_TRIGGER_TAMPER_TASK);
				goto END;
			}
		}

		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthJtag,
			&AuthJtagStatus.JtagTimeOut);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			AuthJtagStatus.AuthFailCounter += 1U;
			AuthJtagStatus.AuthFailCounterTmp += 1U;
			goto END;
		}

		if (AuthJtagStatus.JtagTimeOut == 0U) {
			AuthJtagStatus.JtagTimerEnabled = FALSE;
		}
		else {
			AuthJtagStatus.JtagTimerEnabled = TRUE;
		}
	}
	else {
		if (AuthJtagStatus.JtagTimerEnabled == TRUE) {
			AuthJtagStatus.JtagTimeOut--;
			if (AuthJtagStatus.JtagTimeOut == 0U) {
				Status = (int)XLOADER_DAP_TIMEOUT_DISABLED;
				goto END;
			}
		}
		Status = XST_SUCCESS;
	}

END:
	/* Reset DAP status */
	if (Status != XST_SUCCESS) {
		XLoader_DisableJtag();
		AuthJtagStatus.JtagTimerEnabled = FALSE;
		AuthJtagStatus.JtagTimeOut = (u32)0U;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function authenticates the data pushed in through PMC TAP
 * 			before enabling the JTAG
 *
 * @paramTimeOut Pointer to store the timeout value set by the user
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_AUTH_JTAG_DMA_XFR if failed to get authenticated JTAG
 * 			data with DMA transfer.
 * 			- XLOADER_ERR_AUTH_JTAG_DISABLED if JTAG authentication disable
 * 			bit is set in efuse.
 * 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
 * 			- XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY if PPK is not
 * 			programmed JTAG authentication fails.
 * 			- XLOADER_ERR_AUTH_JTAG_GET_DMA if failed to get DMA instance for
 * 			JTAG authentication.
 * 			- XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL if failed to verify PPK,
 * 			during JTAG authentication.
 * 			- XLOADER_ERR_AUTH_JTAG_SPK_REVOKED if revoke ID is programmed,
 * 			during JTAG authentication.
 * 			- XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL if failed to calculate
 * 			hash.
 * 			- XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL if failed to verify
 * 			signature.
 *
 ******************************************************************************/
static int XLoader_AuthJtag(u32 *TimeOut)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 AuthJtagDis = 0U;
	volatile u32 AuthJtagDisTmp = 0U;
	u32 RevokeId = 0U;
	XLoader_SecureParams SecureParams = {0U};
	XSecure_Sha3Hash Sha3Hash = {0U};
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u32 ReadAuthReg;
	volatile u8 UseDna;
	volatile u8 UseDnaTmp;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);

	/* To reduce stack usage, instance of XLoader_AuthJtagMessage is moved to
	 * a structure called XLoader_StoreSecureData which resides at
	 * XPLMI_PMC_CHUNK_MEMORY_1.
	 */
	SecureParams.AuthJtagMessagePtr = (XLoader_AuthJtagMessage *)
		(UINTPTR)XPLMI_PMCRAM_CHUNK_MEMORY_1;

	Status = XPlmi_MemSetBytes(SecureParams.AuthJtagMessagePtr,
		sizeof(XLoader_AuthJtagMessage), 0U, sizeof(XLoader_AuthJtagMessage));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Read 512 words of authenticated JTAG data from the PMC TAP registers and
	 * store it in a local buffer.
	 */
	Status = XPlmi_DmaXfr(XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET,
			(u64)(u32)SecureParams.AuthJtagMessagePtr,
			XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DMA_XFR, 0);
		goto END;
	}


	/** Check efuse bits for secure debug disable */
	AuthJtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	AuthJtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
					XLOADER_AUTH_JTAG_DIS_MASK;
	if ((AuthJtagDis == XLOADER_AUTH_JTAG_DIS_MASK) ||
		(AuthJtagDisTmp == XLOADER_AUTH_JTAG_DIS_MASK)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_DISABLED, 0);
		goto END;
	}

	/**
	 * Check Secure State of device
	 * If A-HWRoT is not enabled then return error
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		if (ReadAuthReg != SecureStateAHWRoT) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GLITCH_DETECTED, 0);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY, 0);
		}
		goto END;
	}

	SecureParams.PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecureParams.PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_GET_DMA, 0);
		goto END;
	}

	/** Verify PPK in the authenticated JTAG data. */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_PpkVerify, &SecureParams);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
			Status);
		goto END;
	}

	/** Verify if SPK Id is revoked or not. */
	RevokeId = SecureParams.AuthJtagMessagePtr->RevocationIdMsgType &
			XLOADER_AC_AH_REVOKE_ID_MASK;
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifyRevokeId,
		RevokeId);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_SPK_REVOKED,
			Status);
		goto END;
	}

	/** Calculate hash of the Authentication Header in the authenticated
	 * JTAG data.
	 */
	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecureParams.PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
			 Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
						 Status);
		goto END;
	}


	Status = XSecure_Sha3LastUpdate(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
			 Status);
		goto END;
	}


	Status = XSecure_Sha3Update(Sha3InstPtr,
		 (UINTPTR)&(SecureParams.AuthJtagMessagePtr->AuthHdr),
		 XLOADER_AUTH_JTAG_DATA_AH_LENGTH);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL, Status);
		goto END;
	}


	Status = XSecure_Sha3Finish(Sha3InstPtr, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	/** Verify signature of Auth Jtag data */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
		&SecureParams, Sha3Hash.Hash,
		&(SecureParams.AuthJtagMessagePtr->PpkData),
		(u8*)&(SecureParams.AuthJtagMessagePtr->EnableJtagSignature));
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL, Status);
	}
	else {
		UseDna = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
				XLOADER_AC_AH_DNA_MASK);
		UseDnaTmp = (u8)(SecureParams.AuthJtagMessagePtr->Attrb &
				XLOADER_AC_AH_DNA_MASK);
		if ((UseDna != FALSE) || (UseDnaTmp != FALSE)) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT,
				SecureParams.AuthJtagMessagePtr->Dna,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES,
				(void *)XLOADER_EFUSE_DNA_START_OFFSET,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES,
				XLOADER_EFUSE_DNA_LEN_IN_BYTES);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_AUTH_JTAG_INVALID_DNA, 0);
				goto END;
			}
		}
		XLoader_EnableJtag((u32)XLOADER_CONFIG_DAP_STATE_ALL_DBG);
		*TimeOut = SecureParams.AuthJtagMessagePtr->JtagEnableTimeout;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief       This function enables the Jtag
*
* @return
* 			- None
*
******************************************************************************/
void XLoader_EnableJtag(u32 CfgState)
{
	u32 DapCfgMask;

	if (CfgState == XLOADER_CONFIG_DAP_STATE_ALL_DBG) {
		/**
		 * Enable secure/non-secure debug
		 * Enabled invasive & non-invasive debug
		 */
		DapCfgMask = XLOADER_DAP_CFG_ENABLE_ALL_DBG_MASK;
	} else {
		/** Enable only non-secure debug */
		DapCfgMask = (XLOADER_DAP_CFG_NIDEN_MASK | XLOADER_DAP_CFG_DBGEN_MASK);
	}

	XPlmi_Out32(XLOADER_PMC_TAP_DAP_CFG_OFFSET, DapCfgMask);
	/**
	 * Enable all the instructions
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_0_OFFSET,
		XLOADER_PMC_TAP_INST_MASK_ENABLE_MASK);
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_1_OFFSET,
		XLOADER_PMC_TAP_INST_MASK_ENABLE_MASK);

	/**
	 * Disable security gate
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET,
		XLOADER_DAP_SECURITY_GATE_DISABLE_MASK);

	/**
	 * Take DBG module out of reset
	 */
	XPlmi_Out32(XLOADER_CRP_RST_DBG_OFFSET,
		XLOADER_CRP_RST_DBG_ENABLE_MASK);
}

/*****************************************************************************/
/**
* @brief       This function disables the Jtag
*
* @return
* 			- None
*
****************************************************************************/
void XLoader_DisableJtag(void)
{
	/**
	 * Reset DBG module
	 */
	XPlmi_Out32(XLOADER_CRP_RST_DBG_OFFSET,
			(XLOADER_CRP_RST_DBG_DPC_MASK |
			XLOADER_CRP_RST_DBG_RESET_MASK));

	/**
	 * Enable security gate
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET,
		~XLOADER_DAP_SECURITY_GATE_DISABLE_MASK);

	/**
	 * Disable all the instructions
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_0_OFFSET,
			XLOADER_PMC_TAP_INST_DISABLE_MASK_0);
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_1_OFFSET,
			XLOADER_PMC_TAP_INST_DISABLE_MASK_1);

	/**
	 * Disable secure/non-secure debug
	 * Disabled invasive & non-invasive debug
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_CFG_OFFSET,
				0x0U);
}

/*****************************************************************************/
/**
* @brief	This function is called to clear secure critical data related to
* 			authentication and encryption in case of exceptions. The function
* 			also places	AES, ECDSA_RSA in reset.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XST_FAILURE on failure.
*
******************************************************************************/
int XLoader_AuthEncClear(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();

	/*
	 * To ensure reliable initialization of AES state
	 * during exception hardcoded the base address and
	 * state instead of using XSecure_AesInitialize API
	 * as it requires DMA instance as an argument and
	 * there might be cases where DMA might
	 * not be initialized at the time of exception.
	 */
	AesInstPtr->BaseAddress = XSECURE_AES_BASEADDR;
	AesInstPtr->AesState = XSECURE_AES_INITIALIZED;

	/** - Clear keys. */
	Status = XSecure_AesKeyZero(AesInstPtr, XSECURE_AES_ALL_KEYS);

	/** - Place AES in reset and clear AES instance */
	Status |= Xil_SecureOut32(XLOADER_AES_RESET_REG,
				XLOADER_AES_RESET_VAL);
	Status |= Xil_SMemSet(AesInstPtr, sizeof(XSecure_Aes), 0U,
				sizeof(XSecure_Aes));
#if !defined(PLM_RSA_EXCLUDE) || !defined(PLM_ECDSA_EXCLUDE)

	/** - Release RSA from reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	/** - Clear RSA or ECDSA memory */
	SStatus = XSecure_RsaEcdsaZeroizeAndVerifyRam(XSECURE_ECDSA_RSA_BASEADDR);

	/** - Place RSA or ECDSA in reset */
	SStatus |= Xil_SecureOut32(XLOADER_ECDSA_RSA_RESET_REG,
				XLOADER_ECDSA_RSA_RESET_VAL);
#else
	SStatus = XST_SUCCESS;
#endif

	return (Status | SStatus);
}

/*****************************************************************************/
/**
 * @brief	This function updates the KAT status
 *
 * @param	PdiPtr is pointer to the XilPdi instance
 * @param	PlmKatMask is the mask of the KAT that is going to run
 *
 *****************************************************************************/
void XLoader_ClearKatOnPPDI(XilPdi *PdiPtr, u32 PlmKatMask)
{
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) {
		XLoader_ClearKatStatusOnCfg(PdiPtr, PlmKatMask);
	}
}

/*****************************************************************************/
/**
 * @brief	This function masks the KAT status based on KatOnConfig value
 *			when PDI type is partial pdi
 *
 * @param	PdiPtr is pointer to the XilPdi instance
 * @param	PlmKatMask is the mask of the KAT that is going to run
 *
 *****************************************************************************/
static void XLoader_ClearKatStatusOnCfg(XilPdi *PdiPtr, u32 PlmKatMask)
{
	u8 KatOnConfig = (u8)(XPlmi_In32(XPLMI_RTCFG_SECURE_CTRL_ADDR) &
						XLOADER_PPDI_KAT_MASK);

	if (KatOnConfig != 0U) {
		if (PlmKatMask != 0U) {
			PdiPtr->PpdiKatStatus &= PdiPtr->PlmKatStatus;
			if ((PdiPtr->PpdiKatStatus & PlmKatMask) != PlmKatMask) {
				/* Update RTC area before running KAT */
				PdiPtr->PlmKatStatus &= ~PlmKatMask;
				XPlmi_UpdateKatStatus(PdiPtr->PlmKatStatus);
				PdiPtr->PpdiKatStatus |= PlmKatMask;
			}
		}
		else {
			PdiPtr->PpdiKatStatus = 0U;
		}
	}
}

/*****************************************************************************/
/**
* @brief	This function performs authentication and decryption of the
* 			partition
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
* @param	DestAddr is the address to which data is copied
* @param	BlockSize is size of the data block to be processed
* 			which doesn't include padding lengths and hash
* @param	Last notifies if the block to be processed is last or not
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_DMA_TRANSFER if DMA transfer fails while copying.
* 			- XLOADER_ERR_PRTN_DECRYPT_FAIL if partition decryption fails.
* 			- XLOADER_SEC_BUF_CLEAR_SUCCESS if successfully cleared buffer.
* 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
*
******************************************************************************/
int XLoader_ProcessAuthEncPrtn(XLoader_SecureParams *SecurePtr, u64 DestAddr,
	u32 BlockSize, u8 Last)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	u32 TotalSize = BlockSize;
	u64 SrcAddr;
	u64 OutAddr;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	u64 ProcessTimeStart;
	u64 ProcessTimeEnd;
	static u64 ProcessTime;
	XPlmi_PerfTime PerfTime;
#endif

	XPlmi_Printf(DEBUG_INFO,
			"Processing Block %u\n\r", SecurePtr->BlockNum);
	SecurePtr->ProcessedLen = 0U;
	/** - Process the 1st block */
	if (SecurePtr->BlockNum == 0x0U) {
		SrcAddr = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
				((u64)(SecurePtr->PrtnHdr->DataWordOfst) << XPLMI_WORD_LEN_SHIFT);
	}
	else {
		SrcAddr = SecurePtr->NextBlkAddr;
	}

	if ((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) {

		if (SecurePtr->BlockNum == 0x0U) {
			SecurePtr->RemainingEncLen =
				SecurePtr->PrtnHdr->EncDataWordLen << XPLMI_WORD_LEN_SHIFT;
			/** - Verify encrypted partition is revoked or not */
			XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifyRevokeId,
					SecurePtr->PrtnHdr->EncRevokeID);
			if ((Status != XST_SUCCESS) ||
				(SStatus != XST_SUCCESS)) {
				XPlmi_Printf(DEBUG_GENERAL, "Partition is revoked\n\r");
				goto END;
			}
		}

		if (Last == (u8)TRUE) {
			TotalSize = SecurePtr->RemainingEncLen;
		}
		else {
			if (SecurePtr->BlockNum == 0U)  {
				/* To include Secure Header */
				TotalSize = TotalSize + XLOADER_SECURE_HDR_TOTAL_SIZE;
			}
		}
	}

	Status = XLoader_SecureChunkCopy(SecurePtr, SrcAddr, Last,
				BlockSize, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef PLM_PRINT_PERF_CDO_PROCESS
	ProcessTimeStart = XPlmi_GetTimerValue();
#endif

	if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		/** - Verify hash */
		XSECURE_TEMPORAL_CHECK(END, Status,
					XLoader_VerifyAuthHashNUpdateNext,
					SecurePtr, TotalSize, Last);

		if (((SecurePtr->IsEncrypted != (u8)TRUE) &&
			(SecureTempParams->IsEncrypted != (u8)TRUE)) &&
			(SecurePtr->IsCdo != (u8)TRUE)) {
			/* Copy to destination address */
			Status = XPlmi_DmaXfr((u64)SecurePtr->SecureData,
					(u64)DestAddr,
					SecurePtr->SecureDataLen >> XPLMI_WORD_LEN_SHIFT,
					XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_DMA_TRANSFER, Status);
				goto END;
			}
		}
	}

	/** - If encryption is enabled, decrypt the data. */
	if ((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) {
		if ((SecurePtr->IsAuthenticated != (u8)TRUE) ||
			(SecureTempParams->IsAuthenticated != (u8)TRUE)) {
			SecurePtr->SecureData = SecurePtr->ChunkAddr;
			SecurePtr->SecureDataLen = TotalSize;
		}

		if (SecurePtr->IsCdo != (u8)TRUE) {
			OutAddr = DestAddr;
		}
		else {
			OutAddr = SecurePtr->SecureData;
		}
		Status = XLoader_AesDecryption(SecurePtr,
					SecurePtr->SecureData,
					OutAddr,
					SecurePtr->SecureDataLen);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_PRTN_DECRYPT_FAIL, Status);
			goto END;
		}
	}

	XPlmi_Printf(DEBUG_INFO, "Authentication/Decryption of Block %u is "
			"successful\r\n", SecurePtr->BlockNum);

	SecurePtr->NextBlkAddr = SrcAddr + TotalSize;
	SecurePtr->ProcessedLen = TotalSize;
	SecurePtr->BlockNum++;

END:
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	ProcessTimeEnd = XPlmi_GetTimerValue();
	ProcessTime += (ProcessTimeStart - ProcessTimeEnd);
	if (Last == (u8)TRUE) {
		XPlmi_MeasurePerfTime((XPlmi_GetTimerValue() + ProcessTime),
					&PerfTime);
		XPlmi_Printf(DEBUG_PRINT_PERF,
			     "%u.%03u ms Secure Processing time\n\r",
			     (u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
		ProcessTime = 0U;
	}
#endif
	/* Clears whole intermediate buffers on failure */
	if (Status != XST_SUCCESS) {
		ClrStatus = XPlmi_InitNVerifyMem(SecurePtr->ChunkAddr, TotalSize);
		if (ClrStatus != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
		}
		else {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and compares with expected hash.
 * 			Hash is calculated on AC + Data for first block, encrypts the
 * 			ECDSA/RSA signature and compares it with the expected hash.
 * 			After first block, hash is calculated on block of data and
 * 			compared with the expected hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance
 * @param	Size is size of the data block to be processed
 *			which includes padding lengths and hash
 * @param	Last notifies if the block to be processed is last or not
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PRTN_HASH_CALC_FAIL if hash calculation fails for
 * 			partition authentication.
 * 			- XLOADER_ERR_PRTN_AUTH_FAIL if partition authentication fails.
 * 			- XLOADER_ERR_PRTN_HASH_COMPARE_FAIL if failed to compare partition
 * 			hash.
 * 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
 *
 ******************************************************************************/
static int XLoader_VerifyAuthHashNUpdateNext(XLoader_SecureParams *SecurePtr,
	u32 Size, u8 Last)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u8 *Data = (u8 *)SecurePtr->ChunkAddr;
	XSecure_Sha3Hash BlkHash;
	u8 *ExpHash = (u8 *)SecurePtr->Sha3Hash;
	volatile int StatusTmp = XST_FAILURE;
	XLoader_AuthCertificate *AcPtr=
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
	XilPdi_PrtnHashInfo* PrtnHashData = NULL;
	XilPdi_PrtnHashInfo* PrtnHashDataTmp = NULL;

	if (SecurePtr->PmcDmaInstPtr == NULL) {
		goto END;
	}

	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
				Status);
		goto END;
	}

	Status = XLoader_Sha3Kat(SecurePtr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "SHA3 KAT failed\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
			Status);
		goto END;
	}

	/** Hash should be calculated on AC + first chunk */
	if (SecurePtr->BlockNum == 0x00U) {
		Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)AcPtr,
			XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
			goto END;
		}
	}

	Status = XSecure_Sha3Update(Sha3InstPtr, (UINTPTR)Data, Size);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &BlkHash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	/** Verify the hash */
	if (SecurePtr->BlockNum == 0x00U) {
		PrtnHashData = XilPdi_IsPrtnHashPresent(SecurePtr->PdiPtr->PrtnNum,
			SecurePtr->PdiPtr->MetaHdr.DigestTableSize);
		PrtnHashDataTmp = XilPdi_IsPrtnHashPresent(SecurePtr->PdiPtr->PrtnNum,
			SecurePtr->PdiPtr->MetaHdr.DigestTableSize);
		if((PrtnHashData == NULL) || (PrtnHashDataTmp == NULL)){
			 /** Authenticate the signature, if same keys are not used. */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DataAuth, SecurePtr,
				BlkHash.Hash, (u8 *)SecurePtr->AcPtr->ImgSignature);
		}
		else{
			/**
			 * Skip sign verification, compare the hash of respective partition Id
			 * which uses same keys.
			 */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, PrtnHashData->PrtnHash,
			XLOADER_SHA3_LEN, BlkHash.Hash, XLOADER_SHA3_LEN,
			XLOADER_SHA3_LEN);
		}
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_AUTH_FAIL, Status);
			goto END;
		}
	}
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, ExpHash,
			XLOADER_SHA3_LEN, BlkHash.Hash, XLOADER_SHA3_LEN,
			XLOADER_SHA3_LEN);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_INFO, "Hash mismatch error\n\r");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)BlkHash.Hash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "Calculated Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)ExpHash,
				XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "Expected Hash");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
				Status);
			goto END;
		}
	}

	/** Update the next expected hash and data location */
	if (Last != (u8)TRUE) {
		Status = Xil_SMemCpy(ExpHash, XLOADER_SHA3_LEN,
				&Data[Size - XLOADER_SHA3_LEN],
				XLOADER_SHA3_LEN, XLOADER_SHA3_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/**
		 * Remove the hash length at end of the chunk
		 */
		SecurePtr->SecureDataLen = Size - XLOADER_SHA3_LEN;
	}
	else {
		/* This is the last block */
		SecurePtr->SecureDataLen = Size;
	}
	SecurePtr->SecureData = (UINTPTR)Data;

END:
	ClearStatus = XPlmi_MemSetBytes(&BlkHash, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief    This function checks if the secure state of boot matches the
*           expected value or not.
*
* @param    RegVal - Value of secure state stored in register
* @param    Var - Value of secure state stored in variable
* @param    ExpectedValue - Expected value of secure state
*
* @return   XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_CheckSecureState(u32 RegVal, u32 Var, u32 ExpectedValue)
{
	int Status = XST_FAILURE;

	if ((RegVal == Var) && (RegVal == ExpectedValue)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function runs RSA or Ecdsa KAT based on pdi
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_INVALID_AUTH on invalid authentication type.
* 			- XLOADER_ERR_RSA_NOT_ENABLED if RSA is not enabled.
* 			- XLOADER_ERR_ECDSA_NOT_ENABLED if ECDSA is not enabled.
*
******************************************************************************/
static int XLoader_AuthKat(XLoader_SecureParams *SecurePtr) {
	u32 AuthType;
	u32 AuthKatMask;
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
#ifndef PLM_ECDSA_EXCLUDE
	XSecure_EllipticCrvClass CrvClass = XSECURE_ECC_PRIME;
#endif
	/** Get the Authentication type. */
	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
		AuthKatMask = XPLMI_SECURE_RSA_KAT_MASK;
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
		AuthKatMask = XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK;
#ifndef PLM_ECDSA_EXCLUDE
		CrvClass = XSECURE_ECC_PRIME;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
		AuthKatMask = XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK;
#ifndef PLM_ECDSA_EXCLUDE
		CrvClass = XSECURE_ECC_PRIME;
#endif
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	/** Update KAT status based on the user configuration. */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, AuthKatMask);

	/**
	 * Skip running the KAT for ECDSA or RSA if it is already run.
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set.
	 * If KAT fails device will go into a secure lockdown state.
	 */
	if ((SecurePtr->PdiPtr->PlmKatStatus & AuthKatMask) == 0U) {
		if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
#ifndef PLM_RSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XLoader_RsaKat, SecurePtr->PmcDmaInstPtr);
#else
			XPlmi_Printf(DEBUG_GENERAL, "RSA code is excluded\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_RSA_NOT_ENABLED, 0U);
			goto END;
#endif

		}
		else if ((AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
			(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
#ifndef PLM_ECDSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XSecure_EllipticVerifySignKat, CrvClass);
#else
                        XPlmi_Printf(DEBUG_GENERAL, "ECDSA code is excluded\n\r");
                        Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, 0U);
                        goto END;
#endif
		}
		else {
			/* Not supported */
			XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
			goto END;
		}
		if (Status != XST_SUCCESS) {
			if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
				XPlmi_Printf(DEBUG_GENERAL, "RSA KAT Failed\n\r");
			}
			else {
				XPlmi_Printf(DEBUG_GENERAL, "ECDSA KAT Failed\n\r");
			}
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_KAT_FAILED_ERROR, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= AuthKatMask;

		/* Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
#if (defined(PLM_RSA_EXCLUDE) && defined(PLM_ECDSA_EXCLUDE))
	(void)StatusTmp;
#endif
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function runs SHA3 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_Sha3Kat(XLoader_SecureParams *SecurePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();

	/* Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SECURE_SHA3_KAT_MASK);

	/* Set the data context of previous SHA operation */
	XSecure_Sha3SetDataContext(Sha3InstPtr);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SECURE_SHA3_KAT_MASK) == 0U) {
		/*
		 * Skip running the KAT for SHA3 if it is already run
		 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_Sha3Kat, Sha3InstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "SHA3 KAT failed\n\r");
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_KAT_FAILED_ERROR, Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SECURE_SHA3_KAT_MASK;

		/* Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

#endif /* END OF PLM_SECURE_EXCLUDE */
