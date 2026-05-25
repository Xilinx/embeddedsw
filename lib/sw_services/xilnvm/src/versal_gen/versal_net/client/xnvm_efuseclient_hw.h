/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal_net/client/xnvm_efuseclient_hw.h
*
* This file contains register offsets for EFUSE_CACHE module and other related
* definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 3.0   har  07/06/22   Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/

#ifndef XNVM_EFUSECLIENT_HW_H
#define XNVM_EFUSECLIENT_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
/*************************** Constant Definitions *****************************/
/**
 * @name eFUSE Cache Register Offsets
 * @{
 */
#define XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET		(0x00000180U)
                        /**< eFUSE cache meta header IV range 0 offset */
#define XNVM_EFUSE_CACHE_BLACK_IV_0_OFFSET			(0x000001D0U)
                        /**< eFUSE cache black IV 0 offset */
#define XNVM_EFUSE_CACHE_PLM_IV_RANGE_0_OFFSET			(0x000001DCU)
                        /**< eFUSE cache PLM IV range 0 offset */
#define XNVM_EFUSE_CACHE_DATA_PARTITION_IV_RANGE_0_OFFSET	(0x000001E8U)
                        /**< eFUSE cache data partition IV range 0 offset */
#define XNVM_EFUSE_CACHE_REVOCATION_ID_0_OFFSET			(0x000000B0U)
                        /**< eFUSE cache revocation ID 0 offset */
#define XNVM_EFUSE_CACHE_USER_FUSE_OFFSET			(0x00000240U)
                        /**< eFUSE cache user fuse offset */
#define XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET			(0x000000A0U)
                        /**< eFUSE cache miscellaneous control offset */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET		(0x000000ACU)
                        /**< eFUSE cache security control offset */
#define XNVM_EFUSE_CACHE_OFFCHIP_REVOKE_0_OFFSET		(0x00000160U)
                        /**< eFUSE cache off-chip revoke 0 offset */
#define XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET			(0x00000100U)
                        /**< eFUSE cache PPK0 hash 0 offset */
#define XNVM_EFUSE_CACHE_PPK0_HASH_1_OFFSET			(0x00000120U)
                        /**< eFUSE cache PPK0 hash 1 offset */
#define XNVM_EFUSE_CACHE_PPK0_HASH_2_OFFSET			(0x00000140U)
                        /**< eFUSE cache PPK0 hash 2 offset */
#define XNVM_EFUSE_CACHE_DEC_ONLY_OFFSET			(0x000000E4U)
                        /**< eFUSE cache decrypt only offset */
#define XNVM_EFUSE_CACHE_SEC_MISC1_OFFSET			(0x000000E8U)
                        /**< eFUSE cache security misc 1 offset */
#define XNVM_EFUSE_CACHE_DNA_OFFSET				(0x00000020U)
                        /**< eFUSE cache DNA offset */
#define XNVM_EFUSE_CACHE_PUF_ECC_CTRL_OFFSET			(0x000000A4U)
                        /**< eFUSE cache PUF ECC control offset */
#define XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET			(0x000000A8U)
                        /**< eFUSE cache PUF CHASH offset */
#define XNVM_EFUSE_CACHE_PUF_SYN_DATA_OFFSET			(0x00000300U)
                        /**< eFUSE cache PUF syndrome data offset */
#define XNVM_EFUSE_CACHE_PUF_RO_SWAP_OFFSET			(0x000000D0U)
                        /**< eFUSE cache PUF RO swap offset */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET			(0x00000094U)
                        /**< eFUSE cache boot environment control offset */
#define XNVM_EFUSE_CACHE_DME_FIPS_OFFSET			(0x00000234U)
                        /**< eFUSE cache DME FIPS offset */
#define XNVM_EFUSE_CACHE_BOOT_MODE_DIS_OFFSET			(0x00000238U)
                        /**< eFUSE cache boot mode disable offset */
#define XNVM_EFUSE_CACHE_USER_FUSE_START_OFFSET			(0x00000240U)
                        /**< eFUSE cache user fuse start offset */
