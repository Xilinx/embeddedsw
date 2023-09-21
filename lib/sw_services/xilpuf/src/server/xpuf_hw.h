/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_hw.h
* @addtogroup xpuf_apis XilPuf APIs
* @{
* @cond xpuf_internal
* This file contains PUF hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
* 1.1   har  01/27/2020 Added offsets and register definitions for registers of
*                       EFUSE_CACHE module
*                       Moved definition of XPuf_ReadReg and XPuf_WriteReg to
*                       xpuf.c
*                       Renamed macros related to PUF Command register
* 1.2	am   08/19/2020 Resolved MISRA C violations.
*       har  09/30/2020 Removed header files which were not required
* 1.3   har  01/06/2021 Added offset for PUF_CLEAR register and its definition
*       har  03/08/2021 Added offset for IRO frequency
* 1.4   har  07/09/2021 Fixed doxygen warnings
* 1.5   har  03/21/2022 Added offset for GLOBAL_CNTRL register
*       kpt  03/24/2021 Added macro XPUF_IRO_TRIM_FUSE_SEL_BIT
* 2.2	vss  09/07/2023	Fixed MISRA-C Rule 2.5 violation
*	vss  09/21/2023 Fixed doxygen warnings
*
* </pre>
*
* @note
*
* @endcond
******************************************************************************/
#ifndef XPUF_HW_H
#define XPUF_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
/*************************** Constant Definitions *****************************/
#define XPUF_PMC_GLOBAL_BASEADDR			(0xF1110000U)
					/**< PMC_GLOBAL Base Address */

/**
 * @name  Offsets for PUF registers in PMC_GLOBAL module
 * @{
 */
/**< PUF register offsets */
#define XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET		(0x0U)
#define XPUF_PMC_GLOBAL_PUF_CMD_OFFSET			(0x00040000U)
#define XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET			(0x00040004U)
#define XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET			(0x00040008U)
#define XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET			(0x0004000CU)
#define XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET		(0x00040010U)
#define XPUF_PMC_GLOBAL_PUF_WORD_OFFSET			(0x00040018U)
#define XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET		(0x00040020U)
#define XPUF_PMC_GLOBAL_PUF_AUX_OFFSET			(0x00040024U)
#define XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET		(0x00040028U)
#define XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET		(0x0004002CU)
#define XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET			(0x00040030U)
/** @} */

/**
 * @name  GLOBAL Control register definition
 * @{
 */
/**< GLOBAL_CNTRL register definition */
#define XPUF_SLVERR_ENABLE_MASK		(0x1U << 1U)
/** @} */

/**
 * @name  PUF Command register definition
 * @{
 */
/**< PUF_CMD register definition */
#define XPUF_CMD_REGISTRATION		(0x01U)
#define XPUF_CMD_REGEN_ON_DEMAND	(0x02U)
#define XPUF_CMD_REGEN_ID_ONLY		(0x03U)
/** @} */

/**
 * @name  PUF Configuration 0 register definition
 * @{
 */
/**< PUF_CFG0 register definition */
#define XPUF_CFG0_GLOBAL_FILTER_ENABLE		(0x01U)
#define XPUF_CFG0_HASH_SEL			(0x02U)
/** @} */

/**
 * @name  PUF Configuration 1 register definition
 * @{
 */
/**< PUF_CFG1 register definition */
#define XPUF_CFG1_INIT_VAL_4K			(0x0C230090U)
#define XPUF_CFG1_INIT_VAL_12K			(0x00230150U)
/** @} */

/**
 * @name  PUF Status register definition
 * @{
 */
/**< PUF_STATUS register definition */
#define XPUF_STATUS_SYNDROME_WORD_RDY		((u32)0x01U << 0U)
#define XPUF_STATUS_ID_ZERO			((u32)0x01U << 1U)
#define XPUF_STATUS_ID_RDY			((u32)0x01U << 2U)
#define XPUF_STATUS_KEY_RDY			((u32)0x01U << 3U)
#define XPUF_STATUS_PUF_DONE			((u32)0x01U << 30U)
/** @} */
/**< Clear PUF ID bit in PUF_STATUS register */
#define XPUF_CLEAR_ID				(0x1U)

#define XPUF_EFUSE_CACHE_BASEADDR		(0xF1250000U)
				/**< EFUSE_CACHE Base Address */

/**
 * @name  Offsets for registers in EFUSE_CACHE module
 * @{
 */
/**< EFUSE_CACHE register offsets. */
#define XPUF_PUF_ECC_PUF_CTRL_OFFSET		(0x000000A4U)
#define XPUF_EFUSE_CACHE_SECURITY_CONTROL	(0x000000ACU)

/* EFUSE_CACHE PUF_ECC_PUF_CTRL register definition */
#define XPUF_PUF_REGEN_DIS			((u32)1U << 31U)
#define XPUF_PUF_HD_INVLD			((u32)1U << 30U)

/* EFUSE_CACHE SECURITY_CONTROL register definition */
#define XPUF_PUF_DIS				((u32)1U << 18U)

/** @} */

#define XPUF_EFUSE_CTRL_BASEADDR		(0xF1240000U)
					/**< EFUSE_CTRL Base Address */
#define XPUF_ANLG_OSC_SW_1LP_OFFSET		(0x00000060U)
					/**< IRO Trim Fuse Select */
#define XPUF_IRO_TRIM_FUSE_SEL_BIT		((u32)0x01U << 0U)
					/**< IRO Trim Fuse Select bit */
#define XPUF_EFUSE_CTRL_IRO_TRIM_FAST		(0x1U)
					/**< Trim IRO to fast frequency */
#define XPUF_EFUSE_CTRL_WR_LOCK_OFFSET		(0x0U)
					/**< Write lock offset */
#define XPUF_EFUSE_CTRL_WR_UNLOCK_VAL		(0x0000DF0DU)
					/**< Write unlock value */
#define XPUF_EFUSE_CTRL_WR_LOCK_VAL		(0x1U)
					/**< Write lock value */

/** @} */
/***************** Macros (Inline Functions) Definitions ********************/
/*****************************************************************************/
/**
 *
 * @brief	This function reads the value from the given register.
 *
 * @param	BaseAddress is the base address of the module which consists
 *			the register
 * @param	RegOffset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 * ***************************************************************************/
static inline u32 XPuf_ReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
}

/*****************************************************************************/
/**
 *
 * @brief	This function writes the value into the given register.
 *
 * @param	BaseAddress is the base address of the module which consists
 *			the register
 * @param	RegOffset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 *****************************************************************************/
static inline void XPuf_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), Data);
}

#ifdef __cplusplus
}
#endif

#endif /* XPUF_HW_H */
