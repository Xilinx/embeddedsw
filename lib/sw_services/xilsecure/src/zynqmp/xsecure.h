/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure.h
*
* This is the header file which contains secure library interface function prototype
* for authentication and decryption of images.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  dp   02/15/17 Initial release
* 2.2   vns  09/18/17 Added APIs to support generic functionality
*                     for SHA3 and RSA hardware at linux level.
* 3.0   vns  02/19/18 Added error codes and macros for secure image.
*
* 3.1   ka   04/10/18 Added support for user-efuse revocation
*       ka   04/18/18 Added support for Zeroization of the memory in case of
*                     Gcm-Tag mismatch
* 3.2   ka   08/03/18 Added XSecure_Aes Api's to encrypt or decrypt data-blobs.
* 4.0   arc  18/12/18 Fixed MISRA-C violation.
*       arc  12/02/19 Added support for validate image format.
*       rama 18/03/19 Fixed IAR compiler errors and warnings
*       psl  03/26/19 Fixed MISRA-C violation
* 4.1   psl  07/31/19 Fixed MISRA-C violation.
* 4.2   kal  03/12/20 Authenticate SizeofImgHdr before use, in case of failure
*                     return XSECURE_IMAGE_HEADER_SIZE_ERR.
*       ana  04/03/20 Removed support of storing key in global array
*       rpo  04/09/20 Aligned buffers used by DMA to 64 bytes
*		rpo	 09/10/20 Added a new error for RSA input validation parameter
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_H
#define XSECURE_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Include Files ***********************************/

#include "xcsudma.h"
#include "xsecure_aes.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"

/************************** Constant Definitions *****************************/

#define XSECURE_AES		1U
#define XSECURE_RSA		2U
#define XSECURE_RSA_AES		3U

#define XSECURE_SHA3_INIT	1U
#define XSECURE_SHA3HASH_UPDATE	2U
#define XSECURE_SHA3_FINAL	4U
#define XSECURE_SHA3_MASK	(XSECURE_SHA3_INIT | \
				XSECURE_SHA3HASH_UPDATE | XSECURE_SHA3_FINAL)

#define XSECURE_SPKID_EFUSE				1U /**< SPK ID eFUSE */
#define XSECURE_USER_EFUSE				2U /**< User eFuse */
#define XSECURE_USER_EFUSE_MIN_VALUE	1U
#define XSECURE_USER_EFUSE_MAX_VALUE	256U

#define XSECURE_ENC		1U
#define XSECURE_DEC		0U
#define XSECURE_AES_KUP_KEY	0U

#define XSECURE_RSA_CORE_OPERATION	1U
#define XSECURE_RSA_KEY_SELECT  2U

#define XSECURE_MASK		(XSECURE_AES | XSECURE_RSA)

#define XSECURE_KEY_STR_LEN		64U	/* String length */
#define XSECURE_IV_STR_LEN		24U	/* String length */
#define XSECURE_KEY_LEN			8U
#define XSECURE_IV_LEN			3U
#define XSECURE_GCM_TAG_LEN		128U
#define XSECURE_WORD_LEN		4U
#define XSECURE_MAX_NIBBLES		8U

#define XSECURE_WORD_SHIFT		32U

#define XSECURE_ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define XSECURE_ERROR_CSUDMA_INIT_FAIL	0x1U
#define XSECURE_STRING_INVALID_ERROR	0x2U
#define XSECURE_INVALID_FLAG		0x3U
#define XSECURE_ISNOT_SECURE_IMAGE	0x4U
#define XSECURE_SHA3_INIT_FAIL		0x5U
#define XSECURE_SIZE_ERR		0x6U

#define XSECURE_SEL_ERR			0x7U
#define XSECURE_REVOKE_ERR		0x8U
#define XSECURE_VERIFY_ERR		0x9U
#define XSECURE_RSA_INIT_ERR		0xAU
#define XSECURE_RSA_ENCRYPT_ERR		0xBU
#define XSECURE_SHA3_PADSELECT_ERR	0xCU
#define XSECURE_IMAGE_WITH_MUL_PARTITIONS 0xDU
#define XSECURE_AUTH_ISCOMPULSORY	0xEU
#define XSECURE_ENC_ISCOMPULSORY	0xFU

