/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_auth_enc.h
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
*       har  07/18/21 Added description for all macros
*       bsv  08/17/21 Code clean up
*       kpt  09/02/21 Added support to update KAT status in RTC area
*       kpt  09/09/21 Fixed SW-BP-BLIND-WRITE in XLoader_AuthEncClear
*       kpt  09/15/21 Added error code XLOADER_PUF_HD_EFUSE
*       kpt  09/18/21 Updated macro value XLOADER_PDI_DPACM_ENABLED
*                     Renamed BHSignature variable to IHTSignature
* 1.02  kpt  10/04/21 Removed macro XLOADER_SEC_ALL_IDS_REVOKED_ERR
*       kpt  10/07/21 Added function pointer ProcessPrtn in
*                     XLoader_SecureParams
*       kpt  10/20/21 Removed temporal variables from XLoader_SecureParams
*       bsv  10/26/21 Code clean up
*       kpt  10/28/21 Added DmaFlags in XLoader_SecureParams
*       bsv  02/11/22 Code clean up to reduce size
*       bsv  02/13/22 Reduce stack usage of functions
*       har  02/17/22 Added macro XLOADER_AUTH_JTAG_LOCK_DIS_MASK and removed
*                     macro XLOADER_AUTH_FAIL_COUNTER_RST_VALUE
*       bsv  03/18/22 Fix build issues when PLM_SECURE_EXCLUDE is enabled
* 1.03  bm   07/06/22 Refactor versal and versal_net code
*       kpt  07/05/2022 Added support to update KAT status
*       ma   07/08/22 Removed EFUSE_CACHE_MISC_CTRL as it is defined in xplmi_hw.h
*       kpt  07/24/22 Added XLoader_RsaPssSignVerify to support KAT for versal net
*       har  11/17/22 Added function declaration for XLoader_CheckNonZeroPpk
* 1.04  ng   11/23/22 Fixed doxygen file name error
* 1.8   skg  12/07/22 Added Additional PPKs related macros and enums
*       kal  01/05/23 Added XLoader_GetAuthPubAlgo definition
*       sk   02/08/23 Renamed XLoader_UpdateKatStatus to XLoader_ClearKatOnPPDI
*       dd   03/28/23 Updated doxygen comments
*       sk   07/06/23 Corrected DAP Config Mask's
*       dd   08/11/23 Updated doxygen comments
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
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xsecure_aes.h"
#include "xplmi_util.h"
#include "xil_util.h"
#include "xplmi_hw.h"
#include "xplmi_tamper.h"
#include "xpuf.h"
#include "xloader_plat_secure.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
/**
 * @name  RSA PSS Padding
 * @{
 */
/**< Macro definitions related to RSA PSS padding */
#define XLOADER_RSA_SIG_EXP_BYTE	(0xBCU)
#define XLOADER_RSA_EM_MSB_EXP_BYTE	(0x0U)
#define XLOADER_I2OSP_INT_LIMIT		(256U)
#define XLOADER_RSA_PSS_MASKED_DB_LEN	(463U)
#define XLOADER_RSA_PSS_SALT_LEN	(XLOADER_SHA3_LEN)
#define XLOADER_RSA_PSS_DB_LEN		(415U)
#define XLOADER_RSA_PSS_PADDING1	(8U)
#define XLOADER_RSA_PSS_BUFFER_LEN	(480U)
/** @} */

/**
 * @name  Masks for KAT status
 * @{
 */
/**< Masks are used to determine if KAT for the respective crypto hardware
 * has already been run or not.
 */
#define XLOADER_PPDI_KAT_MASK		(0x03U) /**< PPDI KAT mask */

/** @} */
#define XLOADER_RSA_4096_KEY_SIZE	(4096U/8U) /**< RSA 4096 key size */

#define XLOADER_SPK_SIZE		(XLOADER_RSA_4096_KEY_SIZE + \
						XLOADER_RSA_4096_KEY_SIZE \
						+ 4U +4U)
