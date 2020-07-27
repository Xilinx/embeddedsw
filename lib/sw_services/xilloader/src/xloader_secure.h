/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_secure.h
*
* This file contains all security related data.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vns  04/23/19 First release
* 1.01  vns  07/09/19 Added PPK and SPK integrity checks
*                     Updated chunk size for secure partition
*                     Added encryption + authentication support
*       vns  07/23/19 Added functions to load secure headers
*       vns  08/23/19 Added buffer cleaning on failure
*                     Added different key sources support
* 1.02  vns  02/23/20 Added DPA CM enable/disable functionality
*       har  02/28/20 Added minor error codes for security
*       vns  03/01/20 Added PUF KEK decrypt support
*       ana  04/02/20 Added crypto engine KAT test function calls
*       bsv  04/07/20 Change CSUDMA name to PMCDMA
*       vns  04/13/20 Moved Aes instance to Secure structure
* 1.03  ana  06/04/20 Removed XLOADER_ECDSA_INDEXVAL macro and
*                     updated u32 datatype to u8 datatype
*       tar  07/23/20 Fixed MISRA-C required violations
*
* </pre>
*
* @note
*
******************************************************************************/


#ifndef XLOADER_SECURE_H
#define XLOADER_SECURE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_ecdsa.h"
#include "xsecure_sha.h"
#include "xsecure_rsa.h"
#include "xsecure_aes.h"
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_util.h"
#include "xpuf.h"

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief
* This macro updates security related minor error codes for Xilloader
*
* @param	Minor1	To specify the cause of failure of
*					security operation
* @param	Minor2	Libraries / Drivers error code as defined in respective
*					modules
* @note		If MSB of Minor1 is set then error in clearing of security
*			buffer	(16th bit)
*			If bit next to MSB is set then security buffer was successfully
*			cleared (15th bit)
*			For eg: 0x02 : Incorrect authentication type selected
*			0x42 : Incorrect authentication type selected and
*				buffer was successfully cleared
*			0x82 : Incorrect authentication type selected and
*	 			error in clearing buffer
******************************************************************************/
#define XLOADER_UPDATE_MIN_ERR(Minor1, Minor2)		\
						((Minor1<<8U) | (Minor2))
/************************** Constant Definitions *****************************/
#define XLOADER_SHA3_LEN				(48U)
#define XLOADER_RSA_SIG_EXP_BYTE		(0xBCU)
#define XLOADER_I2OSP_INT_LIMIT			(256U)
#define XLOADER_RSA_PSS_MASKED_DB_LEN	(463U)
#define XLOADER_RSA_PSS_SALT_LEN		(XLOADER_SHA3_LEN)
#define XLOADER_RSA_PSS_DB_LEN			(415U)
#define XLOADER_RSA_PSS_PADDING1		(8U)
#define XLOADER_RSA_PSS_BUFFER_LEN		(480U)

#define XLOADER_SHA3_KAT_MASK			(0x00000010U)
#define XLOADER_RSA_KAT_MASK			(0x00000020U)
#define XLOADER_ECDSA_KAT_MASK			(0x00000040U)
#define XLOADER_AES_KAT_MASK			(0x00000080U)
#define XLOADER_DPACM_KAT_MASK			(0x00000100U)

#define XLOADER_SPK_SIZE				(XSECURE_RSA_4096_KEY_SIZE + \
										 XSECURE_RSA_4096_KEY_SIZE \
										 + 4U +4U)
#define XLOADER_PPK_SIZE				(XSECURE_RSA_4096_KEY_SIZE + \
										 XSECURE_RSA_4096_KEY_SIZE \
										 + 4U +12U)
#define XLOADER_PPK_MOD_SIZE			XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_PPK_MOD_EXT_SIZE		XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_SPK_MOD_SIZE			XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_SPK_MOD_EXT_SIZE		XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_SPK_SIG_SIZE			XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_BHDR_SIG_SIZE			XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_PARTITION_SIG_SIZE		XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_RSA_AC_ALIGN			(64U)
#define XLOADER_SPKID_AC_ALIGN			(4U)

#define XLOADER_AUTH_HEADER_SIZE		(8U)

#define XLOADER_AUTH_CERT_USER_DATA		((u32)64U - XLOADER_AUTH_HEADER_SIZE)

#define XLOADER_AUTH_CERT_MIN_SIZE		(XLOADER_AUTH_HEADER_SIZE \
										+ XLOADER_AUTH_CERT_USER_DATA \
										+ XLOADER_PPK_SIZE  \
										+ XLOADER_SPK_SIZE \
										+ XLOADER_SPK_SIG_SIZE \
										+ 8U \
										+ XLOADER_BHDR_SIG_SIZE \
										+ XLOADER_PARTITION_SIG_SIZE)

