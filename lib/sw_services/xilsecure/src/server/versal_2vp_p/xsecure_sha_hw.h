/***************************************************************************************************
* Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/versal_2vp_p/xsecure_sha_hw.h
*
* This file contains SHA3/SHA2 core hardware definitions for versal_2vp_p
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*       tvp  02/23/26 Move SHA mode and auto padding offset from common file
*       tvp  02/23/26 Add SHAKE256 SLH-DSA Chaining algorithm support
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_SHA_HW_H
#define XSECURE_SHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_utils.h"

/************************************** Constant Definitions **************************************/
#define XSECURE_SHA_CHAIN_MODE_EN				/**< Enable SHA chain mode */

#define XSECURE_SHA_NUM_OF_INSTANCES		(2U)	/**< Number of Sha instances*/

#define XSECURE_SHA3_DEVICE_ID			(0U)	/**< SHA3 device id */

#define XSECURE_SHA2_DEVICE_ID			(1U)	/**< SHA2 device id */

#define XSECURE_SHA3_BASE_ADDRESS		(0xF1210000U)	/**< SHA3 base address */

#define XSECURE_SHA2_BASE_ADDRESS		(0xF1218000U)	/**< SHA2 base address */

#define XSECURE_SHA_0_DEVICE_ID			(XSECURE_SHA3_DEVICE_ID)	/**< SHA 0 device id */

#define XSECURE_SHA_1_DEVICE_ID			(XSECURE_SHA2_DEVICE_ID)	/**< SHA 1 device id */

#define XSECURE_MAX_HASH_SIZE_IN_BYTES		(168U)	/**< SHA3 maximum supported hash size */

/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_SHA_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_SHA_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_SHA3_RESET_OFFSET 	XSECURE_SHA_RESET_OFFSET /**< SHA3 Reset Register */
#define XSECURE_SHA_DONE_OFFSET		(0x08U) /**< SHA Done Register */
#define XSECURE_SHA_DIGEST_0_OFFSET	(0x10U)	/**< SHA Digest: Reg 0 */
#define XSECURE_SHA3_MODE_OFFSET		(0xC0U) /**< SHA3 Mode Register */
#define XSECURE_SHA3_AUTO_PADDING_OFFSET	(0xC4U) /**< SHA3 Auto Padding Register */
#define XSECURE_SHA2_MODE_OFFSET		(0xA0U) /**< SHA2 Mode Register */
#define XSECURE_SHA2_AUTO_PADDING_OFFSET	(0xA4U) /**< SHA2 Auto Padding Register */
#define XSECURE_SHA3_CHAIN_STEPS_BIT_POS	(16U)	/**< SHA3 chaining steps bit position */
#define XSECURE_SHA3_CHAIN_OFFSET		(0xC8U)	/**< SHA chain control register */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
