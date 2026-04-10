/***************************************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_hw.h
*
* This file contains versal_net related hardware macros and declarations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.6   tvp  05/16/25 Initial release
* 1.7   rmv  01/30/26 Refactor OCP library
*       rpu  02/18/26 Fixed Doxygen warnings
* </pre>
*
***************************************************************************************************/
#ifndef XOCP_HW_H
#define XOCP_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_hw.h"

/************************************ Constant Definitions ****************************************/
/** @cond xocp_internal
 * @{
 */

/**
 * Register: XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0
 */
#define XOCP_PMC_GLOBAL_PMC_FW_AUTH_HASH_0	(PMC_GLOBAL_BASEADDR + 0x00000750U)

/**
 * Register: XOCP_PMC_GLOBAL_PCR_0_0
 */
#define XOCP_PMC_GLOBAL_PCR_0_0			(PMC_GLOBAL_BASEADDR + 0x00005200U)
/**
 * Register: XOCP_PMC_GLOBAL_PCR_0_0
 */
#define XOCP_PMC_GLOBAL_PCR_7_0			(PMC_GLOBAL_BASEADDR + 0x00005350U)

/**
 * Register:  XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0	(PMC_GLOBAL_BASEADDR + 0x00005468U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0	(PMC_GLOBAL_BASEADDR + 0x00005498U)
/** @} */

#define XOCP_PMC_GLOBAL_PCR_OP_IDX_SHIFT		(3U)	/**< Index shift option for PCR */

/**
 * Register:  XOCP_PMC_GLOBAL_PCR_OP
 * @{
 */
