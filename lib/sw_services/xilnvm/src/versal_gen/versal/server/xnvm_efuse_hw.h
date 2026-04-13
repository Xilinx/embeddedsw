/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse_hw.h
* This file contains NVM library eFUSE controller register definitions
*
* @cond xnvm_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
* 2.0	kal  02/27/2019	Added eFuse rows, Cache offsets
* 2.1   kal  07/09/2020 Replaced hardcoded CLK_REF_FREQ to the
*                       XPAR_PSU_PSS_REF_CLK_FREQ_HZ
*	am   08/19/2020 Resolved MISRA C violations.
*	kal  09/03/2020 Fixed Security CoE review comments
*	am   10/13/2020 Resolved MISRA C violations
* 2.3   am   11/23/2020 Resolved MISRA C violations
*	kal  01/07/2021	Added new macros for BootEnvCtrl and SecurityMisc1
*			eFuse rows
*	kal  02/20/2021 Added new macros for sysmon measure registers
*   kpt  05/20/2021 Added macro XNVM_EFUSE_PUF_SYN_CACHE_READ_ROW for
*                   PUF cache read
* 2.4   kal  07/13/2021 Fixed doxygen warnings
*       har  09/16/2021 Added macro for offset for ANLG TRIM 2 register
* 3.0	kal  07/12/2022	Moved common hw definitions to xnvm_efuse_common_hw.h
* 3.1   skg  12/07/2022 Added Additional PPKs related macros
* 3.3   har  12/04/2023 Added support for HWTSTBITS_DIS and PMC_SC_EN efuse bits
* 3.6   hj   09/19/2025 Program PUF_RSVD efuse as user data
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
#include "xparameters.h"
#include "xnvm_efuse_common_hw.h"

/*************************** Constant Definitions *****************************/
/**
 * @name  EFUSE row numbers
 * @{
 */
#define XNVM_EFUSE_TBITS_XILINX_CTRL_ROW		(0U) /**< eFUSE Xilinx control TBITS row */
#define XNM_EFUSE_GLITCH_ANLG_TRIM_3			(4U) /**< eFUSE glitch analog trim 3 row */
#define XNVM_EFUSE_DNA_START_ROW			(8U) /**< eFUSE DNA start row */
#define XNVM_EFUSE_AES_KEY_START_ROW			(12U) /**< eFUSE AES key start row */
#define XNVM_EFUSE_USER_KEY_0_START_ROW			(20U) /**< eFUSE user key 0 start row */
#define XNVM_EFUSE_USER_KEY_1_START_ROW			(28U) /**< eFUSE user key 1 start row */
#define XNVM_EFUSE_USER_KEY_1_END_ROW			(35U) /**< eFUSE user key 1 end row */
#define XNVM_EFUSE_BOOT_ENV_CTRL_ROW			(37U) /**< eFUSE boot environment control row */
#define XNVM_EFUSE_MISC_CTRL_ROW			(40U) /**< eFUSE miscellaneous control row */
#define XNVM_EFUSE_PUF_AUX_ROW				(41U) /**< eFUSE PUF AUX row */
#define XNVM_EFUSE_PUF_CHASH_ROW			(42U) /**< eFUSE PUF CHASH row */
#define XNVM_EFUSE_SECURITY_CONTROL_ROW			(43U) /**< eFUSE security control row */
#define XNVM_EFUSE_REVOCATION_ID_0_ROW			(44U) /**< eFUSE revocation ID 0 row */
#define XNVM_EFUSE_DEC_EFUSE_ONLY_ROW			(57U) /**< eFUSE decrypt eFUSE only row */
#define XNVM_EFUSE_SECURITY_MISC_1_ROW			(58U) /**< eFUSE security misc 1 row */
#define XNVM_EFUSE_PPK_0_HASH_START_ROW			(64U) /**< eFUSE PPK0 hash start row */
#define XNVM_EFUSE_PPK_1_HASH_START_ROW			(72U) /**< eFUSE PPK1 hash start row */
#define XNVM_EFUSE_PPK_2_HASH_START_ROW			(80U) /**< eFUSE PPK2 hash start row */
#define XNVM_EFUSE_PPK_3_HASH_START_ROW			(176U) /**< eFUSE PPK3 hash start row */
#define XNVM_EFUSE_PPK_4_HASH_START_ROW			(184U) /**< eFUSE PPK4 hash start row */
#define XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW			(88U) /**< eFUSE off-chip revocation ID 0 row */
#define XNVM_EFUSE_META_HEADER_IV_START_ROW		(96U) /**< eFUSE meta header IV start row */
#define XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW		(116U) /**< eFUSE black obfuscated IV start row */
#define XNVM_EFUSE_PLM_IV_START_ROW			(119U) /**< eFUSE PLM IV start row */
#define XNVM_EFUSE_DATA_PARTITION_IV_START_ROW		(122U) /**< eFUSE data partition IV start row */
#define XNVM_EFUSE_PUF_SYN_START_ROW			(129U) /**< eFUSE PUF syndrome data start row */
#define XNVM_EFUSE_USER_FUSE_START_ROW			(129U) /**< eFUSE user fuse start row */

