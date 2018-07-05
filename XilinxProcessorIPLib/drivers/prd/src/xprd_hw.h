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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xprd_hw.h
* @addtogroup prd_v1_1
* @{
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the Xilinx PR Decoupler.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date         Changes
* ----- -----  -----------  ----------------------------------------------
* 1.0   ms     07/14/2016    First release
*
* </pre>
*
******************************************************************************/

#ifndef XPRD_HW_H_ /* prevent circular inclusions */
#define XPRD_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offset
 * @{
 */
#define XPRD_CTRL_OFFSET		(0x000)	/**< Control register Offset
						  *  and status register is
						  *  also mappped to same
						  *  address as Control
						  *  register*/
/*@}*/

/** @name Register Mask
 * @{
 */
#define XPRD_CTRL_DECOUPLER_MASK	(0x00000001)	/**< Decoupler Mask */
/*@}*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads a value from the given register.
*
* @param	Address is the address of the register to read from.
*
* @return	The 32-bit value read from register.
*
* @note		None.
*
******************************************************************************/
#define XPrd_ReadReg(Address)	Xil_In32(Address)

/*****************************************************************************/
/**
*
* This macro writes a value to the given register.
*
* @param	Address is the address of the register to write to.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XPrd_WriteReg(Address, Data)	Xil_Out32(Address, (u32)(Data))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
