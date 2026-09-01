/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file common/xnvm_efuse_common_hw.h
*
* This file contains NVM library eFUSE controller register definitions
*
* @cond xnvm_internal
* This file contains eFUSE controller register definitions which are common
* for Versal and Versal_Net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 3.0   kal  07/16/2022 Initial release
* 3.2   kum  04/11/2023 moved BOOTENV, SYSMON related macros to common to make use for both veral and versalnet
* 3.7   mb   03/26/2026 Add support to read pss ref clock freq from RTCA for PL-Microblaze
*
* </pre>
* @endcond
*
*******************************************************************************/
#ifndef XNVM_EFUSE_COMMON_HW_H
#define XNVM_EFUSE_COMMON_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xparameters.h"

/*************************** Constant Definitions *****************************/


/**
 * @name PMC sysmon sat0 base address
 * @{
 */
#define XNVM_EFUSE_SYSMONPSV_SAT0_BASEADDR		(0xF1280000U)
			/**< SYSMONPSV_SAT0 Base Address */
/** @} */

/**
 * @name PMC sysmon sat1 base address
 * @{
 */
#define XNVM_EFUSE_SYSMONPSV_SAT1_BASEADDR		(0xF1290000U)
			/**< SYSMONPSV_SAT1 Base Address */
/** @} */

/**
 * @name PMC sysmon measure0 offset
 * @{
 */
#define XNVM_EFUSE_SYSMONPSV_SAT_MEASURE0_OFFSET	(0x00000524U)
			/**< SYSMONPSV_SAT measure 0 offset */
/** @} */

/**
 * @name CRP base address definition
 * @{
 */
#define XNVM_CRP_BASE_ADDR				(0xF1260000U)
			/**< CRP Base Address */
/** @} */

/**
 * @name CRP eFuse Clock Control Register
 * @{
 */
#define XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET		(0x00000134U)
			/**< CRP eFUSE reference clock register offset */
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT		(2U)
			/**< CRP eFUSE reference clock select source bit shift */
#define XNVM_CRP_EFUSE_REF_CLK_IN			((u32)0x01U << \
					XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT)
			/**< CRP eFUSE reference clock input bit */
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC		(XNVM_CRP_EFUSE_REF_CLK_IN)
			/**< CRP eFUSE reference clock select source value */
/** @} */



/**
 * @name eFuse Controller base address
 * @{
 */
#define XNVM_EFUSE_CTRL_BASEADDR			(0xF1240000U)
			/**< eFuse Control Base Address */
/** @} */

/**
 * @name eFuse Controller Register Offsets
 * @{
 */
#define XNVM_EFUSE_WR_LOCK_REG_OFFSET			(0x00000000U)
			/**< eFUSE write lock register offset */
#define XNVM_EFUSE_CFG_REG_OFFSET			(0x00000004U)
			/**< eFUSE configuration register offset */
#define XNVM_EFUSE_STATUS_REG_OFFSET			(0x00000008U)
			/**< eFUSE status register offset */
#define XNVM_EFUSE_PGM_ADDR_REG_OFFSET			(0x0000000CU)
			/**< eFUSE program address register offset */
#define XNVM_EFUSE_RD_ADDR_REG_OFFSET			(0x00000010U)
			/**< eFUSE read address register offset */
#define XNVM_EFUSE_RD_DATA_REG_OFFSET			(0x00000014U)
			/**< eFUSE read data register offset */
#define XNVM_EFUSE_TPGM_REG_OFFSET			(0x00000018U)
			/**< eFUSE program timing register offset */
#define XNVM_EFUSE_TRD_REG_OFFSET			(0x0000001CU)
			/**< eFUSE read timing register offset */
#define XNVM_EFUSE_TSU_H_PS_REG_OFFSET			(0x00000020U)
			/**< eFUSE TSU_H_PS timing register offset */
#define XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET		(0x00000024U)
			/**< eFUSE TSU_H_PS_CS timing register offset */
#define XNVM_EFUSE_TRDM_REG_OFFSET			(0x00000028U)
			/**< eFUSE TRDM timing register offset */
#define XNVM_EFUSE_TSU_H_CS_REG_OFFSET			(0x0000002CU)
			/**< eFUSE TSU_H_CS timing register offset */
#define XNVM_EFUSE_ISR_REG_OFFSET			(0x00000030U)
			/**< eFUSE interrupt status register offset */
#define XNVM_EFUSE_CACHE_LOAD_REG_OFFSET		(0x00000040U)
			/**< eFUSE cache load register offset */
