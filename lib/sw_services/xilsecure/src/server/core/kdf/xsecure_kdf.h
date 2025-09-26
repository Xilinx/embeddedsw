/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_kdf.c
* This file contains declarations for xsecure_kdf.c file.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.5   tvp  05/13/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kdf_server_apis Xilsecure KDF Server APIs
* @{
*/

#ifndef XSECURE_KDF_H
#define XSECURE_KDF_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"

#define XSECURE_HKDF_MAX_CONTEXT_LEN		(1024U) /**< Maximum context length */
#define XSECURE_HKDF_MAX_ITERATIONS		(0xFFU) /**< Maximum iterations */
#define XSECURE_HKDF_BLOCK_INDEX_LENGTH		(0x01U) /**< Block index length */

/**
 * Stores the require parameter for the KDF.
 */
typedef struct {
	u8 *Context;	/**< Is a pointer to the context for KDF */
	u32 ContextLen;	/**< Specifies the length of the context pointer */
	u8 *Key;	/**< Is a pointer to the key */
	u32 KeyLen;	/**< length of key pointer */
} XSecure_KdfParams;

int XSecure_Hkdf(XSecure_KdfParams *InDataPtr, u8 *Kdfout, u32 KdfOutLen);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KDF_H */
