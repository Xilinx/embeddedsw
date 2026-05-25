/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file versal_net/common/xnvm_efuse_hw.h
*
* This file contains NVM library eFUSE controller register definitions
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 3.0   kal  07/12/2022 Initial release
* 3.2   har  02/22/2023 Added macros related to ROM Rsvd bits
*       yog  09/13/2023 Moved XNVM_EFUSE_CACHE_DME_FIPS_DME_MODE_MASK
*                       macro to xnvm_validate.c
* 3.7   nik  01/06/2026 Added support to allow use of PUF Helper Data eFUSEs for general purpose.
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/

#ifndef XNVM_EFUSE_HW_H
#define XNVM_EFUSE_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xnvm_efuse_common_hw.h"

/*************************** Constant Definitions *****************************/

/**
 *  @name eFUSE Cache Register Offsets
 * @{
 */
#define XNVM_EFUSE_CACHE_IP_DISABLE_OFFSET		(0x00000018U)
                        /**< eFUSE cache IP disable register offset */
#define XNVM_EFUSE_CACHE_ROM_RSVD_OFFSET		(0x00000090U)
                        /**< eFUSE cache ROM reserved register offset */
#define XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET		(0x000000FCU)
                        /**< eFUSE cache ME ID code register offset */
#define XNVM_EFUSE_CACHE_DME_FIPS_OFFSET		(0x00000234U)
                        /**< eFUSE cache DME FIPS register offset */
#define XNVM_EFUSE_CACHE_CRC_OFFSET			(0x0000023CU)
                        /**< eFUSE cache CRC register offset */
/** @} */

/**
 * @name Register: IP_DISABLE_0
 * @{
 */
#define XNVM_EFUSE_FIPS_VERSION_0_MASK                          (0x00000004U)
                        /**< Bit mask for FIPS version 0 */
#define XNVM_EFUSE_FIPS_VERSION_2_1_MASK                        (0xC0000000U)
                        /**< Bit mask for FIPS version bits */

#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_2_1_SHIFT    (30U)
                        /**< Bit shift for FIPS version bits */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_0_SHIFT      (2U)
                        /**< Bit shift for FIPS version 0 */
/** @} */

/**
 * @name Register: DME_FIPS
 * @{
 */
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_MASK		(0xFF000000U)
                        /**< Bit mask for DME FIPS mode */
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_SHIFT		(24U)
                        /**< Bit shift for DME FIPS mode */
/** @} */

/**
 * @name Register: ANLG_TRIM_3
 * @{
 */
#define XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK			(0x7FFFFFFFU)
                        /**< ANLG_TRIM_3 Masks and Shifts */
/** @} */

/**
 * @name Register: SECURITY_CONTROL
 * @{
 */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_MASK	(0x00000010U)
                        /**< SECURITY_CONTROL Masks and shifts */
/** @} */

/**
 * @name Register: ME_ID_CODE
 * @{
 */
#define XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK		(0xFF000000U)
                        /**< ME_ID_CODE Masks and shifts */
/** @} */

/**
 * @name TBITS masks
 * @{
 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK    		(0x08000000U)
                        /**< Bit mask for TBITS0_SVD anchor 3 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK    		(0x04000000U)
                        /**< Bit mask for TBITS0_SVD anchor 2 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK    		(0x00300000U)
                        /**< Bit mask for TBITS0_SVD CRC protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK    	(0x00080000U)
                        /**< Bit mask for TBITS0_SVD PUF CHASH protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK    	(0x00040000U)
                        /**< Bit mask for TBITS0_SVD security misc 1 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK		(0x00000002U)
                        /**< Bit mask for TBITS0_SVD anchor 1 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK		(0x00000001U)
                        /**< Bit mask for TBITS0_SVD anchor 0 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK 		(0x02000004U)
                        /**< Bit mask for TBITS0_SVD security control protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK 	(0x01000008U)
                        /**< Bit mask for TBITS0_SVD security misc 0 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_PPK_HASH_PROT_MASK		(0x00810000U)
                        /**< Bit mask for TBITS0_SVD PPK hash protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK	(0x00420000U)
                        /**< Bit mask for TBITS0_SVD meta header export protection */
/** @} */

/**
 * @name eFUSE Ctrl Register Offsets
 * @{
 */
#define XNVM_EFUSE_CTRL_UDS_DICE_CRC_OFFSET			(0x00000070U)
                        /**< eFUSE Ctrl Register Offsets */
/** @} */

/**
 * @name UDS in STATUS register in EFUSE_CTRL module
 * @{
 */
#define XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_PASS_MASK		(0x00002000U)
                        /**< Bit mask for UDS DICE CRC pass */
#define XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_DONE_MASK		(0x00001000U)
                        /**< Bit mask for UDS DICE CRC done */
/** @} */

