/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_secure.h
*
* This file contains all common security related data.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   12/16/20 First release
*       har  01/18/21 Added macros related to P521 KAT
*       kpt  01/21/21 Added macro for revoke id mask
*       har  03/17/21 Moved macros required for Secure state out of
*                     PLM_SECURE_EXCLUDE macro
*       kpt  04/14/21 Added macros required to check encrypted data
*                     alignment
*       bm   05/13/21 Updated code to use common crypto instances from xilsecure
* 1.01  kpt  06/23/21 Added macros required to read and compare DNA
*       kpt  07/01/21 Added macros required to disable Jtag
*       har  07/15/21 Fixed doxygen warnings
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_AUTH_ENC_H
#define XLOADER_AUTH_ENC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xloader.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xsecure_rsa.h"
#include "xsecure_aes.h"
#include "xplmi_util.h"
#include "xil_util.h"
#include "xplmi_hw.h"
#include "xpuf.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XLOADER_RSA_SIG_EXP_BYTE	(0xBCU)
#define XLOADER_RSA_EM_MSB_EXP_BYTE	(0x0U)
#define XLOADER_I2OSP_INT_LIMIT		(256U)
#define XLOADER_RSA_PSS_MASKED_DB_LEN	(463U)
#define XLOADER_RSA_PSS_SALT_LEN	(XLOADER_SHA3_LEN)
#define XLOADER_RSA_PSS_DB_LEN		(415U)
#define XLOADER_RSA_PSS_PADDING1	(8U)
#define XLOADER_RSA_PSS_BUFFER_LEN	(480U)

#define XLOADER_SHA3_KAT_MASK		(0x00000010U)
#define XLOADER_RSA_KAT_MASK		(0x00000020U)
#define XLOADER_ECC_P384_KAT_MASK		(0x00000040U)
#define XLOADER_AES_KAT_MASK		(0x00000080U)
#define XLOADER_DPACM_KAT_MASK		(0x00000100U)
#define XLOADER_ECC_P521_KAT_MASK		(0x00000200U)

#define XLOADER_SPK_SIZE		(XSECURE_RSA_4096_KEY_SIZE + \
						XSECURE_RSA_4096_KEY_SIZE \
						+ 4U +4U)
#define XLOADER_PPK_SIZE		(XSECURE_RSA_4096_KEY_SIZE + \
						XSECURE_RSA_4096_KEY_SIZE \
						+ 4U +12U)
#define XLOADER_SPK_SIG_SIZE		XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_BHDR_SIG_SIZE		XSECURE_RSA_4096_KEY_SIZE
#define XLOADER_PARTITION_SIG_SIZE	XSECURE_RSA_4096_KEY_SIZE

#define XLOADER_AUTH_HEADER_SIZE	(8U)

#define XLOADER_AUTH_CERT_USER_DATA	((u32)64U - XLOADER_AUTH_HEADER_SIZE)

#define XLOADER_AUTH_CERT_MIN_SIZE	(XLOADER_AUTH_HEADER_SIZE \
						+ XLOADER_AUTH_CERT_USER_DATA \
						+ XLOADER_PPK_SIZE  \
						+ XLOADER_SPK_SIZE \
						+ XLOADER_SPK_SIG_SIZE \
						+ 8U \
						+ XLOADER_BHDR_SIG_SIZE \
						+ XLOADER_PARTITION_SIG_SIZE)

#define XLOADER_AC_AH_PUB_STRENGTH_MASK		(0xF0U)
#define XLOADER_AC_AH_REVOKE_ID_MASK		(0xFFU)
#define XLOADER_AC_AH_PUB_STRENGTH_SHIFT	(0x4U)
#define XLOADER_PUB_STRENGTH_ECDSA_P384		(0x0U)
#define XLOADER_PUB_STRENGTH_RSA_4096		(0x1U)
#define XLOADER_PUB_STRENGTH_ECDSA_P521		(0x2U)

#define XLOADER_ECDSA_P384_KEYSIZE		(48U)
#define XLOADER_ECDSA_P521_KEYSIZE		(66U)
#define XLOADER_ECDSA_MAX_KEYSIZE		XLOADER_ECDSA_P521_KEYSIZE

#define XLOADER_SECURE_HDR_SIZE			(48U)/**< Secure Header Size in Bytes*/
#define XLOADER_SECURE_GCM_TAG_SIZE		(16U) /**< GCM Tag Size in Bytes */
#define XLOADER_SECURE_HDR_TOTAL_SIZE		(XLOADER_SECURE_HDR_SIZE + \
							XLOADER_SECURE_GCM_TAG_SIZE)
