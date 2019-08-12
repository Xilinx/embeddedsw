/******************************************************************************
*
* Copyright (C) 2017 - 18 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
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
*       vnc  08/21/18 Added support for PMC FW
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_H
#define XSECURE_H

/************************** Include Files ***********************************/

#include "xcsudma.h"
#include "xsecure_aes.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"
#include "xparameters.h"
#include "xpmcfw_hw.h"
/************************** Constant Definitions *****************************/

#define XSECURE_AES		1
#define XSECURE_RSA		2
#define XSECURE_RSA_AES		3

#define XSECURE_SHA3_INIT	1
#define XSECURE_SHA3_UPDATE	2
#define XSECURE_SHA3_FINAL	4
#define XSECURE_SHA3_MASK	(XSECURE_SHA3_INIT | \
				XSECURE_SHA3_UPDATE | XSECURE_SHA3_FINAL)

#define XSECURE_SPKID_EFUSE				1U /**< SPK ID eFUSE */
#define XSECURE_USER_EFUSE				2U /**< User eFuse */
#define XSECURE_USER_EFUSE_MIN_VALUE	1U
#define XSECURE_USER_EFUSE_MAX_VALUE	256U

/* 0th bit of flag tells about encryption or decryption */
#define XSECURE_ENC		1
#define XSECURE_DEC		0

#define XSECURE_RSA_OPERATION	1
#define XSECURE_RSA_KEY_SELECT  2

#define XSECURE_MASK		(XSECURE_AES | XSECURE_RSA)

#define XSECURE_KEY_STR_LEN		64	/* String length */
#define XSECURE_IV_STR_LEN		24	/* String length */
#define XSECURE_KEY_LEN			8
#define XSECURE_IV_LEN			3
#define XSECURE_GCM_TAG_LEN		128
#define XSECURE_WORD_LEN		4
#define XSECURE_MAX_NIBBLES		8
#define XSECURE_SHA3_LEN		48

#define XSECURE_WORD_SHIFT		32

#define XSECURE_ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define XSECURE_ERROR_CSUDMA_INIT_FAIL	0x1
#define XSECURE_STRING_INVALID_ERROR	0x2
#define XSECURE_INVALID_FLAG		0x3
#define XSECURE_ISNOT_SECURE_IMAGE	0x4
#define XSECURE_SHA3_INIT_FAIL		0x5
#define XSECURE_SIZE_ERR		0x6

#define XSECURE_SEL_ERR			0x7
#define XSECURE_REVOKE_ERR		0x8
#define XSECURE_VERIFY_ERR		0x9
#define XSECURE_RSA_INIT_ERR		0xA
#define XSECURE_RSA_ENCRYPT_ERR		0xB
#define XSECURE_SHA3_PADSELECT_ERR	0xC
#define XSECURE_IMAGE_WITH_MUL_PARTITIONS 0xD
#define XSECURE_AUTH_ISCOMPULSORY	0xE
#define XSECURE_ENC_ISCOMPULSORY	0xF

#define XSECURE_BHDR_AUTH_NOT_ALLOWED	0x10
#define	XSECURE_ONLY_BHDR_AUTH_ALLOWED	0x11
#define XSECURE_HDR_NOAUTH_PART_AUTH	0x12
#define XSECURE_DEC_WRONG_KEY_SOURCE	0x13
#define XSECURE_KUP_KEY_NOT_PROVIDED	0x14
#define XSECURE_KUP_KEY_NOT_REQUIRED	0x15
#define XSECURE_AES_GCM_TAG_NOT_MATCH	0x16
#define XSECURE_INVALID_EFUSE_SELECT 	0x17
#define XSECURE_OUT_OF_RANGE_USER_EFUSE_ERROR	0x18
#define XSECURE_AES_ZEROIZATION_ERR		0x19
#define XSECURE_AES_KEY_CLEAR_ERROR		XSECURE_CSU_AES_KEY_CLEAR_ERROR
						/**< 0x20 */
#define XSECURE_AES_KEYCLR_AND_GCMTAG_NOTMATCH	(XSECURE_AES_KEY_CLEAR_ERROR \
					| XSECURE_CSU_AES_GCM_TAG_MISMATCH)
						/**< 0x21*/
#define XSECURE_AES_KEYCLR_AND_ZEROIZATION_ERR	(XSECURE_AES_KEY_CLEAR_ERROR \
					| XSECURE_CSU_AES_ZEROIZATION_ERROR)
						/**< 0x24*/

#define XSECURE_AUTH_NOT_ENABLED 	0xFF