#define XOCP_PMC_GLOBAL_PCR_OP			(PMC_GLOBAL_BASEADDR + 0x00011000U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_PCR_EXTEND_INPUT_0
 * @{
 */
#define XOCP_PMC_GLOBAL_PCR_EXTEND_INPUT_0	(PMC_GLOBAL_BASEADDR + 0x00011004U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_PCR_OP_STATUS
 * @{
 */
#define XOCP_PMC_GLOBAL_PCR_OP_STATUS			(PMC_GLOBAL_BASEADDR + 0x00011034U)
#define XOCP_PMC_GLOBAL_PCR_OP_STATUS_ERROR_MASK	(0x000003FCU) /**< PCR operation status error mask */
#define XOCP_PMC_GLOBAL_PCR_OP_STATUS_PASS_MASK		(0x00000002U) /**< PCR operation status pass mask */
#define XOCP_PMC_GLOBAL_PCR_OP_STATUS_DONE_MASK		(0x00000001U) /**< PCR operation status done mask */
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DICE_CDI_SEED_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_0			(PMC_GLOBAL_BASEADDR + 0x00011100U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID
 * @{
 */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID		(PMC_GLOBAL_BASEADDR + 0x00011130U)
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_VALID_VAL_MASK	(0x00000001U) /**< DICE CDI seed valid value mask */
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY
 * @{
 */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY			(PMC_GLOBAL_BASEADDR + 0x00011134U)
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_SHIFT	(12U) /**< DICE CDI seed parity error shift */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK		(0x00001000U) /**< DICE CDI seed parity error mask */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_VAL_WIDTH		(12U) /**< DICE CDI seed parity value width */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_VAL_MASK		(0x00000FFFU) /**< DICE CDI seed parity value mask */
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_CTRL
 * @{
 */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_CTRL		(PMC_GLOBAL_BASEADDR + 0x00011138U)
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_CTRL_ZEROIZE_MASK	(0x00000001U) /**< DICE CDI seed zeroize control mask */
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_STATUS
 * @{
 */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_STATUS		(PMC_GLOBAL_BASEADDR + 0x0001113CU)
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_STATUS_PASS_MASK	(0x00000002U) /**< Zeroize status pass mask */
#define XOCP_PMC_GLOBAL_DICE_CDI_SEED_ZEROIZE_STATUS_DONE_MASK	(0x00000001U) /**< Zeroize status done mask */
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_0	(PMC_GLOBAL_BASEADDR + 0x00011200U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0	(PMC_GLOBAL_BASEADDR + 0x00011230U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0
 * @{
 */
#define XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_Y_0	(PMC_GLOBAL_BASEADDR + 0x00011260U)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_CTRL
 * @{
 */
#define XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_CTRL		(PMC_GLOBAL_BASEADDR + 0x0001129CU)
/** @} */

/**
 * Register:  XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_STATUS
 * @{
 */
#define XOCP_PMC_GLOBAL_DEV_IK_PRIVATE_ZEROIZE_STATUS		(PMC_GLOBAL_BASEADDR + 0x000112A0U)
/** @} */

/**
 * Register:  XPPU configurations required for DME
 * @{
 */
#define PMC_XPPU_CTRL				(PMC_XPPU_BASEADDR + 0x00000000U) /**< XPPU control register */
#define PMC_XPPU_CTRL_ENABLE_MASK		(0x00000001U) /**< XPPU control enable mask */
#define PMC_XPPU_CTRL_DEFVAL			(0U) /**< XPPU control default value */

#define PMC_XPPU_DYNAMIC_RECONFIG_EN		(PMC_XPPU_BASEADDR + 0x000000FCU) /**< XPPU dynamic reconfiguration enable */
#define PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL	(0x0U) /**< XPPU dynamic reconfig enable default value */
#define PMC_XPPU_MASTER_ID00			(PMC_XPPU_BASEADDR + 0x00000100U) /**< XPPU master ID 00 */
#define PMC_XPPU_MASTER_ID01			(PMC_XPPU_BASEADDR + 0x00000104U) /**< XPPU master ID 01 */

#define PMC_XPPU_DYNAMIC_RECONFIG_APER_ADDR	(PMC_XPPU_BASEADDR + 0x00000150U) /**< XPPU dynamic reconfig aperture address */
#define PMC_XPPU_DYNAMIC_RECONFIG_APER_PERM	(PMC_XPPU_BASEADDR + 0x00000154U) /**< XPPU dynamic reconfig aperture permission */


#define PMC_XPPU_APERPERM_017			(PMC_XPPU_BASEADDR + 0x00001044U) /**< XPPU aperture permission 017 */
#define PMC_XPPU_APERPERM_018			(PMC_XPPU_BASEADDR + 0x00001048U) /**< XPPU aperture permission 018 */
#define PMC_XPPU_APERPERM_019			(PMC_XPPU_BASEADDR + 0x0000104CU) /**< XPPU aperture permission 019 */
#define PMC_XPPU_APERPERM_020			(PMC_XPPU_BASEADDR + 0x00001050U) /**< XPPU aperture permission 020 */
#define PMC_XPPU_APERPERM_021			(PMC_XPPU_BASEADDR + 0x00001054U) /**< XPPU aperture permission 021 */
#define PMC_XPPU_APERPERM_026			(PMC_XPPU_BASEADDR + 0x00001068U) /**< XPPU aperture permission 026 */
#define PMC_XPPU_APERPERM_027			(PMC_XPPU_BASEADDR + 0x0000106CU) /**< XPPU aperture permission 027 */
#define PMC_XPPU_APERPERM_028			(PMC_XPPU_BASEADDR + 0x00001070U) /**< XPPU aperture permission 028 */
#define PMC_XPPU_APERPERM_030			(PMC_XPPU_BASEADDR + 0x00001078U) /**< XPPU aperture permission 030 */
#define PMC_XPPU_APERPERM_033			(PMC_XPPU_BASEADDR + 0x00001084U) /**< XPPU aperture permission 033 */
#define PMC_XPPU_APERPERM_032			(PMC_XPPU_BASEADDR + 0x00001080U) /**< XPPU aperture permission 032 */
#define PMC_XPPU_APERPERM_035			(PMC_XPPU_BASEADDR + 0x0000108CU) /**< XPPU aperture permission 035 */
#define PMC_XPPU_APERPERM_037			(PMC_XPPU_BASEADDR + 0x00001094U) /**< XPPU aperture permission 037 */
#define PMC_XPPU_APERPERM_038			(PMC_XPPU_BASEADDR + 0x00001098U) /**< XPPU aperture permission 038 */
#define PMC_XPPU_APERPERM_049			(PMC_XPPU_BASEADDR + 0x000010C4U) /**< XPPU aperture permission 049 */
#define PMC_XPPU_APERPERM_146			(PMC_XPPU_BASEADDR + 0x00001248U) /**< XPPU aperture permission 146 */
#define PMC_XPPU_APERPERM_147			(PMC_XPPU_BASEADDR + 0x0000124CU) /**< XPPU aperture permission 147 */
#define PMC_XPPU_APERPERM_386			(PMC_XPPU_BASEADDR + 0x00001608U) /**< XPPU aperture permission 386 */
#define PMC_XPPU_LOCK				(PMC_XPPU_BASEADDR + 0x00000020U) /**< XPPU lock register */
#define PMC_XPPU_LOCK_DEFVAL			(0U) /**< XPPU lock default value */
/** @} */

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
/** @}
 * @endcond
 */
/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_HW_H */
