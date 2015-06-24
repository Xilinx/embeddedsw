/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
*
* @file xwdttb_l.h
*
* This header file contains identifiers and basic driver functions (or
* macros) that can be used to access the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm  04/26/02 First release
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  22/10/09 The following macros defined in this file have been
*		      removed -
*		      XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt, XWdtTb_mRestartWdt
*		      XWdtTb_mGetTimebaseReg and XWdtTb_mHasReset.
*		      Added the XWdtTb_ReadReg and XWdtTb_WriteReg
*		      macros. User should XWdtTb_ReadReg/XWdtTb_WriteReg to
*		      acheive the desired functioanality of the macros that
*		      were removed.
* </pre>
*
******************************************************************************/

#ifndef XWDTTB_L_H		/* prevent circular inclusions */
#define XWDTTB_L_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register offsets for the Wdt device. Each register is 32 bits.
 *  @{
 */
#define XWT_TWCSR0_OFFSET	0x0 /**< Control/Status Register 0 Offset */
#define XWT_TWCSR1_OFFSET	0x4 /**< Control/Status Register 1 Offset */
#define XWT_TBR_OFFSET		0x8 /**< Timebase Register Offset */
/* @} */


/** @name Control/Status Register 0 bits
 *  @{
 */
#define XWT_CSR0_WRS_MASK	0x00000008 /**< Reset status Mask */
#define XWT_CSR0_WDS_MASK	0x00000004 /**< Timer state Mask */
#define XWT_CSR0_EWDT1_MASK	0x00000002 /**< Enable bit 1 Mask*/
/* @} */

/** @name Control/Status Register 0/1 bits
 *  @{
 */
#define XWT_CSRX_EWDT2_MASK	0x00000001 /**< Enable bit 2 Mask */
/* @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#define XWdtTb_In32 Xil_In32
#define XWdtTb_Out32 Xil_Out32

/****************************************************************************/
/**
*
* Read from the specified WdtTb device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XWdtTb_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XWdtTb_ReadReg(BaseAddress, RegOffset) \
	XWdtTb_In32((BaseAddress) + (RegOffset))


/***************************************************************************/
/**
*
* Write to the specified WdtTb device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XWdtTb_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
*
******************************************************************************/
#define XWdtTb_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XWdtTb_Out32((BaseAddress) + (RegOffset), (RegisterValue))


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif
