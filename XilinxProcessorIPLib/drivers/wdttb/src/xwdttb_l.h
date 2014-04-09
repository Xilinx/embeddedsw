/* $Id: xwdttb_l.h,v 1.1.2.1 2009/12/04 05:52:37 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2002-2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
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
