/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 *  @file xclk_wiz_hw.h
 * @addtogroup clk_wiz_v1_0
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
