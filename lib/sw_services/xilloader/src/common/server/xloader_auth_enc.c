/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
*                     does not match with plm secure state
*       kpt  08/20/23 Updated check to place ECDSA in reset and clear RAM memory when
*			PLM_ECDSA_EXCLUDE is not defined
*       yog  08/25/23 Removed check to return error code when MH secure state doesn't
*			match with plm secure
*       dd   09/11/23 MISRA-C violation Rule 10.3 fixed
* 2.0   kpt  07/31/23 Run KAT every time when AUTH JTAG request is made
*       kpt  10/09/23 Fixed compilation warning when PLM_EN_ADD_PPKS macro is enabled
* 2.1   sk   10/24/23 Added Redundancy in XLoader_EnableJtag
*       sk   11/02/23 Updated Redundancy in XLoader_EnableJtag
*       kpt  11/22/23 Add support to clear AES keys when RedKeyClear bit is set
*       ng   12/27/23 Reduced log level for less frequent prints
*       ng   01/28/24 u8 variables optimization
*       kpt  02/08/24 Added support to extend secure state to SWPCR during AuthJtag
*       yog  02/23/24 Added support to return error when P-521 curve is disabled.
*       am   03/02/24 Added MH Optimization support
*       kpt  03/15/24 Updated RSA KAT to use 2048-bit key
*       sk   03/13/24 Fixed doxygen comments format
*       har  04/12/24 Moved glitch checks after respective function calls
*       kal  06/04/24 Added XLoader_SecureConfigMeasurement call in
*                     XLoader_ProcessAuthEncPrtn after Block 0 processing is success
*       mb   06/30/24 Fixed AES Decryption issue when KAT is enabled
*       kal  07/24/24 Code refactoring and updates for Versal 2VE and 2VM Devices
*       kal  09/18/24 Updated XLoader_PpkVerify to verify 384 bit ppk hash
*                     for Versal 2VE and 2VM Devices
*       pre  12/09/24 use PMC RAM for Metaheader instead of PPU1 RAM
*       kal  01/30/25 Send LMS and HSS data to signature verification
*                     without pre-hasing
* 2.2   sk   02/04/25 Reset Status before each function call in
*                     XLoader_AuthHdrsWithHashBlock
*       sk   02/04/25 Reset HashStatus before function call in
*                     XLoader_IsPpkValid
*       sk   02/26/25 Reset Status variable before use in XLoader_SecureEncInit
*       pre  03/02/25 Remove data context setting
*       obs  03/22/25 Added redundant security checks to mitigate glitch attacks
*       har  04/07/25 Updated instruction mask in XLoader_EnableJtag
*       pre  05/09/25 Updation is done to do hashBlock1 integrity validation for boot PDI only
*       vss  07/08/25 Initialised DMA instance in read and verify secure headers.
*       vss  07/22/2025 Added Hashblock hash verification check to update major errorcode.
*       vss  08/13/2025 Removed code which masks major errorcodes.
* 2.3   tvp  08/19/2025 Added support to store partition Block hash in PtrnHashTable.
*       pre  08/23/2025 Did versal macro change
*       pre  09/09/2025 Returned error at necessary places and did status reset before reuse
*       vss  09/17/2025 Updated major error codes wherever missed in XLoader_DecHdrs function.
*       tvp  09/17/2025 Store Block 0 partition hash to calculate subsystem image hash for versal_2vp
*       pre  09/30/2025 Updated comments for rtf docs
*       sd   11/10/2025 Added support for VERSAL_2VP_P devices.
* 2.4   abh  11/12/2025 Fixed MISRA-C violations
*       tvp  01/23/2026 Make XLoader_Sha3Kat non-static, as it is required from other file
*                       Remove KAT execution from distributed place, as it
*                       will be called during boot from Pdi_Init.
*       rmv  01/30/2026 Renamed OCP keymanagment macro
*       vss  02/01/2026 Updated PPK revoke error logic.
*       tvp  03/05/2026 Use XLoader_AuthKey to accommodate new algorithms support
*       tvp  03/05/2026 Add authenticated boot support for Versal_2vp_p
*       tvp  03/05/2026 Add support for efuse PPK3-PPK8 hash for Versal_2vp_p
*       sri  03/26/2026 Added new IPI API for verifying Hash block requested by Versal 2Ve 2Vm client
*       vss  03/30/26 Fix for get partition hash index calculation for IPU/Full metaheader.
*       tvp  03/31/2026 Use generic API to increment IV
*       rpu  04/14/2026 Moved Hashblock related code to xloader_secure.c
*       sd   04/13/2026 Added PUF support for VERSAL_2VP_P
*       sri  04/24/2026 Changed LMS KAT and SignVerify APIs from static to global scope to support authenticated JTAG functionality
*
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xloader_auth_enc.h"
#ifndef PLM_SECURE_EXCLUDE
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
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
#include "xsecure_lms_core.h"
#include "xsecure_plat_kat.h"
#endif
#ifdef VERSAL_2VP_P
#include "xsecure_mldsa.h"
#include "xsecure_slhdsa.h"
#endif
#ifdef VERSAL_NET
#ifdef PLM_OCP_NATIVE_KEY_MGMT
#include "xcert_genx509cert.h"
#endif
#endif
#include "xsecure_resourcehandling.h"
#include "xloader_kat.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions *******************************/

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


#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
#define XLOADER_HASH_BLOCK_IV_INCREMENT_VAL             (0x01U)
#define XLOADER_SECURE_IV_LEN_IN_BYTES                  (12U)
#define XLOADER_ENTRY_SIZE_IN_HASHBLOCK                 (52U) /**< Each entry size in HashBlock */
#define XLOADER_MIN_SIGN_SIZE                           (96U) /**< Minimum Signature size excluding padding, actual size */

#ifndef VERSAL_2VP_P
#define XLOADER_MAX_TOTAL_SPK_SIZE                      (1040U)         /**< Maximum SPK size including padding */
#define XLOADER_MIN_TOTAL_SPK_SIZE                      (64U)           /**< Minimum SPK size including padding */

#define XLOADER_MAX_SPK_SIZE                            (1028U)         /** Maximum SPK size excluding padding, actual size */
#define XLOADER_MIN_SPK_SIZE                            (56U)           /** Maximum SPK size excluding padding, actual size */

#define XLOADER_MAX_TOTAL_SIGN_SIZE                     (9952U)         /** Maximum Signature size including padding */
#define XLOADER_MIN_TOTAL_SIGN_SIZE                     (96U)           /** Minimum Signature size including padding */

#define XLOADER_MAX_SIGN_SIZE                           (9940U)         /** Maximum Signature size excluding padding, actual size */

/* Hash Block 0 Partition Indices */
#define XLOADER_HB0_INDEX_4                             (4U) /**< HB0 4th is filled by all 0s now */
#define XLOADER_HB0_INDEX_5                             (5U) /**< HB0 5th is filled by all 0s now */
#else
#define XLOADER_VERIFY_SPK_SIGN				(0U)	/**< Verify SPK signature with PPK */
#define XLOADER_VERIFY_HB_SIGN				(1U)	/**< Verify HashBlock signature with SPK */
#endif
#else

#define XLOADER_GET_PRTN_HASH_INDEX(PdiPtr) ((PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) || \
						 (PdiPtr->PdiType == XLOADER_PDI_TYPE_IPU) || \
						(PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL_METAHEADER)) \
						? (PdiPtr->PrtnNum) : (PdiPtr->PrtnNum +1U)
		/**< Get partition hash index depending on full/IPU/partial/full metaheader PDI */
#endif

/************************** Function Prototypes ******************************/

#ifndef PLM_ECDSA_EXCLUDE
static int XLoader_EcdsaSignVerify(const XSecure_EllipticCrvTyp CrvType, const u8 *DataHash,
	const u8 *Key, const u32 KeySize, const u8 *Signature);
#endif
#ifndef PLM_RSA_EXCLUDE
static int XLoader_RsaSignVerify(u8 *MsgHash, XLoader_RsaKey *Key, u8 *Signature);
#endif
static int XLoader_AesDecryption(XLoader_SecureParams *SecurePtr,
	u64 SrcAddr, u64 DestAddr, u32 Size);
static int XLoader_AesKeySelect(const XLoader_SecureParams *SecurePtr,
	XLoader_AesKekInfo *KeyDetails, XSecure_AesKeySrc *KeySrc);
static int XLoader_PpkCompare(const u32 EfusePpkOffset, const u8 *PpkHash);
static int XLoader_DecHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
static int XLoader_SpkAuthentication(const XLoader_SecureParams *SecurePtr);
static int XLoader_AuthNDecHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr, u64 BufferAddr);
static int XLoader_AuthHdrs(const XLoader_SecureParams *SecurePtr, XilPdi_MetaHdr *MetaHdr);
static int XLoader_CheckAndCompareHashFromIHTOptionalData(XilPdi *PdiPtr, u8 *HashPtr, u32 PrtnHashIndex);
#endif
static int XLoader_VerifyAuthHashNUpdateNext(XLoader_SecureParams *SecurePtr,
	u32 Size, u8 Last);
static int XLoader_SetAesDpaCm(const XSecure_Aes *AesInstPtr, u8 DpaCmCfg);
static int XLoader_DecryptBlkKey(const XSecure_Aes *AesInstPtr,
	const XLoader_AesKekInfo *KeyDetails);
static int XLoader_SecureEncOnlyValidations(const XLoader_SecureParams *SecurePtr);
static int XLoader_ValidateIV(const u32 *IHPtr, const u32 *EfusePtr);
static int XLoader_ClearAesKeysOnCfg(void);

#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
static int XLoader_AuthenticateKeys(XLoader_SecureParams *SecurePtr, XLoader_HBSignParams *HBSignParams);
static int XLoader_AuthenticateHashBlock(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams);
static int XLoader_AuthHashBlockNDecHdrs(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams, u64 BufferAddr);
static int XLoader_AuthHdrsWithHashBlock(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams);
static int XLoader_ValidateHashBlockAAD(XLoader_SecureParams *SecurePtr,
                                XLoader_HBAesParams *HBParams);
#ifdef VERSAL_2VP_P
static int XLoader_ValidateHybridKeyAlgo(u32 Algo1, u32 Algo2);
static int XLoader_AuthHashBlockWithKeyPair(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 *ReadOffset,
		u32 AuthType, u32 SignPaddingSize, u8 *HashBlockHash);
static int XLoader_GetKeySizeSignPadding(u32 AuthType,
		XLoader_HBSignParams *HBSignParams, u32 *SignPaddingSize);
static int XLoader_SlhdsaSignVerify(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 VerifyType);
static int XLoader_MldsaSignVerify(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 VerifyType);
#else
static int XLoader_ValidateSpkHeader(XLoader_SpkHeader *SpkHeader);
#endif
#endif

/************************** Function Definitions *****************************/

/************************** Variable Definitions *****************************/

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
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
	u64 AcOffset;
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
		XPLMI_PMCRAM_CHUNK_MEMORY_1;
#else
	XLoader_HBSignParams HBSignParams;
#endif

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

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
		AcOffset = SecurePtr->PdiPtr->MetaHdr->FlashOfstAddr +
			((u64)SecurePtr->PrtnHdr->AuthCertificateOfst *
				XIH_PRTN_WORD_LEN);
		SecurePtr->AcPtr = AuthCert;

		/* Copy Authentication certificate */
		Status = SecurePtr->PdiPtr->MetaHdr->DeviceCopy(AcOffset,
			(UINTPTR)SecurePtr->AcPtr,
				XLOADER_AUTH_CERT_MIN_SIZE, SecurePtr->DmaFlags);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_INIT_AC_COPY_FAIL, Status);
			goto END;
		}

		SecurePtr->SecureHdrLen += XLOADER_AUTH_CERT_MIN_SIZE;
		SecurePtr->ProcessedLen = XLOADER_AUTH_CERT_MIN_SIZE;

#else
#ifdef VERSAL_2VE_2VM
		HBSignParams.ReadOffset = PrtnHdr->AuthCertificateOfst * XIH_PRTN_WORD_LEN;
		HBSignParams.TotalPpkSize = PrtnHdr->TotalPpkSize;
		HBSignParams.ActualPpkSize = PrtnHdr->ActualPpkSize;
		HBSignParams.TotalHBSignSize = PrtnHdr->TotalHBSignSize;
		HBSignParams.ActualHBSignSize = PrtnHdr->ActualHBSignSize;
		HBSignParams.HBSize = PrtnHdr->HashBlockSize * XIH_PRTN_WORD_LEN;
		HBSignParams.AuthHdr = PrtnHdr->AuthenticationHdr;
#else
		HBSignParams.ReadOffset = PrtnHdr->HashBlockOffset * XIH_PRTN_WORD_LEN;
		HBSignParams.HBSize = PrtnHdr->HashBlockSize * XIH_PRTN_WORD_LEN;
#endif
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_AuthenticateHashBlock,
				SecurePtr, &HBSignParams);
#endif

		SecurePtr->ProcessPrtn = XLoader_ProcessAuthEncPrtn;
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
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	XLoader_HashBlock *HBPtr = XLoader_GetHashBlockInstance();
#endif
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
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
	if ((SecurePtr->IsCheckSumEnabled == (u8)TRUE) &&
		((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		 (SecureTempParams->IsAuthenticated== (u8)TRUE) ||
		 (SecurePtr->IsEncrypted == (u8)TRUE) ||
		 (SecureTempParams->IsEncrypted == (u8)TRUE))) {
		XPlmi_Printf(DEBUG_INFO, "Error: Checksum should not be enabled with "
				"authentication or encryption\n\r");
		Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC, XIL_SIGNED_ZERO);
		goto END;
	}

#endif
	SecurePtr->AesInstPtr = XSecure_GetAesInstance();
	/**
	 * - Run AES Kat test if the image is encrypted
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
					XLOADER_ERR_PRTN_DECRYPT_NOT_ALLOWED, XIL_SIGNED_ZERO);
				goto END;
			}
			if (ReadReg != SecureStateAHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
				goto END;
			}

		}
		else {
			if (ReadReg != SecureStateSHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
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
					XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
				goto END;
			}
		}
		else {
			if ((SecurePtr->PrtnHdr->EncStatus == XLOADER_EFUSE_KEY) ||
				(SecurePtr->PrtnHdr->EncStatus == XLOADER_BBRAM_KEY)) {
				XPlmi_Printf(DEBUG_INFO, "Error: Invalid key source for "
						"decrypt only case\n\r");
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_PRTN_ENC_ONLY_KEYSRC, XIL_SIGNED_ZERO);
				goto END;
			}
		}
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)

		if (SecurePtr->PdiPtr->PdiType != XLOADER_PDI_TYPE_PARTIAL) {
			Status = XST_FAILURE;
			Status = Xil_SMemCpy(SecurePtr->Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
					HBPtr->HashData[SecurePtr->PdiPtr->PrtnNum].PrtnHash,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		} else {
			/* For a partial PDI in the absence of PLM, the partition number
			 * starts with 0, but in HashBlock at index 0 MetaHeader
			 * hash is present, partition hashes start from index 1
			 * Hence it is always PrtnNum + 1 indicates the corresponding
			 * partition hash in HashBlock
			 */
			Status = XST_FAILURE;
			Status = Xil_SMemCpy(SecurePtr->Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
					HBPtr->HashData[SecurePtr->PdiPtr->PrtnNum + XLOADER_HB_PPDI_PRTN_HASH_IDX_OFFSET].PrtnHash,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
#endif
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
	u32 MetaHeaderKeySrc = SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.EncKeySrc;
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
				XPLMI_STATUS_GLITCH_DETECT(Status);
				if (ReadAuthReg != SecureStateAHWRoT) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
				}
				goto END;
			}

			if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
				(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO, XIL_SIGNED_ZERO);
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
						XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY, XIL_SIGNED_ZERO);
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
					XPLMI_STATUS_GLITCH_DETECT(Status);
					if (ReadEncReg != SecureStateSHWRoT) {
						Status = XPlmi_UpdateStatus(
							XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
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
						XLOADER_ERR_ENCONLY_ENC_COMPULSORY, XIL_SIGNED_ZERO);
			goto END;
		}
		XPlmi_Printf(DEBUG_INFO, "Encryption is enabled\n\r");
		/* Enc only validations */
		Status = XLoader_SecureEncOnlyValidations(SecurePtr);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
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
				XLOADER_ERR_METAHDR_KEYSRC_MISMATCH, XIL_SIGNED_ZERO);
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
	IsEncKeySrc = SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.EncKeySrc;
	IsEncKeySrcTmp = SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.EncKeySrc;
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
		XilPdi_GetPufHdMetaHdr(&(SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl))
		>> XIH_PH_ATTRB_PUFHD_SHIFT;
	PufHdLocationTmp =
		XilPdi_GetPufHdMetaHdr(&(SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl))
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
		SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.IvMetaHdr,
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
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	XLoader_HBSignParams HBSignParams;
#endif
	XPlmi_Printf(DEBUG_INFO,
		"Loading secure image headers and partition headers\n\r");

	/* Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_GET_DMA, XIL_SIGNED_ZERO);
		goto ERR_END;
	}

#ifdef VERSAL_2VE_2VM
	HBSignParams.ReadOffset = MetaHdr->ImgHdrTbl.AcOffset * XIH_PRTN_WORD_LEN;
	HBSignParams.TotalPpkSize = MetaHdr->ImgHdrTbl.TotalPpkSize;
	HBSignParams.ActualPpkSize = MetaHdr->ImgHdrTbl.ActualPpkSize;
	HBSignParams.TotalHBSignSize = MetaHdr->ImgHdrTbl.TotalHBSignSize;
	HBSignParams.ActualHBSignSize = MetaHdr->ImgHdrTbl.ActualHBSignSize;
	HBSignParams.HBSize = MetaHdr->ImgHdrTbl.HashBlockSize * XIH_PRTN_WORD_LEN;
	HBSignParams.AuthHdr = MetaHdr->ImgHdrTbl.AuthenticationHdr;
#endif
#ifdef VERSAL_2VP_P
	HBSignParams.ReadOffset = MetaHdr->ImgHdrTbl.HashBlockOffset * XIH_PRTN_WORD_LEN;
	HBSignParams.HBSize = MetaHdr->ImgHdrTbl.HashBlockSize * XIH_PRTN_WORD_LEN;
#endif
	/*
	 * If headers are in encrypted format
	 * either authentication is enabled or not
	 */
	if ((SecurePtr->IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE)) {
		SecurePtr->AesInstPtr = XSecure_GetAesInstance();

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
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
			TotalSize -= XLOADER_AUTH_CERT_MIN_SIZE;
#endif
		}
		TotalSizeTmp = TotalSize;
		/** - Validate Meta header length */
		if ((TotalSize > XLOADER_SECURE_CHUNK_SIZE) ||
			(TotalSizeTmp > XLOADER_SECURE_CHUNK_SIZE)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_METAHDR_LEN_OVERFLOW, XIL_SIGNED_ZERO);
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
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthNDecHdrs,
				SecurePtr, MetaHdr, SecurePtr->ChunkAddr);
