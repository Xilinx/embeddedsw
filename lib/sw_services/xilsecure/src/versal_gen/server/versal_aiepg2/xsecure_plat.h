/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
* @addtogroup xsecure_plat.h XilSecure Versal_Aiepg2 APIs
* @{
* @cond xsecure_internal
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
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
#include "xsecure_error.h"
#include "xpmcdma.h"
#include "xtrngpsx.h"
#include "xsecure_plat_defs.h"
#include "xsecure_trng.h"

/************************** Constant Definitions ****************************/
#define XSECURE_SSS_MAX_SRCS	(8U)	/**< Maximum resources */

#define XSECURE_SSS_SHA3_MASK		(0xF0000U) /**<SSS SHA3 instance mask value*/
#define XSECURE_SSS_SHA2_MASK		(0xF000000U) /**<SSS SHA3 instance mask value*/

#define XSECURE_SSS_SHA3_DMA0_VAL	(0xC0000U) /**<SSS SHA3 instance  DMA0 value*/
#define XSECURE_SSS_SHA3_DMA1_VAL	(0x70000U) /**<SSS SHA3 instance  DMA1 value*/

#define XSECURE_SSS_SHA2_DMA0_VAL	(0xA000000U) /**<SSS SHA2 instance DMA0 value*/
#define XSECURE_SSS_SHA2_DMA1_VAL	(0xF000000U) /**<SSS SHA2 instance DMA1 value*/

#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)  /**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)  /**< Disables data swap in AES */

#if !defined(XSECURE_TRNG_USER_CFG_SEED_LIFE)
#define XSECURE_TRNG_USER_CFG_SEED_LIFE XTRNGPSX_USER_CFG_SEED_LIFE
								/**< User configuration seed life*/
#endif

#if !defined(XSECURE_TRNG_USER_CFG_DF_LENGTH)
#define XSECURE_TRNG_USER_CFG_DF_LENGTH XTRNGPSX_USER_CFG_DF_LENGTH
								/**< User configuration DF length */
#endif

#if !defined(XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF
								/**< Adapt test cutoff */
#endif

#if !defined(XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF XTRNGPSX_USER_CFG_REP_TEST_CUTOFF
								/**< Rep test cutoff */
#endif
#define XSECURE_AES_KTE_GO_ADDRESS	(0xF11E0234U)	/**< AES KTE GO address */
#define XSECURE_AES_KTE_RESTART_ADDRESS	(0xF11E0238U)	/**< AES Key Transfer Engine Restart register */
#define XSECURE_AES_KTE_DONE_ADDRESS	(0xF11E023CU)	/**< AES Key Transfer Engine Done */
#define XSECURE_AES_KTE_CNT_ADDRESS	(0xF11E0240U)	/**< AES Key Transfer Engine Count address */
#define XSECURE_AES_KTE_GO_ENABLE	(0x1U)		/**< AES KTE GO enable */
#define XSECURE_AES_KTE_GO_DISABLE	(0x0U)		/**< AES KTE GO disable */
#define XSECURE_AES_KTE_DONE_MASK	(0x1U) 		/**< AES Key Transfer Engine Done mask */
#define XSECURE_AES_KTE_CNT_MASK	(0x6U)		/**< AES Key Transfer Engine Count value */

/***************************** Type Definitions******************************/
/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1,  /**< Ignore */
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3, /**< SHA3 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_SHA2, /**< SHA2 */
	XSECURE_SSS_INVALID /**< Invalid */
} XSecure_SssSrc;

typedef struct {
	u64 SrcDataAddr;        /**< Address of source buffer */
	u64 DestDataAddr;       /**< Address of destination buffer */
	u8 SrcChannelCfg;       /**< DMA Source channel configuration */
	u8 DestChannelCfg;      /**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;      /**< Flag for last update in source */
	u8 IsLastChunkDest;     /**< Flag for last update in destination */
}XSecure_AesDmaCfg;

/***************************** Function Prototypes ***************************/

void XSecure_SetRsaCryptoStatus(void);
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op);
void XSecure_UpdateTrngCryptoStatus(u32 Op);
void XSecure_ConfigureDmaByteSwap(u32 Op);
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
       XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
int XSecure_ECCRandInit(void);
int XSecure_TrngInitNCfgHrngMode(void);
XTrngpsx_Instance *XSecure_GetTrngInstance(void);

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */

/* @} */
