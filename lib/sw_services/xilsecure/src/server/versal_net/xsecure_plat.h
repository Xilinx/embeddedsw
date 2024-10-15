/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
*
* This file contains Versal Net specific code for Xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
* 5.2   yog  07/10/23 Added support of unaligned data sizes for Versal Net
*       kpt  07/09/23 Added XSecure_GetRandomNum function
*	vss  09/21/23 Fixed doxygen warnings
* 5.3	vss  10/12/23 Removed XSECURE_SSS_IGNORE to fix MISRA-C Rule 18.1 violation
		      and OVER_RUN coverity warning
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*       kpt  06/30/24 Added XSecure_MemCpyAndChangeEndianness
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xpmcdma.h"
#include "xsecure_aes_core_hw.h"
#include "xsecure_error.h"
#include "xtrngpsx.h"
#include "xsecure_trng.h"
#include "xsecure_core.h"

/************************** Constant Definitions ****************************/

#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */

#define XSECURE_SSS_MAX_SRCS	(8U)	/**< Maximum resources */

#define XSECURE_SSS_SHA3_0_MASK		(0xF0000U) /**<SSS SHA3 instance 0 mask value*/
#define XSECURE_SSS_SHA3_1_MASK		(0xF000000U) /**<SSS SHA3 instance 1 mask value*/

#define XSECURE_SSS_SHA3_0_DMA0_VAL	(0xC0000U) /**<SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_SSS_SHA3_0_DMA1_VAL	(0x70000U) /**<SSS SHA3 instance 0 DMA1 value*/

#define XSECURE_SSS_SHA3_1_DMA0_VAL	(0xA000000U) /**<SSS SHA3 instance 1 DMA0 value*/
#define XSECURE_SSS_SHA3_1_DMA1_VAL	(0xF000000U) /**<SSS SHA3 instance 1 DMA1 value*/
#define XSECURE_AES_NO_CFG_DST_DMA	(0xFFFFFFFFU) /**< Not to configure Dst DMA at this address in AES*/
#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)  /**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)  /**< Disables data swap in AES */

#define XSECURE_AES_ECB_OFFSET		  (0x20U) /**< AES ECB offset */

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

#define XSECURE_SSS_SBI_MASK	(0xF00000U)
#define XSECURE_SSS_AES_MASK	(0xF000U)
#define XSECURE_SSS_DMA1_MASK	(0xF0U)
#define XSECURE_SSS_DMA0_MASK	(0xFU)
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)
#define XSECURE_SSS_SBI_DMA0_VAL	(0x500000U)
#define XSECURE_SSS_SBI_DMA1_VAL	(0xB00000U)
#define XSECURE_SSS_AES_DMA0_VAL	(0xE000U)
#define XSECURE_SSS_AES_DMA1_VAL	(0x5000U)

/***************************** Type Definitions******************************/

/** Sources to be selected to configure secure stream switch. */
typedef enum {
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3_0, /**< SHA3_0 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_SHA3_1, /**< SHA3_1 */
	XSECURE_SSS_INVALID /**< Invalid */
}XSecure_SssSrc;

/** Data structure contains parameters to configure DMA for AES */
typedef struct {
	u64 SrcDataAddr;        /**< Address of source buffer */
	u64 DestDataAddr;       /**< Address of destination buffer */
	u8 SrcChannelCfg;       /**< DMA Source channel configuration */
	u8 DestChannelCfg;      /**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;      /**< Flag for last update in source */
	u8 IsLastChunkDest;     /**< Flag for last update in destination */
}XSecure_AesDmaCfg;

typedef struct {
	u32 RegOffset;	/**< Register offset for key source */
	u32 KeySrcSelVal;	/**< Selection value for key source */
	u8  UsrWrAllowed;	/**< User write allowed or not for key source */
	u8  DecAllowed;		/**< Decryption allowed or not for key source */
	u8  EncAllowed;		/**< Encryption allowed or not for key source */
	u8  KeyDecSrcAllowed;	/**< Key decryption source allowed */
	u32 KeyDecSrcSelVal;	/**< Selection value for key decryption source*/
	u32 KeyClearVal;	/**< Key source clear value*/
} XSecure_AesKeyLookup;

/***************************** Function Prototypes ***************************/

void XSecure_SetRsaCryptoStatus(void);
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op);
void XSecure_UpdateTrngCryptoStatus(u32 Op);
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
       XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
int XSecure_ECCRandInit(void);
int XSecure_TrngInitNCfgHrngMode(void);
XTrngpsx_Instance *XSecure_GetTrngInstance(void);
int XSecure_MemCpyAndChangeEndianness(u64 DestAddress, u64 SrcAddress, u32 Length);

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */
/** @} */