#else
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthHashBlockNDecHdrs,
                                SecurePtr, &HBSignParams, SecurePtr->ChunkAddr);
#endif
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
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_COPY_FAIL, Status);
			goto ERR_END;
		}
	}
	/* If only authentication is enabled */
	else if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
			(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		XPlmi_Printf(DEBUG_INFO, "Headers are only authenticated\n\r");
		/** - Authenticate Image headers and partition headers */
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthHdrs, SecurePtr, MetaHdr);
#else
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_AuthHdrsWithHashBlock, SecurePtr, &HBSignParams);
#endif
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			goto ERR_END;
		}
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Headers are not secure\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_NOT_SECURE, XIL_SIGNED_ZERO);
		goto END;
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
* @brief	This function verifies the signature (RSA/ECDSA) using
* 			the provided key and compares the result with the expected hash.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	Hash is pointer to the expected hash to be verified
* @param	AuthKey is pointer to the RSA/ECDSA public key to be used for
* 		verification
* @param	Signature is pointer to the signature to be verified
*
* @return
* 		- XST_SUCCESS on successful signature verification.
* 		- XLOADER_SEC_INVALID_AUTH on invalid authentication type.
* 		- XLOADER_ERR_RSA_NOT_ENABLED if RSA is excluded from build.
* 		- XLOADER_ERR_ECDSA_NOT_ENABLED if ECDSA is excluded from build.
* 		- XLOADER_SEC_CURVE_NOT_SUPPORTED if P-521 curve is not supported.
*
******************************************************************************/
int XLoader_VerifySignature(const XLoader_SecureParams *SecurePtr,
		u8 *Hash, XLoader_AuthKey *AuthKey, u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	u32 AuthType;

#ifndef VERSAL_2VP_P
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		AuthType = XLoader_GetAuthPubAlgo(&(SecurePtr->AuthJtagMessagePtr->AuthHdr));
	}
	else {
		AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	}
#else
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		XPlmi_Printf(DEBUG_INFO, "Authenticated JTAG not supported\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_NOT_SUPPORTED, XIL_SIGNED_ZERO);
		goto END;
	} else {
		AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);
	}
#endif

	/* RSA authentication */
	if (AuthType ==	XLOADER_PUB_STRENGTH_RSA_4096) {
#ifndef PLM_RSA_EXCLUDE
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_RsaSignVerify, Hash,
				       (XLoader_RsaKey *)AuthKey->Key, Signature);
#else

		XPlmi_Printf(DEBUG_INFO, "RSA code is excluded\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_RSA_NOT_ENABLED, XIL_SIGNED_ZERO);
		goto END;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
#ifndef PLM_ECDSA_EXCLUDE
		/* ECDSA P384 authentication */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_EcdsaSignVerify,
			XSECURE_ECC_NIST_P384, Hash, (u8 *)AuthKey->Key,
			XLOADER_ECDSA_P384_KEYSIZE, Signature);
#else
		XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
		goto END;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
#ifndef PLM_ECDSA_EXCLUDE
#ifdef XSECURE_ECC_SUPPORT_NIST_P521
		/* ECDSA P521 authentication */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_EcdsaSignVerify,
			XSECURE_ECC_NIST_P521, Hash, (u8 *)AuthKey->Key,
			XLOADER_ECDSA_P521_KEYSIZE, Signature);
#else
		XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_CURVE_NOT_SUPPORTED, 0U);
		goto END;
#endif
#else
		XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
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
	(void)AuthKey;
	(void)Signature;
#endif

END:
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
int XLoader_VerifyRevokeId(u32 RevokeId)
{
	int Status = XST_FAILURE;
	volatile u32 Quo;
	volatile u32 QuoTmp;
	volatile u32 Mod;
	volatile u32 ModTmp;
	volatile u32 Value;
	volatile u32 ValueTmp;

	XPlmi_Printf(DEBUG_INFO, "Validating Revocation ID\n\r");
	/** - Verify if provided revocation ID is in range of 0 to 255 */
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
* @brief	This function compares calculated PPK hash with the efuse PPK
* 		hash.
*
* @param	EfusePpkOffset is PPK hash address of efuse.
* @param	PpkHash is pointer to the PPK hash to be verified.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if hash comparison failed.
*
******************************************************************************/
static int XLoader_PpkCompare(const u32 EfusePpkOffset, const u8 *PpkHash)
{
	int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;

	XSECURE_TEMPORAL_IMPL(HashStatus,HashStatusTmp,Xil_SMemCmp_CT,PpkHash,
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
* @brief	This function validates PPK hash against eFUSE stored hash.
*		It iterates through all PPKs and checks if the provided hash
*		matches any valid (non-revoked) PPK.
*
* @param	PpkHash  Pointer to the PPK hash to be verified.
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_SEC_INVALID_PPK_CHOICE_ERR if invalid PPK selection.
* 		- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if PPK hash comparison failed.
* 		- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
* 		- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
* 		- XLOADER_SEC_SEL_PPK_REVOKED_ERR if selected PPK is revoked.
*
******************************************************************************/
int XLoader_IsPpkValid(const u8 *PpkHash)
{
	volatile int Status = XST_FAILURE;
	u32 PpkOffset = 0U;
	volatile u32 PpkIndex;
	u32 InvalidMask = 0U;

	/** - Validate PPK hash for all PPKs in a loop */
	for (PpkIndex = XLOADER_PPK_SEL_0; PpkIndex < XLOADER_PPK_SEL_MAX; PpkIndex++) {

		/** - Get invalid mask and PPK offset */
		Status = XLoader_GetPpkInvalidMaskAndOffset((XLoader_PpkSel)PpkIndex, &InvalidMask, &PpkOffset);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}

		Status = XST_FAILURE;

		/** - Compare PPK hash with efuse stored PPK hash */
		Status = XLoader_ValidatePpkHash(PpkHash, PpkOffset);
		if (Status == XST_SUCCESS) {
			break;
		}
	}

	if (Status != XST_SUCCESS) {
		if (PpkIndex != XLOADER_PPK_SEL_MAX) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Error: PPK validation failed for all PPKs\r\n");
			goto END;
		}
	}
	else {
		/** - Check if corresponding PPK is revoked*/
		Status = XLoader_IsPpkRevoked(InvalidMask);
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
* @param	PpkSize	is Size of the PPK to be verified
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
* 			- XLOADER_SEC_PPK_HASH_CALCULATION_FAIL if PPK hash calculation fails
* 			- XLOADER_SEC_ALL_PPK_INVALID_ERR if all PPKs are invalid.
* 			- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
*
******************************************************************************/
int XLoader_PpkVerify(const XLoader_SecureParams *SecurePtr, const u32 PpkSize)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	XSecure_Sha384Hash Sha3Hash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/** - Calculate PPK hash */
	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/** - Update PPK  */
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
		Status = XSecure_ShaUpdate(ShaInstPtr,
			(UINTPTR)&(SecurePtr->AuthJtagMessagePtr->PpkData), PpkSize);
#elif defined(VERSAL_2VE_2VM)
		Status = XSecure_ShaUpdate(ShaInstPtr,
			(UINTPTR)&(SecurePtr->AuthJtagMessagePtr->AuthJtagData), PpkSize);
#else
		XPlmi_Printf(DEBUG_INFO, "Authenticated JTAG not supported\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_NOT_SUPPORTED, XIL_SIGNED_ZERO);
		goto END;
#endif
	}
	else {
#ifndef VERSAL_2VP_P
		Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)&SecurePtr->AcPtr->Ppk, PpkSize);
#else
		Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)&SecurePtr->AcPtr->PPK.Header,
			PpkSize);
#endif
	}
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/* Validate PPK hash and check revocation */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid, Sha3Hash.Hash);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
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
 * @param   MsgHash of the data to be authenticated.
 * @param   RsaInstPtr is pointer to the XSecure_Rsa instance.
 * @param   Signature is pointer to RSA signature for data to be authenticated.
 * @param   KeySize   is size of RSA key in bytes.
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
int XLoader_RsaPssSignVerify( u8 *MsgHash, XSecure_Rsa *RsaInstPtr, u8 *Signature, u32 KeySize)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	volatile u32 DbTmp;
	XSecure_Sha384Hash MPrimeHash;
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
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u8 *DataHash = (u8 *)MsgHash;
	u32 MaskedDbLen = KeySize - XSECURE_SHA_384_HASH_SIZE_IN_BYTES - 1U;
	u32 DbLen = KeySize - (2U * XSECURE_SHA_384_HASH_SIZE_IN_BYTES) - 2U;

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
					KeySize,
					XSecure_RsaSha3Array);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
							 Status);
		goto END;
	}

	/**
	 * - Check for signature encrypted message.
	 */
	if (XSecure_RsaSha3Array[KeySize - 1U] !=
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
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, Xsecure_Varsocm.EmHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
		&XSecure_RsaSha3Array[MaskedDbLen], XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

	/**
	 * - Extract Salt and Generate DB from masked DB and Hash.
	 */
	Status = XLoader_MaskGenFunc(ShaInstPtr, Buffer,
			MaskedDbLen, Xsecure_Varsocm.EmHash);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/** - XOR MGF output with masked DB from EM to get DB. */
	for (Index = 0U; Index <MaskedDbLen; Index++) {
		Buffer[Index] = Buffer[Index] ^ XSecure_RsaSha3Array[Index];
	}

	/** - Check DB = PS <414 zeros> || 0x01. */
	for (Index = 0U; Index < DbLen; Index++) {
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
	if (Index != DbLen) {
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
		&Buffer[DbLen + 1U], XLOADER_RSA_PSS_SALT_LEN, XLOADER_RSA_PSS_SALT_LEN);

	/** - Hash on M prime */
	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/* Padding 1 */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)Xsecure_Varsocm.Padding1,
			XLOADER_RSA_PSS_PADDING1);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	/* Hash the Message */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)DataHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
                                XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
                goto END;
	}
#endif
	/* Salt */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)Xsecure_Varsocm.Salt,
			XLOADER_RSA_PSS_SALT_LEN);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&MPrimeHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL, Status);
		goto END;
	}

	Status = XST_FAILURE;
	IndexTmp = MaskedDbLen;
	/** - Compare MPrime Hash with Hash from EM */
	for (Index = 0U; Index < XSECURE_SHA_384_HASH_SIZE_IN_BYTES; Index++) {
		HashTmp = MPrimeHash.Hash[Index];
		if ((MPrimeHash.Hash[Index] != XSecure_RsaSha3Array[IndexTmp]) ||
			(HashTmp != XSecure_RsaSha3Array[IndexTmp])) {
			XPlmi_Printf(DEBUG_INFO, "Failed at RSA PSS signature "
				"verification\n\r");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)MPrimeHash.Hash,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "M prime Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)XSecure_RsaSha3Array,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "RSA Encrypted Signature");
			Status = XLoader_UpdateMinorErr(
				XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE, 0);
			goto END;
		}
		IndexTmp++;
	}

	if (Index == XSECURE_SHA_384_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}
	XPlmi_Printf(DEBUG_INFO, "RSA PSS verification is successful\n\r");

END:
	ClearStatus = XPlmi_MemSetBytes(&MPrimeHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
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
 * @param   MsgHash of the data to be authenticated.
 * @param   Key is pointer to the XSecure_Rsa instance.
 * @param   Signature is pointer to RSA signature for data to be authenticated.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SEC_RSA_AUTH_FAIL if failed to verify RSA signature.
 *
 ******************************************************************************/
static int XLoader_RsaSignVerify(u8 *MsgHash, XLoader_RsaKey *Key, u8 *Signature) {

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
	Status = XLoader_RsaPssSignVerify(MsgHash, RsaInstPtr,
				Signature, XSECURE_RSA_4096_KEY_SIZE);
END:
	return Status;

}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function verifies the ECDSA signature provided with
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
	XSecure_Sha384Hash Sha3Hash;
	u32 Index;
	XSecure_EllipticKey PublicKey;
	XSecure_EllipticSign Sign;
	const u8 *HashPtr;

	/*
	 * Data generated by Bootgen is in BE, but Elliptic APIs are operated
	 * on LE, so the conversion is happened here.
	 * If User configured Elliptic APIs to be operated in BE
	 * This API is skipping the endianness change in else section.
	 */

	if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
		for (Index = 0U; Index < KeySize; Index++) {
			Qx[Index] = XKey[KeySize - Index - 1U];
			Qy[Index] = YKey[KeySize - Index - 1U];
			SigR[Index] = RSign[KeySize - Index - 1U];
			SigS[Index] = SSign[KeySize - Index - 1U];
		}

		for (Index = 0U; Index < XSECURE_SHA_384_HASH_SIZE_IN_BYTES; Index++) {
			Sha3Hash.Hash[Index] = DataHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES - Index - 1U];
		}
		PublicKey.Qx = Qx;
		PublicKey.Qy = Qy;
		Sign.SignR = SigR;
		Sign.SignS = SigS;
		HashPtr = (const u8 *)Sha3Hash.Hash;
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
			CrvType, HashPtr, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, &PublicKey, &Sign);
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

	/** - Configure AES engine to push Key and IV */
	XPlmi_Printf(DEBUG_INFO, "Decrypting Secure header\n\r");
	Status = XSecure_AesCfgKupKeyNIv(SecurePtr->AesInstPtr, (u8)TRUE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** - Push secure header */
	Status = XSecure_AesDecryptUpdate(SecurePtr->AesInstPtr, SrcAddr,
		XSECURE_AES_NO_CFG_DST_DMA, XLOADER_SECURE_HDR_SIZE, (u8)TRUE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** - Verify GCM Tag */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_AesDecryptFinal,
		SecurePtr->AesInstPtr, SrcAddr + XLOADER_SECURE_HDR_SIZE);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Decrypting Secure header failed in "
			"AesDecrypt Final\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			Status);
		goto END;
	}

	/** - Check if the encrypted data is 128 bit aligned */
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
	StatusTmp = XSecure_AesCfgKupKeyNIv(SecurePtr->AesInstPtr, (u8)FALSE);
	if ((Status == XST_SUCCESS) && (StatusTmp != XST_SUCCESS)) {
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
			StatusTmp);
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
	u32 Index;
	u32 RegVal;

	do {
		for (Index = 0U; Index < XLOADER_SECURE_IV_LEN; Index++) {
			RegVal = XPlmi_In32(SecurePtr->AesInstPtr->BaseAddress +
					(XSECURE_AES_IV_0_OFFSET +
					(Index * XIH_PRTN_WORD_LEN)));
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

		/** - Decrypt the data */
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

		/** - Decrypt Secure footer */
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
		KeyDetails.PufHdLocation = XilPdi_GetPufHdPh(SecurePtr->PrtnHdr)
			>> XIH_PH_ATTRB_PUFHD_SHIFT;
		KeyDetails.PdiKeySrc = SecurePtr->PrtnHdr->EncStatus;
		KeyDetails.KekIvAddr = (UINTPTR)SecurePtr->PrtnHdr->KekIv;
		Status = XLoader_AesKeySelect(SecurePtr,&KeyDetails, &KeySrc);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		/** - Configure DPA counter measure */
		DpaCmCfg = XilPdi_IsDpaCmEnable(SecurePtr->PrtnHdr);
		DpaCmCfgTmp = XilPdi_IsDpaCmEnable(SecurePtr->PrtnHdr);
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SetAesDpaCm,
			SecurePtr->AesInstPtr, (DpaCmCfg | DpaCmCfgTmp));
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_DPA_CM_ERR, Status);
			goto END;
		}
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
		XSecure_ConfigureDmaByteSwap(XSECURE_ENABLE_BYTE_SWAP);