/**< Size of Secondary Public Key(in bytes) in Authentication Certificate */
#define XLOADER_PPK_SIZE		(XLOADER_RSA_4096_KEY_SIZE + \
						XLOADER_RSA_4096_KEY_SIZE \
						+ 4U +12U)
/**< Size of Primary Public Key(in bytes) in Authentication Certificate */
#define XLOADER_SPK_SIG_SIZE		XLOADER_RSA_4096_KEY_SIZE
/**< Size of SPK signature(in bytes) in Authentication Certificate */
#define XLOADER_BHDR_SIG_SIZE		XLOADER_RSA_4096_KEY_SIZE
/**< Size of Bootheader signature(in bytes) in Authentication Certificate */
#define XLOADER_PARTITION_SIG_SIZE	XLOADER_RSA_4096_KEY_SIZE
/**< Size of Partition signature(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_HEADER_SIZE	(8U)
/**< Size of Authentication header(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_CERT_USER_DATA	((u32)64U - XLOADER_AUTH_HEADER_SIZE)
	/**< Size of User Data(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_CERT_MIN_SIZE	(XLOADER_AUTH_HEADER_SIZE \
						+ XLOADER_AUTH_CERT_USER_DATA \
						+ XLOADER_PPK_SIZE  \
						+ XLOADER_SPK_SIZE \
						+ XLOADER_SPK_SIG_SIZE \
						+ 8U \
						+ XLOADER_BHDR_SIG_SIZE \
						+ XLOADER_PARTITION_SIG_SIZE)
		/**< Minimum Size of Authentication Certificate(in bytes) */

#define XLOADER_AC_AH_PUB_STRENGTH_MASK		(0xF0U)
		/**< Mask for Public Strength in Authentication Certificate */
#define XLOADER_AC_AH_REVOKE_ID_MASK		(0xFFU)
		/**< Mask for Revocation ID in Authentication Certificate */
#define XLOADER_AC_AH_PUB_STRENGTH_SHIFT	(0x4U)
		/**< Shift for Public Strength in Authentication Certificate */
#define XLOADER_PUB_STRENGTH_ECDSA_P384		(0x0U)
	/**< Value of ECDSA P-384 as Public Strength in Authentication Certificate */
#define XLOADER_PUB_STRENGTH_RSA_4096		(0x1U)
	/**< Value of RSA 4096 as Public Strength in Authentication Certificate */
#define XLOADER_PUB_STRENGTH_ECDSA_P521		(0x2U)
	/**< Value of ECDSA P-521 as Public Strength in Authentication Certificate */

#define XLOADER_ECDSA_P384_KEYSIZE		(48U)
			/**< Key size(in bytes) for ECDSA P-384 curve */
#define XLOADER_ECDSA_P521_KEYSIZE		(66U)
			/**< Key size(in bytes) for ECDSA P-521 curve */
#define XLOADER_ECDSA_MAX_KEYSIZE		XLOADER_ECDSA_P521_KEYSIZE
			/**< ECDSA Max Key size(in bytes) */

#define XLOADER_SECURE_HDR_SIZE			(48U)
			/**< Secure Header Size(in bytes) */
#define XLOADER_SECURE_GCM_TAG_SIZE		(16U)
			/**< GCM Tag Size(in bytes) */
#define XLOADER_SECURE_HDR_TOTAL_SIZE		(XLOADER_SECURE_HDR_SIZE + \
							XLOADER_SECURE_GCM_TAG_SIZE)
			/**< Total size of Secure Header (in bytes) */

#define XLOADER_128_BIT_ALIGNED_MASK		(0x0FU)
			/**< Mask to check if data is 128-bit aligned */

/**< AES key source */
#define XLOADER_EFUSE_KEY		(0xA5C3C5A3U)
						/**< eFUSE Key */
#define XLOADER_EFUSE_BLK_KEY		(0xA5C3C5A5U)
						/**< eFUSE Black Key */
#define XLOADER_BBRAM_KEY		(0x3A5C3C5AU)
						/**< BBRAM Key */
#define XLOADER_BBRAM_BLK_KEY		(0x3A5C3C59U)
						/**< BBRAM Black Key */
#define XLOADER_BH_BLK_KEY		(0xA35C7C53U)
						/**< Boot Header Black Key */
#define XLOADER_EFUSE_USR_KEY0		(0x5C3CA5A3U)
						/**< eFUSE User Key 0 */
#define XLOADER_EFUSE_USR_BLK_KEY0	(0x5C3CA5A5U)
						/**< eFUSE User key 0 Black */
#define XLOADER_EFUSE_USR_KEY1		(0xC3A5C5A3U)
						/**< eFUSE User Key 1 */
#define XLOADER_EFUSE_USR_BLK_KEY1	(0xC3A5C5A5U)
						/**< eFUSE User key 1 Black */

#define XLOADER_USR_KEY0		(0xC5C3A5A3U)
						/**< User Key 0 */
#define XLOADER_USR_KEY1		(0xC3A5C5B3U)
						/**< User Key 1 */
#define XLOADER_USR_KEY2		(0xC5C3A5C3U)
						/**< User Key 2 */
#define XLOADER_USR_KEY3		(0xC3A5C5D3U)
						/**< User Key 3 */
#define XLOADER_USR_KEY4		(0xC5C3A5E3U)
						/**< User Key 4 */
#define XLOADER_USR_KEY5		(0xC3A5C5F3U)
						/**< User Key 5 */
#define XLOADER_USR_KEY6		(0xC5C3A563U)
						/**< User Key 6 */
#define XLOADER_USR_KEY7		(0xC3A5C573U)
						/**< User Key 7 */

/**< eFUSE related macro definitions */
					/**< Misc Ctrl register address */
#define XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD		(0x0000000CU)
					/**< PPK0 invalid value */
#define XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD		(0x00000030U)
					/**< PPK1 invalid value */
#define XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD		(0x000000C0U)
					/**< PPK2 invalid value */
#define XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD		(0x000000FCU)
					/**< All PPKs invalid value */

#define XLOADER_EFUSE_PPK_HASH_LEN			(32U)
					/**< PPK hash length stored in eFUSE */

#define XLOADER_SECURE_IV_LEN				(4U)
				/**< Secure IV length in words */
#define XLOADER_SECURE_IV_NUM_ROWS			(3U)
				/**< No. of eFUSE rows for Secure IV */
#define XLOADER_EFUSE_IV_METAHDR_START_OFFSET		(0xF1250180U)
				/**< Metaheader IV start register address */
#define XLOADER_EFUSE_IV_METAHDR_END_OFFSET		(0xF1250188U)
				/**< Metaheader IV end register address */
#define XLOADER_EFUSE_IV_BLACK_OBFUS_START_OFFSET	(0xF12501D0U)
				/**< Black IV start register address */
#define XLOADER_EFUSE_IV_BLACK_OBFUS_END_OFFSET		(0xF12501D8U)
				/**< Black IV start register address */

#define XLOADER_EFUSE_REVOCATION_ID_0_OFFSET		(0xF12500B0U)
				/**< Revocation ID 0 register address */
#define XLOADER_EFUSE_REVOCATION_ID_7_OFFSET		(0xF12500CCU)
				/**< Revocation ID 7 register address */

#define XLOADER_EFUSE_SEC_MISC1_OFFSET			(0xF12500E8U)
				/**< Security Misc1 register address */
#define XLOADER_EFUSE_SEC_DPA_DIS_MASK			(0xFFFF0000U)
				/**< DPA CM disabled mask */

#define XLOADER_EFUSE_DNA_START_OFFSET			(0xF1250020U)
				/**< DNA start register address */
#define XLOADER_EFUSE_DNA_NUM_ROWS			(4U)
				/**< Number of eFUSE rows for DNA */

#define XLOADER_EFUSE_DNA_LEN_IN_BYTES			(XLOADER_EFUSE_DNA_NUM_ROWS * \
							sizeof(u32))
				/**< Size of DNA(in bytes) */