#define XNVM_EFUSE_AES_CRC_REG_OFFSET			(0x00000048U)
			/**< eFUSE AES CRC register offset */
#define XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET		(0x0000004CU)
			/**< eFUSE AES user key 0 CRC register offset */
#define XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET		(0x00000050U)
			/**< eFUSE AES user key 1 CRC register offset */
#define XNVM_EFUSE_PD_REG_OFFSET			(0x00000054U)
			/**< eFUSE power-down register offset */
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET			(0x00000100U)
			/**< eFUSE test control register offset */
/** @} */

/**
 * @name Register: EFUSE_CTRL_CFG
 *  @{
 */
#define XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK    		(0x00000004U)
			/**< eFuse CTRL STATUS Register Masks */
/** @} */

/**
 * @name Register: EFUSE_CTRL_STATUS Register bit masks
 * @{
 */
/* access_type: ro  */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK	(0x00000800U)
			/**< Bit mask for AES user key 1 CRC pass */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK	(0x00000400U)
			/**< Bit mask for AES user key 1 CRC done */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK	(0x00000200U)
			/**< Bit mask for AES user key 0 CRC pass */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK	(0x00000100U)
			/**< Bit mask for AES user key 0 CRC done */
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK		(0x00000080U)
			/**< Bit mask for AES CRC pass */
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK		(0x00000040U)
			/**< Bit mask for AES CRC done */
/** @} */

/**
 * @name WR_UNLOCK Code
 * @{
 */
#define XNVM_EFUSE_WR_UNLOCK_PASSCODE			(0xDF0DU)
			/**< eFuse Write Unlock Passcode */
/** @} */

/**
 * @name eFuse Controller CFG register
 * @{
 */
#define XNVM_EFUSE_CFG_ENABLE_PGM			(0x01U << 1U)
			/**< eFUSE configuration enable program bit */
#define XNVM_EFUSE_CFG_MARGIN_RD			(0x01U << 2U)
			/**< eFUSE configuration margin read bit */
#define XNVM_EFUSE_CFG_NORMAL_RD			(0x00U << 2U)
			/**< eFUSE configuration normal read bit */
/** @} */

/**
 * @name eFuse STATUS register
 * @{
 */
#define XNVM_EFUSE_STATUS_TBIT_0			(0x01U << 0U) /**< eFUSE status TBIT 0 */
#define XNVM_EFUSE_STATUS_TBIT_1			(0x01U << 1U) /**< eFUSE status TBIT 1 */
#define XNVM_EFUSE_STATUS_TBIT_2			(0x01U << 2U) /**< eFUSE status TBIT 2 */
#define XNVM_EFUSE_STATUS_CACHE_DONE			(0x01U << 5U) /**< eFUSE status cache done bit */
/** @} */

/**
 * @name eFuse Controller PGM_ADDR register
 * @{
 */
#define XNVM_EFUSE_ADDR_COLUMN_SHIFT			(0U) /**< eFUSE address column bit shift */
#define XNVM_EFUSE_ADDR_ROW_SHIFT			(5U) /**< eFUSE address row bit shift */
#define XNVM_EFUSE_ADDR_PAGE_SHIFT			(13U) /**< eFUSE address page bit shift */
/** @} */

#define XNVM_EFUSE_CACHE_LOAD_MASK			(0x01U) /**< eFuse Cache load mask */

#define XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK		(0x1FFFU)
			/**< eFuse Security_Misc_1 Protection Mask */

/**
 * @name eFuse ISR Register
 * @{
 */
#define XNVM_EFUSE_ISR_PGM_DONE				(0x01U << 0U)
			/**< eFUSE ISR program done bit */
#define XNVM_EFUSE_ISR_PGM_ERROR			(0x01U << 1U)
			/**< eFUSE ISR program error bit */
#define XNVM_EFUSE_ISR_RD_DONE				(0x01U << 2U)
			/**< eFUSE ISR read done bit */
#define XNVM_EFUSE_ISR_CACHE_ERROR			(0x01U << 4U)
			/**< eFUSE ISR cache error bit */
/** @} */


#define XNVM_EFUSE_PD_ENABLE				(0x01U << 0U)
			/**< eFuse controller Power Down bit mask */

/**
 * @name PS Reference Clock Frequency
 * For PL-Microblaze, read from RTCA; for APU/RPU, use xparameters macro
 * @{
 */
#if defined(__microblaze__) && defined(VERSAL)
#define XNVM_PSU_PSS_REF_CLK_FREQ_RTCA_ADDR		(0xF2014370U)
			/**< RTCA address to store PSU PSS reference clock frequency in Hz */