#define XSECURE_PPK_ERR			0x100
#define XSECURE_SPK_ERR			0x200
#define XSECURE_AUTH_FAILURE		0x300
#define XSECURE_AES_DECRYPTION_FAILURE	0x400

#define XSECURE_BOOT_HDR_FAIL		0x1000
#define XSECURE_IMG_HDR_FAIL		0x2000
#define XSECURE_PARTITION_FAIL		0x3000

#define XSECURE_CSUDMA_DEVICEID		0

#define XSECURE_MOD_LEN			512

#define XSECURE_PH_ATTR_AUTH_ENABLE	0x8000U
#define XSECURE_PH_ATTR_ENC_ENABLE	0x0080U

#define XSECURE_AH_ATTR_PPK_SEL_MASK   	0x30000U
#define XSECURE_AH_ATTR_PPK_SEL_SHIFT	16

#define XSECURE_AH_ATTR_SPK_ID_FUSE_SEL_MASK 0xC0000U
#define XSECURE_AH_ATTR_SPK_ID_FUSE_SEL_SHIFT 18

#define XSECURE_AC_SPKID_OFFSET		0x04U
#define XSECURE_AC_PPK_OFFSET		0x40U
#define XSECURE_AC_SPK_OFFSET		0x480U
#define XSECURE_AC_SPK_SIG_OFFSET	0x8C0U

#define XSECURE_KEY_SIZE		(512U+512U+64U)

#define XSECURE_ENABLED			0xFF
#define XSECURE_NOTENABLED		0


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

#define XSECURE_SPK_SIZE			(512U+512U+4U +4U)
#define XSECURE_PPK_SIZE			(512U+512U+4U +12U)
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

#define XSECURE_AUTH_CERT_MIN_SIZE	(XSECURE_AUTH_HEADER_SIZE 	\
					+ XSECURE_AUTH_CERT_USER_DATA 	\
					+ XSECURE_PPK_SIZE 		\
					+ XSECURE_SPK_SIZE 		\
					+ XSECURE_SPK_SIG_SIZE 		\
					+ 8 \
					+ XSECURE_BHDR_SIG_SIZE 	\
					+ XSECURE_PARTITION_SIG_SIZE)

#define XSECURE_AUTH_CERT_BHDRSIG_OFFSET	0xA60
#define XSECURE_AUTH_CERT_PARTSIG_OFFSET	(XSECURE_RSA_AC_ALIGN+ XSECURE_PPK_SIZE+ XSECURE_SPK_SIZE+ 8 + 512+ 512)//XSECURE_AUTH_CERT_BHDRSIG_OFFSET + 512

#define XSECURE_BOOT_HDR_MIN_SIZE	(0x000008B8U)
#define XSECURE_BOOT_HDR_MAX_SIZE	(XSECURE_BOOT_HDR_MIN_SIZE + \
					(0x00000182U * 4))
			/**< When boot header contains PUF helper data */
#define XSECURE_BUFFER_SIZE		(0x00001080U)
#define XSECURE_IV_SIZE			(4U)

#define XSECURE_IV_OFFSET		(0xA0U)
#define XSECURE_PH_TABLE_OFFSET		(0x9CU)
#define XSECURE_IMAGE_HDR_OFFSET	(0x98U)
#define XSECURE_IMAGE_ATTR_OFFSET	(0x44U)
#define XSECURE_KEY_SOURCE_OFFSET	(0x28U)

#define XSECURE_IMG_ATTR_BHDR_MASK	(0xC000U)
#define XSECURE_IMG_ATTR_PUFHD_MASK	(0x00C0U)

#define XSECURE_PH_OFFSET		(0x8U)
#define XSECURE_AC_IMAGE_HDR_OFFSET	(0x10U)

#define XSECURE_PH_IV_MASK		(0xFFU)

#define XSECURE_KEY_SRC_KUP		(0xA3A5C3C5U)
#define XSECURE_KEY_SRC_BBRAM		(0x3A5C3C5AU)
#define XSECURE_KEY_SRC_BLACK_BH	(0xA35C7C53U)
#define XSECURE_KEY_SRC_GREY_BH		(0xA35C7CA5U)

#define UN_ENCRYPTED 		0U
#define eFUSE_KEY			0xA5C3C5A3U
#define eFUSE_PUF_KEK		0xA5C3C5A5U
#define eFUSE_FAMILY_KEK	0xA5C3C5A7U
#define BBRAM_KEY			0x3A5C3C5AU
#define BBRAM_PUF_KEK		0x3A5C3C59U
#define BBRAM_FAMILY_KEK	0x3A5C3C57U
#define BH_PUF_KEK			0xA35C7C53U
#define BH_FAMILY_KEK		0xA35C7CA5U

