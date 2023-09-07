/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_efuse_common_hw.h
* @addtogroup xnvm_efuse_hw XilNvm eFuse controller registers
* @{
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
* 3.2   kum 04/11/2023 moved BOOTENV, SYSMON related macros to common to make use for both veral and versalnet
*
* </pre>
*
* @note
*
* @endcond
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
 * @name  PMC sysmon sat0 base address
 */
/**< SYSMONPSV_SAT0 Base Address */
#define XNVM_EFUSE_SYSMONPSV_SAT0_BASEADDR		(0xF1280000U)
/** @} */

/**
 * @name  PMC sysmon sat1 base address
 */
/**< SYSMONPSV_SAT1 Base Address */
#define XNVM_EFUSE_SYSMONPSV_SAT1_BASEADDR		(0xF1290000U)
/** @} */

/**
 * @name  PMC sysmon measure0 offset
 */
/**< SYSMONPSV_SAT_MEASURE0 Offset */
#define XNVM_EFUSE_SYSMONPSV_SAT_MEASURE0_OFFSET	(0x00000524U)
/** @} */

/**
 * @name CRP base address definition
 */
/**< CRP Base Address */
#define XNVM_CRP_BASE_ADDR				(0xF1260000U)
/** @} */

/**
 * @name CRP eFuse Clock Control Register
 */
/**< CRP REF_CLK offset and definition */
#define XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET		(0x00000134U)
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT		(2U)
#define XNVM_CRP_EFUSE_REF_CLK_IN			((u32)0x01U << \
					XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT)
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC		(XNVM_CRP_EFUSE_REF_CLK_IN)
/** @} */

/**
 * @name eFuse Controller base address
 */
/**< eFuse Control Base Address */
#define XNVM_EFUSE_CTRL_BASEADDR			(0xF1240000U)
/** @} */

/**
 * @name eFuse Controller Register Offsets
 */
/**< eFuse CTRL Register Offsets */
#define XNVM_EFUSE_WR_LOCK_REG_OFFSET			(0x00000000U)
#define XNVM_EFUSE_CFG_REG_OFFSET			(0x00000004U)
#define XNVM_EFUSE_STATUS_REG_OFFSET			(0x00000008U)
#define XNVM_EFUSE_PGM_ADDR_REG_OFFSET			(0x0000000CU)
#define XNVM_EFUSE_RD_ADDR_REG_OFFSET			(0x00000010U)
#define XNVM_EFUSE_RD_DATA_REG_OFFSET			(0x00000014U)
#define XNVM_EFUSE_TPGM_REG_OFFSET			(0x00000018U)
#define XNVM_EFUSE_TRD_REG_OFFSET			(0x0000001CU)
#define XNVM_EFUSE_TSU_H_PS_REG_OFFSET			(0x00000020U)
#define XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET		(0x00000024U)
#define XNVM_EFUSE_TRDM_REG_OFFSET			(0x00000028U)
#define XNVM_EFUSE_TSU_H_CS_REG_OFFSET			(0x0000002CU)
#define XNVM_EFUSE_ISR_REG_OFFSET			(0x00000030U)
#define XNVM_EFUSE_CACHE_LOAD_REG_OFFSET		(0x00000040U)
#define XNVM_EFUSE_AES_CRC_REG_OFFSET			(0x00000048U)
#define XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET		(0x0000004CU)
#define XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET		(0x00000050U)
#define XNVM_EFUSE_PD_REG_OFFSET			(0x00000054U)
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET			(0x00000100U)
/** @} */

/**
 * @name Register: EFUSE_CTRL_CFG
 *  @{
 */
/**< eFuse CTRL STATUS Register Masks */
#define XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK    		(0x00000004U)

