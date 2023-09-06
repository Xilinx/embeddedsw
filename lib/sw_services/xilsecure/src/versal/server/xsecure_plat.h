/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
* @addtogroup xsecure_plat.h XilSecure Versal APIs
* @{
* @cond xsecure_internal
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
* 5.2   yog  07/10/23 Added support of unaligned data sizes for Versal Net
*       yog  09/04/23 Removed XSecure_ECCRandInit API definition
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_aes_core_hw.h"
#include "xpmcdma.h"
#include "xsecure_error.h"

/************************** Constant Definitions ****************************/
#define XSECURE_SSS_MAX_SRCS		(8U)	/**< SSS Maximum resources */

#define XSECURE_SSS_SHA3_0_MASK		(0xF0000U) /**< SSS SHA3 instance 0 mask value*/

#define XSECURE_SSS_SHA3_0_DMA0_VAL	(0xC0000U) /**< SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_SSS_SHA3_0_DMA1_VAL	(0x70000U) /**< SSS SHA3 instance 0 DMA1 value*/
#define XSECURE_AES_NO_CFG_DST_DMA 	(0xFFFFFFFFU) /**< Not to configure Dst DMA at this address in AES*/
#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)	/**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)	/**< Disables data swap in AES */

/***************************** Type Definitions******************************/
/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1, /**< Ignore */
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3_0, /**< SHA3_0 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_PZI, /**< PZI */
	XSECURE_SSS_INVALID /**< Invalid */
}XSecure_SssSrc;

typedef struct {
	u64 SrcDataAddr;	/**< Address of source buffer */
	u64 DestDataAddr;	/**< Address of destination buffer */
	u8 SrcChannelCfg;	/**< DMA Source channel configuration */
	u8 DestChannelCfg;	/**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;	/**< Flag for last update in source */
	u8 IsLastChunkDest;	/**< Flag for last update in destination */
} XSecure_AesDmaCfg;

/***************************** Function Prototypes ***************************/
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);

/*****************************************************************************/
/**
 * @brief	This function is not applicable for versal
 *
 *****************************************************************************/
static inline void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
	/* Not applicable for versal */
	(void)BaseAddress;
	(void)Op;
}

/*****************************************************************************/
/**
 * @brief	This function is not applicable for versal
 *
 *****************************************************************************/
static inline void XSecure_SetRsaCryptoStatus(void)
{
	/* Not applicable for versal */
}

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */

/* @} */
