/******************************************************************************ed.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_aes.h
*
* This file contains Versal Net specific code for Xilsecure aes server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt   06/20/23 Initial release
* 5.4   yog   04/29/24 Fixed doxygen grouping.
*       kpt   06/13/24 Added AES key unwrap with padding support.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
#ifndef XSECURE_PLAT_AES_H
#define XSECURE_PLAT_AES_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes.h"

/************************** Constant Definitions ****************************/

#define XSECURE_AES_ECB_MODE_EN       (1U)   /**< AES ECB Mode Enable */
#define XSECURE_AES_ECB_MODE_DIS      (0U)   /**< AES ECB Mode Enable */
#define XSECURE_AES_64BIT_BLOCK_SIZE  (8U)   /**< AES 64-bit block size in bytes */

/***************************** Type Definitions******************************/\

/***************************** Function Prototypes ***************************/

int XSecure_AesEcbCfg(XSecure_Aes *InstancePtr, u32 EcbModeFlag);
int XSecure_AesEcbDecrypt(XSecure_Aes *InstancePtr, u64 KeyAddr, XSecure_AesKeySize KeySize, u64 InDataAddr,
			  u64 OutDataAddr, u32 Size);
int XSecure_AesKeyUnwrap(XSecure_Aes *InstancePtr, u8 *EphAesKey, XSecure_AesKeySize KeySize, u8 *AesWrapKey,
			  u64 OutDataAddr, u32 Size);

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_AES_H */
