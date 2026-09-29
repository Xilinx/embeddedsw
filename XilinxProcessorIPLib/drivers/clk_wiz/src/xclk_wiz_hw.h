/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLK_WIZ_HW_H_		/* prevent circular inclusions */
#define XCLK_WIZ_HW_H_

/*****************************************************************************/
/**
 *  @file xclk_wiz_hw.h
 * @brief Defines CLK_WIZ register offsets and masks.
 *
 * Hardware definition file. It defines the register interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver Who Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0 ram 02/12/16 Initial version for Clock Wizard
 * 1.3 sd  4/09/20 Added versal support.
 * 1.4 sd  5/22/20 Added zynqmp set rate.
 * 		   Add Enable and Disable clock APIs.
 * 1.12 ss  8/7/26  Updated clkx5 get/set input and output frequency APIs and
 *         	clk_in_sel register control APIs.
 * 1.12 vn  09/3/26  Fix doxygen warnings.
 * 1.12  tb  09/16/26 Align Doxygen comments with AMD standards.
 * </pre>
 *
 *****************************************************************************/

/**
 * @addtogroup clk_wiz_api CLK_WIZ APIs
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of CLK_WIZ
 *  @{
 */

/** Status register offset. */
#define XCLK_WIZ_STATUS_OFFSET		0x00000004
/** Interrupt status register offset. */
#define XCLK_WIZ_ISR_OFFSET		0x0000000C
/** Interrupt enable register offset. */
#define XCLK_WIZ_IER_OFFSET		0x00000010
/** Reconfiguration register offset. */
#define XCLK_WIZ_RECONFIG_OFFSET	0x00000014
/** Clock input select register offset. */
#define XCLK_WIZ_CLKINSEL_OFFSET	0x00000018
/** Clock configuration register 1 offset. */
#define XCLK_WIZ_REG1_OFFSET		0x00000330
/** Clock configuration register 2 offset. */
#define XCLK_WIZ_REG2_OFFSET		0x00000334
/** Clock configuration register 3 offset. */
#define XCLK_WIZ_REG3_OFFSET		0x00000338
/** Clock configuration register 4 offset. */
#define XCLK_WIZ_REG4_OFFSET		0x0000033C
/** Clock configuration register 12 offset. */
#define XCLK_WIZ_REG12_OFFSET		0x00000380
/** Clock configuration register 13 offset. */
#define XCLK_WIZ_REG13_OFFSET		0x00000384
/** Clock configuration register 11 offset. */
#define XCLK_WIZ_REG11_OFFSET		0x00000378
/** Clock configuration register 14 offset. */
#define XCLK_WIZ_REG14_OFFSET		0x00000398
/** Clock configuration register 15 offset. */
#define XCLK_WIZ_REG15_OFFSET		0x0000039C
/** Clock configuration register 16 offset. */
#define XCLK_WIZ_REG16_OFFSET		0x000003A0
/** Clock configuration register 17 offset. */
#define XCLK_WIZ_REG17_OFFSET		0x000003A8
/** Clock configuration register 19 offset. */
#define XCLK_WIZ_REG19_OFFSET		0x000003CC
/** Clock configuration register 25 offset. */
#define XCLK_WIZ_REG25_OFFSET		0x000003F0
/** Clock configuration register 26 offset. */
#define XCLK_WIZ_REG26_OFFSET		0x000003FC

/** Primary input frequency low register offset. */
#define XCLK_WIZ_PRIM_L_OFFSET		0x000003B0
/** Primary input frequency high register offset. */
#define XCLK_WIZ_PRIM_H_OFFSET		0x000003B4
/** Secondary input frequency low register offset. */
#define XCLK_WIZ_SEC_L_OFFSET		0x000003B8
/** Secondary input frequency high register offset. */
#define XCLK_WIZ_SEC_H_OFFSET		0x000003BC

/** ZynqMP clock configuration register 0 offset. */
#define XCLK_WIZ_ZYNQMP_REG0_OFFSET	0x00000200
/** ZynqMP clock configuration register 2 offset. */
#define XCLK_WIZ_ZYNQMP_REG2_OFFSET	0x00000208