/**
 * @name eFuse row count numbers
 * @{
 */
#define XNVM_EFUSE_PPK_HASH_NUM_OF_CACHE_ROWS		(8U)
                        /**< Number of eFUSE cache rows per PPK hash */
#define XNVM_EFUSE_TOTAL_PPK_HASH_ROWS			(XNVM_EFUSE_PPK_HASH_NUM_OF_CACHE_ROWS * 3U)
                        /**< Total number of eFUSE rows for all PPK hashes */
#define XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS			(3U)
                        /**< Number of eFUSE cache rows for IV */
/** @} */

/**
 * @name eFuse Row numbers
 * @{
 */
#define XNVM_EFUSE_TBITS_XILINX_CTRL_ROW		(0U)
                        /**< eFUSE Xilinx control TBITS row */
#define XNVM_EFUSE_PUF_RO_SWAP_EN_ROW                   (1U)
                        /**< eFUSE PUF RO swap enable row */
#define XNVM_EFUSE_BLACK_IV_START_ROW			(4U)
                        /**< eFUSE black IV start row */
#define XNVM_EFUSE_PLM_IV_START_ROW			(4U)
                        /**< eFUSE PLM IV start row */
#define XNVM_EFUSE_DATA_PARTITION_IV_START_ROW		(4U)
                        /**< eFUSE data partition IV start row */
#define XNVM_EFUSE_AES_KEY_0_TO_127_START_ROW		(16U)
                        /**< eFUSE AES key bits 0-127 start row */
#define XNVM_EFUSE_AES_KEY_128_TO_255_START_ROW		(16U)
                        /**< eFUSE AES key bits 128-255 start row */
#define XNVM_EFUSE_BOOT_MODE_START_ROW			(16U)
                        /**< eFUSE boot mode start row */
#define XNVM_EFUSE_MISC_CTRL_START_ROW			(20U)
                        /**< eFUSE miscellaneous control start row */
#define XNVM_EFUSE_ANLG_TRIM_3_START_ROW		(24U)
                        /**< eFUSE analog trim 3 start row */
#define XNVM_EFUSE_GLITCH_DET_WR_LK_ROW			(27U)
                        /**< eFUSE glitch detection write lock row */
#define XNVM_EFUSE_BOOT_ENV_CTRL_START_ROW		(28U)
                        /**< eFUSE boot environment control start row */
#define XNVM_EFUSE_AES_KEY_0_TO_255_END_ROW		(31U)
                        /**< eFUSE AES key bits 0-255 end row */
#define XNVM_EFUSE_REVOKE_ID_START_ROW			(32U)
                        /**< eFUSE revocation ID start row */
#define XNVM_EFUSE_SECURITY_MISC_0_START_ROW		(40U)
                        /**< eFUSE security misc 0 start row */
#define XNVM_EFUSE_SEC_CTRL_START_ROW			(44U)
                        /**< eFUSE security control start row */
#define XNVM_EFUSE_SECURITY_MISC1_START_ROW		(48U)
                        /**< eFUSE security misc 1 start row */
#define XNVM_EFUSE_DICE_UDS_0_TO_63_START_ROW		(48U)
                        /**< eFUSE DICE UDS bits 0-63 start row */
#define XNVM_EFUSE_USER_KEY1_0_TO_63_START_ROW		(48U)
                        /**< eFUSE user key 1 bits 0-63 start row */
#define XNVM_EFUSE_DME_REVOKE_0_AND_1_ROW		(52U)
                        /**< eFUSE DME revoke 0 and 1 row */
#define XNVM_EFUSE_DME_MODE_START_ROW			(52U)
                        /**< eFUSE DME mode start row */
#define XNVM_EFUSE_DME_REVOKE_2_AND_3_ROW		(53U)
                        /**< eFUSE DME revoke 2 and 3 row */
#define XNVM_EFUSE_USER_KEY1_0_TO_63_END_ROW		(55U)
                        /**< eFUSE user key 1 bits 0-63 end row */
#define XNVM_EFUSE_DME_FIPS_ROW				(55U)
                        /**< eFUSE DME FIPS row */
#define XNVM_EFUSE_DICE_UDS_64_TO_191_START_ROW		(56U)
                        /**< eFUSE DICE UDS bits 64-191 start row */
#define XNVM_EFUSE_USER_KEY0_0_TO_63_START_ROW		(56U)
                        /**< eFUSE user key 0 bits 0-63 start row */
#define XNVM_EFUSE_USER_KEY0_0_TO_63_END_ROW		(63U)
                        /**< eFUSE user key 0 bits 0-63 end row */
#define XNVM_EFUSE_DICE_UDS_192_TO_255_START_ROW	(64U)
                        /**< eFUSE DICE UDS bits 192-255 start row */