#define XLOADER_AC_AH_DNA_MASK				(0x03U)
			/**< Mask for DNA in Authentication Certificate */

#define XLOADER_REVOCATION_IDMAX			(0xFFU)
			/**< Maximum value of Revocation ID */

#define XLOADER_PUF_HD_BHDR				(0x3U)
			/**< Value of PUF HD stored in bootheader */
#define XLOADER_PUF_HD_EFUSE				(0x0U)
			/**< Value of PUF HD stored in efuse */


/**< KEK key decryption status */
#define XLOADER_BBRAM_RED_KEY				(0x00000001U)
			/**< Decrypted key stored in BBRAM */
#define XLOADER_BHDR_RED_KEY				(0x00000002U)
			/**< Decrypted key stored in Bootheader */
#define XLOADER_EFUSE_RED_KEY				(0x00000004U)
			/**< Decrypted key stored in eFUSE AES key */
#define XLOADER_EFUSE_USR0_RED_KEY			(0x00000008U)
			/**< Decrypted key stored in eFUSE User 0 key */
#define XLOADER_EFUSE_USR1_RED_KEY			(0x00000010U)
			/**< Decrypted key stored in eFUSE User 1 key */

#define XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET	(0xF12500ACU)
				/**< Security Control register address */
#define XLOADER_PMC_TAP_AUTH_JTAG_DATA_OFFSET		(0xF11B0030U)
			/**< Authenticated JTAG Data start register address */
#define XLOADER_PMC_TAP_DAP_CFG_OFFSET			(0xF11B0008U)
				/**< DAP CFG register address */
#define XLOADER_PMC_TAP_INST_MASK_0_OFFSET		(0xF11B0000U)
				/**< Instruction Mask 0 register address */
#define XLOADER_PMC_TAP_INST_MASK_1_OFFSET		(0xF11B0004U)
				/**< Instruction Mask 1 register address */
#define XLOADER_PMC_TAP_DAP_SECURITY_OFFSET		(0xF11B000CU)
				/**< DAP security register address */
#define XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_OFFSET	(0xF11B0018U)
		/**< Authenticated JTAG interrupt status register address */
#define XLOADER_CRP_RST_DBG_OFFSET			(0xF1260400U)
				/**< CRP reset debug register address */

#define XLOADER_PMC_TAP_AUTH_JTAG_INT_STATUS_MASK	(0x1U)
			/**< Mask for Authenticated JTAG interrupt status */
#define XLOADER_AUTH_JTAG_DIS_MASK			(0x180000U)
			/**< Mask for disabling Authenticated JTAG */
#define XLOADER_AUTH_JTAG_LOCK_DIS_MASK			(0x600000U)
	/**< Mask for disabling Authenticated JTAG after secure lockdown */
#define XLOADER_AUTH_JTAG_DATA_LEN_IN_WORDS		(512U)
			/**< Authenticated JTAG data length(in words) */
#define XLOADER_AUTH_JTAG_DATA_AH_LENGTH		(104U)
	/**< Length of Authentication Header in Authenticated JTAG message */
#define XLOADER_AUTH_JTAG_MAX_ATTEMPTS			(1U)
		/**< Maximum allowed attempts to authenticate JTAG message */

#define XLOADER_AUTH_JTAG_PADDING_SIZE			(18U)
			/**< Authenticated JTAG padding size */
#define XLOADER_AUTH_JTAG_SHA_PADDING_SIZE		(3U)
			/**< Authenticated SHA padding size */
#define XLOADER_ENABLE_AUTH_JTAG_SIGNATURE_SIZE		(226U)
			/**< Authenticated JTAG signature size */
#define XLOADER_CONFIG_DAP_STATE_SECURE_DBG		(0x01U)
			/**< DAP State enable secure Debug */
#define XLOADER_CONFIG_DAP_STATE_NONSECURE_DBG		(0x02U)
			/**< DAP State enable non-secure Debug */
