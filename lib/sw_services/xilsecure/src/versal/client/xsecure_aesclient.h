/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_aesclient.h
* @addtogroup xsecure_aes_client_apis XilSecure AES Versal Client APIs
* @{
* @cond xsecure_internal
* This file Contains the client function prototypes, defines and macros for
* the AES hardware module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Added XSecure_AesEncryptData and XSecure_AesDecryptData
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_AES_CLIENT_H
#define XSECURE_AES_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**************************** Type Definitions *******************************/
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
	XSECURE_AES_PUF_KEY,
	XSECURE_AES_USER_KEY_0,
	XSECURE_AES_USER_KEY_1,
	XSECURE_AES_USER_KEY_2,
	XSECURE_AES_USER_KEY_3,
	XSECURE_AES_USER_KEY_4,
	XSECURE_AES_USER_KEY_5,
	XSECURE_AES_USER_KEY_6,
	XSECURE_AES_USER_KEY_7,
	XSECURE_AES_EXPANDED_KEYS,
	XSECURE_AES_ALL_KEYS,
} XSecure_AesKeySource;

typedef enum {
	XSECURE_AES_KEY_SIZE_128 = 0,   /**< Key Length = 32 bytes = 256 bits */
	XSECURE_AES_KEY_SIZE_256 = 2,   /**< Key Length = 16 bytes = 128 bits */
}XSecure_AesKeySize;

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_AES_INVALID_PARAM       0x51    /**< Invalid Argument for AES */

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_AesInitialize(void);
int XSecure_AesEncryptInit(XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr);
int XSecure_AesDecryptInit(XSecure_AesKeySource KeySrc, u32 Size, u64 IvAddr);
int XSecure_AesUpdateAad(u64 AadAddr, u32 AadSize);
int XSecure_AesEncryptUpdate(u64 InDataAddr, u64 OutDataAddr,
	u32 Size, u32 IsLast);
int XSecure_AesDecryptUpdate(u64 InDataAddr, u64 OutDataAddr,
	u32 Size, u32 IsLast);
int XSecure_AesDecryptFinal(u64 GcmTagAddr);
int XSecure_AesEncryptFinal(u64 GcmTagAddr);
int XSecure_AesKeyZero(XSecure_AesKeySource KeySrc);
int XSecure_AesWriteKey(XSecure_AesKeySource KeySrc, u32 Size, u64 KeyAddr);
int XSecure_AesKekDecrypt(u64 IvAddr, XSecure_AesKeySource DstKeySrc,
	XSecure_AesKeySource DecKeySrc, XSecure_AesKeySize Size);
int XSecure_AesSetDpaCm(u8 DpaCmCfg);
int XSecure_AesDecryptKat(void);
int XSecure_AesDecryptCmKat(void);
int XSecure_AesEncryptData(XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr);
int XSecure_AesDecryptData(XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_CLIENT_H */
