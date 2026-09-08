/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XAXIS_SWITCH_HW_H_
#define XAXIS_SWITCH_HW_H_

/*****************************************************************************/
/**
*
* @file xaxis_switch_hw.h
* @addtogroup axis_switch Overview
* @{
*
* @brief Hardware definition file for the AXI4-Stream Switch register interface.
*
* Contains identifiers and register-level core functions (or macros)
* to access the Xilinx AXI4-Stream Switch Control Router core.
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
* 1.9   vn  07/30/26 Fix Doxygen warnings
* 1.9  vn   09/08/26 Align Doxygen comments with AMD standards
* </pre>
*
*****************************************************************************/

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

/** @} */

/** @name MI MUX Control register mask
* @{
*/
#define XAXIS_SCR_CTRL_REG_UPDATE_MASK	0x02	/**< Register Update
						  *  mask */
/** @} */

/** @brief Defines MI MUX register masks.
* @name MI MUX register mask
*
* Applicable for MI[0...15] registers.
* @{
*/
#define XAXIS_SCR_MI_X_MUX_MASK		0x0F		/**< MI MUX mask */
#define XAXIS_SCR_MI_X_DISABLE_MASK	0x80000000	/**< MI Disable mask */
#define XAXIS_SCR_MI_X_DISABLE_SHIFT	31	/**< MI Disable shift */
/** @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @brief Defines register access macros.
* @name Register access macro definition
* @{
*/
#define XAxisScr_In32		Xil_In32	/**< Input Operations */
#define XAxisScr_Out32		Xil_Out32	/**< Output Operations */

/*****************************************************************************/
/**
* @brief Reads a value from an AXI4-Stream Switch register.
*
* Performs a 32-bit read. If the component is implemented in a smaller
* width, only the least significant data is read from the register.
* The most significant data reads as 0.
*
* @param[in]	BaseAddress is the base address of the XAxis_Switch core
*		instance.
* @param[in]	RegOffset is the register offset of the register (defined at
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
* @brief Writes a value to an AXI4-Stream Switch register.
*
* Performs a 32-bit write. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param[in]	BaseAddress is the base address of the XAxis_Switch core
*		instance.
* @param[in]	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param[in]	Data is the 32-bit value to write into the register.
*
* @note		C-style signature:
*		void XAxisScr_WriteReg(u32 BaseAddress, u32 RegOffset,
*		u32 Data)
*
******************************************************************************/
#define XAxisScr_WriteReg(BaseAddress, RegOffset, Data) \
	XAxisScr_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/** @} */

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
