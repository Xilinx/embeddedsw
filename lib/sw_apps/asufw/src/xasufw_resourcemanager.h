/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_resourcemanager.h
 *
 * Header file for managing hardware resources which contains the list of hardware resources and
 * declarations for xasufw_resourcemanager.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  02/08/24 Initial release
 *       ma   05/14/24 Modify resource manager functionality to check resources availability based
 *                     on resources mask and allocate resources
 *       ma   06/04/24 Check if random bytes are available or not for TRNG GetRandomBytes command
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       ma   01/15/25 Added KDF to the resources list
 *       yog  02/20/25 Added ECIES to the resources list
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_RESOURCEMANAGER_H_
#define XASUFW_RESOURCEMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_modules.h"
#include "xasufw_dma.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_REQUEST_ID_PLI			0xFFFFFFFFU		/**< Request ID of PLI */

#define XASUFW_DMA_RESOURCE_MASK        0x1U /**< DMA resource mask */
#define XASUFW_AES_RESOURCE_MASK        0x2U /**< AES resource mask */
#define XASUFW_SHA2_RESOURCE_MASK       0x4U /**< SHA2 resource mask */
#define XASUFW_SHA3_RESOURCE_MASK       0x8U /**< SHA3 resource mask */
#define XASUFW_PLI_RESOURCE_MASK        0x10U /**< PLI resource mask */
#define XASUFW_TRNG_RESOURCE_MASK       0x20U /**< TRNG resource mask */
#define XASUFW_TRNG_RANDOM_BYTES_MASK	0x40U /**< TRNG random bytes mask */
#define XASUFW_ECC_RESOURCE_MASK        0x80U /**< ECC resource mask */
#define XASUFW_RSA_RESOURCE_MASK        0x100U /**< RSA resource mask */
#define XASUFW_HMAC_RESOURCE_MASK       0x200U /**< HMAC resource mask */
#define XASUFW_KDF_RESOURCE_MASK		0x400U /**< KDF resource mask */
#define XASUFW_ECIES_RESOURCE_MASK		0x800U /**< ECIES resource mask */
#define XASUFW_KEYWRAP_RESOURCE_MASK	0x1000U /**< Keywrap unwrap resource mask */
#define XASUFW_RSA_SHA_RESOURCE_MASK	0x2000U /**< SHA resource mask for Edward curves */

/************************************** Type Definitions *****************************************/
/**
 * @brief Enumeration of ASUFW resources managed by the resource manager.
 */
typedef enum {
	XASUFW_DMA0,	/**< 0: DMA 0 */
	XASUFW_AES,		/**< 1: AES */
	XASUFW_SHA2,	/**< 2: SHA2 */
	XASUFW_SHA3,	/**< 3: SHA3 */
	XASUFW_PLI,		/**< 4: PLI */
	XASUFW_DMA1,	/**< 5: DMA 1 */
	XASUFW_TRNG,	/**< 6: TRNG */
	XASUFW_ECC,		/**< 7: ECC */
	XASUFW_RSA,		/**< 8: RSA */
	XASUFW_HMAC,	/**< 9: HMAC */
	XASUFW_KDF,		/**< 10: KDF */
	XASUFW_ECIES,	/**< 11: ECIES */
	XASUFW_KEYWRAP,	/**< 12: Key wrap unwrap */
	XASUFW_NONE,	/**< 13: None */
	XASUFW_INVALID,	/**< 14: Invalid */
} XAsufw_Resource;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XAsufw_ResourceInit(void);
s32 XAsufw_CheckResourceAvailability(XAsufw_ResourcesRequired Resources, u32 ReqId,
	const XAsu_ReqBuf *ReqBuf);
XAsufw_Dma *XAsufw_AllocateDmaResource(XAsufw_Resource Resource, u32 ReqId);
s32 XAsufw_ReleaseResource(XAsufw_Resource Resource, u32 ReqId);
void XAsufw_AllocateResource(XAsufw_Resource Resource, XAsufw_Resource MainResource, u32 ReqId);
s32 XAsufw_ReleaseDmaResource(XAsufw_Dma *AsuDmaPtr, u32 ReqId);
void XAsufw_IdleResource(XAsufw_Resource Resource);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_RESOURCEMANAGER_H_ */
/** @} */