#define XLOADER_CONFIG_DAP_STATE_ALL_DBG		(0x03U)
			/**< DAP State enable all Debug modes */
#define XLOADER_DAP_SECURITY_GATE_DISABLE_MASK		(0xFFFFFFFFU)
			/**< MAsk to disable DAP security gate */
#define XLOADER_DAP_CFG_SPNIDEN_MASK			(0x8U)
			/**< Mask to enable secure non-invasive debug */
#define XLOADER_DAP_CFG_SPIDEN_MASK			(0x4U)
			/**< Mask to enable secure invasive debug */
#define XLOADER_DAP_CFG_NIDEN_MASK			(0x2U)
			/**< Mask to enable non-secure non-invasive debug */
#define XLOADER_DAP_CFG_DBGEN_MASK			(0x1U)
			/**< Mask to enable non-secure invasive debug */
#define XLOADER_DAP_CFG_ENABLE_ALL_DBG_MASK		(XLOADER_DAP_CFG_SPNIDEN_MASK | \
							XLOADER_DAP_CFG_SPIDEN_MASK | \
							XLOADER_DAP_CFG_NIDEN_MASK |  \
							XLOADER_DAP_CFG_DBGEN_MASK)
			/**< Mask to enable all types of debug */
#define XLOADER_PMC_TAP_INST_MASK_ENABLE_MASK		(0U)
		/**< Value to mask all instructions for Instruction mask 0/1 register */
#define XLOADER_CRP_RST_DBG_ENABLE_MASK			(0U)
			/**< Mask to enable debug for CRP_RST */

#define XLOADER_PMC_TAP_INST_DISABLE_MASK_0		(0x3DFFF8FDU)
		/**< Value to unmask instructions for Instruction mask 0 register */
#define XLOADER_PMC_TAP_INST_DISABLE_MASK_1		(0x05DBFF8FU)
		/**< Value to unmask instructions for Instruction mask 1 register */
#define XLOADER_CRP_RST_DBG_DPC_MASK            (0x00000002U)
				/**< Value to reset DPC within the PMC only */
#define XLOADER_CRP_RST_DBG_RESET_MASK          (0x00000001U)
				/**< Value to reset all debug in the LPD/FPD */

#define XLOADER_DAP_TIMEOUT_DISABLED			(2U)
				/**< Timeout disabled for DAP */

#define XLOADER_PDI_DPACM_ENABLED			(0x3U)
				/**< DPA counter measures are enabled in PDI */
#define XLOADER_PDI_DPACM_DISABLED			(0U)
				/**< DPA counter measures are disabled in PDI */

				/**< Misc Control register address */
#define EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK	(0X00008000U)
			/**< Mask to enable running of KAT for Crypto engines */

#define XLOADER_KAT_DONE				(0x000005F0U)
			/**< Value to indicate that KAT is done */

#define XLOADER_WORD_IN_BITS					(32U)
						/**< Word length in bits */
#define XLOADER_WORD_IN_BITS_SHIFT		(5U) /**< Value to shift word */
#define XLOADER_WORD_IN_BITS_MASK		(0x1FU) /**< Value to mask word */

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
	u32 IHTSignature[128U];	/**< Image Header Table signature */
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
	XLOADER_PPK_SEL_2,	/**< 2 - PPK 2 */
#ifdef PLM_EN_ADD_PPKS
	XLOADER_PPK_SEL_3,	/**< 3 - PPK 3 */
	XLOADER_PPK_SEL_4	/**< 4 - PPK 4 */
#endif
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

