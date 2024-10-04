/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_aesclient.h
*
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
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_AesDecryptKat and XSecure_AesDecryptCMKat
*                     into xsecure_katclient.c
*       kpt  08/19/22 Added GMAC support
* 5.2   mmd  07/09/23 Included header file for crypto algorithm information
*	vss  09/21/23 Fixed doxygen warnings
* 5.3	vss  10/03/23 Added single API support for AES AAD and GMAC operations
*	vss  03/04/24 Removed code redundancy for AesPerformOperation API
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_client_apis XilSecure AES Client APIs
* @{
*/
#ifndef XSECURE_AES_CLIENT_H
#define XSECURE_AES_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_mailbox.h"
#include "xsecure_defs.h"
#include "xsecure_aesalginfo.h"

/**************************** Type Definitions *******************************/
/** For selecting the Key source to AES Core. */
typedef enum {
	XSECURE_AES_BBRAM_KEY = 0, /**< Bbram key */
	XSECURE_AES_BBRAM_RED_KEY, /**< Bbram red key */
	XSECURE_AES_BH_KEY, /**< Boot header key */
	XSECURE_AES_BH_RED_KEY, /**< Boot header red key */
	XSECURE_AES_EFUSE_KEY, /**< eFuse key */
	XSECURE_AES_EFUSE_RED_KEY, /**< eFuse red key */
	XSECURE_AES_EFUSE_USER_KEY_0, /**< eFuse user key 0 */
	XSECURE_AES_EFUSE_USER_KEY_1, /**< eFuse user key 1 */
	XSECURE_AES_EFUSE_USER_RED_KEY_0, /**< User red key 0 */
	XSECURE_AES_EFUSE_USER_RED_KEY_1, /**< User red key 1 */
	XSECURE_AES_KUP_KEY, /**< Kup key */
	XSECURE_AES_PUF_KEY, /**< Puf key */
	XSECURE_AES_USER_KEY_0, /**< User key 0 */
	XSECURE_AES_USER_KEY_1, /**< User key 1 */
	XSECURE_AES_USER_KEY_2, /**< User key 2 */
	XSECURE_AES_USER_KEY_3, /**< User key 3 */
	XSECURE_AES_USER_KEY_4, /**< User key 4 */
	XSECURE_AES_USER_KEY_5, /**< User key 5 */
	XSECURE_AES_USER_KEY_6, /**< User key 6 */
	XSECURE_AES_USER_KEY_7, /**< User key 7 */
	XSECURE_AES_EXPANDED_KEYS, /**< Expanded keys */
	XSECURE_AES_ALL_KEYS, /**< All keys */
} XSecure_AesKeySource;

/** For selecting the Key size to AES Core. */
typedef enum {
	XSECURE_AES_KEY_SIZE_128 = 0,  /**< Key Length = 16 bytes = 128 bits */
	XSECURE_AES_KEY_SIZE_256 = 2,  /**< Key Length = 32 bytes = 256 bits */
}XSecure_AesKeySize;

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_AES_INVALID_PARAM       (0x51U)    /**< Invalid Argument for AES */

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_AesInitialize(XSecure_ClientInstance *InstancePtr);
int XSecure_AesEncryptInit(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc,
						u32 Size, u64 IvAddr);
int XSecure_AesDecryptInit(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 Size,
						u64 IvAddr);
int XSecure_AesUpdateAad(XSecure_ClientInstance *InstancePtr, u64 AadAddr, u32 AadSize);
int XSecure_AesEncryptUpdate(XSecure_ClientInstance *InstancePtr, u64 InDataAddr, u64 OutDataAddr,
	u32 Size, u32 IsLast);
int XSecure_AesDecryptUpdate(XSecure_ClientInstance *InstancePtr, u64 InDataAddr, u64 OutDataAddr,
	u32 Size, u32 IsLast);
int XSecure_AesDecryptFinal(XSecure_ClientInstance *InstancePtr, u64 GcmTagAddr);
int XSecure_AesEncryptFinal(XSecure_ClientInstance *InstancePtr, u64 GcmTagAddr);
int XSecure_AesKeyZero(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc);
int XSecure_AesWriteKey(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 Size, u64 KeyAddr);
int XSecure_AesKekDecrypt(XSecure_ClientInstance *InstancePtr, u64 IvAddr, XSecure_AesKeySource DstKeySrc,
	XSecure_AesKeySource DecKeySrc, XSecure_AesKeySize Size);
int XSecure_AesSetDpaCm(XSecure_ClientInstance *InstancePtr, u8 DpaCmCfg);
int XSecure_AesEncryptData(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr);
int XSecure_AesDecryptData(XSecure_ClientInstance *InstancePtr, XSecure_AesKeySource KeySrc, u32 KeySize, u64 IvAddr,
	u64 InDataAddr, u64 OutDataAddr, u32 Size, u64 GcmTagAddr);
int XSecure_AesPerformOperation(const XSecure_ClientInstance *InstancePtr,
		const XSecure_AesDataBlockParams *AesDataParams);
int XSecure_AesGmacUpdateAad(XSecure_ClientInstance *InstancePtr, u64 AadAddr, u32 AadSize, u32 IsLastChunkSrc);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_CLIENT_H */
/** @} */