#define	XNVM_EFUSE_DICE_UDS_256_TO_383_START_ROW	(66U)
                        /**< eFUSE DICE UDS bits 256-383 start row */
#define XNVM_EFUSE_USER_KEY0_64_TO_191_START_ROW	(66U)
                        /**< eFUSE user key 0 bits 64-191 start row */
#define XNVM_EFUSE_USER_KEY1_64_TO_127_START_ROW	(66U)
                        /**< eFUSE user key 1 bits 64-127 start row */
#define XNVM_EFUSE_USER_KEY0_64_TO_191_END_ROW		(73U)
                        /**< eFUSE user key 0 bits 64-191 end row */
#define XNVM_EFUSE_USER_KEY1_64_TO_127_END_ROW		(73U)
                        /**< eFUSE user key 1 bits 64-127 end row */
#define XNVM_EFUSE_USER_KEY0_192_TO_255_START_ROW	(74U)
                        /**< eFUSE user key 0 bits 192-255 start row */
#define XNVM_EFUSE_USER_KEY1_128_TO_255_START_ROW	(74U)
                        /**< eFUSE user key 1 bits 128-255 start row */
#define XNVM_EFUSE_USER_KEY0_192_TO_255_END_ROW		(81U)
                        /**< eFUSE user key 0 bits 192-255 end row */
#define XNVM_EFUSE_USER_KEY1_128_TO_255_END_ROW		(81U)
                        /**< eFUSE user key 1 bits 128-255 end row */
#define XNVM_EFUSE_OFFCHIP_REVOKE_ID_START_ROW		(82U)
                        /**< eFUSE off-chip revocation ID start row */
#define XNVM_EFUSE_META_HEADER_IV_START_ROW		(90U)
                        /**< eFUSE meta header IV start row */
#define XNVM_EFUSE_PUF_CHASH_ROW			(93U)
                        /**< eFUSE PUF CHASH row */
#define XNVM_EFUSE_IP_DISABLE_ROW			(94U)
                        /**< eFUSE IP disable row */
#define XNVM_EFUSE_PUF_AUX_ROW				(95U)
                        /**< eFUSE PUF AUX row */
#define XNVM_EFUSE_PPK1_HASH_START_ROW			(96U)
                        /**< eFUSE PPK1 hash start row */
#define XNVM_EFUSE_DME_USER_KEY_0_START_ROW		(96U)
                        /**< eFUSE DME user key 0 start row */
#define XNVM_EFUSE_DME_USER_KEY_2_START_ROW		(96U)
                        /**< eFUSE DME user key 2 start row */
#define XNVM_EFUSE_PPK2_HASH_START_ROW			(128U)
                        /**< eFUSE PPK2 hash start row */
#define XNVM_EFUSE_DME_USER_KEY_1_START_ROW		(144U)
                        /**< eFUSE DME user key 1 start row */
#define XNVM_EFUSE_DME_USER_KEY_3_START_ROW		(144U)
                        /**< eFUSE DME user key 3 start row */
#define XNVM_EFUSE_PPK0_HASH_START_ROW			(160U)
                        /**< eFUSE PPK0 hash start row */
#define XNVM_EFUSE_CRC_SALT_ROW				(179U)
                        /**< eFUSE CRC salt row */
#define XNVM_EFUSE_DISABLE_PLM_UPDATE_ROW		(185U)
                        /**< eFUSE disable PLM update row */
#define XNVM_EFUSE_ROM_RSVD_START_ROW			(184U)
                        /**< eFUSE ROM reserved start row */
#define XNVM_EFUSE_CRC_ROW				(188U)
                        /**< eFUSE CRC row */
#define XNVM_EFUSE_PAGE_0_PUF_SYN_DATA_ROW		(192U)
                        /**< eFUSE page 0 PUF syndrome data row */
#define XNVM_EFUSE_PAGE_1_PUF_SYN_DATA_ROW		(192U)
                        /**< eFUSE page 1 PUF syndrome data row */

#ifdef VERSAL_2VE_2VM
#define XNVM_EFUSE_ADD_PPK0_HASH_START_ROW		(144U)
                        /**< eFUSE additional PPK0 hash start row */
#define XNVM_EFUSE_ADD_PPK0_HASH_START_COL_NUM		(8U)
                        /**< eFUSE additional PPK0 hash start column number */
#define XNVM_EFUSE_ADD_PPK0_HASH_END_COL_NUM            (15U)
                        /**< eFUSE additional PPK0 hash end column number */
#define XNVM_EFUSE_ADD_PPK0_HASH_NUM_OF_ROWS		(16U)
                        /**< eFUSE number of rows for additional PPK0 hash */

#define XNVM_EFUSE_ADD_PPK1_HASH_START_ROW		(160U)
                        /**< eFUSE additional PPK1 hash start row */
