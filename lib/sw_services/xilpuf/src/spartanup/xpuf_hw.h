/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define XPUF_PMC_GLOBAL_BASEADDR	(0x040A0000U)

#define XPUF_BASEADDR			(0x04080000U)
					/**< PMC_GLOBAL Base Address */

/**
 * @name  Offsets for PUF registers in PMC_GLOBAL module
 * @{
 */
/**< PUF register offsets */
#define XPUF_PUF_CMD_OFFSET			(0x00000000U)
#define XPUF_PUF_CFG0_OFFSET		(0x00000004U)
#define XPUF_PUF_CFG1_OFFSET		(0x00000008U)
#define XPUF_PUF_SHUT_OFFSET		(0x0000000CU)
#define XPUF_PUF_STATUS_OFFSET		(0x00000024U)
#define XPUF_PUF_WORD_OFFSET		(0x00000040U)
#define XPUF_PUF_CHASH_OFFSET		(0x000003C0U)
#define XPUF_PUF_DBG_OFFSET			(0x00000068U)

#define XPUF_PMC_GLOBAL_PUF_RST_OFFSET			(0x00000064U)
#define XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET		(0x0001F310U)
#define XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET		(0x0001F32CU)
#define XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET			(0x0001F330U)
#define XPUF_PMC_GLOBAL_PUF_RO_SWAP_OFFSET		(0x0001F35CU)
#define XPUF_PMC_GLOBAL_PMC_PUF_CAPTURE_OFFSET	(0x0001F354U)

/** @} */

/**
 * @name  PUF Command register definition
 * @{
 */
/**< PUF_CMD register definition */
#define XPUF_CMD_PAUSE				(0x0U)
#define XPUF_CMD_REGISTRATION		(0x01U)
#define XPUF_CMD_REGEN_ON_DEMAND	(0x02U)
#define XPUF_CMD_STOP				(0x03U)
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
#define XPUF_STATUS_ZZ_MASK			((u32)0x01U << 1U)
#define XPUF_STATUS_PUF_DONE		((u32)0x01U << 30U)
#define XPUF_STATUS_SEG_RDY			((u32)0x01U << 2U)
/** @} */
/**< Clear PUF ID bit in PUF_STATUS register */
#define XPUF_CLEAR_ID				(0x1U)

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