#endif
		/* Decrypting SH */
		Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr, KeySrc,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)SecurePtr->PrtnHdr->PrtnIv);
		if (Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
				Status);
			goto END;
		}
		/** - Decrypt Secure header */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecryptSecureBlk, SecurePtr,
				SrcAddr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
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
	const XilPdi_BootHdr *BootHdr = SecurePtr->PdiPtr->MetaHdr->BootHdrPtr;
	u32 KekStat = 0U;
	volatile u32 PdiKeySrcTmp;
	u32 DecryptBlkKey = (u32)FALSE;

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
			DecryptBlkKey = (u32)TRUE;
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
			DecryptBlkKey = (u32)TRUE;
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
				XPLMI_STATUS_GLITCH_DETECT(Status);
				goto END;
			}
			DecryptBlkKey = (u32)TRUE;
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
			DecryptBlkKey = (u32)TRUE;
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
			DecryptBlkKey = (u32)TRUE;
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
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
	}

	if ((KeyDetails->PdiKeySrc != PdiKeySrcTmp) || (*KeySrc == XSECURE_AES_INVALID_KEY)) {
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0);
		goto END;
	}

	if (DecryptBlkKey == (u32)TRUE) {
		KeyDetails->PufShutterValue = BootHdr->PufShutterVal;
#if (defined(VERSAL_NET) || defined(VERSAL_2VP_P))
		KeyDetails->PufRoSwapEn = BootHdr->PufRingOscConfig;
#endif
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
	XSecure_AesKeySrc KeySrc = XSECURE_AES_BBRAM_KEY;
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen * XIH_PRTN_WORD_LEN;
	u64 SrcAddr = BufferAddr;
	u8 PdiDpaCmCfg = XilPdi_IsDpaCmEnableMetaHdr(&MetaHdr->ImgHdrTbl);
	u32 EfuseDpaCmCfg = XPlmi_In32(XLOADER_EFUSE_SEC_MISC1_OFFSET) &
		(XLOADER_EFUSE_SEC_DPA_DIS_MASK);
	XLoader_AesKekInfo KeyDetails;
	u32 ReadEncReg;
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	XLoader_HBAesParams HBAesParams;
#endif
	if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
#endif
	}

	if ((SecurePtr->IsEncrypted != (u8)TRUE) &&
		(SecureTempParams->IsEncrypted != (u8)TRUE)) {
		XPlmi_Printf(DEBUG_INFO, "Headers are not encrypted\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_NOT_ENCRYPTED, XIL_SIGNED_ZERO);
		goto END;
	}

	/**
	 * - Check Secure State of device
	 * If S-HWRoT is enabled then it is mandatory to use Black IV
	 */
	ReadEncReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
		XPLMI_RTCFG_SECURESTATE_SHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadEncReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			if (ReadEncReg != SecureStateSHWRoT) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_GLITCH_DETECTED, XIL_SIGNED_ZERO);
			}
			goto END;
		}
	}
	else {
		if ((SecurePtr->PdiPtr->DecKeySrc & XLOADER_EFUSE_RED_KEY) == 0x0U) {
			Status = Xil_SMemCpy(SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.KekIv,
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)),
				(u32*)XLOADER_EFUSE_IV_BLACK_OBFUS_START_OFFSET,
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)),
				(XLOADER_SECURE_IV_NUM_ROWS * sizeof(u32)));
			if (Status != XST_SUCCESS) {
				XPLMI_STATUS_GLITCH_DETECT(Status);
				goto ERR_END;
			}
		}
	}

	KeyDetails.PufHdLocation = XilPdi_GetPufHdMetaHdr(&MetaHdr->ImgHdrTbl) >>
		XIH_IHT_ATTR_PUFHD_SHIFT;
	KeyDetails.PdiKeySrc = MetaHdr->ImgHdrTbl.EncKeySrc;
	KeyDetails.KekIvAddr = (UINTPTR)SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl.KekIv;

	/** - Select the Key Source */
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
		goto ERR_END;
	}

	/** - Configure DPA CM */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SetAesDpaCm,
		SecurePtr->AesInstPtr, PdiDpaCmCfg);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_DPA_CM_ERR, Status);
		goto ERR_END;
	}
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	/**
	 * - Endianness swap is not required for the first chunk as the IV will be transferred
	 * using the DMA, and in all other cases, as the IV is part of chunk and decrypted and stored
	 * in IV registers in different format, so, endianness swap is required.
	 * It is forcing to implement enabling/disabling of endianness based on user input,
	 * hence below condition is placed.
	 * If DMA Byteswap is required, Enable DMA Swap.
	 * If DMA Byteswap is not required, it will be in disable state.
	 */
	XSecure_ConfigureDmaByteSwap(XSECURE_ENABLE_BYTE_SWAP);

	if ((SecurePtr->IsAuthenticated != (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated != (u8)TRUE)) {

		/**
		 * - Validate the HashBlock 1 integrity with HashBlock 1 hash
		 * present in HashBlock 0
		 */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_ValidateMHHashBlockIntegrity,
			SecurePtr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			goto END;
		}

		/**
		 * - Validate HashBlock AAD.
		 */
		HBAesParams.HashBlockOffset = MetaHdr->ImgHdrTbl.HashBlockOffset * XIH_PRTN_WORD_LEN;
		HBAesParams.HashBlockSize = MetaHdr->ImgHdrTbl.HashBlockSize * XIH_PRTN_WORD_LEN;
		HBAesParams.KeySrc = KeySrc;
		HBAesParams.IvPtr = (u8 *)&MetaHdr->ImgHdrTbl.IvMetaHdr;
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_ValidateHashBlockAAD,
			SecurePtr, &HBAesParams);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_AAD_VALIDATE, Status);
			goto END;
		}
	}

	/**
	 * - Verify Integrity of Total MetaHeader
	 */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_ValidateMetaHdrIntegrity, SecurePtr);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status |= StatusTmp;
		goto END;
	}
#endif
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

	/** - Decrypt Secure header */
	Status = XLoader_DecryptSecureBlk(SecurePtr, SrcAddr);
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		XPlmi_Printf(DEBUG_INFO, "SH decryption failed during header "
			"decryption\n\r");
		goto ERR_END;
	}

	SrcAddr = SrcAddr + XLOADER_SECURE_HDR_TOTAL_SIZE;
	TotalSize = TotalSize - XLOADER_SECURE_HDR_TOTAL_SIZE;
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DataDecrypt, SecurePtr,
		(UINTPTR)SrcAddr, (UINTPTR)SecurePtr->ChunkAddr, TotalSize);
ERR_END:
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
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Decrypting PUF KEK\n\r");
	PufData->ShutterValue = KeyDetails->PufShutterValue;
	PufData->PufOperation = XPUF_REGEN_ON_DEMAND;
#ifndef VERSAL_2VP_P
	PufData->GlobalVarFilter = (u8)(PufData->ShutterValue >>
		XLOADER_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT);
#endif
#if (defined(VERSAL_NET) || defined(VERSAL_2VP_P))
	PufData->ShutterValue = PufData->ShutterValue &
					~(XPLMI_BIT(XLOADER_PUF_SHUT_GLB_VAR_FLTR_EN_SHIFT));
	PufData->RoSwapVal = KeyDetails->PufRoSwapEn;
#endif

	if (KeyDetails->PufHdLocation == XLOADER_PUF_HD_BHDR) {
		PufData->ReadOption = XPUF_READ_FROM_RAM;
		PufData->SyndromeAddr = XIH_BH_PRAM_ADDR + XIH_BH_PUF_HD_OFFSET;
		PufData->Chash = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_CHASH_OFFSET);
#ifndef VERSAL_2VP_P
		PufData->Aux = *(u32 *)(XIH_BH_PRAM_ADDR + XIH_BH_PUF_AUX_OFFSET);
		XPlmi_Printf(DEBUG_INFO, "BHDR PUF HELPER DATA with CHASH:"
			"%0x and AUX:%0x\n\r", PufData->Chash, PufData->Aux);
#else
		XPlmi_Printf(DEBUG_INFO, "BHDR PUF HELPER DATA with CHASH:"
			"%0x\n\r", PufData->Chash);
#endif
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "EFUSE PUF HELPER DATA\n\r");
		PufData->ReadOption = XPUF_READ_FROM_EFUSE_CACHE;
	}

	Status = XPuf_Regeneration(PufData);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Failed at PUF regeneration with status "
			"%0x\n\r", Status);
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XSecure_AesKekDecrypt(AesInstPtr, KeyDetails->KeySrc,
		KeyDetails->KeyDst, (UINTPTR)KeyDetails->KekIvAddr,
		XSECURE_AES_KEY_SIZE_256);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Failed during AES KEK decrypt\n\r");
		Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "Black key decryption is successful\r\n");

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
* EfusePtr[0]=C6ED8E37 -> IV[31:0]
* EfusePtr[1]=8674A28D -> IV[63:32]
* EfusePtr[2]=F7F8FDE0 -> IV[95:64]]
*
* Spec says:
* IV[95:32] defined by user in meta header should match with eFUSEIV[95:32]
* IV[31:0] defined by user in meta header should >= eFUSEIV[31:0]
*
******************************************************************************/
static int XLoader_ValidateIV(const u32 *IHPtr, const u32 *EfusePtr)
{
	int Status = (int)XLOADER_SEC_IV_METAHDR_RANGE_ERROR;
	volatile u32 IHValue95_64;
	volatile u32 IHValueTmp95_64;
	volatile u32 IHValue63_32;
	volatile u32 IHValueTmp63_32;
	volatile u32 IHValue31_0;
	volatile u32 IHValueTmp31_0;

	IHValue95_64 = Xil_Htonl(IHPtr[0]);
	IHValueTmp95_64 = Xil_Htonl(IHPtr[0]);
	IHValue63_32 = Xil_Htonl(IHPtr[1]);
	IHValueTmp63_32 = Xil_Htonl(IHPtr[1]);
	IHValue31_0 = Xil_Htonl(IHPtr[2]);
	IHValueTmp31_0 = Xil_Htonl(IHPtr[2]);

	if ((IHValue95_64 != EfusePtr[2U]) || (IHValueTmp95_64 != EfusePtr[2U])) {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[95:64]\r\n");
	}
	else if ((IHValue63_32 != EfusePtr[1U]) || (IHValueTmp63_32 != EfusePtr[1U])) {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[63:32]\r\n");
	}
	else if ((IHValue31_0 >= EfusePtr[0U]) && (IHValueTmp31_0 >= EfusePtr[0U])) {
		Status = XST_SUCCESS;
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "IV range check failed for bits[31:0]\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief       This function enables the Jtag
*
* @param       CfgState is the configuration state for DAP
*
* @return
* 			- XST_SUCCESS on SUCCESS
* 			- Errorcode upon failure
*
******************************************************************************/
int XLoader_EnableJtag(volatile u32 CfgState)
{
	int Status = XST_FAILURE;
	volatile u32 DapCfgMask = 0U;

	if ((CfgState == XLOADER_CONFIG_DAP_STATE_ALL_DBG) &&
			(CfgState == XLOADER_CONFIG_DAP_STATE_ALL_DBG)) {
		/**
		 * - Enable secure/non-secure debug
		 * Enabled invasive & non-invasive debug
		 */
		DapCfgMask = XLOADER_DAP_CFG_ENABLE_ALL_DBG_MASK;
	} else {
		/** - Enable only non-secure debug */
		DapCfgMask = (XLOADER_DAP_CFG_NIDEN_MASK | XLOADER_DAP_CFG_DBGEN_MASK);
	}

	XPlmi_Out32(XLOADER_PMC_TAP_DAP_CFG_OFFSET, DapCfgMask);
	/**
	 * - Enable all the instructions
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_0_OFFSET,
		XLOADER_PMC_TAP_INST_MASK_0_ENABLE_MASK);
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_1_OFFSET,
		XLOADER_PMC_TAP_INST_MASK_1_ENABLE_MASK);

	/**
	 * - Disable security gate
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET,
		XLOADER_DAP_SECURITY_GATE_DISABLE_MASK);

	/**
	 * - Take DBG module out of reset
	 */
	XPlmi_Out32(XLOADER_CRP_RST_DBG_OFFSET,
		XLOADER_CRP_RST_DBG_ENABLE_MASK);

	Status = XLoader_CheckAndUpdateSecureState();

	return Status;
}

/*****************************************************************************/
/**
* @brief       This function disables the Jtag
*
* @return
* 			- XST_SUCCESS on SUCCESS
* 			- Errorcode upon failure
*
****************************************************************************/
int XLoader_DisableJtag(void)
{
	int Status = XST_FAILURE;

	/**
	 * - Reset DBG module
	 */
	XPlmi_Out32(XLOADER_CRP_RST_DBG_OFFSET,
			(XLOADER_CRP_RST_DBG_DPC_MASK |
			XLOADER_CRP_RST_DBG_RESET_MASK));

	/**
	 * - Enable security gate
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET,
		~XLOADER_DAP_SECURITY_GATE_DISABLE_MASK);

	/**
	 * - Disable all the instructions
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_0_OFFSET,
			XLOADER_PMC_TAP_INST_DISABLE_MASK_0);
	XPlmi_Out32(XLOADER_PMC_TAP_INST_MASK_1_OFFSET,
			XLOADER_PMC_TAP_INST_DISABLE_MASK_1);

	/**
	 * - Disable secure/non-secure debug
	 * - Disabled invasive & non-invasive debug
	 */
	XPlmi_Out32(XLOADER_PMC_TAP_DAP_CFG_OFFSET,
				0x0U);

	Status = XLoader_CheckAndUpdateSecureState();

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function is called to clear secure critical data related to
* 			authentication and encryption in case of exceptions. The function
* 			also places	AES, ECDSA_RSA in reset.
*
* @return
* 		- XST_SUCCESS on success.
* 		- XST_FAILURE on failure.
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
 * @brief	This function clears the AES keys when RedKeyClear is set in PMC RAM
 *
 * @param	DecKeySrc is pointer to the Decrypt Key source
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_SEC_KEY_CLEAR_FAILED_ERROR if AES key clear fails.
 *
 *****************************************************************************/
int XLoader_ClearAesKey(u32 *DecKeySrc)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RedKeyClear = XPlmi_In32(XPLMI_RTCFG_SECURE_CTRL_ADDR) &
						XLOADER_PPDI_RED_KEY_CLR_MASK;
	volatile u32 RedKeyClearTmp = XPlmi_In32(XPLMI_RTCFG_SECURE_CTRL_ADDR) &
						XLOADER_PPDI_RED_KEY_CLR_MASK;

	if ((RedKeyClear != 0U) || (RedKeyClearTmp != 0U)) {
		*DecKeySrc = 0x0U;
		Status = XLoader_ClearAesKeysOnCfg();
		if (Status != XST_SUCCESS) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_KEY_CLEAR_FAILED_ERROR,
						Status);
		}
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears the AES PUF,RED and expanded keys
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XST_FAILURE if AES key zero or AES reset fails.
 *
 *****************************************************************************/
static int XLoader_ClearAesKeysOnCfg(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesKeyZero(AesInstPtr, XSECURE_AES_PUF_RED_EXPANDED_KEYS);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	/** - Place AES in reset */
	SStatus = Xil_SecureOut32(XLOADER_AES_RESET_REG,
				XLOADER_AES_RESET_VAL);
	if (Status == XST_SUCCESS) {
		Status |= SStatus;
	}

	return Status;
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
	u32 PcrInfo = SecurePtr->PdiPtr->MetaHdr->ImgHdr[SecurePtr->PdiPtr->ImageNum].PcrInfo;

	XPlmi_Printf(DEBUG_INFO,
			"Processing Block %u\n\r", SecurePtr->BlockNum);
	SecurePtr->ProcessedLen = 0U;
	/** - Process the 1st block */
	if (SecurePtr->BlockNum == 0x0U) {
		SrcAddr = SecurePtr->PdiPtr->MetaHdr->FlashOfstAddr +
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
		}
		if (Last == (u8)TRUE) {
			TotalSize = SecurePtr->RemainingEncLen;
		}
		else {
			if (SecurePtr->BlockNum == 0x0U) {
				/* To include Secure Header */
				TotalSize = TotalSize + XLOADER_SECURE_HDR_TOTAL_SIZE;
			}
		}
	}

	Status = XLoader_SecureChunkCopy(SecurePtr, SrcAddr, Last,
				BlockSize, TotalSize);
	if (Status != XST_SUCCESS) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
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

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
			SecurePtr->SecureDataLen = TotalSize;
#else
			if (Last != (u8)TRUE) {
				SecurePtr->SecureDataLen = TotalSize - XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
			}
			else {
				SecurePtr->SecureDataLen = TotalSize;
			}

			/** - Verify hash on the data */
			XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyHashNUpdateNext,
			SecurePtr, SecurePtr->SecureData, SecurePtr->SecureDataLen, Last);
#endif

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
	/* In case of Authentication enabled, it is mandatory to use
	 * same SPK for all partitions of an image. Hence secure config extend is
	 * called only for the first partition of an image for Block 0.
	 */
	if ((SecurePtr->BlockNum == 0x0U) &&
		(SecurePtr->PdiPtr->ImagePrtnId == 0x00U)) {
		if ((SecurePtr->PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) ||
			(SecurePtr->PdiPtr->PdiType == XLOADER_PDI_TYPE_IPU) ||
			(SecurePtr->PdiPtr->PdiType == XLOADER_PDI_TYPE_IAU) ||
			(SecurePtr->PdiPtr->IsSubsystemRestart == (u8)TRUE)) {
			XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_SecureConfigMeasurement,
					SecurePtr, PcrInfo, &SecurePtr->PdiPtr->DigestIndex, TRUE);
		} else {
			XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_SecureConfigMeasurement,
					SecurePtr, PcrInfo, &SecurePtr->PdiPtr->DigestIndex, FALSE);
		}
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != SStatus)) {
			Status |= SStatus;
		}
	}

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
* @brief    This function checks if the secure state of boot matches the
*           expected value or not.
*
* @param    RegVal - Value of secure state stored in register
* @param    Var - Value of secure state stored in variable
* @param    ExpectedValue - Expected value of secure state
*
* @return
* 		- XST_SUCCESS on success.
* 		- XST_FAILURE if RegVal, Var, or ExpectedValue do not all match.
*
******************************************************************************/
int XLoader_CheckSecureState(u32 RegVal, u32 Var, u32 ExpectedValue)
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
int XLoader_AuthKat(XLoader_SecureParams *SecurePtr) {
	u32 AuthType;
	u32 AuthKatMask;
	u32 AuthKatStatus = 0U;
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
#ifndef PLM_ECDSA_EXCLUDE
	XSecure_EllipticCrvClass CrvClass = XSECURE_ECC_PRIME;
#endif

	/** Get the Authentication type. */
#ifndef VERSAL_2VP_P
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		AuthType = XLoader_GetAuthPubAlgo(&(SecurePtr->AuthJtagMessagePtr->AuthHdr));
	}
	else {
		AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	}