#define XNVM_EFUSE_ADD_PPK1_HASH_START_COL_NUM		(8U)
                        /**< eFUSE additional PPK1 hash start column number */

#define XNVM_EFUSE_ADD_PPK1_HASH_END_COL_NUM            (15U)
                        /**< eFUSE additional PPK1 hash end column number */
#define XNVM_EFUSE_ADD_PPK1_HASH_NUM_OF_ROWS		(16U)
                        /**< eFUSE number of rows for additional PPK1 hash */
#define XNVM_EFUSE_ADD_PPK2_HASH_START_ROW		(176U)
                        /**< eFUSE additional PPK2 hash start row */
#define XNVM_EFUSE_ADD_PPK2_HASH_START_COL_NUM		(8U)
                        /**< eFUSE additional PPK2 hash start column number */
#define XNVM_EFUSE_ADD_PPK2_HASH_END_COL_NUM          	(15U)
                        /**< eFUSE additional PPK2 hash end column number */
#define XNVM_EFUSE_ADD_PPK2_HASH_NUM_OF_ROWS		(16U)
                        /**< eFUSE number of rows for additional PPK2 hash */

#endif

/** @} */

/**
 * @name eFuse column numbers
 * @{
 */
#define XNVM_EFUSE_ROW_0_SEC_CTRL_PROT_0_COL_NUM	(2U)
                        /**< eFUSE row 0 security control protection 0 column */
#define XNVM_EFUSE_ROW_0_SEC_CTRL_PROT_1_COL_NUM	(25U)
                        /**< eFUSE row 0 security control protection 1 column */
#define XNVM_EFUSE_ROW_0_SEC_MISC0_PROT_1_COL_NUM	(24U)
                        /**< eFUSE row 0 security misc 0 protection 1 column */
#define XNVM_EFUSE_ROW_0_SEC_MISC0_PROT_0_COL_NUM	(3U)
                        /**< eFUSE row 0 security misc 0 protection 0 column */
#define XNVM_EFUSE_ROW_0_PPK_HASH_PROT_0_COL_NUM	(16U)
                        /**< eFUSE row 0 PPK hash protection 0 column */
#define XNVM_EFUSE_ROW_0_PPK_HASH_PROT_1_COL_NUM	(23U)
                        /**< eFUSE row 0 PPK hash protection 1 column */
#define XNVM_EFUSE_ROW_0_META_HEADER_EXPORT_DFT_PROT_0_COL_NUM	(17U) /**< eFUSE row 0 meta header
                                        * export DFT protection 0 column */
#define XNVM_EFUSE_ROW_0_META_HEADER_EXPORT_DFT_PROT_1_COL_NUM	(22U) /**< eFUSE row 0 meta header
                                        * export DFT protection 1 column */
#define XNVM_EFUSE_ROW_0_CRC_PROT_0_COL_NUM		(21U)
                        /**< eFUSE row 0 CRC protection 0 column */
#define XNVM_EFUSE_ROW_0_CRC_PROT_1_COL_NUM		(20U)
                        /**< eFUSE row 0 CRC protection 1 column */
#define XNVM_EFUSE_ROW_0_PUF_CHASH_PROT_COL_NUM		(19U)
                        /**< eFUSE row 0 PUF CHASH protection column */
#define XNVM_EFUSE_ROW_0_SEC_MISC1_PROT_COL_NUM		(18U)
                        /**< eFUSE row 0 security misc 1 protection column */
#define XNVM_EFUSE_AES_KEY_0_TO_127_COL_START_NUM	(8U)
                        /**< eFUSE AES key bits 0-127 start column */
#define XNVM_EFUSE_AES_KEY_0_TO_127_COL_END_NUM		(15U)
                        /**< eFUSE AES key bits 0-127 end column */
#define XNVM_EFUSE_AES_KEY_128_TO_255_COL_START_NUM	(16U)
                        /**< eFUSE AES key bits 128-255 start column */
#define XNVM_EFUSE_AES_KEY_128_TO_255_COL_END_NUM	(23U)
                        /**< eFUSE AES key bits 128-255 end column */
#define XNVM_EFUSE_USER_KEY0_0_TO_63_COL_START_NUM	(8U)
                        /**< eFUSE user key 0 bits 0-63 start column */
#define XNVM_EFUSE_USER_KEY0_0_TO_63_COL_END_NUM	(15U)
                        /**< eFUSE user key 0 bits 0-63 end column */
#define XNVM_EFUSE_USER_KEY0_64_TO_191_COL_START_NUM	(8U)
                        /**< eFUSE user key 0 bits 64-191 start column */
#define XNVM_EFUSE_USER_KEY0_64_TO_191_COL_END_NUM	(23U)
                        /**< eFUSE user key 0 bits 64-191 end column */