#define XLOADER_SECURE_METAHDR_RD_IMG_PRTN_HDRS (0x0U)
#define XLOADER_SECURE_METAHDR_RD_IMG_HDRS      (0x1U)
#define XLOADER_SECURE_METAHDR_RD_PRTN_HDRS     (0x2U)

#define XLOADER_128_BIT_ALIGNED_MASK            (0x0FU)

/* AES key source */
#define XLOADER_EFUSE_KEY		(0xA5C3C5A3U) /* eFuse Key */
#define XLOADER_EFUSE_BLK_KEY		(0xA5C3C5A5U) /* eFUSE Black Key */

#define XLOADER_BBRAM_KEY		(0x3A5C3C5AU) /* BBRAM Key */
#define XLOADER_BBRAM_BLK_KEY		(0x3A5C3C59U) /* BBRAM Black Key */

#define XLOADER_BH_BLK_KEY		(0xA35C7C53U) /*Boot Header Black Key */

#define XLOADER_EFUSE_USR_KEY0		(0x5C3CA5A3U) /* eFuse User Key 0 */
#define XLOADER_EFUSE_USR_BLK_KEY0	(0x5C3CA5A5U) /* eFUSE User key 0 Black */

#define XLOADER_EFUSE_USR_KEY1		(0xC3A5C5A3U) /* eFuse User Key 1 */
#define XLOADER_EFUSE_USR_BLK_KEY1	(0xC3A5C5A5U) /* eFUSE User key 1 Black */

#define XLOADER_USR_KEY0		(0xC5C3A5A3U) /* User Key 0 */
#define XLOADER_USR_KEY1		(0xC3A5C5B3U) /* User Key 1 */
#define XLOADER_USR_KEY2		(0xC5C3A5C3U) /* User Key 2 */
#define XLOADER_USR_KEY3		(0xC3A5C5D3U) /* User Key 3 */
#define XLOADER_USR_KEY4		(0xC5C3A5E3U) /* User Key 4 */
#define XLOADER_USR_KEY5		(0xC3A5C5F3U) /* User Key 5 */
#define XLOADER_USR_KEY6		(0xC5C3A563U) /* User Key 6 */
#define XLOADER_USR_KEY7		(0xC3A5C573U) /* User Key 7 */

/* Efuse addresses */
#define XLOADER_EFUSE_MISC_CTRL_OFFSET			(0xF12500A0U)
#define XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD		(0x0000000CU)
#define XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD		(0x00000030U)
#define XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD		(0x000000C0U)
#define XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD		(0x000000FCU)

#define XLOADER_EFUSE_PPK0_START_OFFSET			(0xF1250100U)
#define XLOADER_EFUSE_PPK1_START_OFFSET			(0xF1250120U)
#define XLOADER_EFUSE_PPK2_START_OFFSET			(0xF1250140U)
#define XLOADER_EFUSE_PPK2_END_OFFSET			(0xF125015CU)
#define XLOADER_EFUSE_PPK_HASH_LEN			(32U)

#define XLOADER_SECURE_IV_LEN				(4U)
#define XLOADER_SECURE_IV_NUM_ROWS			(3U)
#define XLOADER_EFUSE_IV_METAHDR_START_OFFSET		(0xF1250180U)
#define XLOADER_EFUSE_IV_METAHDR_END_OFFSET		(0xF1250188U)
#define XLOADER_EFUSE_IV_BLACK_OBFUS_START_OFFSET	(0xF12501D0U)
#define XLOADER_EFUSE_IV_BLACK_OBFUS_END_OFFSET		(0xF12501D8U)

#define XLOADER_EFUSE_REVOCATION_ID_0_OFFSET		(0xF12500B0U)
#define XLOADER_EFUSE_REVOCATION_ID_7_OFFSET		(0xF12500CCU)

#define XLOADER_EFUSE_SEC_MISC1_OFFSET			(0xF12500E8U)
#define XLOADER_EFUSE_SEC_DPA_DIS_MASK			(0xFFFF0000U)

#define XLOADER_EFUSE_DNA_START_OFFSET			(0xF1250020U)
#define XLOADER_EFUSE_DNA_NUM_ROWS			(4U)

#define XLOADER_EFUSE_DNA_LEN_IN_BYTES			(XLOADER_EFUSE_DNA_NUM_ROWS * \
							sizeof(u32))

