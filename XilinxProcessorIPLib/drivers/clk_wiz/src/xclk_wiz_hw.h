/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *  @file xclk_wiz_hw.h
 * @addtogroup clk_wiz_v1_3
 * @{
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
 * </pre>
 *
 *****************************************************************************/

#ifndef XCLK_WIZ_HW_H_	/* prevent circular inclusions */
#define XCLK_WIZ_HW_H_

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

#define XCLK_WIZ_ISR_OFFSET	0x0000000C  /**< Interrupt Status Register */
#define XCLK_WIZ_IER_OFFSET	0x00000010  /**< Interrupt Enable Register */
#define XCLK_WIZ_REG1_OFFSET	0x00000330
#define XCLK_WIZ_REG2_OFFSET	0x00000334
#define XCLK_WIZ_REG3_OFFSET	0x00000338
#define XCLK_WIZ_REG4_OFFSET	0x0000033C
#define XCLK_WIZ_REG12_OFFSET	0x00000380
#define XCLK_WIZ_REG13_OFFSET	0x00000384
#define XCLK_WIZ_REG11_OFFSET	0x00000378
#define XCLK_WIZ_REG14_OFFSET	0x00000398
#define XCLK_WIZ_REG15_OFFSET	0x0000039C
#define XCLK_WIZ_REG16_OFFSET	0x000003A0
#define XCLK_WIZ_REG17_OFFSET	0x000003A8
#define XCLK_WIZ_REG25_OFFSET	0x000003F0
#define XCLK_WIZ_REG26_OFFSET	0x000003FC

#define XCLK_WIZ_REG3_PREDIV2			(1 << 11)    /**< Prediv2  3*/
#define XCLK_WIZ_REG3_USED			(1 << 12)    /**< Prediv2  3*/
#define XCLK_WIZ_REG3_MX			(1 << 9)    /**< MX*/
#define XCLK_WIZ_REG1_PREDIV2			(1 << 12)    /**< Prediv2  3*/
#define XCLK_WIZ_REG1_EN			(1 << 9)    /**< FBout enable*/
#define XCLK_WIZ_REG1_MX			(1 << 10)    /**< MX  3*/

#define XCLK_WIZ_CLKOUT0_PREDIV2_SHIFT		11   /**< Shift bits for Prediv2 */
#define XCLK_WIZ_CLKOUT0_MX_SHIFT		9   /**< Shift bits for MUX */
#define XCLK_WIZ_CLKOUT0_P5EN_SHIFT		13   /**< Shift bits for P5EN */
#define XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT		15  /**< Shift bits for P5EDGE */
#define XCLK_WIZ_REG12_EDGE_SHIFT		10  /**< Shift bits for Edge */
#define XCLK_WIZ_REG1_EDGE_SHIFT		8  /**< Shift bits for Edge */
/*@}*/

/** @name Bitmasks and offsets of XCLK_WIZ_ISR_OFFSET register
 * This register is used to display interrupt status register
 * @{
 */

#define XCLK_WIZ_ISR_CLK3_STOP_MASK	0x00008000    /**< User clock 3
							stopped */
#define XCLK_WIZ_ISR_CLK2_STOP_MASK	0x00004000    /**< User clock 2
							stopped */
#define XCLK_WIZ_ISR_CLK1_STOP_MASK	0x00002000    /**< User clock 1
							stopped */
#define XCLK_WIZ_ISR_CLK0_STOP_MASK	0x00001000    /**< User clock 0
							stopped */
#define XCLK_WIZ_ISR_CLK3_GLITCH_MASK	0x00000800    /**< User clock 3
							has glitch */
#define XCLK_WIZ_ISR_CLK2_GLITCH_MASK	0x00000400    /**< User clock 2
							has glitch */
#define XCLK_WIZ_ISR_CLK1_GLITCH_MASK	0x00000200    /**< User clock 1
							has glitch */
#define XCLK_WIZ_ISR_CLK0_GLITCH_MASK	0x00000100    /**< User clock 0
							has glitch */
#define XCLK_WIZ_ISR_CLK3_MINFREQ_MASK	0x00000080    /**< User clock 3 is less
							than specification */
#define XCLK_WIZ_ISR_CLK2_MINFREQ_MASK	0x00000040    /**< User clock 2 is less
							than specification */
#define XCLK_WIZ_ISR_CLK1_MINFREQ_MASK	0x00000020    /**< User clock 1 is less
							than specification */
#define XCLK_WIZ_ISR_CLK0_MINFREQ_MASK	0x00000010    /**< User clock 0 is less
							than specification */
#define XCLK_WIZ_ISR_CLK3_MAXFREQ_MASK	0x00000008    /**< User clock 3 is max
							than specification */
#define XCLK_WIZ_ISR_CLK2_MAXFREQ_MASK	0x00000004    /**< User clock 2 is max
							than specification */
#define XCLK_WIZ_ISR_CLK1_MAXFREQ_MASK	0x00000002    /**< User clock 1 is max
							than specification */
#define XCLK_WIZ_ISR_CLK0_MAXFREQ_MASK	0x00000001    /**< User clock 0 is max
							than specification */

#define XCLK_WIZ_ISR_CLKALL_STOP_MASK	0x0000F000    /**< User clock[0-3]
							has stopped*/
#define XCLK_WIZ_ISR_CLKALL_GLITCH_MASK	0x00000F00    /**< User clock[0-3]
							has glitch */
#define XCLK_WIZ_ISR_CLKALL_MINFREQ_MASK 0x000000F0    /**< User clock[0-3]
							is min than
							specification */
#define XCLK_WIZ_ISR_CLKALL_MAXFREQ_MASK 0x0000000F    /**< User clock[0-3]
							is max than
							specification */

#define XCLK_WIZ_ISR_CLK3_STOP_SHIFT              15   /**< Shift bits for
							User clock 3 stop*/
#define XCLK_WIZ_ISR_CLK2_STOP_SHIFT              14   /**< Shift bits for
							User clock 2 stop*/
#define XCLK_WIZ_ISR_CLK1_STOP_SHIFT              13   /**< Shift bits for
							User clock 1 stop*/
#define XCLK_WIZ_ISR_CLK0_STOP_SHIFT              12   /**< Shift bits for
							User clock 0 stop*/
#define XCLK_WIZ_ISR_CLK3_GLITCH_SHIFT            11   /**< Shift bits for
							User clock 3 glitch */
#define XCLK_WIZ_ISR_CLK2_GLITCH_SHIFT            10   /**< Shift bits for
							User clock 2 glitch */
#define XCLK_WIZ_ISR_CLK1_GLITCH_SHIFT             9   /**< Shift bits for
							User clock 1 glitch */
#define XCLK_WIZ_ISR_CLK0_GLITCH_SHIFT             8   /**< Shift bits for
							User clock 0 glitch */
#define XCLK_WIZ_ISR_CLK3_MINFREQ_SHIFT            7   /**< Shift bits for
							User clock 3 less */
#define XCLK_WIZ_ISR_CLK2_MINFREQ_SHIFT            6   /**< Shift bits for
							User clock 2 less */
#define XCLK_WIZ_ISR_CLK1_MINFREQ_SHIFT            5   /**< Shift bits for
							User clock 1 less */
#define XCLK_WIZ_ISR_CLK0_MINFREQ_SHIFT            4   /**< Shift bits for
							User clock 0 less */
#define XCLK_WIZ_ISR_CLK3_MAXFREQ_SHIFT            3   /**< Shift bits for
							User clock 3 max */
#define XCLK_WIZ_ISR_CLK2_MAXFREQ_SHIFT            2   /**< Shift bits for
							User clock 2 max */
#define XCLK_WIZ_ISR_CLK1_MAXFREQ_SHIFT            1   /**< Shift bits for
							User clock 1 max */
#define XCLK_WIZ_ISR_CLK0_MAXFREQ_SHIFT            0   /**< Shift bits for
							User clock 0 max */
/*@}*/

/** @name Bitmasks and offsets of XCLK_WIZ_IER_OFFSET register
 * This register is used to display interrupt status register
 * @{
 */

#define XCLK_WIZ_IER_CLK3_STOP_MASK	0x00008000    /**< User clock 3
							stopped */
#define XCLK_WIZ_IER_CLK2_STOP_MASK	0x00004000    /**< User clock 2
							stopped */
#define XCLK_WIZ_IER_CLK1_STOP_MASK	0x00002000    /**< User clock 1
							stopped */
#define XCLK_WIZ_IER_CLK0_STOP_MASK	0x00001000    /**< User clock 0
							stopped */
#define XCLK_WIZ_IER_CLK3_GLITCH_MASK	0x00000800    /**< User clock 3
							has glitch */
#define XCLK_WIZ_IER_CLK2_GLITCH_MASK	0x00000400    /**< User clock 2
							has glitch */
#define XCLK_WIZ_IER_CLK1_GLITCH_MASK	0x00000200    /**< User clock 1
							has glitch */
#define XCLK_WIZ_IER_CLK0_GLITCH_MASK	0x00000100    /**< User clock 0
							has glitch */
#define XCLK_WIZ_IER_CLK3_MINFREQ_MASK	0x00000080    /**< User clock 3 is less
							than specification */
#define XCLK_WIZ_IER_CLK2_MINFREQ_MASK	0x00000040    /**< User clock 2 is less
							than specification */
#define XCLK_WIZ_IER_CLK1_MINFREQ_MASK	0x00000020    /**< User clock 1 is less
							than specification */
#define XCLK_WIZ_IER_CLK0_MINFREQ_MASK	0x00000010    /**< User clock 0 is less
							than specification */
#define XCLK_WIZ_IER_CLK3_MAXFREQ_MASK	0x00000008    /**< User clock 3 is max
							than specification */
#define XCLK_WIZ_IER_CLK2_MAXFREQ_MASK	0x00000004    /**< User clock 2 is max
							than specification */
#define XCLK_WIZ_IER_CLK1_MAXFREQ_MASK	0x00000002    /**< User clock 1 is max
							than specification */
#define XCLK_WIZ_IER_CLK0_MAXFREQ_MASK	0x00000001    /**< User clock 0 is max
							than specification */

#define XCLK_WIZ_IER_CLK3_STOP_SHIFT              15   /**< Shift bits for
							User clock 3 stop*/
#define XCLK_WIZ_IER_CLK2_STOP_SHIFT              14   /**< Shift bits for
							User clock 2 stop*/
#define XCLK_WIZ_IER_CLK1_STOP_SHIFT              13   /**< Shift bits for
							User clock 1 stop*/
#define XCLK_WIZ_IER_CLK0_STOP_SHIFT              12   /**< Shift bits for
							User clock 0 stop*/
#define XCLK_WIZ_IER_CLK3_GLITCH_SHIFT            11   /**< Shift bits for
							User clock 3 glitch */
#define XCLK_WIZ_IER_CLK2_GLITCH_SHIFT            10   /**< Shift bits for
							User clock 2 glitch */
#define XCLK_WIZ_IER_CLK1_GLITCH_SHIFT             9   /**< Shift bits for
							User clock 1 glitch */
#define XCLK_WIZ_IER_CLK0_GLITCH_SHIFT             8   /**< Shift bits for
							User clock 0 glitch */
#define XCLK_WIZ_IER_CLK3_MINFREQ_SHIFT            7   /**< Shift bits for
							User clock 3 less */
#define XCLK_WIZ_IER_CLK2_MINFREQ_SHIFT            6   /**< Shift bits for
							User clock 2 less */
#define XCLK_WIZ_IER_CLK1_MINFREQ_SHIFT            5   /**< Shift bits for
							User clock 1 less */
#define XCLK_WIZ_IER_CLK0_MINFREQ_SHIFT            4   /**< Shift bits for
							User clock 0 less */
#define XCLK_WIZ_IER_CLK3_MAXFREQ_SHIFT            3   /**< Shift bits for
							User clock 3 max */
#define XCLK_WIZ_IER_CLK2_MAXFREQ_SHIFT            2   /**< Shift bits for
							User clock 2 max */
#define XCLK_WIZ_IER_CLK1_MAXFREQ_SHIFT            1   /**< Shift bits for
							User clock 1 max */
#define XCLK_WIZ_IER_CLK0_MAXFREQ_SHIFT            0   /**< Shift bits for
							User clock 0 max */
/*@}*/

#define XCLK_WIZ_IER_ALLINTR_MASK	0x0000FFFF /**< All interrupts enable
							mask */
#define XCLK_WIZ_IER_ALLINTR_SHIFT	         0 /**< All interrupts enable
							shift bits */

#define XCLK_WIZ_ISR_ALLINTR_MASK	0x0000FFFF /**< All interrupt status
							register mask */
#define XCLK_WIZ_ISR_ALLINTR_SHIFT	         0 /**< All interrupts status
							register shift */

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
* Macro to read register.
*
* @param	BaseAddress is the base address of CLK_WIZ
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		C-style signature:
* 		u32 XClk_Wiz_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
static inline u32 XClk_Wiz_ReadReg(UINTPTR BaseAddress, u32 RegOffset)  {
			return (Xil_In32((BaseAddress) + (u32)(RegOffset)));
}

/*****************************************************************************/
/**
* Macro to write to register.
*
* @param	BaseAddress is the base address of CLK_WIZ
* @param	RegOffset is the register offset.
* @param	Data is the value to be written to the register.
*
* @return	None.
*
* @note		C-style signature:
* 		void XClk_Wiz_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
* 								u32 Data)
*
******************************************************************************/
static inline void XClk_Wiz_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
								u32 Data) {
		Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data));
}

#ifdef __cplusplus
}
#endif

#endif
/** @} */