#define XLOADER_AC_AH_PUB_ALG_MASK		(0x3U)
#define XLOADER_AC_AH_PUB_ALG_RSA		(0x1U)
#define XLOADER_AC_AH_PUB_ALG_ECDSA		(0x2U)

#define XLOADER_ECDSA_KEYSIZE			(0x0000000CU)

#define XLOADER_SECURE_HDR_SIZE			(48U)/**< Secure Header Size in Bytes*/
#define XLOADER_SECURE_GCM_TAG_SIZE		(16U) /**< GCM Tag Size in Bytes */
#define XLOADER_SECURE_HDR_TOTAL_SIZE	\
					(XLOADER_SECURE_HDR_SIZE + XLOADER_SECURE_GCM_TAG_SIZE)
#define XLOADER_SECURE_IV_LEN			(4U)

/* AES key source */
#define XLOADER_UNENCRYPTED			(0x00000000U) /* Unencrypted */
#define XLOADER_EFUSE_KEY			(0xA5C3C5A3U) /* eFuse Key */
#define XLOADER_EFUSE_BLK_KEY		(0xA5C3C5A5U) /* eFUSE Black Key */
#define XLOADER_EFUSE_OBFUS_KEY		(0xA5C3C5A7U) /* eFuse Obfuscated Key */

#define XLOADER_BBRAM_KEY			(0x3A5C3C5AU) /* BBRAM Key */
#define XLOADER_BBRAM_BLK_KEY		(0x3A5C3C59U) /* BBRAM Black Key */
#define XLOADER_BBRAM_OBFUS_KEY		(0x3A5C3C57U) /* BBRAM Obfuscated Key */

#define XLOADER_BH_BLK_KEY			(0xA35C7C53U) /*Boot Header Black Key */
#define XLOADER_BH_OBFUS_KEY		(0xA35C7CA5U)
									/* Boot Header Obfuscated Key */

#define XLOADER_EFUSE_USR_KEY0		(0x5C3CA5A3U) /* eFuse User Key 0 */
#define XLOADER_EFUSE_USR_BLK_KEY0	(0x5C3CA5A5U) /* eFUSE User key 0 Black */
#define XLOADER_EFUSE_USR_OBFUS_KEY0 (0x5C3CA5A7U) /* eFuse User key 0
													* Obfuscated */
#define XLOADER_EFUSE_USR_KEY1		(0xC3A5C5A3U) /* eFuse User Key 1 */
#define XLOADER_EFUSE_USR_BLK_KEY1	(0xC3A5C5A5U) /* eFUSE User key 1 Black */
#define XLOADER_EFUSE_USR_OBFUS_KEY1 (0xC3A5C5A7U) /* eFuse User key 1
													* Obfuscated */
#define XLOADER_USR_KEY0			(0xC5C3A5A3U) /* User Key 0 */
#define XLOADER_USR_KEY1			(0xC3A5C5B3U) /* User Key 1 */
#define XLOADER_USR_KEY2			(0xC5C3A5C3U) /* User Key 2 */
#define XLOADER_USR_KEY3			(0xC3A5C5D3U) /* User Key 3 */
#define XLOADER_USR_KEY4			(0xC5C3A5E3U) /* User Key 4 */
#define XLOADER_USR_KEY5			(0xC3A5C5F3U) /* User Key 5 */
#define XLOADER_USR_KEY6			(0xC5C3A563U) /* User Key 6 */
#define XLOADER_USR_KEY7			(0xC3A5C573U) /* User Key 7 */

/* Efuse addresses */
#define XLOADER_EFUSE_MISC_CTRL_OFFSET			(0xF12500A0U)
#define XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD		(0x0000000CU)
#define XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD		(0x00000030U)
#define XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD		(0x000000C0U)
#define XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD	(0x000000FCU)

#define XLOADER_EFUSE_PPK0_START_OFFSET			(0xF1250100U)
#define XLOADER_EFUSE_PPK1_START_OFFSET			(0xF1250120U)
#define XLOADER_EFUSE_PPK2_START_OFFSET			(0xF1250140U)
#define XLOADER_EFUSE_PPK2_END_OFFSET			(0xF125015CU)
#define XLOADER_EFUSE_PPK_HASH_LEN				(32U)

