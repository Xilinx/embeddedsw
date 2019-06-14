/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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
* @file xloader_secure.h
*
* This file contains all security related data.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  04/23/19 First release
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
#include "xloader.h"
#include "xplmi_util.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XLOADER_SHA3_LEN		(48U)
#define XLOADER_RSA_SIG_EXP_BYTE	(0xBCU)
#define XLOADER_RSA_SIZE		(512U)

#define XLOADER_SPK_SIZE		(XLOADER_RSA_SIZE + XLOADER_RSA_SIZE \
					    + 4U +4U)
#define XLOADER_PPK_SIZE		(XLOADER_RSA_SIZE + XLOADER_RSA_SIZE \
					    + 4U +12U)
#define XLOADER_PPK_MOD_SIZE		(XLOADER_RSA_SIZE)
#define XLOADER_PPK_MOD_EXT_SIZE	(XLOADER_RSA_SIZE)
#define XLOADER_SPK_MOD_SIZE		XLOADER_PPK_MOD_SIZE
#define XLOADER_SPK_MOD_EXT_SIZE	XLOADER_PPK_MOD_EXT_SIZE
#define XLOADER_SPK_SIG_SIZE		(XLOADER_RSA_SIZE)
#define XLOADER_BHDR_SIG_SIZE		(XLOADER_RSA_SIZE)
#define XLOADER_PARTITION_SIG_SIZE	(XLOADER_RSA_SIZE)
#define XLOADER_RSA_AC_ALIGN		(64U)
#define XLOADER_SPKID_AC_ALIGN		(4U)

#define XLOADER_AUTH_HEADER_SIZE	(8U)

#define XLOADER_AUTH_CERT_USER_DATA	((u32)64U - XLOADER_AUTH_HEADER_SIZE)

#define XLOADER_AUTH_CERT_MIN_SIZE	(XLOADER_AUTH_HEADER_SIZE \
					+ XLOADER_AUTH_CERT_USER_DATA \
					+ XLOADER_PPK_SIZE  \
					+ XLOADER_SPK_SIZE \
					+ XLOADER_SPK_SIG_SIZE \
					+ 8 \
					+ XLOADER_BHDR_SIG_SIZE \
					+ XLOADER_PARTITION_SIG_SIZE)

#define XLOADER_AC_AH_PUB_ALG_MASK	(0x3U)
#define XLOADER_AC_AH_PUB_ALG_RSA	(0x1U)
#define XLOADER_AC_AH_PUB_ALG_ECDSA	(0x2U)

#define XLOADER_ECDSA_KEYSIZE		(0x0000000CU)
#define XLOADER_ECDSA_INDEXVAL		(0x0000000B)

#define XLOADER_SECURE_HDR_SIZE		(48U)
					/**< Secure Header Size in Bytes*/
#define XLOADER_SECURE_GCM_TAG_SIZE	(16U) /**< GCM Tag Size in Bytes */

/* AES key source */
#define XLOADER_UNENCRYPTED		0x00000000 	/* Unencrypted */
#define XLOADER_EFUSE_KEY		0xA5C3C5A3  /* eFuse Key */
#define XLOADER_EFUSE_KEK_KEY	0xA5C3C5A5  /* eFUSE Black Key */
#define XLOADER_EFUSE_OBFUS_KEY	0xA5C3C5A7  /* eFuse Obfuscated Key */

#define XLOADER_BBRAM_KEY		0x3A5C3C5A  /* BBRAM Key */
#define XLOADER_BBRAM_KEK_KEY	0x3A5C3C59  /* BBRAM Black Key */
#define XLOADER_BBRAM_OBFUS_KEY	0x3A5C3C57  /* BBRAM Obfuscated Key */

#define XLOADER_BH_KEK_KEY		0xA35C7C53 /*Boot Header Black Key */
#define XLOADER_BH_OBFUS_KEY	0xA35C7CA5  /* Boot Header Obfuscated Key */

#define XLOADER_EFUSE_USR_KEY0			0x5C3CA5A3 /* eFuse User Key 0 */
#define XLOADER_EFUSE_USR_KEK_KEY0		0x5C3CA5A5 /* eFUSE User key 0 Black */
#define XLOADER_EFUSE_USR_OBFUS_KEY0	0x5C3CA5A7 /* eFuse User key 0 Obfuscated */

#define XLOADER_EFUSE_USR_KEY1			0xC3A5C5A3 /* eFuse User Key 1 */
#define XLOADER_EFUSE_USR_KEK_KEY1	0xC3A5C5A5 /* eFUSE User key 1 Black */
#define XLOADER_EFUSE_USR_OBFUS_KEY1	0xC3A5C5A7 /* eFuse User key 1 Obfuscated */


