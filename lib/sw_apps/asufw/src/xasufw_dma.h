/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_dma.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xasufw_dma.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   03/23/24 Initial release
 *       ma   04/26/24 Change XAsufw_DmaXfr to XAsufw_StartDmaXfr
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_DMA_H
#define XASUFW_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasudma.h"
#include "xasufw_sss.h"

/************************************ Constant Definitions ***************************************/
/** DMA XFER flags */
#define XASUFW_SRC_CH_AXI_FIXED		(0x1U)
#define XASUFW_DST_CH_AXI_FIXED	    ((u32)0x1U << 16U)

/************************************** Type Definitions *****************************************/
typedef struct {
	XAsuDma AsuDma;
	XAsufw_SssSrc SssDmaCfg;
} XAsufw_Dma;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_DmaInit(void);
XAsufw_Dma *XAsufw_GetDmaInstance(u32 BaseAddress);
s32 XAsufw_WaitForDmaDone(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel);
s32 XAsufw_StartDmaXfr(XAsufw_Dma *DmaPtr, u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
s32 XAsufw_DmaMemSet(XAsufw_Dma *DmaPtr, u32 DestAddr, u32 Val, u32 Len);
s32 XAsufw_DmaXfr(XAsufw_Dma *AsuDmaPtr, u64 SrcAddr, u64 DstAddr, const u32 Size, u32 Flags);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_DMA_H */
/** @} */