#define XILCDO_PARTITION_PADDING	(16 + 104 + 100)
					/* DMA 16 byte alignment + SHA padding + Encryption over head */

/**************************** Type Definitions *******************************/


/* RSA key components */
typedef struct {
	u8 *Modulus;		/**< Modulus */
	u8 *Exponentiation;	/**< Exponentitation */
	u8 *Exponent;		/**< Exponent */
}XSecure_RsaKey;



/** @name XFsblPs_PlAuthentication
 * @{
 */
typedef struct {
	u32 AcOfset;	/**< Offset of first authentication certificate
			  *  of bitstream */
	u8 *HashsOfChunks;/** To store hashes of all chunks of block */
	u32 NoOfHashs;	/**< HashsOfChunks buffer size provided */
	u32 BlockSize;	/**< Block size of bitstream */
	u32 ChunkNum;
	u32 BlockNum;
	u32 NoOfChunks;
	u32 CurBlockSize;
} XSecure_Authentication;
/*@}*/

/** @name XFsblPs_PlEncryption
 * @{
 */
typedef struct {
	XSecure_Aes *SecureAes; /**< AES initialized structure */
	u32 NextBlkLen; /**< Not required for user, used for storing
			  *  next block size */
	u32 DataDecrypted;
	u32 Left;
} XSecure_Encryption;
/*@}*/

/** @name XFsblPs_PlPartition
 * @{
 */
typedef struct {
	u32 IsAuthenticated;	/**< Authentication flag */
	u32 IsEncrypted;		/**< Encryption flag */
	u64 StartAddress;	/** Start address of the partition */
	u32 UnEncryptLen;	/**< un encrypted length of bitstream */
	u32 TotalSize;		/**< Total partition length */
	u32 ChunkSize;		/**< Chunk size */
	u8 *ChunkBuffer;	/**< Buffer for storing chunk of data */
	u32 SecureHdr[(XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE)/4];
	u32 Hdr;
	u32 Init;
	XStatus (*DeviceCopy) (u32, u64, u32, u32);
				/**< Device copy for DDR less system */
	XSecure_Encryption PlEncrypt;	/**< Encryption parameters */
	XSecure_Authentication PlAuth;/**< Authentication parameters */
} XSecure_Partition;

/* Device copy */
typedef XStatus (*DeviceCopy) (u32, u64, u32, u32);

/*@}*/

/************************** Variable Definitions *****************************/
u8 EfusePpk[XSECURE_PPK_SIZE]__attribute__ ((aligned (32)));
			/**< eFUSE verified PPK */
u8 AcBuf[XSECURE_AUTH_CERT_MIN_SIZE]__attribute__ ((aligned (32)));
			/**< Buffer to store authentication certificate */
u8 Buffer[XSECURE_BUFFER_SIZE] __attribute__ ((aligned (32)));
			/**< Buffer to store */

u32 Iv[XSECURE_IV_LEN];
u32 Key[XSECURE_KEY_LEN];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XSecure_RsaAes(u32 SrcAddrHigh, u32 SrcAddrLow, u32 WrSize, u32 flags);
u32 XSecure_Sha3Hash(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);
u32 XSecure_RsaCore(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags);

/* Memory copy */
u32 XSecure_MemCopy(void * DestPtr, void * SrcPtr, u32 Size);

/* Keys verification */

u32 XSecure_PpkVerify(XCsuDma *CsuDmaInstPtr, u8 *AuthCert);
u32 XSecure_SpkRevokeCheck(u8 *AuthCert);


/* eFUSE read functions */
u32 XSecure_IsRsaEnabled();
u32 XSecure_IsEncOnlyEnabled();

u32 XSecure_PrtnAuth(u64 PrtnAddr, u64 *AcBuffer, u64 PrtnSize);
u32 XSecure_PrtnDec(u8 *Iv, u64 PrtnAddr, u64 Size, u32 KeySrc);
u32 XPmcFw_ProcessSecurePrtn(XSecure_Partition *SecurePtr);
void XSecure_AesInit(XSecure_Aes *InstancePtr, u32 KeySel, u8 *Iv);
u32 XSecure_CheckSum(DeviceCopy CopyFunc, u64 PrtnAddr, u64 PrtnSize, u8 *Hash);
/*****************************************************************************/

#endif  /* XSECURE_HW_H */