typedef struct XLoader_SecureParams {
	volatile u8 SecureEn;	/**< Security enabled or disabled */
	u8 IsNextChunkCopyStarted;	/**< Next chunk copy started or not */
	u8 IsCheckSumEnabled;	/**< Checksum enabled or disabled */
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
	int (*ProcessPrtn)(struct XLoader_SecureParams *SecurePtr, u64 DestAddr,
				u32 BlockSize, u8 Last); /**< Function pointer to process
				                          * partition chunk */
	u16 DmaFlags;    /**< Flags indicate mode of copying */
#ifndef PLM_SECURE_EXCLUDE
	XLoader_AuthType SigType;	/**< Signature type */
	XLoader_AuthCertificate *AcPtr;/**< Authentication certificate pointer */
	XSecure_Aes *AesInstPtr;	/**< AES instance pointer */
	XLoader_AuthJtagMessage* AuthJtagMessagePtr;
					/**< Auth JTAG message pointer */
	u8 IsEncrypted;		/**< Encryption enabled or disabled */
	u8 IsAuthenticated;	/**< Authentication enabled or disabled */
#endif
} XLoader_SecureParams;

/* To reduce stack usage, instances of XLoader_AuthCertificate and XPufData
 * and arrays named RsaSha3Array and Buffer are moved to this structure
 * which resides at XPLMI_PMC_CHUNK_MEMORY_1.
 */
#ifndef PLM_SECURE_EXCLUDE
typedef struct {
	XLoader_AuthCertificate AuthCert; /**< Authentication certificate */
	u8 RsaSha3Array[XLOADER_RSA_4096_KEY_SIZE]; /**< RSA Sha3 array */
	u8 Buffer[XLOADER_RSA_PSS_BUFFER_LEN] __attribute__ ((aligned(32U))); /**< Buffer */
	XPuf_Data PufData; /**< Puf data */
} XLoader_StoreSecureData;

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

/*****************************************************************************/
/**
* @brief	This function returns the public algorithm used for authentication
*
* @param	AuthHdrPtr is a pointer to the Authentication header of the AC.
*
* @return	- XLOADER_PUB_STRENGTH_ECDSA_P384
*		- XLOADER_PUB_STRENGTH_RSA_4096
*		- XLOADER_PUB_STRENGTH_ECDSA_P521
*
******************************************************************************/
static INLINE u32 XLoader_GetAuthPubAlgo(const u32 *AuthHdrPtr)
{
	return ((*AuthHdrPtr & XLOADER_AC_AH_PUB_STRENGTH_MASK) >>
		XLOADER_AC_AH_PUB_STRENGTH_SHIFT);
}

/***************************** Function Prototypes ***************************/
int XLoader_ImgHdrTblAuth(XLoader_SecureParams *SecurePtr);
int XLoader_ReadAndVerifySecureHdrs(XLoader_SecureParams *SecurePtr,
	XilPdi_MetaHdr *MetaHdr);
int XLoader_SecureValidations(const XLoader_SecureParams *SecurePtr);
int XLoader_AddAuthJtagToScheduler(void);
int XLoader_SecureAuthInit(XLoader_SecureParams *SecurePtr,
	const XilPdi_PrtnHdr *PrtnHdr);
int XLoader_SecureEncInit(XLoader_SecureParams *SecurePtr,
	const XilPdi_PrtnHdr *PrtnHdr);
int XLoader_AuthEncClear(void);
int XLoader_ProcessAuthEncPrtn(XLoader_SecureParams *SecurePtr, u64 DestAddr,
	u32 BlockSize, u8 Last);
#ifndef PLM_RSA_EXCLUDE
int XLoader_RsaPssSignVerify(XPmcDma *PmcDmaInstPtr,
		u8 *MsgHash, XSecure_Rsa *RsaInstPtr, u8 *Signature);
#endif
void XLoader_ClearKatOnPPDI(XilPdi *PdiPtr, u32 PlmKatMask);
int XLoader_CheckAuthJtagIntStatus(void *Arg);
int XLoader_IsPpkValid(XLoader_PpkSel PpkSelect, const u8 *PpkHash);
int XLoader_IsAdditionalPpkValid(const u8 *PpkHash);
int XLoader_AdditionalPpkSelect(XLoader_PpkSel PpkSelect, u32 *InvalidMask, u32 *PpkOffset);
#endif
int XLoader_CheckSecureStateAuth(volatile u32* AHWRoT);

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_AUTH_ENC_H */