#define XSECURE_BHDR_AUTH_NOT_ALLOWED	0x10U
#define	XSECURE_ONLY_BHDR_AUTH_ALLOWED	0x11U
#define XSECURE_HDR_NOAUTH_PART_AUTH	0x12U
#define XSECURE_DEC_WRONG_KEY_SOURCE	0x13U
#define XSECURE_KUP_KEY_NOT_PROVIDED	0x14U
#define XSECURE_KUP_KEY_NOT_REQUIRED	0x15U
#define XSECURE_INVALID_EFUSE_SELECT 	0x17U
#define XSECURE_OUT_OF_RANGE_USER_EFUSE_ERROR	0x18U
#define XSECURE_INVALID_IMAGE_ERROR    0x19U
#define XSECURE_SHA3_UPDATE_FAIL        0x20U
#define XSECURE_IMAGE_HEADER_SIZE_ERR	0x21U

#define XSECURE_AES_ERROR		0x80U
#define XSECURE_RSA_INVALID_PARAM_RESERVED 0x82U
#define XSECURE_AUTH_NOT_ENABLED 	0xFFU

#define XSECURE_PPK_ERR			0x100U
#define XSECURE_SPK_ERR			0x200U
#define XSECURE_AUTH_FAILURE		0x300U
#define XSECURE_AES_DECRYPTION_FAILURE	0x400U

#define XSECURE_BOOT_HDR_FAIL		0x1000U
#define XSECURE_IMG_HDR_FAIL		0x2000U
#define XSECURE_PARTITION_FAIL		0x3000U

#define XSECURE_CSUDMA_DEVICEID		0U

#define XSECURE_MOD_LEN			512U

#define XSECURE_PH_ATTR_AUTH_ENABLE	0x8000U
#define XSECURE_PH_ATTR_ENC_ENABLE	0x0080U

#define XSECURE_AH_ATTR_PPK_SEL_MASK   	0x30000U
#define XSECURE_AH_ATTR_PPK_SEL_SHIFT	16U

#define XSECURE_AH_ATTR_SPK_ID_FUSE_SEL_MASK 0xC0000U
#define XSECURE_AH_ATTR_SPKID_FUSESEL_SHIFT 18U

#define XSECURE_AC_SPKID_OFFSET		0x04U
#define XSECURE_AC_PPK_OFFSET		0x40U
#define XSECURE_AC_SPK_OFFSET		0x480U
#define XSECURE_AC_SPK_SIG_OFFSET	0x8C0U

#define XSECURE_KEY_SIZE		(512U+512U+64U)

#define XSECURE_ENABLED			0xFFU
#define XSECURE_NOTENABLED		0U


#define XSECURE_EFUSE_BASEADDR		(0XFFCC0000U)

/*User eFuse 0 */
#define XSECURE_USER_EFUSE_START_ADDR	(XSECURE_EFUSE_BASEADDR + 0x00001020U)

/* Register PPK0_0 */
#define XSECURE_EFUSE_PPK0	(XSECURE_EFUSE_BASEADDR + 0x000010A0U)

/* Register PPK1_0 */
#define XSECURE_EFUSE_PPK1	(XSECURE_EFUSE_BASEADDR + 0x000010D0U)

/* Register SPK ID */
#define XSECURE_EFUSE_SPKID	(XSECURE_EFUSE_BASEADDR + 0x0000105CU)

/* Register: EFUSE_SEC_CTRL */
#define XSECURE_EFUSE_SEC_CTRL    (XSECURE_EFUSE_BASEADDR  + 0X00001058U)
#define XSECURE_EFUSE_SEC_CTRL_PPK0_REVOKE	(0x18000000U)
#define XSECURE_EFUSE_SEC_CTRL_PPK1_REVOKE	(0xC0000000U)
#define XSECURE_EFUSE_SEC_CTRL_RSA_ENABLE   	(0x03FFF800U)
#define XSECURE_EFUSE_SEC_CTRL_ENC_ONLY		(0x00000004U)

#define XSECURE_SPK_SIZE			(512U+512U+64U)
#define XSECURE_PPK_SIZE			(XSECURE_SPK_SIZE)
#define XSECURE_PPK_MOD_SIZE			(512U)
#define XSECURE_PPK_MOD_EXT_SIZE		(512U)
#define XSECURE_SPK_MOD_SIZE			XSECURE_PPK_MOD_SIZE
#define XSECURE_SPK_MOD_EXT_SIZE		XSECURE_PPK_MOD_EXT_SIZE
#define XSECURE_SPK_SIG_SIZE			(512U)
#define XSECURE_BHDR_SIG_SIZE			(512U)
#define XSECURE_PARTITION_SIG_SIZE		(512U)
#define XSECURE_RSA_AC_ALIGN			(64U)
#define XSECURE_SPKID_AC_ALIGN			(4U)

