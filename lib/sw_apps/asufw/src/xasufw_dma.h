/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_dma.h
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       rmv  09/11/25 Rename XAsufw_DmaNonBlockingWait() to XAsufw_DmaCfgNonBlocking()
 *       rmv  09/12/25 Removed XAsufw_DmaMemSet() function as it is not used
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_DMA_H_
#define XASUFW_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasudma.h"
#include "xasufw_sss.h"
#include "xasu_sharedmem.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_SRC_CH_AXI_FIXED		(0x1U) /**< DMA XFER flags for source channel */
#define XASUFW_DST_CH_AXI_FIXED		((u32)0x1U << 16U) /**< DMA XFER flags for destination channel */
#define XASUFW_DMA_BLOCKING_SIZE	(4096U) /**< DMA blocking size. If the SHA/AES update contains
	data size more than this size, DMA transfer will happen in non-blocking mode */

/************************************** Type Definitions *****************************************/
/** @brief
 * The enum XAsufw_DmaState defines if the DMA needs to be released or not when non-blocking DMA
 * done interrupt is received
 */
typedef enum {
	XASUFW_DEFAULT = 0U, /**< Default */
	XASUFW_RELEASE_DMA, /**< Release DMA */
	XASUFW_BLOCK_DMA, /**< Block DMA */
} XAsufw_DmaState;
/** @} */

/** The structure XAsufw_Dma is DMA instance data structure. */
typedef struct {
	XAsuDma AsuDma; /**< ASU DMA driver instance data structure */
	XAsufw_SssSrc SssDmaCfg; /**< SSS configuration */
	XAsuDma_Channel Channel; /**< ASU DMA non blocking wait on source or destination channel */
	const XAsu_ReqBuf *ReqBuf; /**< Pointer to XAsu_ReqBuf waiting on the DMA */
	u32 ReqId; /**< Requester ID which is waiting on the DMA */
	XAsufw_DmaState DmaState; /**< DMA state to block or release after non-blocking wait */
} XAsufw_Dma;

/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_DmaInit(void);
XAsufw_Dma *XAsufw_GetDmaInstance(u32 BaseAddress);
s32 XAsufw_WaitForDmaDone(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel);
s32 XAsufw_StartDmaXfr(XAsufw_Dma *DmaPtr, u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
s32 XAsufw_DmaXfr(XAsufw_Dma *AsuDmaPtr, u64 SrcAddr, u64 DstAddr, const u32 Size, u32 Flags);
void XAsufw_DmaCfgNonBlocking(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel,
			      const XAsu_ReqBuf *ReqBuf, u32 ReqId, XAsufw_DmaState DmaState);
void XAsufw_HandleDmaDoneIntr(u32 DmaIntrNum);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_DMA_H_ */
/** @} */
