/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3/SHA2 core hardware definitions for versal_2ve_2vm.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       tri  10/07/24 Added maximum supported hash size macro
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_SHA_HW_H
#define XSECURE_SHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xsecure_utils.h"

/************************** Constant Definitions ****************************/
#define XSECURE_SHA_NUM_OF_INSTANCES            (2) /**< Number of Sha instances*/

#define XSECURE_SHA3_DEVICE_ID			(0U) /**< SHA3 device id */

#define XSECURE_SHA2_DEVICE_ID			(1U) /**< SHA2 device id */

#define XSECURE_SHA3_BASE_ADDRESS		(0xF1210000U) /**< SHA3 base address */

#define XSECURE_SHA2_BASE_ADDRESS		(0xF1800000U) /**< SHA2 base address */

#define XSECURE_SHA_0_DEVICE_ID			(XSECURE_SHA3_DEVICE_ID)

#define XSECURE_SHA_1_DEVICE_ID			(XSECURE_SHA2_DEVICE_ID)

#define XSECURE_MAX_HASH_SIZE_IN_BYTES          (64U)  /**< SHA3 maximum supported hash size */

/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_SHA_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_SHA_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_SHA3_RESET_OFFSET 	XSECURE_SHA_RESET_OFFSET
#define XSECURE_SHA_DONE_OFFSET		(0x08U) /**< SHA Done Register */
#define XSECURE_SHA_DIGEST_0_OFFSET	(0x10U)	/**< SHA Digest: Reg 0 */
/* @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