#else
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		XPlmi_Printf(DEBUG_INFO, "Authenticated JTAG not supported\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_NOT_SUPPORTED, XIL_SIGNED_ZERO);
		goto END;
	}

	AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);
#endif

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
#if !defined(PLM_ECDSA_EXCLUDE) && defined(XSECURE_ECC_SUPPORT_NIST_P521)
		CrvClass = XSECURE_ECC_PRIME;
#endif
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if (SecurePtr->AuthJtagMessagePtr == NULL) {
		/** - Update KAT status based on the user configuration. */
		XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, AuthKatMask);
		AuthKatStatus = SecurePtr->PdiPtr->PlmKatStatus & AuthKatMask;
	}

	/**
	 * - Skip running the KAT for ECDSA or RSA if it is already run.
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set.
	 * If KAT fails device will go into a secure lockdown state.
	 * For AUTH JTAG KAT will be run every time when request is made.
	 */
	if (AuthKatStatus == 0U) {
		if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
#ifndef PLM_RSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XLoader_RsaKat);
#else
			XPlmi_Printf(DEBUG_GENERAL, "RSA code is excluded\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_RSA_NOT_ENABLED, XIL_SIGNED_ZERO);
			goto END;
#endif

		}
		else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
#ifndef PLM_ECDSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XSecure_EllipticVerifySignKat, CrvClass);
#else
                        XPlmi_Printf(DEBUG_GENERAL, "ECDSA code is excluded\n\r");
                        Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
                        goto END;
#endif
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
#ifndef PLM_ECDSA_EXCLUDE
#ifdef XSECURE_ECC_SUPPORT_NIST_P521
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XSecure_EllipticVerifySignKat, CrvClass);
#else
			XPlmi_Printf(DEBUG_GENERAL, "ECDSA code is excluded\n\r");
                        Status = XLoader_UpdateMinorErr(XLOADER_SEC_CURVE_NOT_SUPPORTED, 0U);
                        goto END;
#endif
#else
                        XPlmi_Printf(DEBUG_GENERAL, "ECDSA code is excluded\n\r");
                        Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
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
				XPlmi_Printf(DEBUG_INFO, "RSA KAT Failed\n\r");
			}
			else {
				XPlmi_Printf(DEBUG_INFO, "ECDSA KAT Failed\n\r");
			}
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		if (SecurePtr->AuthJtagMessagePtr == NULL) {
			SecurePtr->PdiPtr->PlmKatStatus |= AuthKatMask;
			/* Update KAT status */
			XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
		}
	}
#if (defined(PLM_RSA_EXCLUDE) && defined(PLM_ECDSA_EXCLUDE))
	(void)StatusTmp;
#endif
	Status = XST_SUCCESS;
END:
	return Status;
}

#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)

/******************************************************************************/
/**
* @brief	This function will Calculate Digest for Input Data
*
* @param	InData - Input Data
* @param	DataSize - Input Data Size
* @param	Hash - Pointer the hash u8 array
*
* @return
* 		- XST_SUCCESS on success.
* 		- XST_FAILURE if SHA digest calculation fails.
*
*******************************************************************************/
int XLoader_ShaDigestCalculation(u8 *InData, u32 DataSize, u8 *Hash)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	Status = XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384,
			(u64)(UINTPTR)InData, DataSize,
			(u64)(UINTPTR)Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function authenticates given HashBlock.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams.
 * @param	HBSignParams	Pointer to the XLoader_HBSignParams.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_IMGHDR if image header read fails.
 * 		- XLOADER_ERR_PRTNHDR if partition header read fails.
 *
 ******************************************************************************/
static int XLoader_AuthHdrsWithHashBlock(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams)
{
	volatile int Status = XST_FAILURE;
	XilPdi_MetaHdr *MetaHdr = SecurePtr->PdiPtr->MetaHdr;

	/** - Read IHT and PHT to structures and verify checksum */
	XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Image Headers\n\r",
			MetaHdr->ImgHdrTbl.NoOfImgs);

	/** - Authenticate Image headers and partition headers */
	Status = XilPdi_ReadImgHdrs(MetaHdr);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Partition Headers\n\r",
			MetaHdr->ImgHdrTbl.NoOfPrtns);

	Status = XST_FAILURE;
	Status = XilPdi_ReadPrtnHdrs(MetaHdr);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTNHDR, Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XilPdi_VerifyImgHdrs(MetaHdr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_IH_VERIFY_FAIL, Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XilPdi_VerifyPrtnHdrs(MetaHdr);
	if(Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEC_PH_VERIFY_FAIL, Status);
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_AuthenticateHashBlock,
			SecurePtr, HBSignParams);
	/**
	 * - Verify Integrity of Total MetaHeader.
	 */
	Status = XST_FAILURE;
	Status = XLoader_ValidateMetaHdrIntegrity(SecurePtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function authenticates HashBlock with respect to headers
 * 		and decrypts the headers.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	HBSignParams	Pointer to the XLoader_HBSignParams.
 * @param	BufferAddr	Read whole headers to the mentioned buffer
 *				address.
 *
 * @return
 *          - XST_SUCCESS on success.
 *          - XST_FAILURE on failure.
 *
 ******************************************************************************/
static int XLoader_AuthHashBlockNDecHdrs(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams, u64 BufferAddr)
{
	volatile int Status = XST_FAILURE;
	XilPdi_MetaHdr *MetaHdr = SecurePtr->PdiPtr->MetaHdr;
	/** - Authenticate HashBlock corresponding to MetaHeader */
	Status = XLoader_AuthenticateHashBlock(SecurePtr, HBSignParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/** - Decrypt the Headers */
	Status = XLoader_DecHdrs(SecurePtr, MetaHdr, BufferAddr);

END:
	return Status;
}

#ifndef VERSAL_2VP_P
/******************************************************************************/
/**
* @brief	This function performs PPK and SPK authentication
*
* @param	SecurePtr       Pointer to the XLoader_SecureParams
* @param	HBSignParams    Pointer to the XLoader_HBSignParams
*
* @return
*		- XLOADER_ERR_KAT_FAILED if KAT failed
*		- XLOADER_SEC_GLITCH_DETECTED_ERROR if Error glitch detected
*		- XLOADER_ERR_PPK_HASH_CALC_FAIL if Error in PPK hash calculation
*		- XLOADER_ERR_SPK_HASH_CALC_FAIL if Error in SPK hash calculation
* 		- XST_SUCCESS in case of success
*******************************************************************************/
static int XLoader_AuthenticateKeys(XLoader_SecureParams *SecurePtr, XLoader_HBSignParams *HBSignParams)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XSecure_Sha384Hash SpkHash;
	volatile u32 IsEfuseAuth = (u32)TRUE;
	volatile u32 IsEfuseAuthTmp = (u32)TRUE;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 ReadAuthReg = 0x0U;
	u32 AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);

	if ((AuthType != XLOADER_PUB_STRENGTH_LMS) &&
		(AuthType != XLOADER_PUB_STRENGTH_LMS_HSS)) {
		Status = XLoader_AuthKat(SecurePtr);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Auth KAT failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
	}

	/** - Check Secure state of device
	 * If A-HWRoT is disabled then BHDR authentication is allowed
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
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
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkVerify, SecurePtr,
				HBSignParams->ActualPpkSize);
	}

	Status = XLoader_ShaDigestCalculation((u8 *)&SecurePtr->AcPtr->SpkHeader,
			XLOADER_SPK_HEADER_SIZE + SecurePtr->AcPtr->SpkHeader.SPKSize,
			&SpkHash.Hash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HASH_CALC_FAIL,
                                        Status);
		goto END;
	}

	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature, SecurePtr,
			SpkHash.Hash, &SecurePtr->AcPtr->Ppk,
			(u8 *)&SecurePtr->AcPtr->SPKSignature);
	}
	else if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyLmsSignature, SecurePtr,
			(u8 *)&SecurePtr->AcPtr->SPKSignature,
			SecurePtr->AcPtr->SpkHeader.SignatureSize,
			(u8 *)&SecurePtr->AcPtr->Ppk,
			HBSignParams->ActualPpkSize,
			(u8 *)&SecurePtr->AcPtr->SpkHeader,
			XLOADER_SPK_HEADER_SIZE + SecurePtr->AcPtr->SpkHeader.SPKSize);
        }
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	/* Check for SPK ID revocation */
	if ((IsEfuseAuth == (u8)TRUE) || (IsEfuseAuthTmp == (u8)TRUE)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyRevokeId,
			SecurePtr->AcPtr->SpkHeader.SPKId);
	}

	XPlmi_Printf(DEBUG_INFO, "SPK authentication is successful\n\r");

END:
	/** - Zeroize the SpkHash buffer used for storing calculated Spk hash */
	ClearStatus = XPlmi_MemSetBytes(&SpkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Reads Authentication Data and validate.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams
 * @param	HBSignParams	Pointer to the XLoader_HBSignParams
 *
 * @return
 * 		- XST_SUCCESS on successful authentication.
 * 		- XLOADER_ERR_PPK_COPY_FAIL if PPK copy from flash fails.
 * 		- XLOADER_ERR_GET_LMS_ALGO_FAILED if getting LMS hash algorithm fails.
 * 		- XLOADER_ERR_KAT_FAILED if LMS KAT fails.
 * 		- XLOADER_ERR_SPK_HEADER_COPY_FAIL if SPK header copy fails.
 * 		- XLOADER_ERR_SPK_HEADER_VALIDATE_FAIL if SPK header validation fails.
 * 		- XLOADER_ERR_SPK_COPY_FAIL if SPK copy fails.
 * 		- XLOADER_ERR_SPK_SIGN_COPY_FAIL if SPK signature copy fails.
 * 		- XLOADER_ERR_KEY_AUTH_FAIL if SPK authentication fails.
 * 		- XLOADER_ERR_HASH_BLOCK_COPY_FAIL if HashBlock copy fails.
 * 		- XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL if HashBlock signature copy fails.
 * 		- XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL if HashBlock hash calculation fails.
 * 		- XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL if HashBlock signature verification fails.
 * 		- XLOADER_SEC_BUF_CLEAR_ERR if buffer clear fails.
 *
 ******************************************************************************/
static int XLoader_AuthenticateHashBlock(XLoader_SecureParams *SecurePtr,
	XLoader_HBSignParams *HBSignParams)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XSecure_Sha384Hash HashBlockHash;
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
                XPLMI_PMCRAM_CHUNK_MEMORY_1;
	u32 ReadOffset;
	u32 AuthType;

	XPlmi_Printf(DEBUG_INFO, "Authentication of HashBlock\n\r");

	SecurePtr->AcPtr = AuthCert;
	/**
	 * - Read PPK, SPK Header, SPK and SPK Signature from PDI and
	 * copy to PMCRAM memory.
	 */
	ReadOffset = HBSignParams->ReadOffset;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->Ppk,
			HBSignParams->TotalPpkSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PPK_COPY_FAIL, Status);
		goto END;
	}

	SecurePtr->AcPtr->AuthHdr = HBSignParams->AuthHdr;
	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		/** - Get the Lms Hash algorithm present in public key */
		Status = XSecure_GetLmsHashAlgo(AuthType, (u8 *)&SecurePtr->AcPtr->Ppk,
			&SecurePtr->SignHashAlgo);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GET_LMS_ALGO_FAILED, Status);
			goto END;
		}
		/** - If AuthType is LMS/LMS_HSS run LMS Kat */
		Status = XLoader_LmsKat(SecurePtr, AuthType);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
	}

	/** - Read Spk header */
	ReadOffset += HBSignParams->TotalPpkSize;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->SpkHeader,
			XLOADER_SPK_HEADER_SIZE, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HEADER_COPY_FAIL, Status);
		goto END;
	}

	/** - Validate SPK header */
	Status = XLoader_ValidateSpkHeader(&SecurePtr->AcPtr->SpkHeader);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HEADER_VALIDATE_FAIL, Status);
		goto END;
	}

	/** - Read Spk */
	ReadOffset += XLOADER_SPK_HEADER_SIZE;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->Spk,
			SecurePtr->AcPtr->SpkHeader.TotalSPKSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_COPY_FAIL, Status);
		goto END;
	}

	/** - Read Spk Signature */
	ReadOffset += SecurePtr->AcPtr->SpkHeader.TotalSPKSize;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->SPKSignature,
			SecurePtr->AcPtr->SpkHeader.TotalSignatureSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, Status);
		goto END;
	}

	SecurePtr->AcPtr->SpkId = SecurePtr->AcPtr->SpkHeader.SPKId;

	/**
	 * - Verify PPK Hash
	 * - Authenticate the SPK
	 * - Return error if selected/all PPK revoked
	 * - Compare PPK hash with efuse stored value (if eFUSE auth)
	 * - Skip PPK hash comparison for BH auth
	 * - Authenticate SPK using PPK
	 */
	XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_AuthenticateKeys, SecurePtr, HBSignParams);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Spk Authentication Failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KEY_AUTH_FAIL, Status);
		goto END;
	}

	/** - Read HashBlock */
	ReadOffset += SecurePtr->AcPtr->SpkHeader.TotalSignatureSize;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
                        (u64)(UINTPTR)MetaHdrPtr->HashBlock.HashData,
			HBSignParams->HBSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_COPY_FAIL, Status);
		goto END;
	}

	/** - Read HashBlock Signature */
	ReadOffset += HBSignParams->HBSize;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
                        (u64)(UINTPTR)&SecurePtr->AcPtr->HBSignature,
                        HBSignParams->TotalHBSignSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL, Status);
		goto END;
	}

	/** - Calculate HashBlock hash */
	Status = XLoader_ShaDigestCalculation((u8 *)&MetaHdrPtr->HashBlock.HashData,
			HBSignParams->HBSize, &HashBlockHash.Hash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL,
                                Status);
		goto END;
	}

	/** - Verify HashBlock Signature with SPK */
	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifySignature, SecurePtr,
				HashBlockHash.Hash, &SecurePtr->AcPtr->Spk,
				(u8 *)&SecurePtr->AcPtr->HBSignature);
	}
	else if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifyLmsSignature, SecurePtr,
				(u8 *)&SecurePtr->AcPtr->HBSignature,
				HBSignParams->ActualHBSignSize,
				(u8 *)&SecurePtr->AcPtr->Spk,
				SecurePtr->AcPtr->SpkHeader.SPKSize,
				(u8 *)&MetaHdrPtr->HashBlock.HashData,
				HBSignParams->HBSize);
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Verification of hash block signature failed %x \r\n", AuthType);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "HashBlock Authentication is successful\n\r");

	/** - Copy Authenticated HashBlock to PPU1 RAM */
	Status = SecurePtr->CopyHashBlock(HBSignParams->HBSize, &MetaHdrPtr->HashBlock);
