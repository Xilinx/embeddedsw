/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************************** Type Definitions *****************************************/
typedef struct _XSha_Config XSha_Config;
                /**< This typedef is to create alias name for _XSha_Config. */
typedef struct _XSha XSha; /**< This typedef is to create alias name for _XSha. */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
XSha *XSha_GetInstance(u16 DeviceId);
s32 XSha_CfgInitialize(XSha *InstancePtr);
s32 XSha_Start(XSha *InstancePtr, u32 ShaMode);
s32 XSha_Update(XSha *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u32 Size, u32 EndLast);
s32 XSha_Finish(XSha *InstancePtr, u64 HashAddr, u32 HashBufSize, u8 NextXofOutput);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSHA_H */
/** @} */
