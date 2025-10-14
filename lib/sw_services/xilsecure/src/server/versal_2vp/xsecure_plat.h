/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_plat.h
* This file contains versal_2vp specific code for xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.6   tvp  07/07/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xsecure_sha_hw.h"
#include "xsecure_aes_core_hw.h"
#include "xpmcdma.h"
#include "xsecure_error.h"
#include "xsecure_core.h"

/************************************ Constant Definitions ****************************************/

#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */

#define XSECURE_SSS_MAX_SRCS		(8U)	/**< SSS Maximum resources */

#define XSECURE_SSS_SHA3_0_MASK		(0xF0000U) /**< SSS SHA3 instance 0 mask value*/

#define XSECURE_SSS_SHA3_0_DMA0_VAL	(0xC0000U) /**< SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_SSS_SHA3_0_DMA1_VAL	(0x70000U) /**< SSS SHA3 instance 0 DMA1 value*/
#define XSECURE_AES_NO_CFG_DST_DMA 	(0xFFFFFFFFU) /**< Not to configure Dst DMA at this address in AES*/
#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)	/**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)	/**< Disables data swap in AES */

#define XSECURE_SSS_SBI_MASK		(0xF00000U)	/**< SSS SBI mask */
#define XSECURE_SSS_AES_MASK		(0xF000U)	/**< SSS AES mask */
#define XSECURE_SSS_DMA1_MASK		(0xF0U)	/**< SSS DMA1 mask */
#define XSECURE_SSS_DMA0_MASK		(0xFU)	/**< SSS DMA0 mask */
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)	/**< SSS Source Select mask */
#define XSECURE_SSS_SBI_DMA0_VAL	(0x500000U)	/**< SSS SBI DMA0 value */
#define XSECURE_SSS_SBI_DMA1_VAL	(0xB00000U)	/**< SSS SBI DMA1 value */
#define XSECURE_SSS_AES_DMA0_VAL	(0xE000U)	/**< SSS AES DMA0 value */
#define XSECURE_SSS_AES_DMA1_VAL	(0x5000U)	/**< SSS AES DMA1 value */

/************************************** Type Definitions ******************************************/

/** Sources to be selected to configure secure stream switch. */
typedef enum {
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3_0, /**< SHA3_0 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_PZI, /**< PZI */
	XSECURE_SSS_INVALID /**< Invalid */
}XSecure_SssSrc;

/** This structure contains parameters to configure DMA for AES */
typedef struct {
	u64 SrcDataAddr;	/**< Address of source buffer */
	u64 DestDataAddr;	/**< Address of destination buffer */
	u8 SrcChannelCfg;	/**< DMA Source channel configuration */
	u8 DestChannelCfg;	/**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;	/**< Flag for last update in source */
	u8 IsLastChunkDest;	/**< Flag for last update in destination */
} XSecure_AesDmaCfg;

/** This structure contains parameters to configure key source for AES */
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

/************************************ Function Prototypes *****************************************/
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, const XSecure_AesDmaCfg *AesDmaCfg,
				    u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr, XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
void XSecure_ConfigureDmaByteSwap(u32 Op);
int XSecure_GetRandomNum(u8 *Output, u32 Size);
int XSecure_ECCRandInit(void);
/**
 * @}
 * @endcond
 */
/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @param	BaseAddress	 Base address of the core.
 * @param	Op		 To set or clear the bit.
 *
 **************************************************************************************************/
static inline void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
	/* Not applicable for versal_2vp */
	(void)BaseAddress;
	(void)Op;
}

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 **************************************************************************************************/
static inline void XSecure_SetRsaCryptoStatus(void)
{
	/* Not applicable for versal_2vp */
}

/************************************* Variable Prototypes ****************************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */
/** @} */