#define XNVM_PSU_PSS_REF_CLK_FREQ		(Xil_In32(XNVM_PSU_PSS_REF_CLK_FREQ_RTCA_ADDR))
			/**< PSU PSS reference clock definition in Hz */
#else
#define XNVM_PSU_PSS_REF_CLK_FREQ		(XPAR_PSU_PSS_REF_CLK_FREQ_HZ)
			/**< PSU PSS reference clock definition in Hz */
#endif
/** @} */

#define XNVM_NUM_OF_ROWS_PER_PAGE			(256U) /**< Number of Rows per Page */


/**
 * @name Timeout values
 * @{
 */
/** Timeout in term of number of times status register polled to check eFUSE
 * read operation complete
 */
#define XNVM_EFUSE_RD_TIMEOUT_VAL			(100U)

/** Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL			(100U)

/** Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL		(0x800U)

/** Timeout in term of number of times status register polled to check eFuse
 * Crc check id done.
 */
#define XNVM_POLL_TIMEOUT				(0x400U)
/** @} */

/**
 * @name EFUSE_CACHE Base Address
 * @{
 */
#define XNVM_EFUSE_CACHE_BASEADDR				(0xF1250000U)
			/**< eFuse Cache Base Address */
/** @} */

/**
 *  @name eFUSE Cache Register Offsets
 *  @{
 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_OFFSET    			(0x00000000U)
			/**< eFUSE cache TBITS0_SVD register offset */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_2_OFFSET			(0x0000000CU)
			/**< eFUSE cache analog trim 2 register offset */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_3_OFFSET			(0x00000010U)
			/**< eFUSE cache analog trim 3 register offset */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET			(0x00000094U)
			/**< eFUSE cache boot environment control register offset */
#define XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET			(0x000000A0U)
			/**< eFUSE cache miscellaneous control register offset */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_OFFSET		(0x000000A4U)
			/**< eFUSE cache PUF ECC control register offset */
#define XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET			(0x000000A8U)
			/**< eFUSE cache PUF CHASH register offset */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET		(0x000000ACU)
			/**< eFUSE cache security control register offset */
#define XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET			(0x000000E4U)
			/**< eFUSE cache security miscellaneous 0 register offset */
#define XNVM_EFUSE_CACHE_SECURITY_MISC_1_OFFSET			(0x000000E8U)
			/**< eFUSE cache security miscellaneous 1 register offset */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_6_OFFSET			(0x000000F4U)
			/**< eFUSE cache analog trim 6 register offset */
#define XNVM_EFUSE_CACHE_ANLG_TRIM_7_OFFSET			(0x000000F8U)
			/**< eFUSE cache analog trim 7 register offset */
#define XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET			(0x00000100U)
			/**< eFUSE cache PPK0 hash 0 register offset */
#define	XNVM_EFUSE_CACHE_PPK1_HASH_0_OFFSET			(0x00000120U)
			/**< eFUSE cache PPK1 hash 0 register offset */
#define	XNVM_EFUSE_CACHE_PPK2_HASH_0_OFFSET			(0x00000140U)
			/**< eFUSE cache PPK2 hash 0 register offset */
#define XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET		(0x00000180U)
			/**< eFUSE cache meta header IV range 0 register offset */
#define XNVM_EFUSE_CACHE_BLACK_IV_0_OFFSET			(0x000001D0U)
			/**< eFUSE cache black IV 0 register offset */
#define XNVM_EFUSE_CACHE_PLM_IV_RANGE_0_OFFSET			(0x000001DCU)
			/**< eFUSE cache PLM IV range 0 register offset */
#define XNVM_EFUSE_CACHE_DATA_PARTITION_IV_RANGE_0_OFFSET	(0x000001E8U)
			/**< eFUSE cache data partition IV range 0 register offset */
#define XNVM_EFUSE_CACHE_TRIM_AMS_12_OFFSET			(0x000001B0U)
			/**< eFUSE cache AMS trim 12 register offset */
#define XNVM_EFUSE_CACHE_PUF_SYN_DATA_OFFSET			(0x00000300U)
			/**< eFUSE cache PUF syndrome data register offset */
#ifdef VERSAL_2VE_2VM
#define XNVM_EFUSE_CACHE_PPK0_USER_FUSE_OFFSET			(0x000002D0U)
			/**< eFUSE cache PPK0 user fuse register offset */
#define XNVM_EFUSE_CACHE_PPK1_USER_FUSE_OFFSET			(0x000002E0U)
			/**< eFUSE cache PPK1 user fuse register offset */