#define XNVM_EFUSE_CACHE_ROM_RSVD_OFFSET			(0x00000090U)
                        /**< eFUSE cache ROM reserved offset */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_OFFSET			(0x00000018U)
                        /**< eFUSE cache IP disable 0 offset */
/** @} */

/**
 * @name Register: EFUSE_CACHE_DME_REVOKE_ID_REG
 * @{
 */
#define XNVM_EFUSE_CACHE_DME_REVOKE_ID_0_MASK			(0x00000030U)
                        /**< Bit mask for DME revoke ID 0 */
#define XNVM_EFUSE_CACHE_DME_REVOKE_ID_1_MASK			(0x000000C0U)
                        /**< Bit mask for DME revoke ID 1 */
#define XNVM_EFUSE_CACHE_DME_REVOKE_ID_2_MASK			(0x00000300U)
                        /**< Bit mask for DME revoke ID 2 */
#ifndef VERSAL_2VE_2VM
#define XNVM_EFUSE_CACHE_DME_REVOKE_ID_3_MASK			(0x00000C00U)
                        /**< Bit mask for DME revoke ID 3 */
#endif
/** @} */

/**
 * @name Register: EFUSE_CACHE_SECURITY_CONTROL_REG
 * @{
 */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK		(0xc0000000U)
                        /**< Bit mask for register init disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK		(0x10000000U)
                        /**< Bit mask for boot environment write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_MASK		(0x03800000U)
                        /**< Bit mask for PMC SC enable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_LOCK_DIS_MASK	(0x00600000U)
                        /**< Bit mask for authenticated JTAG lock disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_DIS_MASK		(0x00180000U)
                        /**< Bit mask for authenticated JTAG disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK			(0x00040000U)
                        /**< Bit mask for PUF disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK		(0x00020000U)
                        /**< Bit mask for PUF test2 disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK		(0x00010000U)
                        /**< Bit mask for PUF syndrome lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK		(0x00008000U)
                        /**< Bit mask for user key 1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK		(0x00004000U)
                        /**< Bit mask for user key 1 CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK		(0x00002000U)
                        /**< Bit mask for user key 0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK		(0x00001000U)
                        /**< Bit mask for user key 0 CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK		(0x00000800U)
                        /**< Bit mask for AES write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK		(0x00000600U)
                        /**< Bit mask for AES CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK		(0x00000100U)
                        /**< Bit mask for PPK2 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK		(0x00000080U)
                        /**< Bit mask for PPK1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK		(0x00000040U)
                        /**< Bit mask for PPK0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_MASK		(0x00000010U)
                        /**< Bit mask for UDS write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_MASK		(0x00000008U)
                        /**< Bit mask for HW test bits disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK			(0x00000004U)
                        /**< Bit mask for JTAG disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK		(0x00000002U)
                        /**< Bit mask for JTAG error out disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK			(0x00000001U)
                        /**< Bit mask for AES disable */

#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT	(30U)
                        /**< Bit shift for register init disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT		(28U)
                        /**< Bit shift for boot environment write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_SHIFT		(23U)
                        /**< Bit shift for PMC SC enable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_LOCK_DIS_1_0_SHIFT	(21U)
                        /**< Bit shift for authenticated JTAG lock disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_DIS_1_0_SHIFT	(19U)
                        /**< Bit shift for authenticated JTAG disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_SHIFT			(18U)
                        /**< Bit shift for PUF disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_SHIFT		(17U)
                        /**< Bit shift for PUF test2 disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_SHIFT		(16U)
                        /**< Bit shift for PUF syndrome lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_SHIFT		(15U)
                        /**< Bit shift for user key 1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_0_SHIFT	(14U)
                        /**< Bit shift for user key 1 CRC lock bit 0 */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_SHIFT		(13U)
                        /**< Bit shift for user key 0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_0_SHIFT	(12U)
                        /**< Bit shift for user key 0 CRC lock bit 0 */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_SHIFT		(11U)
                        /**< Bit shift for AES write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_SHIFT		(9U)
                        /**< Bit shift for AES CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_SHIFT		(8U)
                        /**< Bit shift for PPK2 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_SHIFT		(7U)
                        /**< Bit shift for PPK1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_SHIFT		(6U)
                        /**< Bit shift for PPK0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_SHIFT		(4U)
                        /**< Bit shift for UDS write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_SHIFT		(3U)
                        /**< Bit shift for HW test bits disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_SHIFT		(2U)
                        /**< Bit shift for JTAG disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROR_OUT_DIS_SHIFT	(1U)
                        /**< Bit shift for JTAG error out disable */
