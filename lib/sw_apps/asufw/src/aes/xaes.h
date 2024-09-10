/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes.h
 * @addtogroup Overview
 * @{
 *
 * This file Contains the function prototypes, defines and macros for the AES module.
 *
 * This driver supports the following features:
 *  - Initialization & Configuration
 *  - AES encryption and decryption
 *
 * The AES driver instance can be initialized by calling the following APIs:
 *   - XAes_GetInstance(u16 DeviceId)
 *   - XAes_CfgInitialize(XAes *InstancePtr)
 *
 * A pointer to ASU AES instance has to be passed in initialization as ASU AES will be used
 * for the data transfer to AES module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   am   06/26/24 Initial release
 *       am   08/01/24 Replaced variables of type enums with u32 type.
 *       am   08/24/24 Added AES DPA CM KAT support
 *
 * </pre>
 *
 * @note
 *
 * @endcond
 *
**************************************************************************************************/
#ifndef XAES_H_
#define XAES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/
#include "xasufw_sss.h"
#include "xasufw_dma.h"
#include "xaes_hw.h"
#include "xasu_aesinfo.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/
typedef struct _XAes_Config XAes_Config;
typedef struct _XAes XAes;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
XAes *XAes_GetInstance(u16 DeviceId);
s32 XAes_CfgInitialize(XAes *InstancePtr);
s32 XAes_WriteKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr);
s32 XAes_Init(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr, u64 IvAddr, u32 IvLen,
	      u8 EngineMode, u8 OperationType);
s32 XAes_Update(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u64 OutDataAddr,
		u32 DataLength, u8 IsLastChunk);
s32 XAes_Final(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 TagAddr, u32 TagLen);
s32 XAes_DpaCmDecryptData(XAes *InstancePtr, XAsufw_Dma *DmaPtr, XAsu_AesKeyObject *KeyObjPtr,
			  u32 InputDataAddr, u32 OutputDataAddr, u32 DataLength);

#ifdef __cplusplus
}
#endif

#endif  /* XAES_H */
/** @} */