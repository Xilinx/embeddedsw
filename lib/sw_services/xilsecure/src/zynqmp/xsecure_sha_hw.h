/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3 core hardware definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/09/19 Initial release
* 4.2   har  11/07/19 Typo correction to enable compilation in C++
*                     Typo correction in comments
*       har  03/20/20 Moved the file to zynqmp directory and removed versal
*                     related code
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

/************************** Constant Definitions ****************************/
/**< SHA3 base address */
#define XSECURE_CSU_SHA3_BASE		(0xFFCA2000U)

/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_CSU_SHA3_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_CSU_SHA3_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_CSU_SHA3_DONE_OFFSET	(0x08U) /**< SHA Done Register */

#define XSECURE_CSU_SHA3_DIGEST_0_OFFSET (0x10U)
					/**< SHA3 Digest: Reg 0 */
#define XSECURE_CSU_SHA3_DIGEST_11_OFFSET (0x3CU)
					/**< SHA3 Digest: Last Register */
/* @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA_HW_H */