/** Feedback-multiplier field shift. */
#define XCLK_WIZ_REG0_FBMULT_SHIFT	8
/** Feedback-multiplier field width. */
#define XCLK_WIZ_REG0_FBMULT_WIDTH	8
/** Feedback-multiplier field mask. */
#define XCLK_WIZ_REG0_FBMULT_MASK	0xFF00
/** Input-divider field mask. */
#define XCLK_WIZ_REG0_DIV_MASK		0xFF
/** Input-divider field width. */
#define XCLK_WIZ_REG0_DIV_WIDTH		8
/** Output-divider field mask. */
#define XCLK_WIZ_REG2_DIV_MASK		0xFF

/** Register 1 edge-bit shift. */
#define XCLK_WIZ_REG1_EDGE_SHIFT	8
/** Register 1 edge-bit mask. */
#define XCLK_WIZ_REG1_EDGE_MASK		0x100
/** Feedback-divider low-time mask. */
#define XCLK_WIZ_CLKFBOUT_L_MASK	0xFF
/** Feedback-divider high-time mask. */
#define XCLK_WIZ_CLKFBOUT_H_MASK	0xFF00
/** Feedback-divider high-time shift. */
#define XCLK_WIZ_CLKFBOUT_H_SHIFT	8

#define XCLK_WIZ_EDGE_MASK			(1 << 10)	/**< Edge. */
#define XCLK_WIZ_P5EN_MASK			(1 << 8)	/**< p5en. */
#define XCLK_WIZ_LOCK			1	/**< Lock. */
/** Prediv2  3. */
#define XCLK_WIZ_REG3_PREDIV2			(1 << 11)
/** p5en. */
#define XCLK_WIZ_REG3_P5EN_MASK			(1 << 13)
/** Prediv2  3. */
#define XCLK_WIZ_REG3_USED			(1 << 12)
#define XCLK_WIZ_REG3_MX			(1 << 9)    /**< MX. */
/** Register 3 edge-bit mask. */
#define XCLK_WIZ_REG3_EDGE_MASK			(1 << 8)
/** Prediv2  3. */
#define XCLK_WIZ_REG1_PREDIV2			(1 << 12)
/** FBout enable. */
#define XCLK_WIZ_REG1_EN			(1 << 9)
#define XCLK_WIZ_REG1_MX			(1 << 10)    /**< MX  3. */
/** Reconfiguration load value. */
#define XCLK_WIZ_RECONFIG_LOAD		1
/** Reconfiguration start-address value. */
#define XCLK_WIZ_RECONFIG_SADDR		2

/** Bit[0] clk_in_sel_pin (RO). */
#define XCLK_WIZ_CLKINSEL_PIN_MASK		(1U << 0)
/** Bit[16] clk_in_sel_src shift. */
#define XCLK_WIZ_CLKINSEL_SRC_SHIFT	16
/** Bit[16] clk_in_sel_src (R/W). */
#define XCLK_WIZ_CLKINSEL_SRC_MASK		(1U << 16)
/** Bit[17] clk_in_sel_reg shift. */
#define XCLK_WIZ_CLKINSEL_REG_SHIFT	17
/** Bit[17] clk_in_sel_reg (R/W). */
#define XCLK_WIZ_CLKINSEL_REG_MASK		(1U << 17)
/** Bit[18] clk_in_sel_final shift. */
#define XCLK_WIZ_CLKINSEL_FINAL_SHIFT	18
/** Bit[18] clk_in_sel_final (RO). */
#define XCLK_WIZ_CLKINSEL_FINAL_MASK		(1U << 18)

/** Clock input 2 (secondary). */
#define XCLK_WIZ_CLKINSEL_CLKIN2	0U
/** Clock input 1 (primary). */
#define XCLK_WIZ_CLKINSEL_CLKIN1	1U