#define XNVM_EFUSE_USER_KEY0_192_TO_255_COL_START_NUM	(8U)
                        /**< eFUSE user key 0 bits 192-255 start column */
#define XNVM_EFUSE_USER_KEY0_192_TO_255_COL_END_NUM	(15U)
                        /**< eFUSE user key 0 bits 192-255 end column */
#define XNVM_EFUSE_USER_KEY1_0_TO_63_START_COL_NUM	(16U)
                        /**< eFUSE user key 1 bits 0-63 start column */
#define XNVM_EFUSE_USER_KEY1_0_TO_63_END_COL_NUM	(23U)
                        /**< eFUSE user key 1 bits 0-63 end column */
#define XNVM_EFUSE_USER_KEY1_64_TO_127_START_COL_NUM	(24U)
                        /**< eFUSE user key 1 bits 64-127 start column */
#define XNVM_EFUSE_USER_KEY1_64_TO_127_END_COL_NUM	(31U)
                        /**< eFUSE user key 1 bits 64-127 end column */
#define XNVM_EFUSE_USER_KEY1_128_TO_255_START_COL_NUM	(16U)
                        /**< eFUSE user key 1 bits 128-255 start column */
#define XNVM_EFUSE_USER_KEY1_128_TO_255_END_COL_NUM	(31U)
                        /**< eFUSE user key 1 bits 128-255 end column */
#define XNVM_EFUSE_PPK0_HASH_START_COL_NUM		(16U)
                        /**< eFUSE PPK0 hash start column */
#define XNVM_EFUSE_PPK0_HASH_END_COL_NUM		(23U)
                        /**< eFUSE PPK0 hash end column */
#define XNVM_EFUSE_PPK1_HASH_START_COL_NUM		(24U)
                        /**< eFUSE PPK1 hash start column */
#define XNVM_EFUSE_PPK1_HASH_END_COL_NUM		(31U)
                        /**< eFUSE PPK1 hash end column */
#define XNVM_EFUSE_PPK2_HASH_START_COL_NUM              (24U)
                        /**< eFUSE PPK2 hash start column */
#define XNVM_EFUSE_PPK2_HASH_END_COL_NUM                (31U)
                        /**< eFUSE PPK2 hash end column */
#define XNVM_EFUSE_METAHEADER_IV_RANGE_START_COL_NUM	(0U)
                        /**< eFUSE meta header IV range start column */
#define XNVM_EFUSE_METAHEADER_IV_RANGE_END_COL_NUM	(31U)
                        /**< eFUSE meta header IV range end column */
#define XNVM_EFUSE_BLACK_IV_START_COL_NUM		(8U)
                        /**< eFUSE black IV start column */
#define XNVM_EFUSE_BLACK_IV_END_COL_NUM			(15U)
                        /**< eFUSE black IV end column */
#define XNVM_EFUSE_PLM_IV_RANGE_START_COL_NUM		(16U)
                        /**< eFUSE PLM IV range start column */
#define XNVM_EFUSE_PLM_IV_RANGE_END_COL_NUM		(23U)
                        /**< eFUSE PLM IV range end column */
#define XNVM_EFUSE_DATA_PARTITION_IV_START_COL_NUM	(24U)
                        /**< eFUSE data partition IV start column */
#define XNVM_EFUSE_DATA_PARTITION_IV_END_COL_NUM	(31U)
                        /**< eFUSE data partition IV end column */
#define XNVM_EFUSE_REVOKE_ID_0_TO_127_START_COL_NUM	(16U)
                        /**< eFUSE revocation ID 0-127 start column */
#define XNVM_EFUSE_REVOKE_ID_128_TO_255_START_COL_NUM	(24U)
                        /**< eFUSE revocation ID 128-255 start column */
#define XNVM_EFUSE_MISC_CTRL_START_COL_NUM		(24U)
                        /**< eFUSE miscellaneous control start column */
#define XNVM_EFUSE_MISC_CTRL_END_COL_NUM		(31U)
                        /**< eFUSE miscellaneous control end column */
#define XNVM_EFUSE_SEC_CTRL_START_COL_NUM		(8U)
                        /**< eFUSE security control start column */
#define XNVM_EFUSE_SEC_CTRL_END_COL_NUM			(15U)
                        /**< eFUSE security control end column */
#define XNVM_EFUSE_GLITCH_DET_CONFIG_START_COL_NUM	(24U)
                        /**< eFUSE glitch detection config start column */
#define XNVM_EFUSE_GLITCH_DET_CONFIG_END_COL_NUM	(31U)
                        /**< eFUSE glitch detection config end column */
#define XNVM_EFUSE_GLITCH_DET_WR_LK_COL_NUM             (31U)
                        /**< eFUSE glitch detection write lock column */
#define XNVM_EFUSE_DISABLE_PLM_UPDATE_COL_NUM		(26U)
                        /**< eFUSE disable PLM update column */
