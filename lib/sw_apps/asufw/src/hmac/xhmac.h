/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xhmac.h
*
* This file contains declarations for xhmac.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  01/02/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xhmac_server_apis HMAC Server APIs
* @{
*/

#ifndef XHMAC_H_
#define XHMAC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xasufw_dma.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_HMAC_SHA_HASH_MAX_LEN		(64U) /**< Length of the maximum hash length. */
#define HMAC_UPDATE_IN_PROGRESS			(0x1U) /**< HMAC update done stage for DMA
							non-blocking wait */

/************************************ Variable Definitions ***************************************/

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/
typedef struct _XHmac XHmac; /**< This typedef is to create alias name for _XHmac. */

/************************************ Function Prototypes ****************************************/
XHmac *XHmac_GetInstance(void);
s32 XHmac_CfgInitialize(XHmac *InstancePtr);
s32 XHmac_Init(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, XSha *ShaInstancePtr, u64 KeyAddr,
	       u32 KeyLen, u8 ShaMode, u32 HashLen);
s32 XHmac_Update(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u64 DataAddr, u32 DataLen,
		 u32 IsLastUpdate);
s32 XHmac_Final(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u32 *HmacOutPtr);

#ifdef __cplusplus
}
#endif

#endif /* XHMAC_H_ */
/** @} */