/** Mask for Charge Pump. */
#define XCLK_WIZ_REG11_CP_MASK			(0xF)
/** Shift bits for Res. */
#define XCLK_WIZ_REG17_RES_SHIFT		1
/** Shift bits for Res. */
#define XCLK_WIZ_REG17_RES_MASK			(0xF)
/** Shift bits for FB DELAY. */
#define XCLK_WIZ_REG15_LOCK_FB_DLY_SHIFT	10
/** Shift bits for Reference DELAY. */
#define XCLK_WIZ_REG16_LOCK_REF_DLY_SHIFT	10
/** Shift bits for DIVCLK_EDGE. */
#define XCLK_WIZ_REG12_EDGE_SHIFT		10
/** Mask for DIVCLK_EDGE. */
#define XCLK_WIZ_REG12_EDGE_MASK		(1 << 10)
/** Shift bits for Prediv2. */
#define XCLK_WIZ_CLKOUT0_PREDIV2_SHIFT	11
#define XCLK_WIZ_CLKOUT0_MX_SHIFT	9	/**< Shift bits for MUX. */
/** Shift bits for P5EN. */
#define XCLK_WIZ_CLKOUT0_P5EN_SHIFT	13
/** Shift bits for P5EDGE. */
#define XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT	15
/** Mask for P5EDGE. */
#define XCLK_WIZ_CLKOUT0_P5FEDGE_MASK		(1 << 15)
#define XCLK_WIZ_REG12_EDGE_SHIFT	10	/**< Shift bits for Edge. */
#define XCLK_WIZ_REG1_EDGE_SHIFT	8	/**< Shift bits for Edge. */
/** @} */

/** @name Bitmasks and offsets of XCLK_WIZ_ISR_OFFSET register
 * This register is used to display interrupt status register
 * @{
 */

/** User clock 3 stopped. */
#define XCLK_WIZ_ISR_CLK3_STOP_MASK	0x00008000
/** User clock 2 stopped. */
#define XCLK_WIZ_ISR_CLK2_STOP_MASK	0x00004000
/** User clock 1 stopped. */
#define XCLK_WIZ_ISR_CLK1_STOP_MASK	0x00002000
/** User clock 0 stopped. */
#define XCLK_WIZ_ISR_CLK0_STOP_MASK	0x00001000
/** User clock 3 has a glitch. */
#define XCLK_WIZ_ISR_CLK3_GLITCH_MASK	0x00000800
/** User clock 2 has a glitch. */
#define XCLK_WIZ_ISR_CLK2_GLITCH_MASK	0x00000400
/** User clock 1 has a glitch. */
#define XCLK_WIZ_ISR_CLK1_GLITCH_MASK	0x00000200
/** User clock 0 has a glitch. */
#define XCLK_WIZ_ISR_CLK0_GLITCH_MASK	0x00000100
/** User clock 3 is below minimum specification. */
#define XCLK_WIZ_ISR_CLK3_MINFREQ_MASK	0x00000080
/** User clock 2 is below minimum specification. */
#define XCLK_WIZ_ISR_CLK2_MINFREQ_MASK	0x00000040
/** User clock 1 is below minimum specification. */
#define XCLK_WIZ_ISR_CLK1_MINFREQ_MASK	0x00000020
/** User clock 0 is below minimum specification. */
#define XCLK_WIZ_ISR_CLK0_MINFREQ_MASK	0x00000010
/** User clock 3 is greater than specification. */
#define XCLK_WIZ_ISR_CLK3_MAXFREQ_MASK	0x00000008
/** User clock 2 is greater than specification. */
#define XCLK_WIZ_ISR_CLK2_MAXFREQ_MASK	0x00000004
/** User clock 1 is greater than specification. */
#define XCLK_WIZ_ISR_CLK1_MAXFREQ_MASK	0x00000002
/** User clock 0 is greater than specification. */
#define XCLK_WIZ_ISR_CLK0_MAXFREQ_MASK	0x00000001

/** User clocks 0-3 stopped. */
#define XCLK_WIZ_ISR_CLKALL_STOP_MASK		0x0000F000
/** User clocks 0-3 have a glitch. */
#define XCLK_WIZ_ISR_CLKALL_GLITCH_MASK		0x00000F00
/** User clocks 0-3 are below minimum specification. */
#define XCLK_WIZ_ISR_CLKALL_MINFREQ_MASK	0x000000F0
/** User clocks 0-3 are greater than specification. */
#define XCLK_WIZ_ISR_CLKALL_MAXFREQ_MASK	0x0000000F