#define XNVM_EFUSE_CACHE_PPK2_USER_FUSE_OFFSET			(0x000002F0U)
			/**< eFUSE cache PPK2 user fuse register offset */
#define XNVM_EFUSE_PPK_HASH_USER_FUSE_NUM_OF_CACHE_ROWS		(4U)
			/**< Number of cache rows allocated for PPK hash user fuses */
#endif

/** @} */

/**
 * @name Register: EFUSE_CACHE_TBITS0_SVD
 * @{
 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK    		(0x08000000U)
			/**< Bit mask for TBITS0_SVD anchor 3 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK    		(0x04000000U)
			/**< Bit mask for TBITS0_SVD anchor 2 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_37_PROT_MASK    	(0x00200000U)
			/**< Bit mask for TBITS0_SVD row 37 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_40_PROT_MASK    	(0x00100000U)
			/**< Bit mask for TBITS0_SVD row 40 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_42_PROT_MASK    	(0x00080000U)
			/**< Bit mask for TBITS0_SVD row 42 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_58_PROT_MASK    	(0x00040000U)
			/**< Bit mask for TBITS0_SVD row 58 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK		(0x00000002U)
			/**< Bit mask for TBITS0_SVD anchor 1 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK		(0x00000001U)
			/**< Bit mask for TBITS0_SVD anchor 0 */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_43_PROT_MASK 		(0x02000004U)
			/**< Bit mask for TBITS0_SVD row 43 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_57_PROT_MASK 		(0x01000008U)
			/**< Bit mask for TBITS0_SVD row 57 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_64_87_PROT_MASK		(0x00810000U)
			/**< Bit mask for TBITS0_SVD rows 64-87 protection */
#define XNVM_EFUSE_CACHE_TBITS0_SVD_ROW_96_99_PROT_MASK		(0x00420000U)
			/**< Bit mask for TBITS0_SVD rows 96-99 protection */
/** @} */

/**
 * @name Register: EFUSE_CACHE_SECURITY_CONTROL_REG
 * @{
 */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK	(0xc0000000U)
			/**< Bit mask for register init disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK	(0x10000000U)
			/**< Bit mask for boot environment write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_MASK	(0x03800000U)
			/**< Bit mask for PMC SC enable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_MASK	(0x00600000U)
			/**< Bit mask for secure lock debug disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK	(0x00180000U)
			/**< Bit mask for secure debug disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK		(0x00040000U)
			/**< Bit mask for PUF disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK	(0x00020000U)
			/**< Bit mask for PUF test2 disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK	(0x00010000U)
			/**< Bit mask for PUF syndrome lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK	(0x00008000U)
			/**< Bit mask for user key 1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK	(0x00004000U)
			/**< Bit mask for user key 1 CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK	(0x00002000U)
			/**< Bit mask for user key 0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK	(0x00001000U)
			/**< Bit mask for user key 0 CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK	(0x00000800U)
			/**< Bit mask for AES write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK	(0x00000600U)
			/**< Bit mask for AES CRC lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK	(0x00000100U)
			/**< Bit mask for PPK2 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK	(0x00000080U)
			/**< Bit mask for PPK1 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK	(0x00000040U)
			/**< Bit mask for PPK0 write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_MASK	(0x00000008U)
			/**< Bit mask for HW test bits disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK		(0x00000004U)
			/**< Bit mask for JTAG disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK	(0x00000002U)
			/**< Bit mask for JTAG error out disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK		(0x00000001U)
			/**< Bit mask for AES disable */

#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT	(30U)
			/**< Bit shift for register init disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT		(28U)
			/**< Bit shift for boot environment write lock */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_SHIFT		(23U)
			/**< Bit shift for PMC SC enable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_1_0_SHIFT	(21U)
			/**< Bit shift for secure lock debug disable */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_1_0_SHIFT	(19U)
			/**< Bit shift for secure debug disable */
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
			/**< Bit mask for PUF ECC bits */

#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT		(31U)
			/**< Bit shift for PUF regeneration disable */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT		(30U)
			/**< Bit shift for PUF helper data invalid */
/** @} */

/**
 * @name Register: EFUSE_CACHE_SECURITY_MISC_0
 * @{
 */
#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)
			/**< eFuse Cache DEC_EFUSE_ONLY Mask */
/** @} */

/**
 * @name Register: EFUSE_CACHE_SECURITY_MISC_1
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
 * @name Register: EFUSE_CACHE_MISC_CTRL
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
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_MASK		(0x00000600U)
			/**< Bit mask for PPK3 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_MASK		(0x00001800U)
			/**< Bit mask for PPK4 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_ADD_PPK_1_0_MASK         (0x00030000U)
			/**< Bit mask for additional PPK */

