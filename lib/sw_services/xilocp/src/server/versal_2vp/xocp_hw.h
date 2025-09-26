/***************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_hw.h
*
* This file contains versal_2vp related hardware macros and declarations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- --------   -------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
*
* </pre>
*
******************************************************************************/
#ifndef XOCP_HW_H
#define XOCP_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_hw.h"

/************************************ Constant Definitions ****************************************/
/**
 * Register: XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0
 */
#define XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0	(PMC_GLOBAL_BASEADDR + 0x00000750U)

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/
#define XOCP_EFUSE_CACHE_BLACK_IV_0		(0xF12501D0U) /**< Black IV 0 register address*/
#define XOCP_UDS_EFUSE_CACHE_ADDR		(0xF1250230U) /**< Encrypted UDS Key corresponding
							       * User eFuse address */
#define XOCP_DME_USER0_EFUSE_CACHE_ADDR		(0xF1250260U) /**< DME Encrypted Private Key0
							       * corresponding User eFuse address */
#define XOCP_DME_USER1_EFUSE_CACHE_ADDR		(0xF1250290U) /**< DME Encrypted Private Key1
							       * corresponding User eFuse address */
#define XOCP_PUF_CHASH_EFUSE_CACHE_ADDR		(0xF12500A8U) /**< PUF CHASH corresponding eFuse
							       * address */

#define XOCP_DME_KEY_0_REVOKE_MASK		(0x01800000U) /**< Revoke mask for DME key 0*/
#define XOCP_DME_KEY_1_REVOKE_MASK		(0x06000000U) /**< Revoke mask for DME key 1*/

#define XOCP_CDI_SEED_VALID			(1U) /**< To set CDI seed is valid */

#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK		(0x00001000U)
								/**< Error mask for CDI SEED parity*/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_HW_H */
