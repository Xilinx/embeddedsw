/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes.h
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
 * 1.1   am   01/20/25 Added AES CCM support
 *       yog  02/26/25 Added XAes_Compute() API
 *       am   03/14/25 Fixed alignment of function prototype
 *       yog  04/04/25 Added XAes_KeyClear() API
 *       am   04/14/25 Added macros related to non-blocking update stages.
 * 1.2   am   05/18/25 Moved non-blocking update stages macros to aes handler
 *       am   07/18/25 Modified XAes_SetReset() visibility from static to non-static
 *
 * </pre>
 *
**************************************************************************************************/
/**
* @addtogroup xaes_server_apis AES Server APIs
* @{
*/
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
typedef struct _XAes_Config XAes_Config; /**< This typedef is to create alias name
						for _XAes_Config */
typedef struct _XAes XAes; /**< This typedef is to create alias name for _XAes */

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XAES_AAD_UPDATE_NO_OUTPUT_ADDR	(0U) /**< Output address should be zero during AAD update. */

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
s32 XAes_CcmFormatAadAndXfer(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 AadAddr, u32 AadLen,
	u64 NonceAddr, u8 NonceLen, u32 PlainTextLen, u8 TagLen);
s32 XAes_DpaCmDecryptData(XAes *InstancePtr, XAsufw_Dma *DmaPtr, XAsu_AesKeyObject *KeyObjPtr,
	u32 InputDataAddr, u32 OutputDataAddr, u32 DataLength);
s32 XAes_DecryptEfuseBlackKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u32 DecKeySel, u8 DecKeySize,
	u64 IvAddr, u32 IvLen);
s32 XAes_Compute(XAes *InstancePtr, XAsufw_Dma *AsuDmaPtr, XAsu_AesParams *AesParams);
u8 XAes_GetEngineMode(const XAes *InstancePtr);
s32 XAes_KeyClear(const XAes *InstancePtr, u32 KeySrc);
void XAes_SetReset(XAes *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif  /* XAES_H_ */
/** @} */