#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_SHIFT	   	(30U)
			/**< Bit shift for glitch detector halt boot enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_SHIFT   		(29U)
			/**< Bit shift for glitch detector ROM monitor enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_SHIFT   		(21U)
			/**< Bit shift for halt boot error */
#define XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_SHIFT   		(19U)
			/**< Bit shift for halt boot environment */
#define XNVM_EFUSE_CACHE_MISC_CTRL_ADD_PPK_EN_SHIFT				(16U)
			/**< Bit shift for additional PPK enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_SHIFT   		(15U)
			/**< Bit shift for crypto KAT enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_SHIFT   			(14U)
			/**< Bit shift for LBIST enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK4_INVLD_1_0_SHIFT			(11U)
			/**< Bit shift for PPK4 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK3_INVLD_1_0_SHIFT			(9U)
			/**< Bit shift for PPK3 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_SHIFT   	(8U)
			/**< Bit shift for safety mission enable */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_SHIFT			(6U)
			/**< Bit shift for PPK2 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_SHIFT			(4U)
			/**< Bit shift for PPK1 invalid */
#define XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_SHIFT			(2U)
			/**< Bit shift for PPK0 invalid */
/** @} */

/**
 * @name Register: EFUSE_CACHE_BOOT_ENV_CTRL
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
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK	(0x00000200U)
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
 * @name Register: SYSMON_SAT_REG
 * @{
 */
#define XNVM_EFUSE_SYSMON_SAT_ADDR_ID_MASK		(0x3fc00000U)
			/**< Bit mask for sysmon satellite address ID */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_MASK		(0x00300000U)
			/**< Bit mask for sysmon satellite config mode */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_MASK	(0x000f0000U)
			/**< Bit mask for sysmon satellite AMUX control */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_MASK	(0x0000ff00U)
			/**< Bit mask for sysmon satellite ABUS SW1 */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_MASK	(0x000000ffU)
			/**< Bit mask for sysmon satellite ABUS SW0 */

#define XNVM_EFUSE_SYSMON_SAT_ADDR_ID_SHIFT		(22U)
			/**< Bit shift for sysmon satellite address ID */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_SHIFT		(20U)
			/**< Bit shift for sysmon satellite config mode */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_SHIFT	(16U)
			/**< Bit shift for sysmon satellite AMUX control */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_SHIFT	(8U)
			/**< Bit shift for sysmon satellite ABUS SW1 */
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_SHIFT	(0U)
			/**< Bit shift for sysmon satellite ABUS SW0 */
/** @} */

/**
 * @name EFUSE row count
 * @{
 */
#define XNVM_EFUSE_AES_KEY_NUM_OF_ROWS			(8U)
			/**< Number of eFUSE rows for AES key */
#define XNVM_EFUSE_USER_KEY_NUM_OF_ROWS			(8U)
			/**< Number of eFUSE rows for user key */
#ifndef VERSAL_NET
#define XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS			(8U)
			/**< Number of eFUSE rows for PPK hash */
#define XNVM_EFUSE_BOOT_ENV_CTRL_NUM_OF_ROWS	(1U)
			/**< Number of eFUSE rows for boot environment control */
#else
#define XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS         (32U)
			/**< Number of eFUSE rows for PPK hash */
#define XNVM_EFUSE_BOOT_ENV_CTRL_NUM_OF_ROWS    (4U)
			/**< Number of eFUSE rows for boot environment control */
#endif
#define XNVM_EFUSE_DEC_EFUSE_ONLY_NUM_OF_ROWS		(1U)
			/**< Number of eFUSE rows for decrypt eFUSE only */
#define XNVM_EFUSE_SECURITY_MISC_1_NUM_OF_ROWS		(1U)
			/**< Number of eFUSE rows for security misc 1 */
#define XNVM_EFUSE_GLITCH_NUM_OF_ROWS			(1U)
			/**< Number of eFUSE rows for glitch detection */
#define XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS		(127U)
			/**< Number of eFUSE rows for PUF syndrome data */
#define XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS		(1U)
			/**< Number of eFUSE rows for PUF CHASH */
#define XNVM_EFUSE_PUF_AUX_NUM_OF_ROWS			(1U)
			/**< Number of eFUSE rows for PUF AUX */
#define XNVM_EFUSE_IV_NUM_OF_ROWS			(3U)
			/**< Number of eFUSE rows for IV */
#define XNVM_EFUSE_DNA_NUM_OF_ROWS			(4U)
			/**< Number of eFUSE rows for DNA */
/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_COMMON_HW_H */