#define XLOADER_EFUSE_REVOCATION_ID_0_OFFSET	(0xF12500B0U)
#define XLOADER_EFUSE_REVOCATION_ID_1_OFFSET	(0xF12500B4U)
#define XLOADER_EFUSE_REVOCATION_ID_2_OFFSET	(0xF12500B8U)
#define XLOADER_EFUSE_REVOCATION_ID_3_OFFSET	(0xF12500BCU)
#define XLOADER_EFUSE_REVOCATION_ID_4_OFFSET	(0xF12500C0U)
#define XLOADER_EFUSE_REVOCATION_ID_5_OFFSET	(0xF12500C4U)
#define XLOADER_EFUSE_REVOCATION_ID_6_OFFSET	(0xF12500C8U)
#define XLOADER_EFUSE_REVOCATION_ID_7_OFFSET	(0xF12500CCU)

#define XLOADER_EFUSE_SEC_MISC0_OFFSET			(0xF12500E4U)
#define XLOADER_EFUSE_SEC_MISC1_OFFSET			(0xF12500E8U)

#define XLOADER_EFUSE_SEC_DPA_DIS_MASK			(0xFFFF0000U)
#define XLOADER_EFUSE_SEC_DEC_MASK				(0x0000FFFFU)

#define XLOADER_REVOCATION_IDMAX				(0xFFU)

#define XLOADER_WORD_IN_BITS					(32U)

#define XLOADER_PUF_HD_BHDR						(0x3U)

#define XLOADER_SEC_BUF_CLEAR_ERR		(XLOADER_SEC_ERR_BUF_CLR_FAILED << 8U)
#define XLOADER_SEC_BUF_CLEAR_SUCCESS	(XLOADER_SEC_ERR_BUF_CLR_SUCCESS << 8U)

/* KEK key decryption status */
#define XLOADER_BBRAM_RED_KEY					(0x00000001U)
#define XLOADER_BHDR_RED_KEY					(0x00000002U)
#define XLOADER_EFUSE_RED_KEY					(0x00000004U)
#define XLOADER_EFUSE_USR0_RED_KEY				(0x00000008U)
#define XLOADER_EFUSE_USR1_RED_KEY				(0x00000010U)

/**************************** Type Definitions *******************************/

typedef struct {
	u32 PubModulus[128U];
	u32 PubModulusExt[128U];
	u32 PubExponent;
}XLoader_RsaKey;

typedef struct {
	u32 AuthHdr;
	u32 SpkId;
	u32 UserData[14U];
	XLoader_RsaKey Ppk;
	u32 PPKPadding[3U];
	XLoader_RsaKey Spk;
	u32 SPKPadding;
	u32 Alignment1[2U];
	u32 SPKSignature[128U];
	u32 BHSignature[128U];
	u32 ImgSignature[128U];
}XLoader_AuthCertificate;

typedef enum {
	XLOADER_ECDSA,	/**< 0x0 - ECDSA */
	XLOADER_RSA		/**< 0x1 - RSA */
}XLoader_AuthType;

typedef enum {
	XLOADER_PPK_SEL_0,	/**< 0 - PPK 0 */
	XLOADER_PPK_SEL_1,	/**< 1 - PPK 1 */
	XLOADER_PPK_SEL_2	/**< 2 - PPK 2 */
}XLoader_PpkSel;

typedef struct
{
	u8 EmHash[48];	/**< EM hash */
	u8 Salt[48];	/**< Salt */
	u8 Padding1[8];	/**< Padding 1 */
}XLoader_Vars;

typedef struct {
	u32 PdiKeySrc;
	u64 KekIvAddr;
	u32 PufHdLocation;
	XSecure_AesKekType KekType;
	XSecure_AesKeySrc KeySrc;
	XSecure_AesKeySrc KeyDst;
}XLoader_AesKekKey;

typedef struct {
	u8 SecureEn;
	u8 IsNextChunkCopyStarted;
	u8 IsCheckSumEnabled;
	u8 IsEncrypted;
	u8 IsAuthenticated;
	u8 IsDoubleBuffering;
	XLoader_AuthType SigType;
	XilPdi *PdiPtr;
	XilPdi_PrtnHdr *PrtnHdr;
	u8 IsCdo; /**< CDO or Elf */
	u32 NextBlkAddr;
	u32 ChunkAddr;
	/* verified data is at */
	u32 SecureData;
	u32 SecureDataLen;
	u32 RemainingEncLen;
	u32 BlockNum;
	u32 Sha3Hash[XLOADER_SHA3_LEN / 4U];
	XLoader_AuthCertificate *AcPtr;
	XPmcDma *PmcDmaInstPtr;
	XSecure_Aes AesInstance;
}XLoader_SecureParms;