END:
	/** - Zeroize the HashBlockHash buffer used for storing calculated HashBlock hash */
	ClearStatus = XPlmi_MemSetBytes(&HashBlockHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}

#else
/******************************************************************************/
/**
 * @brief	This function gets key sizes and signature padding for the
 * 		given authentication type.
 *
 * @param	AuthType	Authentication algorithm type.
 * @param	HBSignParams	Pointer to the XLoader_HBSignParams.
 * @param	SignPaddingSize	Pointer to the signature padding size.
 *
 * @return
 * 		- XST_SUCCESS if parameters are updated successfully.
 * 		- XST_FAILURE if authentication type is invalid.
 *
 ******************************************************************************/
static int XLoader_GetKeySizeSignPadding(u32 AuthType,
		XLoader_HBSignParams *HBSignParams, u32 *SignPaddingSize)
{
	volatile int Status = XST_SUCCESS;
	u32 TotalKeySize = 0U;
	u32 ActualKeySize = 0U;
	u32 LocalSignPaddingSize = 0U;

	if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
		TotalKeySize = XLOADER_RSA_TOTAL_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_RSA_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_RSA_SIGN_PADDING;
	} else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
		TotalKeySize = XLOADER_ECDSA_P384_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_ECDSA_P384_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_ECDSA_SIGN_PADDING;
	} else if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
		TotalKeySize = XLOADER_LMS_HSS_TOTAL_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_LMS_HSS_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_LMS_HSS_SIGN_PADDING;
	} else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
		TotalKeySize = XLOADER_LMS_TOTAL_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_LMS_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_LMS_SIGN_PADDING;
	} else if (AuthType == XLOADER_PUB_STRENGTH_MLDSA) {
		TotalKeySize = XLOADER_MLDSA_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_MLDSA_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_MLDSA_SIGN_PADDING;
	} else if (AuthType == XLOADER_PUB_STRENGTH_SLHDSA) {
		TotalKeySize = XLOADER_SLHDSA_KEY_SIZE_IN_BYTES;
		ActualKeySize = XLOADER_SLHDSA_KEY_SIZE_IN_BYTES;
		LocalSignPaddingSize = XLOADER_SLHDSA_SIGN_PADDING;
	} else {
		Status = XST_FAILURE;
	}

	if (Status == XST_SUCCESS) {
		HBSignParams->TotalPpkSize = TotalKeySize;
		HBSignParams->ActualPpkSize = ActualKeySize;
		HBSignParams->TotalSpkSize = TotalKeySize;
		HBSignParams->ActualSpkSize = ActualKeySize;
		*SignPaddingSize = LocalSignPaddingSize;
	}

	return Status;
}


/******************************************************************************/
/**
 * @brief	This function verifies the MLDSA signature using the provided
 * 		public key.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	HBSignParams is pointer to the XLoader_HBSignParams instance.
 * @param	VerifyType indicates whether to verify SPK or HashBlock signature.
 *
 * @return
 *		- XST_SUCCESS on successful signature verification.
 *		- Error code on failure.
 *
 *******************************************************************************/
static int XLoader_MldsaSignVerify(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 VerifyType)
{
	volatile int Status = XST_FAILURE;
	XSecure_MldsaSignVerifyParams MldsaParams;
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;

	if (VerifyType == XLOADER_VERIFY_SPK_SIGN) {
		/* Verify SPK signature using PPK */
		MldsaParams.DataAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Header;
		MldsaParams.DataLen = HBSignParams->ActualSpkSize + XLOADER_KEY_HDR_SIZE_IN_BYTES;
		MldsaParams.PubKeyAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->PPK.Key;
		MldsaParams.PubKeyLen = HBSignParams->ActualPpkSize;
	} else {
		/* Verify HashBlock signature using SPK */
		MldsaParams.DataAddr = (u64)(UINTPTR)&MetaHdrPtr->HashBlock.HashData;
		MldsaParams.DataLen = HBSignParams->HBSize;
		MldsaParams.PubKeyAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Key;
		MldsaParams.PubKeyLen = HBSignParams->ActualSpkSize;
	}

	MldsaParams.SignAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignature;
	MldsaParams.SignLen = HBSignParams->ActualAuthSignSize;
	MldsaParams.ContextAddr = 0U;
	MldsaParams.ContextLen = 0U;

	Status = XSecure_MldsaSignVerify(&MldsaParams);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function verifies the SLH-DSA (Stateless Hash-Based Digital
 *		Signature Algorithm) signature using the provided public key.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	HBSignParams is pointer to the XLoader_HBSignParams instance.
 * @param	VerifyType indicates whether to verify SPK or HashBlock signature.
 *
 * @return
 *		- XST_SUCCESS on successful signature verification.
 *		- Error code on failure.
 *
 *******************************************************************************/
static int XLoader_SlhdsaSignVerify(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 VerifyType)
{
	volatile int Status = XST_FAILURE;
	XSecure_SlhdsaInputParams SlhdsaParams;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;

	if (VerifyType == XLOADER_VERIFY_SPK_SIGN) {
		/* Verify SPK signature using PPK */
		SlhdsaParams.DataAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Header;
		SlhdsaParams.DataLen = HBSignParams->ActualSpkSize + XLOADER_KEY_HDR_SIZE_IN_BYTES;
		SlhdsaParams.PublicKeyAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->PPK.Key;
		SlhdsaParams.PublicKeyLen = HBSignParams->ActualPpkSize;
	} else {
		/* Verify HashBlock signature using SPK */
		SlhdsaParams.DataAddr = (u64)(UINTPTR)&MetaHdrPtr->HashBlock.HashData;
		SlhdsaParams.DataLen = HBSignParams->HBSize;
		SlhdsaParams.PublicKeyAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Key;
		SlhdsaParams.PublicKeyLen = HBSignParams->ActualSpkSize;
	}

	SlhdsaParams.SignatureAddr = (u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignature;
	SlhdsaParams.SignatureLen = HBSignParams->ActualAuthSignSize;
	SlhdsaParams.ContextAddr = 0U;
	SlhdsaParams.ContextLen = 0U;

	Status = XSecure_SlhdsaVerify(ShaInstPtr, &SlhdsaParams);

	return Status;
}

/******************************************************************************/
/**
* @brief	This function performs PPK and SPK authentication.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams.
* @param	HBSignParams is pointer to the XLoader_HBSignParams.
*
* @return
*		- XLOADER_ERR_KAT_FAILED if KAT failed.
*		- XLOADER_SEC_GLITCH_DETECTED_ERROR if Error glitch detected.
*		- XLOADER_ERR_PPK_HASH_CALC_FAIL if Error in PPK hash calculation.
*		- XLOADER_ERR_SPK_HASH_CALC_FAIL if Error in SPK hash calculation.
*		- XST_SUCCESS in case of success.
*
*******************************************************************************/
static int XLoader_AuthenticateKeys(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u8 SpkHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 ReadAuthReg = 0x0U;
	u32 AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);

	if ((AuthType != XLOADER_PUB_STRENGTH_LMS) &&
		(AuthType != XLOADER_PUB_STRENGTH_LMS_HSS) &&
		(AuthType != XLOADER_PUB_STRENGTH_MLDSA) &&
		(AuthType != XLOADER_PUB_STRENGTH_SLHDSA)) {
		Status = XLoader_AuthKat(SecurePtr);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Auth KAT failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
	}

	/** - Check Secure state of device
	 * If A-HWRoT is disabled then BHDR authentication is allowed
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			if (ReadAuthReg != SecureStateAHWRoT) {
				Status = XLoader_UpdateMinorErr(
					XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0);
			}
			goto END;
		}
		else {
			/* Misra-C Compliance */
		}
	}
	else {
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkVerify,
				SecurePtr,
				(HBSignParams->ActualPpkSize +
				 XLOADER_KEY_HDR_SIZE_IN_BYTES));
	}

	Status = XLoader_ShaDigestCalculation((u8 *)&SecurePtr->AcPtr->SPK.Header,
			(HBSignParams->ActualSpkSize +
			 XLOADER_KEY_HDR_SIZE_IN_BYTES), &SpkHash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HASH_CALC_FAIL,
				Status);
		goto END;
	}

	AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);

	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
	    (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
	    (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifySignature,
				SecurePtr,
				(u8 *)&SpkHash, &SecurePtr->AcPtr->PPK,
				(u8 *)&SecurePtr->AcPtr->AuthSignature);
	} else if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		   (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyLmsSignature,
				SecurePtr,
				(u8 *)&SecurePtr->AcPtr->AuthSignature,
				HBSignParams->ActualAuthSignSize,
				(u8 *)&SecurePtr->AcPtr->PPK.Key,
				HBSignParams->ActualPpkSize,
				(u8 *)&SecurePtr->AcPtr->SPK.Header,
				(HBSignParams->ActualSpkSize +
					XLOADER_KEY_HDR_SIZE_IN_BYTES));
	} else if (AuthType == XLOADER_PUB_STRENGTH_MLDSA) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_MldsaSignVerify,
			SecurePtr, HBSignParams, XLOADER_VERIFY_SPK_SIGN);
	} else if (AuthType == XLOADER_PUB_STRENGTH_SLHDSA) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SlhdsaSignVerify,
			SecurePtr, HBSignParams, XLOADER_VERIFY_SPK_SIGN);
	} else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "SPK authentication is successful\n\r");

