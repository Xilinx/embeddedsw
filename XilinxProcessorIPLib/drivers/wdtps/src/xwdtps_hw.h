/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XWDTPS_HW_H
#define XWDTPS_HW_H

/****************************************************************************/
/**
*
* @file xwdtps_hw.h
* @brief Defines the hardware interface to the System Watch Dog Timer (WDT).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 1.02a  sg    07/15/12 Removed defines related to  External Signal
*			Length functionality for CR 658287
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.10  vn     07/30/26 Fix Doxygen warnings
* 3.10  vn     09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup wdtps Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 * Offsets of registers from the start of the device
 * @{
 */

#define XWDTPS_ZMR_OFFSET	0x00000000U /**< Zero Mode Register. */
#define XWDTPS_CCR_OFFSET	0x00000004U /**< Counter Control Register. */
#define XWDTPS_RESTART_OFFSET	0x00000008U /**< Restart Register. */
#define XWDTPS_SR_OFFSET	0x0000000CU /**< Status Register. */
/** @} */


/** @name Zero Mode Register
 * This register controls how the time out is indicated and also contains
 * the access code (0xABC) to allow writes to the register
 * @{
 */
#define XWDTPS_ZMR_WDEN_MASK	0x00000001U /**< Enable the WDT. */
#define XWDTPS_ZMR_RSTEN_MASK	0x00000002U /**< Enable the reset output. */
#define XWDTPS_ZMR_IRQEN_MASK	0x00000004U /**< Enable the IRQ output. */

#define XWDTPS_ZMR_RSTLN_MASK	0x00000070U /**< Set length of reset pulse. */
#define XWDTPS_ZMR_RSTLN_SHIFT	4U	   /**< Shift for reset pulse. */

/** Set length of interrupt pulse. */
#define XWDTPS_ZMR_IRQLN_MASK	0x00000180U
#define XWDTPS_ZMR_IRQLN_SHIFT	7U	   /**< Shift for interrupt pulse. */

#define XWDTPS_ZMR_ZKEY_MASK	0x00FFF000U /**< Mask for writing access key. */
/** Access key, 0xABC << 12. */
#define XWDTPS_ZMR_ZKEY_VAL		0x00ABC000U

/** @} */

/** @name  Counter Control register
 * This register controls how fast the timer runs and the reset value
 * and also contains the access code (0x248) to allow writes to the
 * register
 * @{
 */

#define XWDTPS_CCR_CLKSEL_MASK	0x00000003U /**< Counter clock prescale. */

#define XWDTPS_CCR_CRV_MASK	0x00003FFCU /**< Counter reset value. */
#define XWDTPS_CCR_CRV_SHIFT	2U	   /**< Shift for writing value. */

#define XWDTPS_CCR_CKEY_MASK	0x03FFC000U /**< Mask for writing access key. */
#define XWDTPS_CCR_CKEY_VAL	0x00920000U /**< Access key, 0x248 << 14. */

/* Bit patterns for Clock prescale divider values */

#define XWDTPS_CCR_PSCALE_0008  0x00000000U /**< Divide clock by 8. */
#define XWDTPS_CCR_PSCALE_0064  0x00000001U /**< Divide clock by 64. */
#define XWDTPS_CCR_PSCALE_0512  0x00000002U /**< Divide clock by 512. */
#define XWDTPS_CCR_PSCALE_4096  0x00000003U /**< Divide clock by 4096. */

/** @} */

/** @name  Restart register
 * This register resets the timer preventing a timeout. Value is specific
 * 0x1999
 * @{
 */

#define XWDTPS_RESTART_KEY_VAL	0x00001999U /**< Valid key. */

/** @}*/

/** @name Status register
 * This register indicates timer reached zero count.
 * @{
 */
#define XWDTPS_SR_WDZ_MASK	0x00000001U /**< Time out occurred. */

/** @}*/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Register offset to be read.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XWdtPs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XWdtPs_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (u32)(RegOffset))

/****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Register offset to be written.
* @param[in]	Data 32-bit value to write to the register.
*
* @note		C-style signature:
*		void XWdtPs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
*				     u32 Data)
*
*****************************************************************************/
#define XWdtPs_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

/** @} */
#endif