#ifdef XNVM_ACCESS_PUF_USER_DATA
#define XNVM_EFUSE_PUF_SYN_RSVD_START_ROW		(128U)
                        /**< eFUSE PUF reserved start row, programmed as user data */
#define XNVM_EFUSE_PUF_SYN_USERDATA_NUM_OF_ROWS		(128U)
                        /**< Number of eFUSE rows reserved for PUF user data */
#define XNVM_EFUSE_PUF_SYN_RSVD_CACHE_READ_ROW		(640U)
                        /**< eFUSE cache read row for PUF reserved data */
#endif
/** @} */

/**
 * @name  EFUSE column numbers
 * @{
 */
#define XNVM_EFUSE_ROW_43_1_PROT_COLUMN			(2U) /**< eFUSE row 43 page 1 protection column */
#define XNVM_EFUSE_ROW_57_1_PROT_COLUMN			(3U) /**< eFUSE row 57 page 1 protection column */
#define XNVM_EFUSE_ROW64_87_1_PROT_COLUMN		(16U) /**< eFUSE rows 64-87 page 1 protection column */
#define XNVM_EFUSE_ROW96_99_1_PROT_COLUMN		(17U) /**< eFUSE rows 96-99 page 1 protection column */
#define XNVM_EFUSE_ROW_58_PROT_COLUMN			(18U) /**< eFUSE row 58 protection column */
#define XNVM_EFUSE_ROW_42_PROT_COLUMN			(19U) /**< eFUSE row 42 protection column */
#define XNVM_EFUSE_ROW_40_PROT_COLUMN			(20U) /**< eFUSE row 40 protection column */
#define XNVM_EFUSE_ROW_37_PROT_COLUMN			(21U) /**< eFUSE row 37 protection column */
#define XNVM_EFUSE_ROW96_99_0_PROT_COLUMN		(22U) /**< eFUSE rows 96-99 page 0 protection column */
#define XNVM_EFUSE_ROW64_87_0_PROT_COLUMN		(23U) /**< eFUSE rows 64-87 page 0 protection column */
#define XNVM_EFUSE_ROW_57_0_PROT_COLUMN			(24U) /**< eFUSE row 57 page 0 protection column */
#define XNVM_EFUSE_ROW_43_0_PROT_COLUMN			(25U) /**< eFUSE row 43 page 0 protection column */
#define XNVM_EFUSE_GLITCH_WRLK_COLUMN			(31U) /**< eFUSE glitch write lock column */
#define XNVM_EFUSE_GLITCH_ROM_EN_COLUMN			(29U) /**< eFUSE glitch ROM enable column */
#define XNVM_EFUSE_GLITCH_HALT_EN_0_COLUMN		(30U) /**< eFUSE glitch halt enable 0 column */
#define XNVM_EFUSE_GLITCH_HALT_EN_1_COLUMN		(31U) /**< eFUSE glitch halt enable 1 column */
#define XNVM_EFUSE_HALT_BOOT_ERROR_1			(22U) /**< eFUSE halt boot error 1 column */
#define XNVM_EFUSE_HALT_BOOT_ERROR_0			(21U) /**< eFUSE halt boot error 0 column */
#define XNVM_EFUSE_HALT_ENV_ERROR_1			(20U) /**< eFUSE halt environment error 1 column */
#define XNVM_EFUSE_HALT_ENV_ERROR_0			(19U) /**< eFUSE halt environment error 0 column */
#define XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN	(31U) /**< eFUSE PUF ECC regeneration disable column */
#define XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN	(30U) /**< eFUSE PUF ECC helper data invalid column */
/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_HW_H */