#define XLOADER_USR_KEY0		0xC5C3A5A3 /* User Key 0 */
#define XLOADER_USR_KEK_KEY0	0xC5C3A5A5 /* User key 0 Black */
#define XLOADER_USR_OBFUS_KEY0	0xC5C3A5A7 /* User key 0 Obfuscated */

#define XLOADER_USR_KEY1	0xC3A5C5B3 /* User Key 1 */
#define XLOADER_USR_KEK_KEY1	0xC3A5C5B5 /* User key 1 Black */
#define XLOADER_USR_OBFUS_KEY1	0xC3A5C5B7 /* User key 1 Obfuscated */

#define XLOADER_USR_KEY2	0xC5C3A5C3 /* User Key 2 */
#define XLOADER_USR_KEK_KEY2	0xC5C3A5C5 /*User key 2 Black */
#define XLOADER_USR_OBFUS_KEY2	0xC5C3A5C7 /* User key 2 Obfuscated */

#define XLOADER_USR_KEY3	0xC3A5C5D3 /* User Key 3 */
#define XLOADER_USR_KEK_KEY3	0xC3A5C5D5 /* User key 3 Black */
#define XLOADER_USR_OBFUS_KEY3	0xC3A5C5D7 /* User key 3 Obfuscated */

#define XLOADER_USR_KEY4	0xC5C3A5E3 /* User Key 4 */
#define XLOADER_USR_KEK_KEY4	0xC5C3A5E5 /* User key 4 Black */
#define XLOADER_USR_OBFUS_KEY4 0xC5C3A5E7  /* User key 4 Obfuscated */

#define XLOADER_USR_KEY5	0xC3A5C5F3 /* User Key 5 */
#define XLOADER_USR_KEK_KEY5	0xC3A5C5F5 /* User key 5 Black */
#define XLOADER_USR_OBFUS_KEY5	0xC3A5C5F7 /* User key 5 Obfuscated */

#define XLOADER_USR_KEY6	0xC5C3A563 /* User Key 6 */
#define XLOADER_USR_KEK_KEY6	0xC5C3A565 /* User key 6 Black */
#define XLOADER_USR_OBFUS_KEY6	0xC5C3A567 /* User key 6 Obfuscated */

#define XLOADER_USR_KEY7	0xC3A5C573 /* User Key 7 */
#define XLOADER_USR_KEK_KEY7	0xC3A5C575 /* User key 7 Black */
#define XLOADER_USR_OBFUS_KEY7	0xC3A5C577 /* User key 7 Obfuscated */


/**************************** Type Definitions *******************************/

typedef struct {

	u32 PubModulus[128];
	u32 PubModulusExt[128];
	u32 PubExponent;
}XLoader_RsaKey;

typedef struct {
	u32 AuthHeader;
	u32 SpkId;
	u32 UserData[14];
	XLoader_RsaKey Ppk;
	u32 PPKPadding[3];
	XLoader_RsaKey Spk;
	u32 SPKPadding;
	u32 Alignment1[2];
	u32 SPKSignature[128];
	u32 BHSignature[128];
	u32 ImageSignature[128];
}XLoader_AuthCertificate;


typedef enum {
	XLOADER_ECDSA,
	XLOADER_RSA
}XLoader_AuthType;

typedef struct
{
	u8 EmHash[48];
	u8 Salt[48];
	u8 Convert[4];
	u8 Padding1[8];
}XLoader_Vars;

typedef struct {
	u32 SecureEn;
	u32 IsCheckSumEnabled;
	u32 IsEncrypted;
	u32 IsAuthenticated;
	XLoader_AuthType SigType;
	XilPdi *PdiPtr;
	XilPdi_PrtnHdr *PrtnHdr;
	u32 IsCdo; // CDO or Elf
	u32 NextBlkAddr;
	u32 ChunkAddr;
	/* verified data is at */
	u64 SecureData;
	u32 BlockNum;
	u32 Sha3Hash[XLOADER_SHA3_LEN/4];
	u32 EncNextBlkSize;
	XLoader_AuthCertificate *AcPtr;
	XCsuDma *CsuDmaInstPtr;
}XLoader_SecureParms;

/***************************** Function Prototypes ***************************/

u32 XLoader_SecureInit(XLoader_SecureParms *SecurePtr, XilPdi *PdiPtr, u32 PrtnNum);
u32 XLoader_SecurePrtn(XLoader_SecureParms *SecurePtr, u64 DstAddr, u32 Size, u8 Last);
u32 XLoader_SecureCopy(XLoader_SecureParms *SecurePtr, u64 DestAddr, u32 Size);

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_SECURE_H */
