/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes.h
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   vns  04/24/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XSECURE_AES_H_
#define XSECURE_AES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes_core_hw.h"
#include "xsecure_utils.h"
#include "xcsudma.h"


/************************** Constant Definitions *****************************/

#define XSECURE_AES_KEY_DEC_SEL_BBRAM_RED		(0x0U)
#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED		(0x3U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED		(0x4U)

#define XSECURE_SECURE_GCM_TAG_SIZE			(16U)
						/**< GCM Tag Size in Bytes */
#define XSECURE_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XSECURE_AES_KEY_SIZE_256BIT_WORDS		(8U)

#define XSECURE_WORD_SIZE				(4U)

#define XSECURE_AES_TIMEOUT_MAX				(0x1FFFFU)

#define XSECURE_AES_INVALID_CFG				(0xFFFFFFFFU)

#define XSECURE_AES_NO_CFG_DST_DMA			(0xFFFFFFFFU)

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


/* Make them as enum */
#define XSECURE_CSU_AES_GCM_TAG_MISMATCH	(1L)
					/**< user provided GCM tag does
						not match calculated tag */
#define XSECURE_CSU_AES_IMAGE_LEN_MISMATCH	(2L)
					/**< image length mismatch */
#define XSECURE_CSU_AES_DEVICE_COPY_ERROR	(3L)
					/**< device copy failed */
#define XSECURE_CSU_AES_ZEROIZATION_ERROR	(4L)
					/**< Zeroization error*/
#define XSECURE_CSU_AES_KEY_CLEAR_ERROR		(0x20U)
					/**< AES key clear error */

/**************************** Type Definitions *******************************/

typedef struct {
	u32 RegOffset;
	u32 KeySrcSelVal;
	u8  UsrWrAllowed;
	u8  DecAllowed;
	u8  EncAllowed;
	u8  KeyDecSrcAllowed;
	u32 KeyDecSrcSelVal;
	u32 KeyClearVal;
} XSecure_AesKeyLookup;

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


typedef enum {
	XSECURE_AES_UNINITIALIZED,
	XSECURE_AES_INITIALIZED,
	XSECURE_AES_ENCRYPT_INITIALIZED,
	XSECURE_AES_DECRYPT_INITIALIZED
} XSecure_AesState;

typedef struct {
	u32 BaseAddress;
	XCsuDma *CsuDmaPtr; /**< CSUDMA Instance Pointer */
	XSecure_Sss SssInstance;
	XSecure_AesState AesState; /**< Current Aes State  */
	XSecure_AesKeySrc KeySrc;
} XSecure_Aes;

/************************** Function Prototypes ******************************/
u32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr);

u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc);

u32 XSecure_AesWriteKey(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 KeyAddr);

u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr, XSecure_AesKekType KeyType,
			XSecure_AesKeySrc DecKeySrc, XSecure_AesKeySrc DstKeySrc,
			u64 IvAddr, u32 KeySize);

u32 XSecure_AesCfgKupIv(XSecure_Aes *InstancePtr, u32 EnableCfg);
u32 XSecure_AesGetNxtBlkLen(XSecure_Aes *InstancePtr, u32 *Size);

u32 XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr);

u32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u8 EnLast);
u32 XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr);

u32 XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_H_ */