/** Shift bits for User clock 3 stop. */
#define XCLK_WIZ_ISR_CLK3_STOP_SHIFT	15
/** Shift bits for User clock 2 stop. */
#define XCLK_WIZ_ISR_CLK2_STOP_SHIFT	14
/** Shift bits for User clock 1 stop. */
#define XCLK_WIZ_ISR_CLK1_STOP_SHIFT	13
/** Shift bits for User clock 0 stop. */
#define XCLK_WIZ_ISR_CLK0_STOP_SHIFT	12
/** Shift bits for User clock 3 glitch. */
#define XCLK_WIZ_ISR_CLK3_GLITCH_SHIFT	11
/** Shift bits for User clock 2 glitch. */
#define XCLK_WIZ_ISR_CLK2_GLITCH_SHIFT	10
/** Shift bits for User clock 1 glitch. */
#define XCLK_WIZ_ISR_CLK1_GLITCH_SHIFT	9
/** Shift bits for User clock 0 glitch. */
#define XCLK_WIZ_ISR_CLK0_GLITCH_SHIFT	8
/** Shift bits for User clock 3 less. */
#define XCLK_WIZ_ISR_CLK3_MINFREQ_SHIFT	7
/** Shift bits for User clock 2 less. */
#define XCLK_WIZ_ISR_CLK2_MINFREQ_SHIFT	6
/** Shift bits for User clock 1 less. */
#define XCLK_WIZ_ISR_CLK1_MINFREQ_SHIFT	5
/** Shift bits for User clock 0 less. */
#define XCLK_WIZ_ISR_CLK0_MINFREQ_SHIFT	4
/** Shift bits for User clock 3 max. */
#define XCLK_WIZ_ISR_CLK3_MAXFREQ_SHIFT	3
/** Shift bits for User clock 2 max. */
#define XCLK_WIZ_ISR_CLK2_MAXFREQ_SHIFT	2
/** Shift bits for User clock 1 max. */
#define XCLK_WIZ_ISR_CLK1_MAXFREQ_SHIFT	1
/** Shift bits for User clock 0 max. */
#define XCLK_WIZ_ISR_CLK0_MAXFREQ_SHIFT	0
/** @} */

/** @name Bitmasks and offsets of XCLK_WIZ_IER_OFFSET register
 * This register is used to display interrupt status register
 * @{
 */

/** User clock 3 stopped. */
#define XCLK_WIZ_IER_CLK3_STOP_MASK	0x00008000
/** User clock 2 stopped. */
#define XCLK_WIZ_IER_CLK2_STOP_MASK	0x00004000
/** User clock 1 stopped. */
#define XCLK_WIZ_IER_CLK1_STOP_MASK	0x00002000
/** User clock 0 stopped. */
#define XCLK_WIZ_IER_CLK0_STOP_MASK	0x00001000
/** User clock 3 has a glitch. */
#define XCLK_WIZ_IER_CLK3_GLITCH_MASK	0x00000800
/** User clock 2 has a glitch. */
#define XCLK_WIZ_IER_CLK2_GLITCH_MASK	0x00000400
/** User clock 1 has a glitch. */
#define XCLK_WIZ_IER_CLK1_GLITCH_MASK	0x00000200
/** User clock 0 has a glitch. */
#define XCLK_WIZ_IER_CLK0_GLITCH_MASK	0x00000100
/** User clock 3 is below minimum specification. */
#define XCLK_WIZ_IER_CLK3_MINFREQ_MASK	0x00000080
/** User clock 2 is below minimum specification. */
#define XCLK_WIZ_IER_CLK2_MINFREQ_MASK	0x00000040
/** User clock 1 is below minimum specification. */
#define XCLK_WIZ_IER_CLK1_MINFREQ_MASK	0x00000020
/** User clock 0 is below minimum specification. */
#define XCLK_WIZ_IER_CLK0_MINFREQ_MASK	0x00000010
/** User clock 3 is greater than specification. */
#define XCLK_WIZ_IER_CLK3_MAXFREQ_MASK	0x00000008
/** User clock 2 is greater than specification. */
#define XCLK_WIZ_IER_CLK2_MAXFREQ_MASK	0x00000004
/** User clock 1 is greater than specification. */
#define XCLK_WIZ_IER_CLK1_MAXFREQ_MASK	0x00000002
/** User clock 0 is greater than specification. */
#define XCLK_WIZ_IER_CLK0_MAXFREQ_MASK	0x00000001

