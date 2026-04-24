/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_hw.h
*
* This file contains PUF hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kpt  08/21/2024 Initial release
* 2.6   aa   07/14/2025 Removed unused macros
* 2.7   bha  01/06/2026 Fixed Doxygen warnings
*       mb   04/23/2026 Remove unused macros
*
* </pre>
*
******************************************************************************/
#ifndef XPUF_HW_H
#define XPUF_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
/*************************** Constant Definitions *****************************/

#define XPUF_PMC_GLOBAL_BASEADDR	(0x040A0000U)	/**< PMC global Base Address */

#define XPUF_BASEADDR			(0x04080000U)	/**< PUF Base Address */

/**
 * @name  Offsets for PUF registers in PUF module
 * @{
 */
#define XPUF_PUF_CMD_OFFSET		(0x00000000U)	/**< PUF Command register offset */
#define XPUF_PUF_CFG0_OFFSET		(0x00000004U)	/**< PUF Configuration 0 register offset */
#define XPUF_PUF_CFG1_OFFSET		(0x00000008U)	/**< PUF Configuration 1 register offset */
#define XPUF_PUF_SHUT_OFFSET		(0x0000000CU)	/**< PUF Shutter register offset */
#define XPUF_PUF_STATUS_OFFSET		(0x00000024U)	/**< PUF Status register offset */
#define XPUF_PUF_WORD_OFFSET		(0x00000040U)	/**< PUF Word register offset */
#define XPUF_PUF_DBG_OFFSET		(0x00000068U)	/**< PUF Debug register offset */
#define XPUF_PUF_CHASH_OFFSET		(0x000003C0U)	/**< PUF CHASH register offset */
/** @} */

/**
 * @name  Offsets for PUF registers in PMC_GLOBAL module
 * @{
 */
#define XPUF_PMC_GLOBAL_PUF_RST_OFFSET			(0x00000064U)
			/**< PUF Reset register offset */
#define XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET		(0x0001F310U)
			/**< PUF Status register offset */
#define XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET		(0x0001F32CU)
			/**< PUF Clear register offset */
#define XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET			(0x0001F330U)
			/**< PUF ID 0 register offset */
#define XPUF_PMC_GLOBAL_PUF_RO_SWAP_OFFSET		(0x0001F35CU)
			/**< PUF RO Swap register offset */
#define XPUF_PMC_GLOBAL_PMC_PUF_CAPTURE_OFFSET		(0x0001F354U)
			/**< PUF Capture register offset */
/** @} */

/**
 * @name  PUF Command register definition
 * @{
 */
#define XPUF_CMD_PAUSE			(0x0U)	/**< PUF Pause command */
#define XPUF_CMD_REGISTRATION		(0x01U)	/**< PUF Registration command */
#define XPUF_CMD_REGEN_ON_DEMAND	(0x02U)	/**< PUF Regenerate on Demand command */
#define XPUF_CMD_STOP			(0x03U)	/**< PUF Stop command */
/** @} */

/**
 * @name  PUF Configuration 0 register definition
 * @{
 */
#define XPUF_CFG0_GLOBAL_FILTER_ENABLE		(0x01U)	/**< PUF Global Filter Enable */
#define XPUF_CFG0_HASH_SEL			(0x02U)	/**< PUF Hash Select */
/** @} */

/**
 * @name  PUF Configuration 1 register definition
 * @{
 */
#define XPUF_CFG1_INIT_VAL_4K			(0x0C230090U)
			/**< PUF Configuration 1 register value for 4K Registration mode */
/** @} */

/**
 * @name  PUF Status register definition
 * @{
 */
#define XPUF_STATUS_SYNDROME_WORD_RDY		((u32)0x01U << 0U) /**< Syndrome Word Ready */
#define XPUF_STATUS_ID_ZERO			((u32)0x01U << 1U) /**< ID Zero */
#define XPUF_STATUS_KEY_RDY			((u32)0x01U << 3U) /**< Key Ready */
#define XPUF_STATUS_ZZ_MASK			((u32)0x01U << 1U) /**< ZZ Mask */
#define XPUF_STATUS_SEG_RDY			((u32)0x01U << 2U) /**< Segment Ready */
/** @} */

#define XPUF_CLEAR_ID			(0x1U)	/**< Clear PUF ID bit in PUF_STATUS register */

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