#define XLOADER_AC_AH_DNA_MASK				(0x03U)

#define XLOADER_REVOCATION_IDMAX			(0xFFU)

#define XLOADER_PUF_HD_BHDR				(0x3U)

/* KEK key decryption status */
#define XLOADER_BBRAM_RED_KEY				(0x00000001U)
#define XLOADER_BHDR_RED_KEY				(0x00000002U)
#define XLOADER_EFUSE_RED_KEY				(0x00000004U)
#define XLOADER_EFUSE_USR0_RED_KEY			(0x00000008U)
#define XLOADER_EFUSE_USR1_RED_KEY			(0x00000010U)

#define XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET	(0xF12500ACU)
#define XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET		(0xF11B0030U)
#define XLOADER_PMC_TAP_DAP_CFG_OFFSET			(0xF11B0008U)
#define XLOADER_PMC_TAP_INST_MASK_0_OFFSET		(0xF11B0000U)
#define XLOADER_PMC_TAP_INST_MASK_1_OFFSET		(0xF11B0004U)
#define XLOADER_PMC_TAP_DAP_SECURITY_OFFSET		(0xF11B000CU)
#define XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET	(0xF11B0018U)
#define XLOADER_CRP_RST_DBG_OFFSET			(0xF1260400U)

#define XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK	(0x1U)
#define XLOADER_AUTH_JTAG_DIS_MASK			(0x180000U)
#define XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS		(512U)
#define XLOADER_AUTH_JTAG_DATA_AH_LENGTH		(104U)
#define XLOADER_AUTH_JTAG_MAX_ATTEMPTS			(1U)
#define XLOADER_AUTH_FAIL_COUNTER_RST_VALUE		(0U)

#define XLOADER_AUTH_JTAG_PADDING_SIZE			(18U)
#define XLOADER_AUTH_JTAG_SHA_PADDING_SIZE		(3U)
#define XLOADER_ENABLE_AUTH_JTAG_SIGNATURE_SIZE		(226U)

#define XLOADER_DAP_SECURITY_GATE_DISABLE_MASK		(0xFFFFFFFFU)
#define XLOADER_DAP_CFG_SPNIDEN_MASK			(0x1U)
#define XLOADER_DAP_CFG_SPIDEN_MASK			(0x2U)
#define XLOADER_DAP_CFG_NIDEN_MASK			(0x4U)
#define XLOADER_DAP_CFG_DBGEN_MASK			(0x8U)
#define XLOADER_DAP_CFG_ENABLE_ALL_DBG_MASK		(XLOADER_DAP_CFG_SPNIDEN_MASK | \
							XLOADER_DAP_CFG_SPIDEN_MASK | \
							XLOADER_DAP_CFG_NIDEN_MASK |  \
							XLOADER_DAP_CFG_DBGEN_MASK)
#define XLOADER_PMC_TAP_INST_MASK_ENABLE_MASK		(0U)
#define XLOADER_CRP_RST_DBG_ENABLE_MASK			(0U)

#define XLOADER_PMC_TAP_INST_DISABLE_MASK_0		(0x3DFFF8FDU)
#define XLOADER_PMC_TAP_INST_DISABLE_MASK_1		(0x05DBFF8FU)
#define XLOADER_CRP_RST_DBG_DPC_MASK            (0x00000002U)
#define XLOADER_CRP_RST_DBG_RESET_MASK          (0x00000001U)

#define XLOADER_DAP_TIMEOUT_DISABLED			(2U)

#define XLOADER_PDI_DPACM_ENABLED			(1U)
#define XLOADER_PDI_DPACM_DISABLED			(0U)

#define EFUSE_CACHE_MISC_CTRL				(0xF12500A0U)
#define EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK	(0X00008000U)

#define XLOADER_KAT_DONE				(0x000001F0U)

/**************************** Type Definitions *******************************/
/**< RSA Key */
typedef struct {
	u32 PubModulus[128U];	/**< Public Modulus */
	u32 PubModulusExt[128U];	/**< Public Modulus Extension */
	u32 PubExponent;	/**< Public Exponent */
} XLoader_RsaKey;

