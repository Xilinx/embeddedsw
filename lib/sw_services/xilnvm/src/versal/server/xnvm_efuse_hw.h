/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse_hw.h
* @addtogroup xnvm_versal_Efuse_HW XilNvm Versal Efuse HW Reg
* @{
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
 */
/**< EFUSE Row numbers */
#define XNVM_EFUSE_TBITS_XILINX_CTRL_ROW		(0U)
#define XNM_EFUSE_GLITCH_ANLG_TRIM_3			(4U)
#define XNVM_EFUSE_DNA_START_ROW			(8U)
#define XNVM_EFUSE_AES_KEY_START_ROW			(12U)
#define XNVM_EFUSE_USER_KEY_0_START_ROW			(20U)
#define XNVM_EFUSE_USER_KEY_1_START_ROW			(28U)
#define XNVM_EFUSE_USER_KEY_1_END_ROW			(35U)
#define XNVM_EFUSE_BOOT_ENV_CTRL_ROW			(37U)
#define XNVM_EFUSE_MISC_CTRL_ROW			(40U)
#define XNVM_EFUSE_PUF_AUX_ROW				(41U)
#define XNVM_EFUSE_PUF_CHASH_ROW			(42U)
#define XNVM_EFUSE_SECURITY_CONTROL_ROW			(43U)
#define XNVM_EFUSE_REVOCATION_ID_0_ROW			(44U)
#define XNVM_EFUSE_DEC_EFUSE_ONLY_ROW			(57U)
#define XNVM_EFUSE_SECURITY_MISC_1_ROW			(58U)
#define XNVM_EFUSE_PPK_0_HASH_START_ROW			(64U)
#define XNVM_EFUSE_PPK_1_HASH_START_ROW			(72U)
#define XNVM_EFUSE_PPK_2_HASH_START_ROW			(80U)
#define XNVM_EFUSE_PPK_3_HASH_START_ROW			(176U)
#define XNVM_EFUSE_PPK_4_HASH_START_ROW			(184U)
#define XNVM_EFUSE_OFFCHIP_REVOKE_0_ROW			(88U)
#define XNVM_EFUSE_META_HEADER_IV_START_ROW		(96U)
#define XNVM_EFUSE_BLACK_OBFUS_IV_START_ROW		(116U)
#define XNVM_EFUSE_PLM_IV_START_ROW			(119U)
#define XNVM_EFUSE_DATA_PARTITION_IV_START_ROW		(122U)
#define XNVM_EFUSE_PUF_SYN_START_ROW			(129U)
#define XNVM_EFUSE_USER_FUSE_START_ROW			(129U)
#define XNVM_EFUSE_PUF_SYN_CACHE_READ_ROW       (641U)
/** @} */

/**
 * @name  EFUSE column numbers
 */
/**< EFUSE column numbers */
#define XNVM_EFUSE_ROW_43_1_PROT_COLUMN			(2U)
#define XNVM_EFUSE_ROW_57_1_PROT_COLUMN			(3U)
#define XNVM_EFUSE_ROW64_87_1_PROT_COLUMN		(16U)
#define XNVM_EFUSE_ROW96_99_1_PROT_COLUMN		(17U)
#define XNVM_EFUSE_ROW_58_PROT_COLUMN			(18U)
#define XNVM_EFUSE_ROW_42_PROT_COLUMN			(19U)
#define XNVM_EFUSE_ROW_40_PROT_COLUMN			(20U)
#define XNVM_EFUSE_ROW_37_PROT_COLUMN			(21U)
#define XNVM_EFUSE_ROW96_99_0_PROT_COLUMN		(22U)
#define XNVM_EFUSE_ROW64_87_0_PROT_COLUMN		(23U)
#define XNVM_EFUSE_ROW_57_0_PROT_COLUMN			(24U)
#define XNVM_EFUSE_ROW_43_0_PROT_COLUMN			(25U)
#define XNVM_EFUSE_GLITCH_WRLK_COLUMN			(31U)
#define XNVM_EFUSE_GLITCH_ROM_EN_COLUMN			(29U)
#define XNVM_EFUSE_GLITCH_HALT_EN_0_COLUMN		(30U)
#define XNVM_EFUSE_GLITCH_HALT_EN_1_COLUMN		(31U)
#define XNVM_EFUSE_HALT_BOOT_ERROR_1			(22U)
#define XNVM_EFUSE_HALT_BOOT_ERROR_0			(21U)
#define XNVM_EFUSE_HALT_ENV_ERROR_1				(20U)
#define XNVM_EFUSE_HALT_ENV_ERROR_0				(19U)
#define XNVM_EFUSE_PUF_ECC_PUF_CTRL_REGEN_DIS_COLUMN	(31U)
#define XNVM_EFUSE_PUF_ECC_PUF_CTRL_HD_INVLD_COLUMN	(30U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_HW_H */
