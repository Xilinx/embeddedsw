/***************************************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.7   rmv  01/30/26 Refactor OCP library
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

#define XOCP_EFUSE_PPK_0_HASH_START_OFFSET		(0xF1250100U) /**< PPK Hash 0 cache start offset */
#define XOCP_EFUSE_PPK_1_HASH_START_OFFSET		(0xF1250120U) /**< PPK Hash 1 cache start offset */
#define XOCP_EFUSE_PPK_2_HASH_START_OFFSET		(0xF1250140U) /**< PPK Hash 2 cache start offset */
#define XOCP_EFUSE_REVOCATION_ID_0_START_OFFSET		(0xF12500B0U) /**< Revocation ID 0 cache start offset */
#define XOCP_EFUSE_OFFCHIP_ID_0_START_OFFSET		(0xF1250160U) /**< OffChip Id 0 cache start offset */

#define XOCP_EFUSE_PPK0_WR_LK_MASK			(0x00000040U) /**< PPK 0 write lock eFuse mask */
#define XOCP_EFUSE_PPK1_WR_LK_MASK			(0x00000080U) /**< PPK 1 write lock eFuse mask */
#define XOCP_EFUSE_PPK2_WR_LK_MASK			(0x00000100U) /**< PPK 2 write lock eFuse mask */

#define XOCP_EFUSE_PPK0_INVLD_1_0_MASK			(0x0000000CU) /**< PPK 0 invalid eFuse mask */
#define XOCP_EFUSE_PPK1_INVLD_1_0_MASK			(0x00000030U) /**< PPK 1 invalid eFuse mask */
#define XOCP_EFUSE_PPK2_INVLD_1_0_MASK			(0x000000C0U) /**< PPK 2 invalid eFuse mask */

#define XOCP_EFUSE_DME_REVOKE_0_MASK			(0x00000030U) /**< DME revoke 0 eFuse mask */
#define XOCP_EFUSE_DME_REVOKE_1_MASK			(0x000000C0U) /**< DME revoke 1 eFuse mask */
#define XOCP_EFUSE_DME_REVOKE_2_MASK			(0x00000300U) /**< DME revoke 2 eFuse mask */
#define XOCP_EFUSE_DME_REVOKE_3_MASK			(0x00000C00U) /**< DME revoke 3 eFuse mask */

#define XOCP_EFUSE_UDS_WR_LK_MASK			(0x00000010U) /**< UDS write lock eFuse mask */
#define XOCP_EFUSE_HWTST_BIT_DIS_MASK			(0x00000008U) /**< HWTST bit disable eFuse mask */
#define XOCP_EFUSE_PUF_DIS_MASK				(0x00040000U) /**< PUF Disable eFuse mask */
#define XOCP_EFUSE_PMC_SC_EN_MASK			(0x03800000U) /**< PMC_SC_EN eFuse mask */
#define XOCP_EFUSE_SYSMON_TEMP_MON_EN_MASK		(0x00000003U) /**< SYSMON_TEMP_MON_EN eFuse mask */
#define XOCP_EFUSE_DME_MODE_MASK			(0x0000000FU) /**< DME_MODE eFuse mask */

#define XOCP_PMC_TAP_DAP_CFG_OFFSET			(0xF11B0008U) /**< DAP CFG register address */
#define XOCP_PMC_TAP_INST_MASK_0_OFFSET			(0xF11B0000U) /**< Instruction Mask 0 register address */
#define XOCP_PMC_TAP_INST_MASK_1_OFFSET			(0xF11B0004U) /**< Instruction Mask 1 register address */
#define XOCP_PMC_TAP_DAP_SECURITY_OFFSET		(0xF11B000CU) /**< DAP security register address */

#define XOCP_EFUSE_CACHE_BOOT_ENV_CTRL			(0xF1250094U) /**< Boot environmental register address */
#define XOCP_PMC_LOCAL_BOOT_MODE_DIS			(0xF00441D0U) /**< Boot mode register address */
#define XOCP_EFUSE_CACHE_MISC_CTRL			(0xF12500A0U) /**< MISC control register address */
#define XOCP_EFUSE_CACHE_ANLG_TRIM_3			(0xF1250010U) /**< Analog Trim3 register address */
#define XOCP_EFUSE_CACHE_IP_DISABLE_0			(0xF1250018U) /**< IP disable 0 register address */
#define XOCP_EFUSE_CACHE_IP_DISABLE_1			(0xF125001CU) /**< IP disable 1 register address */
#define XOCP_EFUSE_CACHE_CAHER_1			(0xF12500F0U) /**< Caher1 register address */
#define XOCP_EFUSE_CACHE_SECURITY_MISC_0		(0xF12500E4U) /**< MISC 0 control register address */
#define XOCP_EFUSE_CACHE_SECURITY_CONTROL		(0xF12500ACU) /**< security control register address */
#define XOCP_EFUSE_CACHE_SECURITY_MISC_1		(0xF12500E8U) /**< security misc 1 control register address */
#define XOCP_EFUSE_CACHE_DME_FIPS			(0xF1250234U) /**< DME FIPS register address */
#define XOCP_EFUSE_CACHE_ROM_RSVD			(0xF1250090U) /**< ROM reserved register address */
#define XOCP_EFUSE_CACHE_RO_SWAP_EN			(0xF12500D0U) /**< RO SWAP enable address */

#define XOCP_CAHER_1_MEASURED_MASK			(0x00000F00U) /**< HNIC DIS| DDR XTS export | HNIC DDR export | DDR XTS GCM DIS */
#define XOCP_DEC_ONLY_MEASURED_MASK			(0x0000FFFFU) /**< Decrypt only mask */
#define XOCP_SEC_CTRL_MEASURED_MASK			(0x03FF001FU) /**< Security control measured mask */
#define XOCP_PMC_LOCAL_BOOT_MODE_DIS_FULLMASK		(0x0000FFFFU) /**< PMC local disable mask */
#define XOCP_MISC_CTRL_MEASURED_MASK			(0xE018C100U) /**< MISC control mask */
#define XOCP_DME_FIPS_MEASURED_MASK			(0xFF00000FU) /**< DME FIPS mask */
#define XOCP_IP_DISABLE0_MEASURED_MASK			(0xF0000F04U) /**< IP disable mask */
#define XOCP_ROM_RSVD_MEASURED_MASK			(0x000007C0U) /**< ROM reserved mask */

#define XOCP_PMC_PLM_HASH_ADDR				(0xF1110750U) /**< PLM hash address */
#define XOCP_PMC_ROM_HASH_ADDR				(0xF1110704U) /**< ROM hash address */

#define XOCP_EFUSE_DEVICE_DNA_CACHE			(0xF1250020U) /**< DNA cache */
/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_HW_H */