/** Shift bits for User clock 3 stop. */
#define XCLK_WIZ_IER_CLK3_STOP_SHIFT	15
/** Shift bits for User clock 2 stop. */
#define XCLK_WIZ_IER_CLK2_STOP_SHIFT	14
/** Shift bits for User clock 1 stop. */
#define XCLK_WIZ_IER_CLK1_STOP_SHIFT	13
/** Shift bits for User clock 0 stop. */
#define XCLK_WIZ_IER_CLK0_STOP_SHIFT	12
/** Shift bits for User clock 3 glitch. */
#define XCLK_WIZ_IER_CLK3_GLITCH_SHIFT	11
/** Shift bits for User clock 2 glitch. */
#define XCLK_WIZ_IER_CLK2_GLITCH_SHIFT	10
/** Shift bits for User clock 1 glitch. */
#define XCLK_WIZ_IER_CLK1_GLITCH_SHIFT	9
/** Shift bits for User clock 0 glitch. */
#define XCLK_WIZ_IER_CLK0_GLITCH_SHIFT	8
/** Shift bits for User clock 3 less. */
#define XCLK_WIZ_IER_CLK3_MINFREQ_SHIFT	7
/** Shift bits for User clock 2 less. */
#define XCLK_WIZ_IER_CLK2_MINFREQ_SHIFT	6
/** Shift bits for User clock 1 less. */
#define XCLK_WIZ_IER_CLK1_MINFREQ_SHIFT	5
/** Shift bits for User clock 0 less. */
#define XCLK_WIZ_IER_CLK0_MINFREQ_SHIFT	4
/** Shift bits for User clock 3 max. */
#define XCLK_WIZ_IER_CLK3_MAXFREQ_SHIFT	3
/** Shift bits for User clock 2 max. */
#define XCLK_WIZ_IER_CLK2_MAXFREQ_SHIFT	2
/** Shift bits for User clock 1 max. */
#define XCLK_WIZ_IER_CLK1_MAXFREQ_SHIFT	1
/** Shift bits for User clock 0 max. */
#define XCLK_WIZ_IER_CLK0_MAXFREQ_SHIFT	0
/** @} */

/** All interrupts enable mask. */
#define XCLK_WIZ_IER_ALLINTR_MASK	0x0000FFFF
/** All interrupts enable shift bits. */
#define XCLK_WIZ_IER_ALLINTR_SHIFT	0

/** All interrupt status register mask. */
#define XCLK_WIZ_ISR_ALLINTR_MASK	0x0000FFFF
/** All interrupts status register shift. */
#define XCLK_WIZ_ISR_ALLINTR_SHIFT	0
/** Maximum output-clock identifier. */
#define XCLK_WIZ_MAX_OUTPUT		7
/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
* @brief Reads a 32-bit CLK_WIZ register.
*
* Macro to read register.
*
* @param[in] BaseAddress Base address of CLK_WIZ.
* @param[in] RegOffset Register offset.
*
* @return	Value of the register.
*
******************************************************************************/
static inline u32 XClk_Wiz_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32((BaseAddress) + (u32)(RegOffset)));
}

/*****************************************************************************/
/**
* @brief Writes a 32-bit CLK_WIZ register.
*
* Macro to write to register.
*
* @param[in] BaseAddress Base address of CLK_WIZ.
* @param[in] RegOffset Register offset.
* @param[in] Data Value to be written to the register.
*
******************************************************************************/
static inline void XClk_Wiz_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
				     u32 Data)
{
	Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data));
}

#ifdef __cplusplus
}
#endif

#endif
/** @} */