#define XNVM_EFUSE_BOOT_MODE_START_COL_NUM		(24U)
                        /**< eFUSE boot mode start column */
#define XNVM_EFUSE_BOOT_MODE_END_COL_NUM		(31U)
                        /**< eFUSE boot mode end column */
#define XNVM_EFUSE_DME_MODE_START_COL_NUM		(8U)
                        /**< eFUSE DME mode start column */
#define XNVM_EFUSE_DME_MODE_END_COL_NUM			(11U)
                        /**< eFUSE DME mode end column */
#define XNVM_EFUSE_FIPS_MODE_START_COL_NUM		(8U)
                        /**< eFUSE FIPS mode start column */
#define XNVM_EFUSE_FIPS_MODE_END_COL_NUM		(15U)
                        /**< eFUSE FIPS mode end column */
#ifdef VERSAL_2VE_2VM
#define XNVM_EFUSE_FIPS_VERSION_COL_0_NUM		(30U)
                        /**< eFUSE FIPS version column 0 */
#define XNVM_EFUSE_FIPS_VERSION_COL_1_NUM		(31U)
                        /**< eFUSE FIPS version column 1 */
#define XNVM_EFUSE_MAX_FIPS_VERSION	(3U) /**< Max Value of FIPS version */

#else
#define XNVM_EFUSE_FIPS_VERSION_COL_0_NUM		(2U)
                        /**< eFUSE FIPS version column 0 */
#define XNVM_EFUSE_FIPS_VERSION_COL_1_NUM		(30U)
                        /**< eFUSE FIPS version column 1 */
#define XNVM_EFUSE_FIPS_VERSION_COL_2_NUM		(31U)
                        /**< eFUSE FIPS version column 2 */
#define XNVM_EFUSE_MAX_FIPS_VERSION	(7U) /**< Max Value of FIPS version */
#endif

#define XNVM_EFUSE_PUF_SYN_DATA_START_COL_NUM		(0U)
                        /**< eFUSE PUF syndrome data start column */
#define XNVM_EFUSE_PUF_SYN_DATA_END_COL_NUM		(31U)
                        /**< eFUSE PUF syndrome data end column */
#define XNVM_EFUSE_PUF_CHASH_START_COL_NUM		(0U)
                        /**< eFUSE PUF CHASH start column */
#define XNVM_EFUSE_PUF_CHASH_END_COL_NUM		(31U)
                        /**< eFUSE PUF CHASH end column */
#define XNVM_EFUSE_PUF_AUX_START_COL_NUM		(0U)
                        /**< eFUSE PUF AUX start column */
#define XNVM_EFUSE_PUF_AUX_END_COL_NUM			(23U)
                        /**< eFUSE PUF AUX end column */
#define XNVM_EFUSE_PUF_REGIS_DIS_COL_NUM		(29U)
                        /**< eFUSE PUF registration disable column */
#define XNVM_EFUSE_PUF_HD_INVLD_COL_NUM			(30U)
                        /**< eFUSE PUF helper data invalid column */
#define XNVM_EFUSE_PUF_REGEN_DIS_COL_NUM		(31U)
                        /**< eFUSE PUF regeneration disable column */
#define XNVM_EFUSE_PUF_RO_SWAP_EN_START_COL_NUM		(0U)
                        /**< eFUSE PUF RO swap enable start column */
#define XNVM_EFUSE_PUF_RO_SWAP_EN_END_COL_NUM		(31U)
                        /**< eFUSE PUF RO swap enable end column */
#define XNVM_EFUSE_DEC_ONLY_START_COL_NUM		(8U)
                        /**< eFUSE decrypt only start column */
#define XNVM_EFUSE_DEC_ONLY_END_COL_NUM			(15U)
                        /**< eFUSE decrypt only end column */
#define	XNVM_EFUSE_SECURITY_MISC1_START_COL_NUM		(8U)
                        /**< eFUSE security misc 1 start column */
#define XNVM_EFUSE_SECURITY_MISC1_END_COL_NUM		(15U)
                        /**< eFUSE security misc 1 end column */
#define XNVM_EFUSE_BOOT_ENV_CTRL_START_COL_NUM		(24U)
                        /**< eFUSE boot environment control start column */
#define XNVM_EFUSE_BOOT_ENV_CTRL_END_COL_NUM		(31U)
                        /**< eFUSE boot environment control end column */
#define XNVM_EFUSE_DICE_UDS_0_TO_63_START_COL_NUM	(24U)
                        /**< eFUSE DICE UDS bits 0-63 start column */
#define XNVM_EFUSE_DICE_UDS_0_TO_63_END_COL_NUM		(31U)
                        /**< eFUSE DICE UDS bits 0-63 end column */
