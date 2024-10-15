/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3 core hardware definitions for Versalnet.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
* 5.4   tri  09/25/24 Added maximum supported hash size macro
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
#define XSECURE_SHA3_NUM_OF_INSTANCES		(2U) /**< Number of instances*/

#define XSECURE_SHA_NUM_OF_INSTANCES            (XSECURE_SHA3_NUM_OF_INSTANCES)

#define XSECURE_SHA3_0_BASE_ADDRESS		(0xF1210000U) /**< SHA3 0 base address */

#define XSECURE_SHA3_0_DEVICE_ID		(0U) /**< SHA3 0 device id */

#define XSECURE_SHA3_1_BASE_ADDRESS		(0xF1800000U) /**< SHA3 1 base address */

#define XSECURE_SHA3_1_DEVICE_ID		(1U) /**< SHA3 1 device id */

#define XSECURE_SHA_0_DEVICE_ID			(XSECURE_SHA3_0_DEVICE_ID)

#define XSECURE_SHA_1_DEVICE_ID			(XSECURE_SHA3_1_DEVICE_ID)

#define XSECURE_MAX_HASH_SIZE_IN_BYTES          (48U)  /**< SHA3 maximum supported hash size */

/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_SHA3_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_SHA3_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_SHA3_DONE_OFFSET	(0x08U) /**< SHA Done Register */
#define XSECURE_SHA3_DIGEST_0_OFFSET	(0x10U)	/**< SHA3 Digest: Reg 0 */
/* @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