END:
	/** - Zeroize the SpkHash buffer used for storing calculated Spk hash */
	ClearStatus = XPlmi_MemSetBytes(&SpkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function validates the set of keypairs in case of hybrid
 * 		signing. Supported combinations are; one keypair of Non-PQC
 * 		algorithm and one of PQC algorithm.
 *
 * @param	Algo1	Algorithm type for first keypair.
 * @param	Algo2	Algorithm type for second keypair.
 *
 * @return
 * 		- XST_SUCCESS	If hybrid keypairs set is valid.
 * 		- XST_FAILURE	If hybrid keypairs set is invalid.
 *
 ******************************************************************************/
static int XLoader_ValidateHybridKeyAlgo(u32 Algo1, u32 Algo2)
{
	volatile int Status = XST_FAILURE;

	if (((Algo1 == XLOADER_PUB_STRENGTH_RSA_4096 ||
	      Algo1 == XLOADER_PUB_STRENGTH_ECDSA_P384 ||
	      Algo1 == XLOADER_PUB_STRENGTH_ECDSA_P521) &&
	     (Algo2 == XLOADER_PUB_STRENGTH_LMS ||
	      Algo2 == XLOADER_PUB_STRENGTH_LMS_HSS ||
	      Algo2 == XLOADER_PUB_STRENGTH_MLDSA ||
	      Algo2 == XLOADER_PUB_STRENGTH_SLHDSA)) ||
	    ((Algo2 == XLOADER_PUB_STRENGTH_RSA_4096 ||
	      Algo2 == XLOADER_PUB_STRENGTH_ECDSA_P384 ||
	      Algo2 == XLOADER_PUB_STRENGTH_ECDSA_P521) &&
	     (Algo1 == XLOADER_PUB_STRENGTH_LMS ||
	      Algo1 == XLOADER_PUB_STRENGTH_LMS_HSS ||
	      Algo1 == XLOADER_PUB_STRENGTH_MLDSA ||
	      Algo1 == XLOADER_PUB_STRENGTH_SLHDSA))) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function authenticates one PPK/SPK keypair and verifies
 * 		the HashBlock signature using that SPK.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	HBSignParams is pointer to the XLoader_HBSignParams instance.
 * @param	ReadOffset points to current read offset in Metaheader.
 * @param	AuthType is current keypair authentication algorithm.
 * @param	SignPaddingSize is signature padding size for current AuthType.
 * @param	HashBlockHash is pointer to hash buffer used for HB hash.
 *
 * @return
 * 		- XST_SUCCESS on successful verification.
 * 		- XLOADER_ERR_PPK_COPY_FAIL if PPK copy fails.
 * 		- XLOADER_ERR_GET_LMS_ALGO_FAILED if getting LMS hash algorithm fails.
 * 		- XLOADER_ERR_KAT_FAILED if LMS KAT fails.
 * 		- XLOADER_ERR_SPK_HEADER_COPY_FAIL if SPK header copy fails.
 * 		- XLOADER_ERR_INCORRECT_KEY if key permission check fails.
 * 		- XLOADER_ERR_KEY_ALGO_MISMATCH if SPK algorithm does not match PPK algorithm.
 * 		- XLOADER_ERR_SPK_COPY_FAIL if SPK copy fails.
 * 		- XLOADER_ERR_SIGN_HDR_COPY_FAIL if signature header copy fails.
 * 		- XLOADER_ERR_SPK_SIGN_COPY_FAIL if SPK/HB signature copy or size validation fails.
 * 		- XLOADER_ERR_KEY_AUTH_FAIL if SPK authentication fails.
 * 		- XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL if HashBlock signature copy fails.
 * 		- XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL if HashBlock hash calculation fails.
 * 		- XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL if HashBlock signature verification fails.
 * 		- XLOADER_SEC_INVALID_AUTH if authentication type is invalid.
 *
 ******************************************************************************/
static int XLoader_AuthHashBlockWithKeyPair(XLoader_SecureParams *SecurePtr,
		XLoader_HBSignParams *HBSignParams, u32 *ReadOffset,
		u32 AuthType, u32 SignPaddingSize, u8 *HashBlockHash)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u32 SpkAuthType;
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;

	/* Read PPK */
	*ReadOffset += XLOADER_KEY_HDR_SIZE_IN_BYTES;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->PPK.Key,
			HBSignParams->TotalPpkSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PPK_COPY_FAIL, Status);
		goto END;
	}

	if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
			(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		/** - Get the Lms Hash algorithm present in public key */
		Status = XSecure_GetLmsHashAlgo(AuthType,
					(u8 *)&SecurePtr->AcPtr->PPK.Key,
					&SecurePtr->SignHashAlgo);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GET_LMS_ALGO_FAILED, Status);
			goto END;
		}
		/** - If AuthType is LMS/LMS_HSS run LMS Kat */
		Status = XLoader_LmsKat(SecurePtr, AuthType);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
	}

	/* Read SPK Header */
	*ReadOffset += HBSignParams->TotalPpkSize;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Header,
			XLOADER_KEY_HDR_SIZE_IN_BYTES,
			0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HEADER_COPY_FAIL, Status);
		goto END;
	}

	/* Check permission in SPK Header */
	if (SecurePtr->AcPtr->SPK.Header[XLOADER_KEY_HDR_PERMISSION_IDX] !=
			XLOADER_KEY_IS_SPK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INCORRECT_KEY, Status);
		goto END;
	}

	/* Check whether SPK algorithm matches with PPK algorithm */
	SpkAuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->SPK.Header);
	if (SpkAuthType != AuthType) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KEY_ALGO_MISMATCH, Status);
		goto END;
	}

	/* Read SPK */
	*ReadOffset += XLOADER_KEY_HDR_SIZE_IN_BYTES;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->SPK.Key,
			HBSignParams->TotalSpkSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_COPY_FAIL, Status);
		goto END;
	}

	/* Read SPK Signature Header */
	*ReadOffset += HBSignParams->TotalSpkSize;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignHdr,
			XLOADER_SIGN_HDR_SIZE, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SIGN_HDR_COPY_FAIL, Status);
		goto END;
	}

	HBSignParams->TotalAuthSignSize = SecurePtr->AcPtr->AuthSignHdr[0];

	/* Validate total signature size before use */
	if ((HBSignParams->TotalAuthSignSize == 0U) ||
		(HBSignParams->TotalAuthSignSize > XLOADER_MAX_TOTAL_SIGN_SIZE) ||
		(HBSignParams->TotalAuthSignSize < SignPaddingSize)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, XST_FAILURE);
		goto END;
	}

	HBSignParams->ActualAuthSignSize = SecurePtr->AcPtr->AuthSignHdr[0] - SignPaddingSize;
	if (HBSignParams->ActualAuthSignSize < XLOADER_MIN_SIGN_SIZE) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, XST_FAILURE);
		goto END;
	}

	/* Read SPK Signature */
	*ReadOffset += XLOADER_SIGN_HDR_SIZE;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignature,
			HBSignParams->TotalAuthSignSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, Status);
		goto END;
	}

	/**
	 * - Verify PPK Hash
	 * - Authenticate the SPK
	 * - Return error if selected/all PPK revoked
	 * - Compare PPK hash with efuse stored value (if eFUSE auth)
	 * - Skip PPK hash comparison for BH auth
	 * - Authenticate SPK using PPK
	 */
	XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_AuthenticateKeys,
			SecurePtr, HBSignParams);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Spk Authentication Failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KEY_AUTH_FAIL, Status);
		goto END;
	}

	/* Read HB Signature Header */
	*ReadOffset += HBSignParams->TotalAuthSignSize;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignHdr,
			XLOADER_SIGN_HDR_SIZE, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SIGN_HDR_COPY_FAIL,
				Status);
		goto END;
	}

	HBSignParams->TotalAuthSignSize = SecurePtr->AcPtr->AuthSignHdr[0];

	/* Validate total signature size before use */
	if ((HBSignParams->TotalAuthSignSize == 0U) ||
		(HBSignParams->TotalAuthSignSize > XLOADER_MAX_TOTAL_SIGN_SIZE) ||
		(HBSignParams->TotalAuthSignSize < SignPaddingSize)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, XST_FAILURE);
		goto END;
	}

	HBSignParams->ActualAuthSignSize = SecurePtr->AcPtr->AuthSignHdr[0] - SignPaddingSize;
	if (HBSignParams->ActualAuthSignSize < XLOADER_MIN_SIGN_SIZE) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, XST_FAILURE);
		goto END;
	}

	/** - Read HashBlock Signature */
	*ReadOffset += XLOADER_SIGN_HDR_SIZE;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + *ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->AuthSignature,
			HBSignParams->TotalAuthSignSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL,
				Status);
		goto END;
	}

	/** - Calculate HashBlock hash */
	Status = XLoader_ShaDigestCalculation((u8 *)&MetaHdrPtr->HashBlock.HashData,
			HBSignParams->HBSize, HashBlockHash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL,
				Status);
		goto END;
	}

	/** - Verify HashBlock Signature with SPK */
	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifySignature,
				SecurePtr, HashBlockHash,
				&SecurePtr->AcPtr->SPK,
				(u8 *)&SecurePtr->AcPtr->AuthSignature);
	} else if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus,
				XLoader_VerifyLmsSignature, SecurePtr,
				(u8 *)&SecurePtr->AcPtr->AuthSignature,
				HBSignParams->ActualAuthSignSize,
				(u8 *)&SecurePtr->AcPtr->SPK.Key,
				HBSignParams->ActualSpkSize,
				(u8 *)&MetaHdrPtr->HashBlock.HashData,
				HBSignParams->HBSize);
	} else if (AuthType == XLOADER_PUB_STRENGTH_MLDSA) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_MldsaSignVerify,
			SecurePtr, HBSignParams, XLOADER_VERIFY_HB_SIGN);
	} else if (AuthType == XLOADER_PUB_STRENGTH_SLHDSA) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SlhdsaSignVerify,
			SecurePtr, HBSignParams, XLOADER_VERIFY_HB_SIGN);
	} else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Verification of hash block signature failed %x \r\n", AuthType);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL, Status);
		goto END;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function Reads Authentication Data and validate.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams.
 * @param	HBSignParams	Pointer to the XLoader_HBSignParams.
 *
 * @return
 * 		- XST_SUCCESS on successful authentication.
 * 		- XLOADER_ERR_HASH_BLOCK_COPY_FAIL if HashBlock copy fails.
 * 		- XLOADER_ERR_PPK_HEADER_COPY_FAIL if PPK header copy fails.
 * 		- XLOADER_ERR_INCORRECT_KEY if PPK key permission check fails.
 * 		- XLOADER_ERR_INVALID_HYBRID_KEYPAIR if hybrid key pair validation fails.
 * 		- XLOADER_SEC_BUF_CLEAR_ERR if buffer clear fails.
 *
 ******************************************************************************/
static int XLoader_AuthenticateHashBlock(XLoader_SecureParams *SecurePtr,
	XLoader_HBSignParams *HBSignParams)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u8 HashBlockHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;
	XLoader_AuthCertificate *AuthCert =
		(XLoader_AuthCertificate *) XPLMI_PMCRAM_CHUNK_MEMORY_1;
	u32 ReadOffset;
	u32 AuthType;
	u32 SignPaddingSize;
	u32 IsHybridSign = (u32)FALSE;

	XPlmi_Printf(DEBUG_INFO, "Authentication of HashBlock\n\r");

	SecurePtr->AcPtr = AuthCert;
	/**
	 * - Read PPK Header, SPK Header, PPK, SPK and SPK Signature from PDI
	 *   and copy to PMCRAM memory.
	 */
	ReadOffset = HBSignParams->ReadOffset;

	/* Read Hash Block 1 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)MetaHdrPtr->HashBlock.HashData,
			HBSignParams->HBSize, 0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_COPY_FAIL, Status);
		goto END;
	}

	/* Read PPK Header */
	ReadOffset += HBSignParams->HBSize;

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
			(u64)(UINTPTR)&SecurePtr->AcPtr->PPK.Header,
			XLOADER_KEY_HDR_SIZE_IN_BYTES,
			0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PPK_HEADER_COPY_FAIL, Status);
		goto END;
	}

	/* Check permission in PPK Header */
	if (SecurePtr->AcPtr->PPK.Header[XLOADER_KEY_HDR_PERMISSION_IDX] !=
			XLOADER_KEY_IS_PPK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INCORRECT_KEY, Status);
		goto END;
	}

	AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);

	/* Check whether hybrid signing is enabled or not in PPK Header */
	if (SecurePtr->AcPtr->PPK.Header[XLOADER_KEY_HDR_HYBRID_IDX] != 0U) {
		IsHybridSign = (u32)TRUE;
		Status = XLoader_ValidateHybridKeyAlgo(AuthType,
				SecurePtr->AcPtr->PPK.Header[XLOADER_KEY_HDR_HYBRID_IDX]);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Hybrid signing requires PQC and Non-PQC "
					"sets of key pair\r\n");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_HYBRID_KEYPAIR, Status);
			goto END;
		}
	}

	Status = XLoader_GetKeySizeSignPadding(AuthType, HBSignParams,
			&SignPaddingSize);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XLoader_AuthHashBlockWithKeyPair(SecurePtr, HBSignParams,
			&ReadOffset, AuthType, SignPaddingSize,
			&HashBlockHash[0]);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (IsHybridSign == (u32)TRUE) {
		/* Read PPK Header */
		ReadOffset += HBSignParams->TotalAuthSignSize;

		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + ReadOffset,
				(u64)(UINTPTR)&SecurePtr->AcPtr->PPK.Header,
				XLOADER_KEY_HDR_SIZE_IN_BYTES,
				0x0U);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PPK_HEADER_COPY_FAIL, Status);
			goto END;
		}

		/* Check permission in PPK Header */
		if (SecurePtr->AcPtr->PPK.Header[XLOADER_KEY_HDR_PERMISSION_IDX] != XLOADER_KEY_IS_PPK) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_INCORRECT_KEY, Status);
			goto END;
		}

		AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);

		Status = XLoader_GetKeySizeSignPadding(AuthType, HBSignParams,
				&SignPaddingSize);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto END;
		}

		Status = XLoader_AuthHashBlockWithKeyPair(SecurePtr, HBSignParams,
				&ReadOffset, AuthType, SignPaddingSize,
				&HashBlockHash[0]);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	XPlmi_Printf(DEBUG_INFO, "HashBlock Authentication is successful\n\r");

	/** - Copy Authenticated HashBlock to PPU1 RAM */
	Status = XLoader_CopyHashBlock(HBSignParams->HBSize, &MetaHdrPtr->HashBlock);

END:
	/** - Zeroize the HashBlockHash buffer used for storing calculated HashBlock hash */
	ClearStatus = XPlmi_MemSetBytes(&HashBlockHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}
#endif /* VERSAL_2VP_P */

/*****************************************************************************/
/**
* @brief	This function encrypts the LMS signature provided and
* 		compares it with expected hash.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	SignBuff is pointer to the signature buffer
* @param	SignatureLen is length of the signature
* @param	KeyAddr is pointer to the LMS public key to be used
* @param	KeyLen is length of the public key to be used
* @param	Data is pointer to the data to be authenticated
* @param	DataLen is length of the data to be authenticated
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_SEC_LMS_SIGN_VERIFY_FAIL if SHA engine start fails
* 		during LMS verification.
* 		- XLOADER_SEC_LMS_PUBKEY_SIZE_VALIDATE_ERR if the public key
* 		length is invalid.
*
******************************************************************************/
int XLoader_VerifyLmsSignature(XLoader_SecureParams *SecurePtr,
	u8 *SignBuff, u32 SignatureLen, u8 *KeyAddr, u32 KeyLen,
	u8 *Data, u32 DataLen)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = NULL;
	XSecure_HssInitParams HssInitParams;
	XSecure_LmsSignVerifyParams LmsSignVerifyParams;
#ifndef VERSAL_2VP_P
	u32 AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
#else
	u32 AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);
#endif

	XPlmi_Printf(DEBUG_INFO, "LMS Authentication\n\r");

	/** - Get SHA3/SHA2 instance */
	if (SecurePtr->SignHashAlgo == XSECURE_SHA2_256) {
		ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
	}
	else if (SecurePtr->SignHashAlgo == XSECURE_SHAKE_256) {
		ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	}

	/** - Set the SHA MODE and start the SHA engine */
	Status = XSecure_ShaStart(ShaInstPtr, SecurePtr->SignHashAlgo);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_LMS_SIGN_VERIFY_FAIL,
                                                                         Status);
		goto END;
	}

	Status = XST_FAILURE;
	if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
		/** - Validate public key length */
		if (KeyLen != XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_LMS_PUBKEY_SIZE_VALIDATE_ERR,
				Status);
			goto END;
		}

		HssInitParams.SignBuff = SignBuff;
		HssInitParams.SignatureLen = SignatureLen;
		HssInitParams.PublicKey = KeyAddr;
		HssInitParams.PublicKeyLen = KeyLen;

		/** - Initiate data authentication using LMS-HSS */
		Status = XST_FAILURE;
		Status = XSecure_HssInit(ShaInstPtr, SecurePtr->PmcDmaInstPtr, &HssInitParams);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Calculate Digest of data to be authenticated as per LMS-HSS */
		Status = XST_FAILURE;
		Status = XSecure_LmsHashMessage(ShaInstPtr,
				Data, DataLen, SecurePtr->SignHashAlgo);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Complete data authentication using LMS-HSS */
		Status = XST_FAILURE;
		Status = XSecure_HssFinish(ShaInstPtr, SecurePtr->PmcDmaInstPtr,
				SignBuff, SignatureLen);
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
		/** - Validate public key length */
		if (KeyLen != XSECURE_LMS_PUB_KEY_TOTAL_SIZE) {
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_LMS_PUBKEY_SIZE_VALIDATE_ERR,
				Status);
			goto END;
		}

		LmsSignVerifyParams.Data = Data;
		LmsSignVerifyParams.DataLen = DataLen;
		LmsSignVerifyParams.PreHashedMsg = FALSE;
		LmsSignVerifyParams.LmsSign = SignBuff;
		LmsSignVerifyParams.LmsSignLen = SignatureLen;
		LmsSignVerifyParams.ExpectedPubKey = KeyAddr;
		LmsSignVerifyParams.PubKeyLen = KeyLen;

		/** - Perform LMS signature verification */
		Status = XST_FAILURE;
		Status = XSecure_LmsSignatureVerification(ShaInstPtr,
				SecurePtr->PmcDmaInstPtr,
				&LmsSignVerifyParams);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate HashBlock partition
 *		with Additional Authenticated Data(AAD)
 *
 * @param	SecurePtr	- Pointer to the XLoader_SecureParams instance
 * @param	HBParams	- Pointer to the XLoader_HBAesParams instance
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_HASH_BLOCK_TAG_COPY_FAIL if HashBlock GCM tag copy fails.
 * 		- XLOADER_SEC_AES_OPERATION_FAILED if AES decrypt init fails.
 * 		- XLOADER_SEC_AES_AAD_OPERATION_FAILED if AES AAD update and validation fails.
 * 		- XLOADER_SEC_BUF_CLEAR_ERR if GCM tag buffer clear fails.
 *
 ******************************************************************************/
static int XLoader_ValidateHashBlockAAD(XLoader_SecureParams *SecurePtr,
				XLoader_HBAesParams *HBParams)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XilPdi_MetaHdr *MetaHdrPtr = SecurePtr->PdiPtr->MetaHdr;
	u8 HashBlockGcmTag[XSECURE_SECURE_GCM_TAG_SIZE] = {0U};
	u8 Iv[XLOADER_SECURE_IV_LEN_IN_BYTES] = {0U};
	u32 TagOffset;

	XPlmi_Printf(DEBUG_INFO, "HashBlock AAD verification \n\r");

	Status = Xil_SecureMemCpy(&Iv, sizeof(Iv), HBParams->IvPtr, sizeof(Iv));
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/** - Increment the IV by 1 for HashBlock AAD verification */
	Xil_IncrementBuffer(Iv, XLOADER_SECURE_IV_LEN_IN_BYTES,
				XLOADER_HASH_BLOCK_IV_INCREMENT_VAL);

	/** - Read HashBlock tag for validation */
	TagOffset = HBParams->HashBlockOffset + HBParams->HashBlockSize;
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr + TagOffset,
			(u64)(UINTPTR)HashBlockGcmTag, XSECURE_SECURE_GCM_TAG_SIZE,
			0x0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_TAG_COPY_FAIL, Status);
		goto END;
	}

	/** - Initialize the AES with IV and key */
	Status = XSecure_AesDecryptInit(SecurePtr->AesInstPtr, HBParams->KeySrc,
		XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_OPERATION_FAILED,
				Status);
		goto END;
	}

	/** - Calculate HashBlock AAD and validate with the tag read */
	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAadAndValidate(SecurePtr->AesInstPtr,
		(UINTPTR)MetaHdrPtr->HashBlock.HashData, HBParams->HashBlockSize,
		(UINTPTR)HashBlockGcmTag);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_AES_AAD_OPERATION_FAILED,
                                Status);
		goto END;
	}

END:
	/** - Zeroize the GcmTag buffer and verify to make sure buffer is cleared */
	ClearStatus = Xil_SecureZeroize(HashBlockGcmTag, XSECURE_SECURE_GCM_TAG_SIZE);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}

#ifndef VERSAL_2VP_P
/******************************************************************************/
/**
 * @brief	This function validates SPK Header
 *
 * @param	SpkHeader - Pointer to the XLoader_SpkHeader
 *
 * @return
 *		XST_SUCCESS - If SpkHeader validation is successful.
 *		XST_FAILURE - Upon Failure.
 *
 ******************************************************************************/
