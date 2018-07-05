/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
* @file xaxis_switch_hw.h
* @addtogroup axis_switch_v1_2
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx AXI4-Stream Switch Control
* Router core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver
* xaxis_switch.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XAXIS_SWITCH_HW_H_
#define XAXIS_SWITCH_HW_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name MI MUX register offsets
*
* @{
*/
#define XAXIS_SCR_CTRL_OFFSET		0x000	/**< Control Register offset */
#define XAXIS_SCR_MI_MUX_START_OFFSET	0x040	/**< Start of MI MUX Register
						  *  offset */
#define XAXIS_SCR_MI_MUX_END_OFFSET	0x07C	/**< End of MI MUX Register
						  *  offset */

/*@}*/

/** @name MI MUX Control register mask
* @{
*/
#define XAXIS_SCR_CTRL_REG_UPDATE_MASK	0x02	/**< Register Update
						  *  mask */
/*@}*/

/** @name MI MUX register mask
*
* It is applicable for MI[0...15] registers.
* @{
*/
#define XAXIS_SCR_MI_X_MUX_MASK		0x0F		/**< MI MUX mask */
#define XAXIS_SCR_MI_X_DISABLE_MASK	0x80000000	/**< MI Disable mask */
#define XAXIS_SCR_MI_X_DISABLE_SHIFT	31	/**< MI Disable shift */
/*@}*/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XAxisScr_In32		Xil_In32	/**< Input Operations */
#define XAxisScr_Out32		Xil_Out32	/**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a AXI4-Stream Switch register. A 32 bit read
* is performed. If the component is implemented in a smaller width, only the
* least significant data is read from the register. The most significant data
* will be read as 0.
*
* @param	BaseAddress is the base address of the XAxis_Switch core
*		instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XAxisScr_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XAxisScr_ReadReg(BaseAddress, RegOffset) \
	XAxisScr_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a AXI4-Stream Switch register. A 32 bit write
* is performed. If the component is implemented in a smaller width, only the
* least significant data is written.
*
* @param	BaseAddress is the base address of the XAxis_Switch core
*		instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XAxisScr_WriteReg(u32 BaseAddress, u32 RegOffset,
*		u32 Data)
*
******************************************************************************/
#define XAxisScr_WriteReg(BaseAddress, RegOffset, Data) \
	XAxisScr_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