/**< Authentication Certificate */
typedef struct {
	u32 AuthHdr;	/**< Authentication Header */
	u32 SpkId;	/**< SPK ID */
	u32 UserData[14U];	/**< User data */
	XLoader_RsaKey Ppk;	/**< PPK */
	u32 PPKPadding[3U];	/**< PPK padding */
	XLoader_RsaKey Spk;	/**< SPK */
	u32 SPKPadding;		/**< SPK padding */
	u32 Alignment1[2U];	/**< Alignment gap */
	u32 SPKSignature[128U];	/**< SPK signature */
	u32 BHSignature[128U];	/**< Bootheader signature */
	u32 ImgSignature[128U];	/**< Image signature */
} XLoader_AuthCertificate;

/**< Authentication Type */
typedef enum {
	XLOADER_ECDSA,	/**< 0x0 - ECDSA */
	XLOADER_RSA		/**< 0x1 - RSA */
} XLoader_AuthType;

/**< PPK selection type */
typedef enum {
	XLOADER_PPK_SEL_0,	/**< 0 - PPK 0 */
	XLOADER_PPK_SEL_1,	/**< 1 - PPK 1 */
	XLOADER_PPK_SEL_2	/**< 2 - PPK 2 */
} XLoader_PpkSel;

/**< RSA signature vars */
typedef struct
{
	u8 EmHash[48];	/**< EM hash */
	u8 Salt[48];	/**< Salt */
	u8 Padding1[8];	/**< Padding 1 */
} XLoader_Vars;

/**< KEK info */
typedef struct {
	u32 PdiKeySrc;	/**< PDI Key Source */
	u64 KekIvAddr;	/**< KEK IV address */
	u32 PufHdLocation;	/**< PUF helper data location */
	XSecure_AesKeySrc KeySrc;	/**< Source key source */
	XSecure_AesKeySrc KeyDst;	/**< Destination key source */
} XLoader_AesKekInfo;

/**< Authenticated Message structure */
typedef struct {
	u32 AuthHdr;	/**< Authentication Header */
	u32 RevocationIdMsgType;	/**< Revocation ID */
	u32 Attrb;	/**< Attributes */
	u32 Dna[XLOADER_EFUSE_DNA_NUM_ROWS];	/**< DNA */
	u32 JtagEnableTimeout;	/**< JTAG enable timeout */
	u32 AuthJtagPadding[XLOADER_AUTH_JTAG_PADDING_SIZE];
				/**< SHA padding for Auth JTAG signature */
	XLoader_RsaKey PpkData;		/**< PPK */
	u32 AuthJtagPpkShaPadding[XLOADER_AUTH_JTAG_SHA_PADDING_SIZE];
				/**< SHA padding for PPK */
	u32 EnableJtagSignature[XLOADER_ENABLE_AUTH_JTAG_SIGNATURE_SIZE];
				/**< Auth JTAG signature */
} XLoader_AuthJtagMessage;
#endif

typedef struct {
	volatile u8 SecureEn;	/**< Security enabled or disabled */
	volatile u8 SecureEnTmp;	/**< Security enabled or disabled (temp)*/
	u8 IsNextChunkCopyStarted;	/**< Next chunk copy started or not */
	u8 IsCheckSumEnabled;	/**< Checksum enabled or disabled */
	u8 IsDoubleBuffering;	/**< Double buffering enabled or disabled */
	u8 IsCdo; /**< CDO or Elf */
	XilPdi *PdiPtr;		/**< PDI pointer */
	XilPdi_PrtnHdr *PrtnHdr;/**< Partition header */
	u64 NextBlkAddr;	/**< Next block address */
	u32 ChunkAddr;		/**< Chunk address */
	u32 NextChunkAddr;	/**< Next chunk address */
	/* Verified data is at */
	u32 SecureData;		/**< Secure data */
	u32 SecureDataLen;	/**< Secure data length */
	u32 ProcessedLen;	/**< Processed data length */
	u32 RemainingDataLen;	/**< Remaining data length */
	u32 RemainingEncLen;	/**< Remaining encrypted data length */
	u32 BlockNum;		/**< Block number */
	u32 Sha3Hash[XLOADER_SHA3_LEN / 4U];	/**< SHA3 hash */
	u32 SecureHdrLen;	/**< Secure header length */
	XPmcDma *PmcDmaInstPtr;	/**< PMC DMA instance pointer */
#ifndef PLM_SECURE_EXCLUDE
	XLoader_AuthType SigType;	/**< Signature type */
	XLoader_AuthCertificate *AcPtr;/**< Authentication certificate pointer */
	XSecure_Aes *AesInstPtr;	/**< AES instance pointer */
	XLoader_AuthJtagMessage* AuthJtagMessagePtr;
					/**< Auth JTAG message pointer */
	u8 IsEncrypted;		/**< Encryption enabled or disabled */
	u8 IsEncryptedTmp;	/**< Encryption enabled or disabled (temp)*/
	u8 IsAuthenticated;	/**< Authentication enabled or disabled */
	u8 IsAuthenticatedTmp;	/**<  enabled or disabled */
#endif
} XLoader_SecureParams;