static int XLoader_ValidateSpkHeader(XLoader_SpkHeader *SpkHeader)
{
	volatile int Status = XST_FAILURE;

	/**
	 * - Check that Total SPK Size is in range
	 */
	if ((SpkHeader->TotalSPKSize > XLOADER_MAX_TOTAL_SPK_SIZE) ||
		(SpkHeader->TotalSPKSize < XLOADER_MIN_TOTAL_SPK_SIZE)) {
		goto END;
	}

	/**
	 * - Check that SPK Size is in range
	 */
	if ((SpkHeader->SPKSize > XLOADER_MAX_SPK_SIZE) ||
		(SpkHeader->SPKSize < XLOADER_MIN_SPK_SIZE)) {
		goto END;
	}

	/**
	 * - Check that Total SPK Signature Size is in range
	 */
	if ((SpkHeader->TotalSignatureSize > XLOADER_MAX_TOTAL_SIGN_SIZE) ||
		(SpkHeader->TotalSignatureSize < XLOADER_MIN_TOTAL_SIGN_SIZE)){
		goto END;
	}

	/**
	 * - Check that SPK Signature Size is in range
	 */
	if ((SpkHeader->SignatureSize > XLOADER_MAX_SIGN_SIZE) ||
			(SpkHeader->SignatureSize < XLOADER_MIN_SIGN_SIZE)){
		goto END;
	}

	/**
	 * - If we reach here means, all the checks on SPK HEADER passed
	 */
	XPlmi_Printf(DEBUG_INFO, "All checks PASSED on SPK HEADER\r\n");

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
* @brief	This function runs LMS KAT's
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
* @param	AuthType is public algorithm type
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_ERR_LMS_HSS_GET_DMA if DMA instance acquire fails for HSS variant.
* 		- XLOADER_ERR_LMS_GET_DMA if DMA instance acquire fails for LMS variant.
* 		- XLOADER_SEC_KAT_FAILED_ERROR if any LMS KAT fails.
*
******************************************************************************/
int XLoader_LmsKat(XLoader_SecureParams *SecurePtr, u32 AuthType)
{
	int Status = XST_FAILURE;

	if (SecurePtr->SignHashAlgo == XSECURE_SHAKE_256) {
		/**
		 * - If LMS signature algorithm is SHAKE,
		 * run Shake256 KAT prior and then LMS_HSS or LMS KATs
		 */
		Status = XLoader_Shake256Kat(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
			Status = XLoader_HssShake256Kat(SecurePtr);
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
			Status = XLoader_LmsShake256Kat(SecurePtr);
		}
	}
	else if (SecurePtr->SignHashAlgo == XSECURE_SHA2_256) {
		/**
		 * - If LMS signature algorithm is SHA2-256,
		 * run Sha2-256 KAT prior and then LMS_HSS or LMS KATs
		 */
		Status = XLoader_Sha2256Kat(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
			Status = XLoader_HssSha256Kat(SecurePtr);
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
			Status = XLoader_LmsSha2256Kat(SecurePtr);
		}
	}

END:
	return Status;
}

#endif

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)

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
	XSecure_Sha384Hash Sha3Hash;
	XSecure_Sha *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u64 AcOffset;
	XilPdi_ImgHdrTbl *ImgHdrTbl =
		&SecurePtr->PdiPtr->MetaHdr->ImgHdrTbl;
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
		XPLMI_PMCRAM_CHUNK_MEMORY_1;
#ifdef VERSAL_NET
#ifdef PLM_OCP_NATIVE_KEY_MGMT
	u32* SpkId = XCert_GetSpkId();
#endif
#endif

	XPlmi_Printf(DEBUG_INFO, "Authentication of"
			" Image header table\n\r");

	SecurePtr->AcPtr = AuthCert;

	/** - Copy Authentication certificate from PDI to PMCRAM */
	AcOffset = SecurePtr->PdiPtr->MetaHdr->FlashOfstAddr +
		((u64)(ImgHdrTbl->AcOffset) << XPLMI_WORD_LEN_SHIFT);

	Status = SecurePtr->PdiPtr->MetaHdr->DeviceCopy(AcOffset,
			(UINTPTR)SecurePtr->AcPtr, XLOADER_AUTH_CERT_MIN_SIZE, 0U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IHT_COPY_FAIL,
				Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_ShaDigest(Sha3InstPtr, XSECURE_SHA3_384,
				(UINTPTR)XILPDI_PMCRAM_IHT_COPY_ADDR,
				(ImgHdrTbl->OptionalDataLen << XPLMI_WORD_LEN_SHIFT) + XIH_IHT_LEN,
				(u64)(UINTPTR)&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
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
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES >> XPLMI_WORD_LEN_SHIFT, "IHT Hash");
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Authentication of Image header table is "
			"successful\n\r");

#ifdef VERSAL_NET
#ifdef PLM_OCP_NATIVE_KEY_MGMT
	*SpkId = SecurePtr->AcPtr->SpkId;
#endif
#endif

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

	ClrStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks whether Partition hash is present or not
*               and compares the calculated hash with hash which is present in
*               IHT Optional data for the respective partitions.
*
* @param    PdiPtr is pointer to the xilpdi instance
* @param    HashPtr is pointer to Hash array
* @param    PrtnHashIndex is index of partition hash in IHT optional data
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_SEC_PRTN_HASH_NOT_PRESENT_IN_IHT_OP_DATA_ERR if partition
* 		hash is not present in IHT optional data.
* 		- XLOADER_SEC_PRTN_HASH_COMPARE_FAIL_ERR if partition hash comparison fails.
*
******************************************************************************/
static int XLoader_CheckAndCompareHashFromIHTOptionalData(XilPdi *PdiPtr, u8 *HashPtr, u32 PrtnHashIndex)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XilPdi_PrtnHashInfo* PrtnHashData = NULL;
	XilPdi_PrtnHashInfo* PrtnHashDataTmp = NULL;

	if (PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE) {
		XSECURE_REDUNDANT_CALL(PrtnHashData, PrtnHashDataTmp, XilPdi_IsPrtnHashPresent,
			PrtnHashIndex, PdiPtr->MetaHdr->DigestTableSize);
		if((PrtnHashData == NULL) || (PrtnHashDataTmp == NULL)) {
			Status = XLOADER_SEC_PRTN_HASH_NOT_PRESENT_IN_IHT_OP_DATA_ERR;
			goto END;
		} else {
			/**
			 * - Compare the calculated hash of respective partition with the hash which is
			 * present in IHT Optional data.
			 */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, PrtnHashData->PrtnHash,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES, HashPtr, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = XLOADER_SEC_PRTN_HASH_COMPARE_FAIL_ERR;
				goto END;
			}
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
	XSecure_Sha384Hash Sha3Hash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 AuthCertSize = (SecurePtr->PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE)?
		XLOADER_OPTIMIZED_AUTH_CERT_MIN_SIZE :
		(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);

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
	 * - As SPK and PPK are validated during authentication of IHT,
	 * using the same valid SPK to authenticate IHs and PHs.
	 * Calculate hash on the data
	 */

	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/** - Authentication Certificate */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)SecurePtr->AcPtr,
		AuthCertSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/** - Image headers */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)MetaHdr->ImgHdr,
				(MetaHdr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}

	/** - Partition headers */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)MetaHdr->PrtnHdr,
				(MetaHdr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
						 Status);
		goto END;
	}
	/** - Read hash */
	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL, Status);
		goto END;
	}

	if (SecurePtr->PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE) {
		/** - Skip sign verification, compare the MetaHeader hash from IHT Optional data.
		 *  Passing Partition hash index as zero for MetaHeader.
		 */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckAndCompareHashFromIHTOptionalData,
			SecurePtr->PdiPtr, Sha3Hash.Hash, 0U);
	} else {
		/** - Verify the signature */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
			SecurePtr, Sha3Hash.Hash, &SecurePtr->AcPtr->Spk,
			(u8 *)SecurePtr->AcPtr->ImgSignature);
	}
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status |= StatusTmp;
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AUTH_FAIL, Status);
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)Sha3Hash.Hash,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Headers Hash");
		goto END;
	}

	Status = XST_FAILURE;
	Status = XilPdi_VerifyImgHdrs(MetaHdr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Checksum validation of image headers "
			"failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_INFO, "Authentication of image headers is "
		"successful\n\r");

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
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
static int XLoader_AuthNDecHdrs(XLoader_SecureParams *SecurePtr, XilPdi_MetaHdr *MetaHdr,
		u64 BufferAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	XSecure_Sha384Hash CalHash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 TotalSize = MetaHdr->ImgHdrTbl.TotalHdrLen << XPLMI_WORD_LEN_SHIFT;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
	u32 AuthCertSize = (SecurePtr->PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE)?
		XLOADER_OPTIMIZED_AUTH_CERT_MIN_SIZE :
		(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);

	if ((SecurePtr->IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		TotalSize = TotalSize - XLOADER_AUTH_CERT_MIN_SIZE;
	}

	/** - Authenticate the headers */
	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
				Status);
		goto END;
	}

	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)SecurePtr->AcPtr,
		AuthCertSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)BufferAddr, TotalSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&CalHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_HASH_CALC_FAIL,
							 Status);
		goto END;
	}

	if (SecurePtr->PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE) {
		/** - Skip sign verification, compare the MetaHeader hash from IHT Optional data.
		 *  Passing Partition hash index as zero for MetaHeader.
		 */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckAndCompareHashFromIHTOptionalData,
			SecurePtr->PdiPtr, CalHash.Hash, 0U);
	} else {
		/** - Verify the RSA PSS signature */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_VerifySignature,
			SecurePtr, CalHash.Hash, &SecurePtr->AcPtr->Spk,
			(u8 *)SecurePtr->AcPtr->ImgSignature);
			XPlmi_Printf(DEBUG_INFO, "Authentication of the headers is successful\n\r");
	}
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_AUTH_FAIL, Status);
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)CalHash.Hash,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Headers Hash");
		goto END;
	}

	/** - Decrypt the headers and copy to structures */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DecHdrs, SecurePtr,
			MetaHdr, BufferAddr);

END:
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
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
	ClrStatus = XPlmi_MemSetBytes(&CalHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClrStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#endif
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
static int XLoader_VerifyAuthHashNUpdateNext(XLoader_SecureParams *SecurePtr, u32 Size, u8 Last)
{
	volatile int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	XLoader_HashBlock *HBPtr = XLoader_GetHashBlockInstance();
#else
	XLoader_AuthCertificate *AcPtr=
		(XLoader_AuthCertificate *)SecurePtr->AcPtr;
#if (defined(versal) && defined(PLM_TPM)) || (defined(VERSAL_2VP) && defined(PLM_OCP))
	XSecure_Sha3Hash *PtrnHashTablePtr = XLoader_GetPtrnHashTable();
#endif
#endif
	u8 *Data = (u8 *)SecurePtr->ChunkAddr;
	XSecure_Sha384Hash BlkHash;
	u8 *ExpHash = (u8 *)SecurePtr->Sha3Hash;
	volatile int StatusTmp = XST_FAILURE;

	if (SecurePtr->PmcDmaInstPtr == NULL) {
		goto END;
	}

	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
			Status);
		goto END;
	}

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
	/** - Hash should be calculated on AC + first chunk */
	if (SecurePtr->BlockNum == 0x00U) {

		Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)AcPtr,
			(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
			goto END;
		}
	}

#else
	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}
#endif
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)Data, Size);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&BlkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	/** - Verify the hash */
	if (SecurePtr->BlockNum == 0x00U) {
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
		if (SecurePtr->PdiPtr->PdiType != XLOADER_PDI_TYPE_PARTIAL) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT,
					HBPtr->HashData[SecurePtr->PdiPtr->PrtnNum].PrtnHash,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES, BlkHash.Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		} else {
			/* For a partial PDI in the absence of PLM, the partition number
			 * starts with 0, but in HashBlock at index 0 MetaHeader
			 * hash is present, partition hashes start from index 1
			 * Hence it is always PrtnNum + 1 indicates the corresponding
			 * partition hash in HashBlock
			 */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT,
				HBPtr->HashData[SecurePtr->PdiPtr->PrtnNum + XLOADER_HB_PPDI_PRTN_HASH_IDX_OFFSET].PrtnHash,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES, BlkHash.Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		}

		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_INFO, "Hash mismatch error\n\r");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)BlkHash.Hash,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Calculated Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)HBPtr->HashData[SecurePtr->PdiPtr->PrtnNum].PrtnHash,
					XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Expected Hash");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
					Status);
			goto END;
		}

#else
		if (SecurePtr->PdiPtr->MetaHdr->IsAuthOptimized == (u32)TRUE) {
			/**
			 * - Skip sign verification, compare the hash of respective partition
			 * which uses same keys.
			 */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckAndCompareHashFromIHTOptionalData,
				SecurePtr->PdiPtr, BlkHash.Hash, XLOADER_GET_PRTN_HASH_INDEX(SecurePtr->PdiPtr));
		} else {
			 /** - Authenticate the signature, if same keys are not used. */
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_DataAuth, SecurePtr,
				BlkHash.Hash, (u8 *)SecurePtr->AcPtr->ImgSignature);
		}
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_AUTH_FAIL, Status);
			goto END;
		}