#define XSECURE_AUTH_HEADER_SIZE		(8U)

#define	XSECURE_AUTH_CERT_USER_DATA	((u32)64U - XSECURE_AUTH_HEADER_SIZE)

#define XSECURE_AUTH_CERT_MIN_SIZE	((u32)XSECURE_AUTH_HEADER_SIZE 	\
					+ (u32)XSECURE_AUTH_CERT_USER_DATA 	\
					+ (u32)XSECURE_PPK_SIZE 		\
					+ (u32)XSECURE_SPK_SIZE 		\
					+ (u32)XSECURE_SPK_SIG_SIZE 		\
					+ (u32)XSECURE_BHDR_SIG_SIZE 	\
					+ (u32)XSECURE_PARTITION_SIG_SIZE)

#define XSECURE_AUTH_CERT_BHDRSIG_OFFSET	0xAC0U
#define XSECURE_AUTH_CERT_PARTSIG_OFFSET	0xCC0U

#define XSECURE_BOOT_HDR_MIN_SIZE	(0x000008B8U)
#define XSECURE_BOOT_HDR_MAX_SIZE	(XSECURE_BOOT_HDR_MIN_SIZE + \
					(0x00000182U * 4U))
			/**< When boot header contains PUF helper data */
#define XSECURE_BUFFER_SIZE		(0x00001080U)

#define XSECURE_IV_OFFSET		(0xA0U)
#define XSECURE_PH_TABLE_OFFSET		(0x9CU)
#define XSECURE_IMAGE_HDR_OFFSET	(0x98U)
#define XSECURE_IMAGE_ATTR_OFFSET	(0x44U)
#define XSECURE_KEY_SOURCE_OFFSET	(0x28U)

#define XSECURE_IMG_ATTR_BHDR_MASK	(0xC000U)
#define XSECURE_IMG_ATTR_PUFHD_MASK	(0x00C0U)

#define XSECURE_PH_OFFSET		(0x8U)
#define XSECURE_AC_IMAGE_HDR_OFFSET	(0x10U)
#define XSECURE_IMAGE_SYNC_WORD_OFFSET  (0x20U)

#define XSECURE_PH_IV_MASK		(0xFFU)

#define XSECURE_KEY_SRC_KUP		(0xA3A5C3C5U)
#define XSECURE_KEY_SRC_BBRAM		(0x3A5C3C5AU)
#define XSECURE_KEY_SRC_BLACK_BH	(0xA35C7C53U)
#define XSECURE_KEY_SRC_GREY_BH		(0xA35C7CA5U)

/**************************** Type Definitions *******************************/

/* Partition address */
typedef struct {
	u32 AddrHigh;	/**< Partition address high */
	u32 AddrLow;	/**< Partition address low */
}XSecure_DataAddr;

/* RSA key components */
typedef struct {
	u8 *Modulus;		/**< Modulus */
	u8 *Exponentiation;	/**< Exponentitation */
	u8 *Exponent;		/**< Exponent */
}XSecure_RsaKey;

/* AES Params*/
typedef struct {
	u64 Src;
	u64 Iv;
	u64 Key;
	u64 Dst;
	u64 Size;
	u64 AesOp;
	u64 KeySrc;
}XSecure_AesParams;
/**
 * Structure to store the partition header details.
 * It contains all the information of partition header in order.
 */
typedef struct {
	u32 EncryptedDataWordLength; /**< Encrypted word length of partition*/
	u32 UnEncryptedDataWordLength; /**< unencrypted word length */
	u32 TotalDataWordLength;
		/**< Total word length including the authentication
			certificate if any*/
	u32 NextPartitionOffset; /**< Address of the next partition header*/
	u64 DestinationExecutionAddress; /**< Execution address */
	u64 DestinationLoadAddress; /**< Load address in DDR/TCM */
	u32 DataWordOffset; /**< */
	u32 PartitionAttributes; /**< partition attributes */
	u32 SectionCount; /**< section count */
	u32 ChecksumWordOffset; /**< address to checksum when enabled */
	u32 ImageHeaderOffset; /**< address to image header */
	u32 AuthCertificateOffset;
		/**< address to the authentication certificate when enabled */
	u32 Iv; /**< 8 bits are to be added to  and remaining are reserved */
	u32 Checksum; /**< checksum of the partition header */
} XSecure_PartitionHeader;

