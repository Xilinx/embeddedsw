/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xsha.h
 *
 * This file Contains the function prototypes, defines and macros for the SHA2/3 hardware module.
 *
 * This driver supports the following features:
 *  - Initialization & Configuration
 *  - SHA2/3 hash calculation
 *
 * The SHA2/3 driver instance can be initialized by calling the following APIs:
 *   - XSha_GetInstance(u16 DeviceId)
 *   - XSha_CfgInitialize(XSha *InstancePtr)
 *
 * A pointer to ASU SHA instance has to be passed in initialization as ASU SHA 2/3 will be used
 * for data transfers to SHA module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/02/24 Initial release
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       am   10/22/24 Moved hash length macros to common xasu_shainfo header file.
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       yog  01/02/25 Added XSha_GetShaBlockLen() and XSha_Reset() API's and block length macros
 *                     of all sha modes.
 *       ma   01/15/25 Minor updates to XSha_GetHashLen API
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xsha_server_apis SHA Server APIs
* @{
*/
#ifndef XSHA_H
#define XSHA_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasufw_dma.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/
#define SHA_UPDATE_DONE   0x1U /**< SHA update done stage for DMA non-blocking wait */

/************************************** Type Definitions *****************************************/
typedef struct _XSha_Config XSha_Config;
/**< This typedef is to create alias name for _XSha_Config. */
typedef struct _XSha XSha; /**< This typedef is to create alias name for _XSha. */

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_SHA2_256_BLOCK_LEN	(64U) /**< Block length of SHA2-256. */
#define XASUFW_SHA2_384_512_BLOCK_LEN	(128U) /**< Block length of SHA2-384 and SHA2-512. */
#define XASUFW_SHAKE_SHA3_256_BLOCK_LEN	(136U) /**< Block length of SHA3-256. */
#define XASUFW_SHA3_384_BLOCK_LEN	(104U) /**< Block length of SHA3-384. */
#define XASUFW_SHA3_512_BLOCK_LEN	(72U) /**< Block length of SHA3-512. */

/************************************ Function Prototypes ****************************************/
XSha *XSha_GetInstance(u16 DeviceId);
s32 XSha_CfgInitialize(XSha *InstancePtr);
s32 XSha_Start(XSha *InstancePtr, u32 ShaMode);
s32 XSha_Update(XSha *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u32 Size, u32 EndLast);
s32 XSha_Finish(XSha *InstancePtr, u32 *HashAddr, u32 HashBufSize, u8 NextXofOutput);
s32 XSha_GetHashLen(u8 ShaMode, u32 *HashLen);
s32 XSha_GetShaBlockLen(const XSha *InstancePtr, u8 ShaMode, u8* BlockLen);
void XSha_Reset(XSha *InstancePtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSHA_H */
/** @} */
