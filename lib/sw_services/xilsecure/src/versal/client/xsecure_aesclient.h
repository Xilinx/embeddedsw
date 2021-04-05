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
#include "xsecure_ipi.h"

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

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_AesInitialize();
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
	XSecure_AesKeySource DecKeySrc, u32 Size);
int XSecure_AesSetDpaCm(u8 DpaCmCfg);
int XSecure_AesDecryptKat();
int XSecure_AesDecryptCmKat();

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_CLIENT_H */