/* Image info */
typedef struct {
	u32 EfuseRsaenable; /**< 0 if not burnt 0xFF if burnt */
	XSecure_PartitionHeader *PartitionHdr; /**< Pointer to the buffer
					* which is holding image header */
	u32 BhdrAuth; /**< 0 if not enabled and 0xFF if set */
	u32 KeySrc;  /**< Key src from boot header */
	u32 *Iv;
			/**< From Boot header + 8bits from partition header */
	u8 *AuthCerPtr; /**< Buffer allocated to copy AC of the partition */
}XSecure_ImageInfo;

/************************** Variable Definitions *****************************/
#if defined (__GNUC__)
#if defined (PSU_PMU)
extern u8 EfusePpk[XSECURE_PPK_SIZE]__attribute__ ((aligned (32)));
			/**< eFUSE verified PPK */
extern u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE]__attribute__ ((aligned (32)));
			/**< Buffer to store authentication certificate */
extern u8 Buffer[XSECURE_BUFFER_SIZE] __attribute__ ((aligned (32)));
			/**< Buffer to store */
#else	/*!PSU_PMU*/
extern u8 EfusePpk[XSECURE_PPK_SIZE]__attribute__ ((aligned (64)));
			/**< eFUSE verified PPK */
extern u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE]__attribute__ ((aligned (64)));
			/**< Buffer to store authentication certificate */
extern u8 Buffer[XSECURE_BUFFER_SIZE] __attribute__ ((aligned (64)));
			/**< Buffer to store */
#endif
#elif defined (__ICCARM__)
#if defined (PSU_PMU)
#pragma data_alignment = 32
extern u8 EfusePpk[XSECURE_PPK_SIZE];
			/**< eFUSE verified PPK */
#pragma data_alignment = 32
extern u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE];
			/**< Buffer to store authentication certificate */
#pragma data_alignment = 32
extern u8 Buffer[XSECURE_BUFFER_SIZE];
			/**< Buffer to store */
#else	/*!PSU_PMU*/
#pragma data_alignment = 64
extern u8 EfusePpk[XSECURE_PPK_SIZE];
			/**< eFUSE verified PPK */
#pragma data_alignment = 64
extern u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE];
			/**< Buffer to store authentication certificate */
#pragma data_alignment = 64
extern u8 Buffer[XSECURE_BUFFER_SIZE];
			/**< Buffer to store */
#endif
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XSecure_Sha3Hash(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);
u32 XSecure_RsaCore(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);
u32 XSecure_DataAuth(u8 *Signature, XSecure_RsaKey *KeyInst, u8 *Hash);
u32 XSecure_AesOperation(u32 AddrHigh, u32 AddrLow);

/* Memory copy */
u32 XSecure_MemCopy(void * DestPtr, void * SrcPtr, u32 Size);
u32 XSecure_CsuDmaInit(void);
/* Keys verification */
u32 XSecure_VerifySpk(u8 *AcPtr, u32 EfuseRsaenable);
u32 XSecure_PpkVerify(XCsuDma *CsuDmaInstPtr, u8 *AuthCert);
u32 XSecure_SpkAuthentication(XCsuDma *CsuDmaInstPtr, u8 *AuthCert, u8 *Ppk);
u32 XSecure_SpkRevokeCheck(u8 *AuthCert);

/* Authentication functions */
u32 XSecure_PartitionAuthentication(XCsuDma *CsuDmaInstPtr, u8 *Data,
				u32 Size, u8 *AuthCertPtr);
u32 XSecure_AuthenticationHeaders(u8 *StartAddr, XSecure_ImageInfo *ImageInfo);

/* eFUSE read functions */
u32 XSecure_IsRsaEnabled(void);
u32 XSecure_IsEncOnlyEnabled(void);

/* For single partition secure image */
u32 XSecure_SecureImage(u32 AddrHigh, u32 AddrLow,
		u32 KupAddrHigh, u32 KupAddrLow, XSecure_DataAddr *Addr);

/* For getting CsuDma pointer*/
XCsuDma* Xsecure_GetCsuDma(void);
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_HW_H */