#ifndef PLM_SECURE_EXCLUDE
typedef enum {
	XLOADER_SEC_AUTH_EN_PPK_HASH_NONZERO = 0x02,
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
	XLOADER_SEC_ENC_ONLY_KEYSRC_ERR,
	        /**< 0x1D Keysrc should be efuse black key for enc only */
	XLOADER_SEC_ENC_ONLY_PUFHD_LOC_ERR,
			/**< 0x1E PUFHD location should be from eFuse for enc only */
	XLOADER_SEC_METAHDR_IV_ZERO_ERR,
	        /**< 0x1F eFuse IV should be non-zero for enc only */
	XLOADER_SEC_BLACK_IV_ZERO_ERR,
			 /**< 0x20 eFuse IV should be non-zero for enc only */
	XLOADER_SEC_IV_METAHDR_RANGE_ERROR,
		   /**< 0x21 Metahdr IV Range not matched with eFuse IV */
	XLOADER_SEC_EFUSE_DPA_CM_MISMATCH_ERROR,
		/**< 0x22 Metahdr DpaCm & eFuse DpaCm values are not matched */
	XLOADER_SEC_RSA_MEMSET_SHA3_ARRAY_FAIL,
		/**< 0x23 Error during memset for XSecure_RsaSha3Array */
	XLOADER_SEC_RSA_MEMSET_VARSCOM_FAIL,
		/**< 0x24 Error during memset for Xsecure_Varsocm */
	XLOADER_SEC_MASKED_DB_MSB_ERROR,
		/**< 0x25 Error in RSA EM MSB */
	XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR,
		/**< 0x26 Failed to verify DB check */
	XLOADER_SEC_MEMSET_ERROR,
		/**< 0x27 Error during XPlmi_MemSetBytes */
	XLOADER_SEC_GLITCH_DETECTED_ERROR,
		/**<0x28 Error glitch detected */
	XLOADER_SEC_ENC_DATA_NOT_ALIGNED_ERROR,
		/**<0x29 Error encrypted data is not 128 bit aligned */
} XLoader_SecErrCodes;

/*****************************************************************************/
/**
 * @brief
 * This function updates security related minor error codes for Xilloader
 *
 * @param        Minor1  To specify the cause of failure of
 *                                       security operation
 * @param        Minor2  Libraries / Drivers error code as defined in respective
 *                                       modules
 * @note         If MSB of Minor1 is set then error in clearing of security
 *                       buffer  (16th bit)
 *                       If bit next to MSB is set then security buffer was successfully
 *                       cleared (15th bit)
 *                       For eg: 0x02 : Incorrect authentication type selected
 *                       0x42 : Incorrect authentication type selected and
 *                               buffer was successfully cleared
 *                       0x82 : Incorrect authentication type selected and
 *                               error in clearing buffer
 ******************************************************************************/
static inline int XLoader_UpdateMinorErr(XLoader_SecErrCodes Minor1, int Minor2)
{
	u32 UMinor1 = (u32)Minor1;
	u32 UMinor2 = (u32)Minor2;

	UMinor1 = (UMinor1 << 8U) | UMinor2;

	return (int)UMinor1;
}

/***************************** Function Prototypes ***************************/
int XLoader_ImgHdrTblAuth(XLoader_SecureParams *SecurePtr);
int XLoader_ReadAndVerifySecureHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr);
int XLoader_SecureValidations(const XLoader_SecureParams *SecurePtr);
void XLoader_UpdateKekSrc(XilPdi *PdiPtr);
int XLoader_AddAuthJtagToScheduler(void);
int XLoader_SecureAuthInit(XLoader_SecureParams *SecurePtr,
	const XilPdi_PrtnHdr *PrtnHdr);
int XLoader_SecureEncInit(XLoader_SecureParams *SecurePtr,
	const XilPdi_PrtnHdr *PrtnHdr);
void XLoader_AuthEncClear(void);
int XLoader_UpdateKatStatus(XilPdi *PdiPtr);
int XLoader_ProcessAuthEncPrtn(XLoader_SecureParams *SecurePtr, u64 DestAddr,
	u32 BlockSize, u8 Last);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_AUTH_ENC_H */