#if (defined(versal) && defined(PLM_TPM)) || (defined(VERSAL_2VP) && defined(PLM_OCP))
		/** Store Hash of the first block of the partition, required for data measurement */
		Status = Xil_SMemCpy(&PtrnHashTablePtr[SecurePtr->PdiPtr->ImagePrtnId].Hash,
				     XSECURE_SHA_384_HASH_SIZE_IN_BYTES, &BlkHash.Hash,
				     XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
#endif
#endif
	}
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, ExpHash,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES, BlkHash.Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_INFO, "Hash mismatch error\n\r");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)BlkHash.Hash,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Calculated Hash");
			XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)ExpHash,
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES / XIH_PRTN_WORD_LEN, "Expected Hash");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
				Status);
			goto END;
		}
	}

	/** - Update the next expected hash and data location */
	if (Last != (u8)TRUE) {
		Status = Xil_SMemCpy(ExpHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
				&Data[Size - XSECURE_SHA_384_HASH_SIZE_IN_BYTES],
				XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/**
		 * - Remove the hash length at end of the chunk
		 */
		SecurePtr->SecureDataLen = Size - XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
	}
	else {
		/* This is the last block */
		SecurePtr->SecureDataLen = Size;
	}
	SecurePtr->SecureData = (UINTPTR)Data;

END:
	ClearStatus = XPlmi_MemSetBytes(&BlkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#if !defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
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
int XLoader_DataAuth(XLoader_SecureParams *SecurePtr, u8 *Hash,
	u8 *Signature)
{
	volatile int Status = XST_FAILURE;
	volatile u32 IsEfuseAuth = (u32)TRUE;
	volatile u32 IsEfuseAuthTmp = (u32)TRUE;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 ReadAuthReg = 0x0U;

	Status = XLoader_AuthKat(SecurePtr);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Auth KAT failed\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
		goto END;
	}

	/** - Check Secure state of device
	 * If A-HWRoT is disabled then BHDR authentication is allowed
	 */
	ReadAuthReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
	Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
		XPLMI_RTCFG_SECURESTATE_AHWROT);
	if (Status != XST_SUCCESS) {
		Status = XLoader_CheckSecureState(ReadAuthReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT);
		if ((Status != XST_SUCCESS) && (SecurePtr->NoLoad != XLOADER_NOLOAD_VAL)) {
			if (ReadAuthReg != SecureStateAHWRoT) {
				Status = XLoader_UpdateMinorErr(
					XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0);
			}
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		else if ((Status != XST_SUCCESS) && (SecurePtr->NoLoad == XLOADER_NOLOAD_VAL)) {
			Status = XST_SUCCESS;
		}
		else {
			IsEfuseAuth = (u32)FALSE;
			IsEfuseAuthTmp = (u32)FALSE;
		}
	}
	else {
		Status = XST_FAILURE;
		IsEfuseAuth = (u32)TRUE;
		IsEfuseAuthTmp = (u32)TRUE;
		/* Validate PPK hash */
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkVerify, SecurePtr, XLOADER_PPK_SIZE);
	}

	/** - Perform SPK Validation */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SpkAuthentication, SecurePtr);

	/** - Check for SPK ID revocation */
	if ((IsEfuseAuth == (u32)TRUE) || (IsEfuseAuthTmp == (u32)TRUE)) {
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
	XSecure_Sha384Hash SpkHash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	volatile int ClearStatus = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "Performing SPK verification\n\r");

	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/**
	 * - Hash the Authentication Header and SPK
	 */
	/** - Update Authentication Header */
	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(ShaInstPtr,(UINTPTR)&SecurePtr->AcPtr->AuthHdr,
		XLOADER_AUTH_HEADER_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	/** - Update SPK */
	Status = XSecure_ShaUpdate(ShaInstPtr, (UINTPTR)&SecurePtr->AcPtr->Spk,
		XLOADER_SPK_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&SpkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
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
	ClearStatus = XPlmi_MemSetBytes(&SpkHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
                        XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
* @brief	This function calculates the invalid mask and PPK offset based on
* 		the selected PPK.
*
* @param	PpkSelect is PPK selection of eFUSE.
* @param	InvalidMask is pointer to store the invalid mask for the selected PPK.
* @param	PpkOffset is pointer to store the PPK eFUSE offset.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_INVALID_PPK_CHOICE_ERR if invalid PPK selection.
*
******************************************************************************/
int XLoader_GetPpkInvalidMaskAndOffset(XLoader_PpkSel PpkSelect,
		u32 *InvalidMask, u32 *PpkOffset)
{
	volatile int Status = XST_FAILURE;

	switch ((u32)PpkSelect) {
		case XLOADER_PPK_SEL_0:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK0_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_1:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK1_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_2:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK2_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#ifndef VERSAL_2VP_P
#ifdef PLM_EN_ADD_PPKS
		case XLOADER_PPK_SEL_3:
			XSECURE_TEMPORAL_CHECK(END, Status, XLoader_IsAdditionalPpkFeatureEnabled);
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK3_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK3_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_4:
			XSECURE_TEMPORAL_CHECK(END, Status, XLoader_IsAdditionalPpkFeatureEnabled);
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK4_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK4_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#endif
#else
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
		case XLOADER_PPK_SEL_5:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK5_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK5_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_6:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK6_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK6_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_7:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK7_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK7_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_8:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK8_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK8_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#endif
		default:
			/** Unintended condition */
			XPLMI_STATUS_GLITCH_DETECT(Status);
			break;
	}

#ifndef VERSAL_2VP_P
#ifdef PLM_EN_ADD_PPKS
END:
#endif
#endif

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if the selected PPK is revoked by reading
*		the PPK invalid bits from eFUSE.
*
* @param	PpkInvldMask is the PPK invalid bit mask to check in eFUSE.
*
* @return
*			- XST_SUCCESS if PPK is not revoked.
*			- XLOADER_SEC_SEL_PPK_REVOKED_ERR if PPK is revoked (invalid bit set).
*			- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
*
******************************************************************************/
int XLoader_IsPpkRevoked(u32 PpkInvldMask)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;

	/** - Check if all PPKs are revoked */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD) ==
		(XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD)) {
		XPlmi_Printf(DEBUG_INFO, "All PPKs are invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_ALL_PPK_REVOKED_ERR, 0x0);
		goto END;
	}

	/** - Check if PPK is revoked by reading the PPK EFUSE invalid bits */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & PpkInvldMask;
	ReadRegTmp = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & PpkInvldMask;
	if ((ReadReg != 0x0U) || (ReadRegTmp != 0x0U)) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SEL_PPK_REVOKED_ERR, 0x0);
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function compares the provided PPK hash with the hash
* 		stored in eFUSE and validates it is not all zeros.
*
* @param	PpkHash is pointer to the PPK hash to be verified.
* @param	PpkOffset is PPK eFUSE offset.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if hash comparison fails.
* 			- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
*
******************************************************************************/
int XLoader_ValidatePpkHash(const u8 *PpkHash, u32 PpkOffset)
{
	volatile int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;
	const u8 HashZeros[XLOADER_EFUSE_PPK_HASH_LEN] = {0U};
#ifdef VERSAL_2VE_2VM
	u32 UserOffset = 0U;
#endif

	/** - Compare provided PPK hash with eFUSE stored hash */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkCompare, PpkOffset, PpkHash);

	Status = XST_FAILURE;
	/** - Check if PPK hash is all zeros */
	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, HashZeros,
			XLOADER_EFUSE_PPK_HASH_LEN, (void *)PpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN, XLOADER_EFUSE_PPK_HASH_LEN);
	if ((HashStatus == XST_SUCCESS) || (HashStatusTmp == XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0);
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}

#ifdef VERSAL_2VE_2VM
	if (PpkOffset == XLOADER_EFUSE_PPK0_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK0_USER_START_OFFSET;
	}
	else if (PpkOffset == XLOADER_EFUSE_PPK1_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK1_USER_START_OFFSET;
	}
	else if (PpkOffset == XLOADER_EFUSE_PPK2_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK2_USER_START_OFFSET;
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	HashStatus = XST_FAILURE;
	/** - Check if upper 128 PPK bits hash is valid or not by reading the User EFUSE bits */
	HashStatus = Xil_SMemCmp_CT((void *)(PpkHash + XLOADER_EFUSE_PPK_HASH_LEN),
				XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN,
				(void *)UserOffset,
                                XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN,
				XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN);
        HashStatusTmp = HashStatus;
        if ((HashStatus != XST_SUCCESS) || (HashStatusTmp != XST_SUCCESS)) {
                XPlmi_Printf(DEBUG_INFO, "Error: PPK Hash - Upper 128 bits comparison failed\r\n");
                Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_COMPARE_FAIL, 0x0);
		goto END;
        }

	Status = XST_FAILURE;
	/** - Check if valid upper 128 bit PPK hash is all zeros */
	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, HashZeros,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN, (void *)UserOffset,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN, XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN);
	if ((HashStatus == XST_SUCCESS) || (HashStatusTmp == XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0);
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}
#endif

END:
	return Status;
}

#if defined(VERSAL_2VE_2VM)
/*****************************************************************************/
/**
* @brief	This function authenticates a client provided Hash Block using
*			the authentication header and signature parameters and, on
*			success, copies the authenticated Hash Block into client memory.
*
* @param	SecurePtr is a pointer to the XLoader_SecureParams instance
*			used to perform the authentication operation.
* @param	HBSignParams is a pointer to the XLoader_HBSignParams instance
*			containing the authentication header and related signature
*			parameters for the Hash Block.
* @param	ClientHBInstance is a pointer to the XLoader_HashBlock instance
*			provided by the client where the authenticated Hash Block is
*			copied on successful verification.
*
* @return
* 		- XST_SUCCESS on successful authentication and copy.
* 		- XLOADER_ERR_PPK_COPY_FAIL if PPK copy fails.
* 		- XLOADER_ERR_GET_LMS_ALGO_FAILED if getting LMS hash algorithm fails.
* 		- XLOADER_ERR_KAT_FAILED if LMS KAT fails.
* 		- XLOADER_ERR_SPK_HEADER_COPY_FAIL if SPK header copy fails.
* 		- XLOADER_ERR_SPK_HEADER_VALIDATE_FAIL if SPK header validation fails.
* 		- XLOADER_ERR_SPK_COPY_FAIL if SPK copy fails.
* 		- XLOADER_ERR_SPK_SIGN_COPY_FAIL if SPK signature copy fails.
* 		- XLOADER_ERR_KEY_AUTH_FAIL if SPK authentication fails.
* 		- XLOADER_ERR_HASH_BLOCK_COPY_FAIL if HashBlock copy fails.
* 		- XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL if HashBlock signature copy fails.
* 		- XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL if HashBlock hash calculation fails.
* 		- XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL if HashBlock signature verification fails.
* 		- XLOADER_SEC_INVALID_AUTH if authentication type is invalid.
* 		- XLOADER_SEC_BUF_CLEAR_ERR if buffer clear fails.
*
******************************************************************************/
int XLoader_AuthenticateClientHashBlock(XLoader_SecureParams *SecurePtr,
        XLoader_HBSignParams *HBSignParams, XLoader_HashBlock *ClientHBInstance)
{
	XilPdi_HashBlock HashBlock; /**< HashBlock to hold partition hashes */
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u8 HashBlockHash[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
	u8 ExpHb0Idx4And5RsvdHashData[XLOADER_ENTRY_SIZE_IN_HASHBLOCK] = {0U};
	XLoader_AuthCertificate *AuthCert = (XLoader_AuthCertificate *)
                XPLMI_PMCRAM_CHUNK_MEMORY_1;

	u32 ReadOffset;
	u32 AuthType;
	u32 NoOfEntries;
	u32 DstIdx;
	u32 SrcIdx;

	XPlmi_Printf(DEBUG_INFO, "Authentication of Client requested HashBlock started...\r\n");

	SecurePtr->AcPtr = AuthCert;
	/**
	 * - Read PPK, SPK Header, SPK and SPK Signature from PDI and
	 * copy to PMCRAM memory.
	 */
	ReadOffset = HBSignParams->ReadOffset;
	Status = Xil_SMemCpy((void *)(UINTPTR)&SecurePtr->AcPtr->Ppk,
			HBSignParams->TotalPpkSize, (void*)(UINTPTR)(ReadOffset),
			HBSignParams->TotalPpkSize, HBSignParams->TotalPpkSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PPK_COPY_FAIL, Status);
		goto END;
	}

	SecurePtr->AcPtr->AuthHdr = HBSignParams->AuthHdr;
	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		/** - Get the LMS Hash algorithm present in public key */
		Status = XSecure_GetLmsHashAlgo(AuthType, (u8 *)&SecurePtr->AcPtr->Ppk, &SecurePtr->SignHashAlgo);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GET_LMS_ALGO_FAILED, Status);
			goto END;
		}
		/** - If AuthType is LMS/LMS_HSS run LMS KAT */
		Status = XLoader_LmsKat(SecurePtr, AuthType);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_KAT_FAILED, Status);
			goto END;
		}
	}

	/** - Read SPK Header */
	ReadOffset += HBSignParams->TotalPpkSize;
	Status = Xil_SMemCpy((void *)(UINTPTR)&SecurePtr->AcPtr->SpkHeader,
			XLOADER_SPK_HEADER_SIZE, (void*)(UINTPTR)(ReadOffset),
			XLOADER_SPK_HEADER_SIZE, XLOADER_SPK_HEADER_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HEADER_COPY_FAIL, Status);
		goto END;
	}

	/** - Validate SPK Header */
	Status = XLoader_ValidateSpkHeader(&SecurePtr->AcPtr->SpkHeader);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_HEADER_VALIDATE_FAIL, Status);
		goto END;
	}

	/** - Read SPK */
	ReadOffset += XLOADER_SPK_HEADER_SIZE;
	Status = Xil_SMemCpy((void *)(UINTPTR)&SecurePtr->AcPtr->Spk,
			SecurePtr->AcPtr->SpkHeader.TotalSPKSize,
			(void*)(UINTPTR)(ReadOffset),
			SecurePtr->AcPtr->SpkHeader.TotalSPKSize,
			SecurePtr->AcPtr->SpkHeader.TotalSPKSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_COPY_FAIL, Status);
		goto END;
	}

	/** - Read SPK Signature */
	ReadOffset += SecurePtr->AcPtr->SpkHeader.TotalSPKSize;
	Status = Xil_SMemCpy((void *)(UINTPTR)&SecurePtr->AcPtr->SPKSignature,
			SecurePtr->AcPtr->SpkHeader.TotalSignatureSize,
			(void*)(UINTPTR)(ReadOffset),
			SecurePtr->AcPtr->SpkHeader.TotalSignatureSize,
			SecurePtr->AcPtr->SpkHeader.TotalSignatureSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SPK_SIGN_COPY_FAIL, Status);
		goto END;
	}

	SecurePtr->AcPtr->SpkId = SecurePtr->AcPtr->SpkHeader.SPKId;

	/**
	 * - Verify PPK Hash
	 * - Authenticate the SPK
	 * - Return error if selected/all PPK revoked
	 * - Compare PPK Hash with efuse stored value (if eFUSE auth)
	 * - Skip PPK Hash comparison for BH auth
	 * - Authenticate SPK using PPK
	 */
	XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_AuthenticateKeys, SecurePtr, HBSignParams);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "SPK Authentication Failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_KEY_AUTH_FAIL, Status);
		goto END;
	}

	/** - Read HashBlock */
	ReadOffset += SecurePtr->AcPtr->SpkHeader.TotalSignatureSize;
	Status = Xil_SMemCpy((void *)&HashBlock.HashData, HBSignParams->HBSize,
			(void*)(UINTPTR)(ReadOffset), HBSignParams->HBSize,
			HBSignParams->HBSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_COPY_FAIL, Status);
		goto END;
	}

	/** - Read HashBlock Signature */
	ReadOffset += HBSignParams->HBSize;
	Status = Xil_SMemCpy((void *)(UINTPTR)&SecurePtr->AcPtr->HBSignature,
			HBSignParams->TotalHBSignSize, (void*)(UINTPTR)(ReadOffset), HBSignParams->TotalHBSignSize, HBSignParams->TotalHBSignSize);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_COPY_FAIL, Status);
		goto END;
	}

	/** - Calculate HashBlock Hash */
	Status = XLoader_ShaDigestCalculation((u8 *)&HashBlock.HashData,
			HBSignParams->HBSize, &HashBlockHash[0]);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_HASH_CALC_FAIL,
                                Status);
		goto END;
	}

	/** - Verify HashBlock Signature with SPK */
	if ((AuthType == XLOADER_PUB_STRENGTH_RSA_4096) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) ||
		(AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifySignature, SecurePtr,
				(u8 *)&HashBlockHash, &SecurePtr->AcPtr->Spk,
				(u8 *)&SecurePtr->AcPtr->HBSignature);
	}
	else if ((AuthType == XLOADER_PUB_STRENGTH_LMS) ||
		(AuthType == XLOADER_PUB_STRENGTH_LMS_HSS)) {
		XSECURE_TEMPORAL_IMPL(Status, SStatus, XLoader_VerifyLmsSignature, SecurePtr,
			(u8 *)&SecurePtr->AcPtr->HBSignature,
			HBSignParams->ActualHBSignSize,
			(u8 *)&SecurePtr->AcPtr->Spk,
			SecurePtr->AcPtr->SpkHeader.SPKSize,
			(u8 *)&HashBlock.HashData,
			HBSignParams->HBSize);
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\r\n");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Verification of HashBlock signature failed %x\r\n", AuthType);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HASH_BLOCK_SIGN_VERIF_FAIL, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Client requested HashBlock Authentication is successful\r\n");

	/** - Copy authenticated HashBlock content to Client requested address */
	NoOfEntries = HBSignParams->HBSize / XLOADER_ENTRY_SIZE_IN_HASHBLOCK;

	for (SrcIdx = 0; SrcIdx < NoOfEntries; SrcIdx++) {
		/** - For HashBlock0, the 5th and 6th partition entries are expected to be zero */
		if ((SrcIdx == XLOADER_HB0_INDEX_4) ||
			(SrcIdx == XLOADER_HB0_INDEX_5)) {
			Status = Xil_SMemCmp((void*)&HashBlock.HashData[SrcIdx],
					XLOADER_ENTRY_SIZE_IN_HASHBLOCK,
					(void*)(UINTPTR)ExpHb0Idx4And5RsvdHashData,
					XLOADER_ENTRY_SIZE_IN_HASHBLOCK,
					XLOADER_ENTRY_SIZE_IN_HASHBLOCK);
			if (Status == XST_SUCCESS) {
				/* Skip copying this index’s expected data intended for HB0 */
				continue;
			}
		}
		DstIdx = HashBlock.HashData[SrcIdx].PrtnNum;
		Status = Xil_SecureMemCpy(&ClientHBInstance->HashData[DstIdx],
				XLOADER_ENTRY_SIZE_IN_HASHBLOCK,
				&HashBlock.HashData[SrcIdx],
				XLOADER_ENTRY_SIZE_IN_HASHBLOCK);
		if(Status != XST_SUCCESS) {
			/* End copy on failure */
			XPlmi_Printf(DEBUG_INFO, "Failed to copy HashBlock to destination, Status = 0x%x\r\n", Status);
			break;
		}
	}

END:
	/** - Zeroize the HashBlockHash buffer used for storing calculated HashBlock hash */
	ClearStatus = XPlmi_MemSetBytes(&HashBlockHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
			XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	else {
		if (Status != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}

	return Status;
}
#endif /* VERSAL_2VE_2VM */
#endif /* END OF PLM_SECURE_EXCLUDE */

/** @} end of xloader_server_apis group */