/* access_type: ro  */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK	(0x00000800U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK	(0x00000400U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK	(0x00000200U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK	(0x00000100U)
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK		(0x00000080U)
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK		(0x00000040U)
/** @} */

/**
 * @name  EFUSE_CACHE Base Address
 * @{
 */
/**< eFuse Cache Base Address */
#define XNVM_EFUSE_CACHE_BASEADDR				(0xF1250000U)
/** @} */

#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET			(0x00000094U)
						/**< eFuse cache boot env control offset*/
/**
 * @name Register: EFUSE_CACHE_PUF_ECC_CTRL
 *  @{
 */
/**< eFuse CACHE PUF ECC CTRL Register Masks And Shifts */
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK	(0x80000000U)
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK		(0x40000000U)
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK		(0x00ffffffU)

#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT		(31U)
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT		(30U)
/** @} */

/**
 * @name  Register: EFUSE_CACHE_SECURITY_MISC_0
 */
/**< eFuse Cache DEC_EFUSE_ONLY Mask */
#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)

/**< eFuse row count numbers */
#define XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS		(127U)
#define XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS		(1U)
#define XNVM_EFUSE_PUF_AUX_NUM_OF_ROWS			(1U)
/** @} */

/**
 * @name  WR_UNLOCK Code
 */
/**< eFuse Write Unlock Passcode */
#define XNVM_EFUSE_WR_UNLOCK_PASSCODE			(0xDF0DU)
/** @} */

/**
 * @name eFuse Controller CFG register
 */
/**< eFuse CFG Modes */
#define XNVM_EFUSE_CFG_ENABLE_PGM			(0x01U << 1U)
#define XNVM_EFUSE_CFG_MARGIN_RD			(0x01U << 2U)
#define XNVM_EFUSE_CFG_NORMAL_RD			(0x00U << 2U)
/** @} */

/**
 * @name eFuse STATUS register
 */
/**< eFuse Status Register Masks */
#define XNVM_EFUSE_STATUS_TBIT_0			(0x01U << 0U)
#define XNVM_EFUSE_STATUS_TBIT_1			(0x01U << 1U)
#define XNVM_EFUSE_STATUS_TBIT_2			(0x01U << 2U)
#define XNVM_EFUSE_STATUS_CACHE_DONE			(0x01U << 5U)
/** @} */

/**
 * @name eFuse Controller PGM_ADDR register
 */
/**< eFuse Addres Shifts */
#define XNVM_EFUSE_ADDR_COLUMN_SHIFT			(0U)
#define XNVM_EFUSE_ADDR_ROW_SHIFT			(5U)
#define XNVM_EFUSE_ADDR_PAGE_SHIFT			(13U)
/** @} */


#define XNVM_EFUSE_CACHE_LOAD_MASK			(0x01U) /**< eFuse Cache load mask */


#define XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK		(0x1FFFU) /**< eFuse Security_Misc_1 Protection Mask */


/**
 * @name eFuse ISR Register
 */
/**< eFuse ISR registers masks */
#define XNVM_EFUSE_ISR_PGM_DONE				(0x01U << 0U)
#define XNVM_EFUSE_ISR_PGM_ERROR			(0x01U << 1U)
#define XNVM_EFUSE_ISR_RD_DONE				(0x01U << 2U)
#define XNVM_EFUSE_ISR_CACHE_ERROR			(0x01U << 4U)
/** @} */


#define XNVM_EFUSE_PD_ENABLE				(0x01U << 0U) /**< eFuse controller Power Down bit mask */


#define XNVM_PS_REF_CLK_FREQ			(XPAR_PSU_PSS_REF_CLK_FREQ_HZ) /**< PS Ref clock definition in Hz */


#define XNVM_NUM_OF_ROWS_PER_PAGE			(256U) /**< Number of Rows per Page */


/**
 * @name Timeout values
 */
/**< Timeout in term of number of times status register polled to check eFUSE
 * read operation complete
 */
#define XNVM_EFUSE_RD_TIMEOUT_VAL			(100U)

/**< Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL			(100U)

/**< Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL		(0x800U)

/**< Timeout in term of number of times status register polled to check eFuse
 * Crc check id done.
 */
#define XNVM_POLL_TIMEOUT				(0x400U)
/** @} */

/**
 * @name  Register: EFUSE_CACHE_BOOT_ENV_CTRL
 */
/**< eFUSE Cache BOOT_ENV_CTRL Masks And Shifts */
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK	(0x00200000U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK	(0x00100000U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK	(0x00060000U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK	(0x00003000U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK	(0x00000c00U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK	(0x00000200U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK	(0x00000003U)

#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_SHIFT		(21U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_SHIFT		(20U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT		(17U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT		(12U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT		(10U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_SHIFT		(9U)
#define XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT		(0U)
/** @} */

/**
 * @name Register: SYSMON_SAT_REG
 */
/**< SYSMON SAT0/1 Register Masks */
#define XNVM_EFUSE_SYSMON_SAT_ADDR_ID_MASK              (0x3fc00000U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_MASK          (0x00300000U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_MASK     (0x000f0000U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_MASK      (0x0000ff00U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_MASK      (0x000000ffU)

#define XNVM_EFUSE_SYSMON_SAT_ADDR_ID_SHIFT   		(22U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_SHIFT   	(20U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_SHIFT   	(16U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_SHIFT   	(8U)
#define XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_SHIFT   	(0U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_COMMON_HW_H */
/* @} */
