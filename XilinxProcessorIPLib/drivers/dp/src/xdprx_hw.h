/*******************************************************************************
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdprx_hw.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xdprx.h.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * </pre>
 *
*******************************************************************************/

#ifndef XDPRX_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPRX_HW_H_

/***************************** Include Files **********************************/

#include "xil_io.h"

/************************** Constant Definitions ******************************/

/** @name DPRX core registers: Receiver core configuration.
  * @{
  */
#define XDPRX_LINK_ENABLE		0x000	/**< Enable the receiver
							core. */
#define XDPRX_AUX_CLK_DIVIDER		0x004	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDPRX_DTG_ENABLE		0x00C	/**< Enables the display timing
							generator (DTG). */
#define XDPRX_USER_PIXEL_WIDTH		0x010	/**< Selects the width of the
							user data input port. */
#define XDPRX_INTERRUPT_MASK		0x014	/**< Masks the specified
							interrupt sources. */
#define XDPRX_MISC_CTRL			0x018	/**< Miscellaneous control of
							RX behavior. */
#define XDPRX_SOFT_RESET		0x01C	/**< Software reset. */
/* @} */

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XDprx_In32 Xil_In32
#define XDprx_Out32 Xil_Out32
/* @} */

/******************************************************************************/
/**
 * This is a low-level function that reads from the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to be read from.
 *
 * @return	The 32-bit value of the specified register.
 *
 * @note	C-style signature:
 *		u32 XDprx_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XDprx_ReadReg(BaseAddress, RegOffset) \
					XDprx_In32((BaseAddress) + (RegOffset))

/******************************************************************************/
/**
 * This is a low-level function that writes to the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to write to.
 * @param	Data is the 32-bit data to write to the specified register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 *		void XDprx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XDprx_WriteReg(BaseAddress, RegOffset, Data) \
				XDprx_Out32((BaseAddress) + (RegOffset), (Data))

#endif /* XDPRX_HW_H_ */
