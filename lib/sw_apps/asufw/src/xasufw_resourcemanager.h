/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_RESOURCEMANAGER_H
#define XASUFW_RESOURCEMANAGER_H

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

/************************************** Type Definitions *****************************************/
/**
 * @brief Enumeration of hardware resources managed by the resource manager.
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
	XASUFW_INVALID	/**< 10: Invalid */
} XAsufw_Resource;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XAsufw_ResourceInit(void);
s32 XAsufw_AllocateAesResources(u32 RequesterId);
s32 XAsufw_ReleaseAesResources(u32 RequesterId);
s32 XAsufw_CheckResourceAvailability(XAsufw_ResourcesRequired Resources, u32 RequesterId);
XAsufw_Dma *XAsufw_AllocateDmaResource(XAsufw_Resource Resource, u32 RequesterId);
s32 XAsufw_ReleaseResource(XAsufw_Resource Resource, u32 RequesterId);
void XAsufw_AllocateResource(XAsufw_Resource Resource, u32 RequesterId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_RESOURCEMANAGER_H */
/** @} */