#define XNVM_EFUSE_DICE_UDS_64_TO_191_START_COL_NUM	(16U)
                        /**< eFUSE DICE UDS bits 64-191 start column */
#define XNVM_EFUSE_DICE_UDS_64_TO_191_END_COL_NUM	(31U)
                        /**< eFUSE DICE UDS bits 64-191 end column */
#define XNVM_EFUSE_DICE_UDS_192_TO_255_START_COL_NUM	(0U)
                        /**< eFUSE DICE UDS bits 192-255 start column */
#define XNVM_EFUSE_DICE_UDS_192_TO_255_END_COL_NUM	(31U)
                        /**< eFUSE DICE UDS bits 192-255 end column */
#define XNVM_EFUSE_DICE_UDS_256_TO_383_START_COL_NUM	(0U)
                        /**< eFUSE DICE UDS bits 256-383 start column */
#define XNVM_EFUSE_DICE_UDS_256_TO_383_END_COL_NUM	(7U)
                        /**< eFUSE DICE UDS bits 256-383 end column */
#define XNVM_EFUSE_DME_REVOKE_0_0_COL_NUM		(12U)
                        /**< eFUSE DME revoke 0 bit 0 column */
#define XNVM_EFUSE_DME_REVOKE_0_1_COL_NUM		(13U)
                        /**< eFUSE DME revoke 0 bit 1 column */
#define XNVM_EFUSE_DME_REVOKE_1_0_COL_NUM		(14U)
                        /**< eFUSE DME revoke 1 bit 0 column */
#define XNVM_EFUSE_DME_REVOKE_1_1_COL_NUM		(15U)
                        /**< eFUSE DME revoke 1 bit 1 column */
#define XNVM_EFUSE_DME_REVOKE_2_0_COL_NUM		(8U)
                        /**< eFUSE DME revoke 2 bit 0 column */
#define XNVM_EFUSE_DME_REVOKE_2_1_COL_NUM		(9U)
                        /**< eFUSE DME revoke 2 bit 1 column */
#define XNVM_EFUSE_DME_REVOKE_3_0_COL_NUM		(10U)
                        /**< eFUSE DME revoke 3 bit 0 column */
#define XNVM_EFUSE_DME_REVOKE_3_1_COL_NUM		(11U)
                        /**< eFUSE DME revoke 3 bit 1 column */
#define XNVM_EFUSE_DME_USER_KEY_0_START_COL_NUM		(0U)
                        /**< eFUSE DME user key 0 start column */
#define XNVM_EFUSE_DME_USER_KEY_0_END_COL_NUM		(7U)
                        /**< eFUSE DME user key 0 end column */
#define XNVM_EFUSE_DME_USER_KEY_1_START_COL_NUM		(0U)
                        /**< eFUSE DME user key 1 start column */
#define XNVM_EFUSE_DME_USER_KEY_1_END_COL_NUM		(7U)
                        /**< eFUSE DME user key 1 end column */
#define XNVM_EFUSE_DME_USER_KEY_2_START_COL_NUM		(8U)
                        /**< eFUSE DME user key 2 start column */
#define XNVM_EFUSE_DME_USER_KEY_2_END_COL_NUM		(15U)
                        /**< eFUSE DME user key 2 end column */
#define XNVM_EFUSE_DME_USER_KEY_3_START_COL_NUM		(8U)
                        /**< eFUSE DME user key 3 start column */
#define XNVM_EFUSE_DME_USER_KEY_3_END_COL_NUM		(15U)
                        /**< eFUSE DME user key 3 end column */
#define XNVM_EFUSE_CRC_START_COL_NUM			(24U)
                        /**< eFUSE CRC start column */
#define XNVM_EFUSE_CRC_END_COL_NUM			(31U)
                        /**< eFUSE CRC end column */
#define XNVM_EFUSE_CRC_SALT_START_COL_NUM		(24U)
                        /**< eFUSE CRC salt start column */
#define XNVM_EFUSE_CRC_SALT_END_COL_NUM			(31U)
                        /**< eFUSE CRC salt end column */
#define XNVM_EFUSE_ROM_RSVD_START_COL			(24U)
                        /**< eFUSE ROM reserved start column */
#define XNVM_EFUSE_ROM_RSVD_END_COL			(31U)
                        /**< eFUSE ROM reserved end column */
/** @} */

/**
 * @name eFuse number of rows
 * @{
 */
#define XNVM_EFUSE_AES_KEY_0_TO_127_NUM_OF_ROWS		(16U)
                        /**< eFUSE AES key 0 to 127 number of rows */
#define XNVM_EFUSE_AES_KEY_128_TO_255_NUM_OF_ROWS	(16U)
                        /**< eFUSE AES key 128 to 255 number of rows */