typedef enum {
	XLOADER_SEC_AUTH_EN_PPK_HASH_NONZERO = 0x02U,
			/**< 0x02 Incorrect Authentication type selected */
	XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			/**< 0x03 PPK Hash calculation failed */
	XLOADER_SEC_ALL_PPK_REVOKED_ERR,
			/**< 0x04 All PPKs are revoked */
	XLOADER_SEC_PPK_INVALID_BIT_ERR,
			/**< 0x05 PPK Invalid bit is set */
	XLOADER_SEC_PPK_HASH_ALLZERO_INVLD,
			/**< 0x06 PPK HAsh is all zero hence inavalid */
	XLOADER_SEC_PPK_HASH_COMPARE_FAIL,
			/**< 0x07 HAsh comparison failed */
	XLOADER_SEC_ALL_PPK_INVALID_ERR,
			/**< 0x08 All PPKs are invalid */
	XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			/**< 0x09 SPK HAsh calculation failed */
	XLOADER_SEC_RSA_AUTH_FAIL,
			/**< 0x0A RSA signature is not verified */
	XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
			/**< 0x0B RSA Pss signature verification failed */
	XLOADER_SEC_ECDSA_AUTH_FAIL,
			/**< 0x0C ECDSA signature is not verified */
	XLOADER_SEC_ECDSA_INVLD_KEY_COORDINATES,
			/**< 0x0D ECDSA invalid key coordinates */
	XLOADER_SEC_INVALID_AUTH,
			/**< 0x0E Only RSA and ECDSA are supported */
	XLOADER_SEC_ALL_IDS_REVOKED_ERR,
			/**< 0x0F All revocation IDs are invalid */
	XLOADER_SEC_REVOCATION_ID_OUTOFRANGE_ERR,
			/**< 0x10 Revocation ID is out of range */
	XLOADER_SEC_ID_REVOKED,
			/**< 0x11 Revocation ID range not verified */
	XLOADER_SEC_BLACK_KEY_DEC_ERR,
			/**< 0x12 Black key decryption error */
	XLOADER_SEC_OBFUS_KEY_DEC_ERR,
			/**< 0x13 Obfuscated key decryption error */
	XLOADER_SEC_DEC_INVALID_KEYSRC_SEL,
			/**< 0x14 Invalid key source selected for decryption */
	XLOADER_SEC_DATA_LEFT_FOR_DECRYPT_ERR,
			/**< 0x15 Data still remaining for decryption */
	XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH,
			/**< 0x16 Size mismatch for data remaining for
				 decryption */
	XLOADER_SEC_AES_OPERATION_FAILED,
			/**< 0x17 AES Operation failed */
	XLOADER_SEC_DPA_CM_ERR,
			/**< 0x18 DPA CM Cfg Error */
	XLOADER_SEC_PUF_REGN_ERRR,
			/**< 0x19 PUF regeneration error */
	XLOADER_SEC_AES_KEK_DEC,
			/**< 0x1A AES KEK decryption */
	XLOADER_SEC_RSA_PSS_ENC_BC_VALUE_NOT_MATCHED,
			/**< 0x1B RSA ENC 0xbc value is not matched */
	XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE,
			/**< 0x1C RSA PSS verification hash is not matched */

	/* In case of failure of any security operation, the buffer must be
	 * cleared.In case of success/failure in clearing the buffer,
	 * the following error codes shall be updated in the status
	 */
	XLOADER_SEC_ERR_BUF_CLR_SUCCESS = 0x40U,
			/* Buffer is successfully cleared */
	XLOADER_SEC_ERR_BUF_CLR_FAILED = 0x80U,
			/* Error in clearing buffer */
}XLoader_SecErrCodes;

/***************************** Function Prototypes ***************************/
u32 XLoader_SecureInit(XLoader_SecureParms *SecurePtr, XilPdi *PdiPtr,
	u32 PrtnNum);
u32 XLoader_ProcessSecurePrtn(XLoader_SecureParms *SecurePtr, u64 DstAddr,
	u32 Size, u8 Last);
u32 XLoader_SecureCopy(XLoader_SecureParms *SecurePtr, u64 DestAddr, u32 Size);
u32 XLoader_ImgHdrTblAuth(XLoader_SecureParms *SecurePtr);
u32 XLoader_ReadAndVerifySecureHdrs(XLoader_SecureParms *SecurePtr,
	XilPdi_MetaHdr *ImgHdrTbl);
u32 XLoader_SecureValidations(XLoader_SecureParms *SecurePtr);
void XLoader_UpdateKekRdKeyStatus(XilPdi *PdiPtr);
u32 XLoader_StartNextChunkCopy(XLoader_SecureParms *SecurePtr, u32 TotalLen,
	u32 ChunkLen);

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_SECURE_H */
