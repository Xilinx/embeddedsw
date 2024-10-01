/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_dma.h
 *
 * This is the file which contains common code for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_DMA_H
#define XPLM_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * @defgroup dma_flags DMA flags
 * @brief Flags used to configure the DMA for transfers.
 * @{
 */
#define XPLM_SRC_CH_AXI_FIXED		(0x1U)	/**< Set DMA to Fixed mode for source channel. */

#define XPLM_DST_CH_AXI_FIXED		((u32)0x1U << 16U)	/**< Set DMA to Fixed mode for
								  * destination channel. */

#define XPLM_DMA_INCR_MODE		(0x0U)	/**< Set DMA mode to INCR. */
/** @} end of dma_flags group */

/* SSS configurations and masks */
#define XPLM_SSSCFG_SBI_MASK	(0x0000f000U)
#define XPLM_SSSCFG_SHA3_MASK	(0x00000f00U)
#define XPLM_SSSCFG_AES_MASK	(0x000000f0U)
#define XPLM_SSSCFG_DMA_MASK	(0x0000000fU)

#define XPLM_SSS_DMA_DMA	(0x0005U)
#define XPLM_SSS_DMA_SBI	(0x000AU)
#define XPLM_SSS_DMA_AES	(0x0009U)

#define XPLM_SSS_SBI_DMA	(0x5000U)

#define XPLM_SET_CHUNK_SIZE	(0x8U)

#ifndef SDT
#define PMCDMA_0_DEVICE ((u32)PMCDMA_0_DEVICE_ID)
#else
#define PMCDMA_0_DEVICE PMCDMA_0_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_DmaInit(void);
XPmcDma *XPlm_GetDmaInstance(void);
u32 XPlm_DmaXfr(u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags);
u32 XPlm_SbiDmaXfer(u32 DestAddr, u32 Len, u32 Flags);
u32 XPlm_DmaSbiXfer(u32 SrcAddr, u32 Len, u32 Flags);
u32 XPlm_MemSet(u32 DestAddr, u32 Val, u32 Len);
u32 XPlm_SbiRead(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_DMA_H */
