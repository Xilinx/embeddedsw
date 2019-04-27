/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.h
* @addtogroup xsecure_aes_versal_apis XilSecure AES VERSAL APIs
* @{
* @cond xsecure_internal
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 4.0   vns  04/24/2019 Initial release
* 4.1   vns  08/06/2019 Added AES encryption APIs
* 4.2   kpt  01/07/2020 Removed Macro XSECURE_WORD_SIZE
*                       and added in xsecure_utils.h
*       vns  02/10/2020 Added DPA CM enable/disable function
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_AES_H_
#define XSECURE_AES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"
#include "xsecure_sss.h"

/************************** Constant Definitions *****************************/
/** @cond xsecure_internal
@{
*/

#define XSECURE_AES_BUFFER_SIZE					(4U)
#define XSECURE_AES_KEY_DEC_SEL_BBRAM_RED		(0x0U)
#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED		(0x3U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED		(0x4U)

#define XSECURE_SECURE_GCM_TAG_SIZE			(16U)
						/**< GCM Tag Size in Bytes */
#define XSECURE_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XSECURE_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define XSECURE_AES_TIMEOUT_MAX				(0x1FFFFU)

#define XSECURE_AES_INVALID_CFG				(0xFFFFFFFFU)

#define XSECURE_AES_NO_CFG_DST_DMA			(0xFFFFFFFFU)

#define XSECURE_AES_KEY_MASK_INDEX			(0xA0U)
#define XSECURE_AES_DMA_SIZE				(16U)
#define XSECURE_AES_DMA_LAST_WORD_ENABLE		(0x1U)
#define XSECURE_AES_DMA_LAST_WORD_DISABLE		(0x0U)
/* Key select values */
#define XSECURE_AES_KEY_SEL_BBRAM_KEY			(0xBBDE6600)
#define XSECURE_AES_KEY_SEL_BBRAM_RD_KEY		(0xBBDE8200)
#define XSECURE_AES_KEY_SEL_BH_KEY			(0xBDB06600)
#define XSECURE_AES_KEY_SEL_BH_RD_KEY			(0xBDB08200)
#define XSECURE_AES_KEY_SEL_EFUSE_KEY			(0xEFDE6600)
#define XSECURE_AES_KEY_SEL_EFUSE_RED_KEY		(0xEFDE8200)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0		(0xEF856601)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1		(0xEF856602)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0		(0xEF858201)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1		(0xEF858202)
#define XSECURE_AES_KEY_SEL_KUP_KEY			(0xBDC98200)
#define XSECURE_AES_KEY_SEL_FAMILY_KEY			(0xFEDE8200)
#define XSECURE_AES_KEY_SEL_PUF_KEY			(0xDBDE8200)
#define XSECURE_AES_KEY_SEL_USR_KEY_0			(0xBD858201)
#define XSECURE_AES_KEY_SEL_USR_KEY_1			(0xBD858202)
#define XSECURE_AES_KEY_SEL_USR_KEY_2			(0xBD858204)
#define XSECURE_AES_KEY_SEL_USR_KEY_3			(0xBD858208)
#define XSECURE_AES_KEY_SEL_USR_KEY_4			(0xBD858210)
#define XSECURE_AES_KEY_SEL_USR_KEY_5			(0xBD858220)
#define XSECURE_AES_KEY_SEL_USR_KEY_6			(0xBD858240)
#define XSECURE_AES_KEY_SEL_USR_KEY_7			(0xBD858280)

/** @}
@endcond */

/**************************** Type Definitions *******************************/
typedef enum {
	XSECURE_BLACK_KEY,
	XSECURE_OBFUSCATED_KEY
}XSecure_AesKekType;


typedef enum {
	XSECURE_AES_BBRAM_KEY = 0,
	XSECURE_AES_BBRAM_RED_KEY,
	XSECURE_AES_BH_KEY,
	XSECURE_AES_BH_RED_KEY,
	XSECURE_AES_EFUSE_KEY,
	XSECURE_AES_EFUSE_RED_KEY,
	XSECURE_AES_EFUSE_USER_KEY_0,
	XSECURE_AES_EFUSE_USER_KEY_1,
	XSECURE_AES_EFUSE_USER_RED_KEY_0,
	XSECURE_AES_EFUSE_USER_RED_KEY_1,
	XSECURE_AES_KUP_KEY,
	XSECURE_AES_FAMILY_KEY,
	XSECURE_AES_PUF_KEY,
	XSECURE_AES_USER_KEY_0,
	XSECURE_AES_USER_KEY_1,
	XSECURE_AES_USER_KEY_2,
	XSECURE_AES_USER_KEY_3,
	XSECURE_AES_USER_KEY_4,
	XSECURE_AES_USER_KEY_5,
	XSECURE_AES_USER_KEY_6,
	XSECURE_AES_USER_KEY_7,
	XSECURE_AES_EXPANDED_KEYS
} XSecure_AesKeySrc;


typedef enum {
	XSECURE_AES_KEY_SIZE_128 = 0,
	XSECURE_AES_KEY_SIZE_256 = 2,
}XSecure_AesKeySize;

/** @cond xsecure_internal
@{
*/
typedef enum {
	XSECURE_AES_UNINITIALIZED,
	XSECURE_AES_INITIALIZED,
	XSECURE_AES_ENCRYPT_INITIALIZED,
	XSECURE_AES_DECRYPT_INITIALIZED
} XSecure_AesState;

typedef struct {
	u32 BaseAddress;
	XPmcDma *PmcDmaPtr; /**< PMCDMA Instance Pointer */
	XSecure_Sss SssInstance;
	XSecure_AesState AesState; /**< Current Aes State  */
	XSecure_AesKeySrc KeySrc;
} XSecure_Aes;
/** @}
@endcond */
/************************** Function Prototypes ******************************/
u32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XPmcDma *PmcDmaPtr);

u32 XSecure_AesSetDpaCm(XSecure_Aes *InstancePtr, u32 Configuration);

u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc);

u32 XSecure_AesWriteKey(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 KeyAddr);

u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr, XSecure_AesKekType KeyType,
			XSecure_AesKeySrc DecKeySrc, XSecure_AesKeySrc DstKeySrc,
			u64 IvAddr, u32 KeySize);

u32 XSecure_AesCfgKupIv(XSecure_Aes *InstancePtr, u32 Config);

u32 XSecure_AesGetNxtBlkLen(XSecure_Aes *InstancePtr, u32 *Size);

u32 XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr);

u32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u8 IsLastChunk);
u32 XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr);

u32 XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

u32 XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr);

u32 XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u8 IsLastChunk);
u32 XSecure_AesEncryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr);

u32 XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

u32 XSecure_AesDecryptKat(XSecure_Aes *InstancePtr);

u32 XSecure_AesDecryptCmKat(XSecure_Aes *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_H_ */