/** @} */

/**
 * @name Register: EFUSE_CACHE_PUF_ECC_CTRL
 * @{
 */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK	(0x80000000U)
                        /**< Bit mask for PUF regeneration disable */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK		(0x40000000U)
                        /**< Bit mask for PUF helper data invalid */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK		(0x00ffffffU)
                        /**< Bit mask for PUF ECC bits [23:0] */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGIS_DIS_MASK	(0x20000000U)
                        /**< Bit mask for PUF registration disable */

#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT		(31U)
                        /**< Bit shift for PUF regeneration disable */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT		(30U)
                        /**< Bit shift for PUF helper data invalid */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGIS_DIS_SHIFT		(29U)
                        /**< Bit shift for PUF registration disable */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_SECURITY_MISC_0
 * @{
 */
#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)
                        /**< Bit mask for EFUSE cache decryption */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_MISC_CTRL
 * @{
 */
#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_MASK	(0xc0000000U)
                        /**< Bit mask for glitch detector halt boot enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK	(0x20000000U)
                        /**< Bit mask for glitch detector ROM monitor enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK	(0x00600000U)
                        /**< Bit mask for halt boot error */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_MASK	(0x00180000U)
                        /**< Bit mask for halt boot environment */
#define XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK		(0x00008000U)
                        /**< Bit mask for crypto KAT enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK		(0x00004000U)
                        /**< Bit mask for LBIST enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_MASK	(0x00000100U)
                        /**< Bit mask for safety mission enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK		(0x000000c0U)
                        /**< Bit mask for PPK2 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK		(0x00000030U)
                        /**< Bit mask for PPK1 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK		(0x0000000cU)
                        /**< Bit mask for PPK0 invalid */

#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_SHIFT	(30U)
                        /**< Bit shift for glitch detector halt boot enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_SHIFT	(29U)
                        /**< Bit shift for glitch detector ROM monitor enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_SHIFT	(21U)
                        /**< Bit shift for halt boot error */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_SHIFT	(19U)
                        /**< Bit shift for halt boot environment */
#define XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_SHIFT		(15U)
                        /**< Bit shift for crypto KAT enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_SHIFT		(14U)
                        /**< Bit shift for LBIST enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_SHIFT	(8U)
                        /**< Bit shift for safety mission enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_SHIFT		(6U)
                        /**< Bit shift for PPK2 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_SHIFT		(4U)
                        /**< Bit shift for PPK1 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_SHIFT		(2U)
                        /**< Bit shift for PPK0 invalid */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_SECURITY_MISC_1
 * @{
 */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_MASK    	(0x00001c00U)
                        /**< Bit mask for LPD MBIST enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_MASK    	(0x00000380U)
                        /**< Bit mask for PMC MBIST enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_MASK   	(0x00000070U)
                        /**< Bit mask for LPD NOC SC enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_MASK	(0x0000000cU)
                        /**< Bit mask for sysmon voltage monitor enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_MASK	(0x00000003U)
                        /**< Bit mask for sysmon temperature monitor enable */

#define XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_SHIFT   		(10U)
                        /**< Bit shift for LPD MBIST enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_SHIFT   		(7U)
                        /**< Bit shift for PMC MBIST enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_SHIFT  		(4U)
                        /**< Bit shift for LPD NOC SC enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_SHIFT   	(2U)
                        /**< Bit shift for sysmon voltage monitor enable */
#define XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_SHIFT   	(0U)
                        /**< Bit shift for sysmon temperature monitor enable */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_BOOT_ENV_CTRL
 * @{
 */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK	(0x00200000U)
                        /**< Bit mask for sysmon temperature enable */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK	(0x00100000U)
                        /**< Bit mask for sysmon voltage enable */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK	(0x00060000U)
                        /**< Bit mask for sysmon temperature hot threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK	(0x00003000U)
                        /**< Bit mask for sysmon PMC voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK	(0x00000c00U)
                        /**< Bit mask for sysmon PS low-power voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK	(0x00000300U)
                        /**< Bit mask for sysmon SoC voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK	(0x00000003U)
                        /**< Bit mask for sysmon temperature cold threshold */

#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_SHIFT		(21U)
                        /**< Bit shift for sysmon temperature enable */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_SHIFT		(20U)
                        /**< Bit shift for sysmon voltage enable */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT		(17U)
                        /**< Bit shift for sysmon temperature hot threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT		(12U)
                        /**< Bit shift for sysmon PMC voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT		(10U)
                        /**< Bit shift for sysmon PS low-power voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_SHIFT		(9U)
                        /**< Bit shift for sysmon SoC voltage threshold */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT		(0U)
                        /**< Bit shift for sysmon temperature cold threshold */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_ROM_RSVD
 * @{
 */
#define XNVM_EFUSE_CACHE_ROM_RSVD_PLM_UPDATE_MASK		(0x00000400U)
                        /**< Bit mask for ROM reserved PLM update */
#define XNVM_EFUSE_CACHE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK	(0x00000200U)
                        /**< Bit mask for ROM reserved auth keys to hash */
#define XNVM_EFUSE_CACHE_ROM_RSVD_IRO_SWAP_MASK			(0x00000100U)
                        /**< Bit mask for ROM reserved IRO swap */
#define XNVM_EFUSE_CACHE_ROM_RSVD_ROM_SWDT_USAGE_MASK		(0x000000C0U)
                        /**< Bit mask for ROM reserved SWDT usage */

#define XNVM_EFUSE_CACHE_ROM_RSVD_PLM_UPDATE_SHIFT		(10U)
                        /**< Bit shift for ROM reserved PLM update */
#define XNVM_EFUSE_CACHE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT	(9U)
                        /**< Bit shift for ROM reserved auth keys to hash */
#define XNVM_EFUSE_CACHE_ROM_RSVD_IRO_SWAP_SHIFT		(8U)
                        /**< Bit shift for ROM reserved IRO swap */
#define XNVM_EFUSE_CACHE_ROM_RSVD_ROM_SWDT_USAGE_SHIFT		(6U)
                        /**< Bit shift for ROM reserved SWDT usage */
/** @} */

/**
 * @name  Register: EFUSE_CACHE_DME_FIPS
 * @{
 */
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_MASK		(0xFF000000U)
                        /**< Bit mask for DME FIPS mode */
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_SHIFT		(24U)
                        /**< Bit shift for DME FIPS mode */
/** @} */
/**
 * @name  Register: EFUSE_CACHE_IP_DISABLE_0
 * @{
 */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_MASK		(0xC0000000U)
                        /**< Bit mask for upper FIPS version bits in IP disable register */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_SHIFT	(30U)
                        /**< Bit shift for upper FIPS version bits in IP disable register */

#ifndef VERSAL_2VE_2VM
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_LSB_MASK		(0x00000004U)
                        /**< Bit mask for lower FIPS version bit in IP disable register */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_LSB_SHIFT	(2U)
                        /**< Bit shift for lower FIPS version bit in IP disable register */
#endif
/** @} */

/**
 * @name Register: EFUSE_CACHE_ANLG_TRIM_3
 * @{
 */
#define XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK			(0x7FFFFFFFU)
                        /**< Bit mask for glitch config data */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_3_GLITCH_DET_WR_LK_MASK	(0x80000000U)
                        /**< Bit mask for glitch detection write lock in analog trim 3 */
/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSECLIENT_HW_H */