#define XNVM_EFUSE_USER_KEY0_0_TO_63_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 0 bits 0-63 number of rows */
#define XNVM_EFUSE_USER_KEY0_64_TO_191_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 0 bits 64-191 number of rows */
#define XNVM_EFUSE_USER_KEY0_192_TO_255_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 0 bits 192-255 number of rows */
#define XNVM_EFUSE_USER_KEY1_0_TO_63_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 1 bits 0-63 number of rows */
#define XNVM_EFUSE_USER_KEY1_64_TO_127_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 1 bits 64-127 number of rows */
#define XNVM_EFUSE_USER_KEY1_128_TO_255_NUM_OF_ROWS	(8U)
                        /**< eFUSE user key 1 bits 128-255 number of rows */
#define XNVM_EFUSE_METAHEADER_IV_NUM_OF_ROWS		(3U)
                        /**< eFUSE meta header IV number of rows */
#define XNVM_EFUSE_BLACK_IV_NUM_OF_ROWS			(12U)
                        /**< eFUSE black IV number of rows */
#define XNVM_EFUSE_PLM_IV_NUM_OF_ROWS			(12U)
                        /**< eFUSE PLM IV number of rows */
#define XNVM_EFUSE_DATA_PARTITION_IV_NUM_OF_ROWS	(12U)
                        /**< eFUSE data partition IV number of rows */
#define XNVM_EFUSE_GLITCH_DET_CONFIG_NUM_OF_ROWS	(4U)
                        /**< eFUSE glitch detection config number of rows */
#define XNVM_EFUSE_BOOT_MODE_NUM_OF_ROWS		(2U)
                        /**< eFUSE boot mode number of rows */
#define XNVM_EFUSE_DME_MODE_NUM_OF_ROWS			(1U)
                        /**< eFUSE DME mode number of rows */
#define XNVM_EFUSE_DME_FIPS_NUM_OF_ROWS			(1U)
                        /**< eFUSE DME FIPS number of rows */
#define XNVM_EFUSE_PAGE_0_PUF_SYN_DATA_NUM_OF_ROWS	(64U)
                        /**< eFUSE page 0 PUF synthesis data number of rows */
#define XNVM_EFUSE_PAGE_1_PUF_SYN_DATA_NUM_OF_ROWS      (63U)
                        /**< eFUSE page 1 PUF synthesis data number of rows */
#define XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS		(1U)
                        /**< eFUSE PUF CHASH number of rows */
#define XNVM_EFUSE_PUF_AUX_NUM_OF_ROWS			(1U)
                        /**< eFUSE PUF AUX number of rows */
#define XNVM_EFUSE_PUF_RO_SWAP_NUM_OF_ROWS		(1U)
                        /**< eFUSE PUF RO swap number of rows */
#define XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS			(2U)
                        /**< eFUSE DEC only number of rows */
#define XNVM_EFUSE_MISC_CTRL_NUM_OF_ROWS		(4U)
                        /**< eFUSE miscellaneous control number of rows */
#define XNVM_EFUSE_SEC_CTRL_NUM_OF_ROWS			(4U)
                        /**< eFUSE security control number of rows */
#define XNVM_EFUSE_SECURITY_MISC1_NUM_OF_ROWS		(4U)
                        /**< eFUSE security miscellaneous 1 number of rows */
#define XNVM_EFUSE_DICE_UDS_0_TO_63_NUM_OF_ROWS		(8U)
                        /**< eFUSE DICE UDS 0 to 63 number of rows */
#define XNVM_EFUSE_DICE_UDS_64_TO_191_NUM_OF_ROWS	(8U)
                        /**< eFUSE DICE UDS 64 to 191 number of rows */
#define XNVM_EFUSE_DICE_UDS_192_TO_255_NUM_OF_ROWS	(2U)
                        /**< eFUSE DICE UDS 192 to 255 number of rows */
#define XNVM_EFUSE_DICE_UDS_256_TO_383_NUM_OF_ROWS	(16U)
                        /**< eFUSE DICE UDS 256 to 383 number of rows */
#define XNVM_EFUSE_DME_USER_KEY_NUM_OF_ROWS		(48U)
                        /**< eFUSE DME user key number of rows */
#define XNVM_EFUSE_CRC_NUM_OF_ROWS			(4U)
                        /**< eFUSE CRC number of rows */
#define XNVM_EFUSE_CRC_SALT_NUM_OF_ROWS			(1U)
                        /**< eFUSE CRC salt number of rows */
#define XNVM_EFUSE_ROM_RSVD_NUM_OF_ROWS			(4U)
                        /**< eFUSE ROM reserved number of rows */
/** @} */

#ifdef XNVM_ACCESS_PUF_USER_DATA
#define XNVM_EFUSE_PUF_SYN_USERDATA_NUM_OF_ROWS		(128U)  /**< PUF SYN + RSVD rows (127+1) */
#endif

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_HW_H */
