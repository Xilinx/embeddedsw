/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3 core hardware definitions for Versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  03/20/20 Initial release
* 4.2   har  03/20/20 Updated file version to sync with library version
* 4.3   am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_SHA_HW_H
#define XSECURE_SHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xsecure_utils.h"

/************************** Constant Definitions ****************************/
/**< SHA3 0 base address */
#define XSECURE_SHA3_0_BASE_ADDRESS		(0xF1210000U)

/**< SHA3 1 base address */
#define XSECURE_SHA3_1_BASE_ADDRESS		(0xF1800000U)


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
