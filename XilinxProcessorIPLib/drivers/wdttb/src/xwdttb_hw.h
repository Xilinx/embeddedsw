/******************************************************************************
*
* Copyright (C) 2016 - 2018 Xilinx, Inc. All rights reserved.
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
* @file xwdttb_hw.h
* @addtogroup wdttb_v4_3
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the AXI Timebase and Window Watchdog Timer
* core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xwdttb.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------------
* 1.00 sha 12/17/15 First release.
* 4.0  sha 01/29/16 Updated version to 4.0 as it is newly added file in driver
*                   version 4.0.
*                   Added following macros for Window WDT:
*                   XWT_MWR_OFFSET, XWT_ESR_OFFSET, XWT_FCR_OFFSET,
*                   XWT_FWR_OFFSET, XWT_SWR_OFFSET, XWT_TSR0_OFFSET,
*                   XWT_TSR1_OFFSET, XWT_STR_OFFSET.
*
*                   Added following masks and shifts macros for Window WDT:
*                   XWT_MWR_AEN_MASK, XWT_MWR_MWC_MASK, XWT_ESR_LBE_MASK,
*                   XWT_ESR_FCV_MASK, XWT_ESR_WRP_MASK, XWT_ESR_WINT_MASK,
*                   XWT_ESR_WSW_MASK, XWT_ESR_WCFG_MASK, XWT_ESR_WEN_MASK,
*                   XWT_FCR_SBC_MASK, XWT_FCR_BSS_MASK, XWT_FCR_SSTE_MASK,
*                   XWT_FCR_PSME_MASK, XWT_FCR_FCE_MASK, XWT_FCR_WM_MASK,
*                   XWT_FCR_WDP_MASK.
*
*                   XWT_ESR_LBE_SHIFT, XWT_ESR_FCV_SHIFT, XWT_ESR_WRP_SHIFT,
*                   XWT_ESR_WINT_SHIFT, XWT_ESR_WSW_SHIFT, XWT_ESR_WCFG_SHIFT,
*                   XWT_FCR_SBC_SHIFT, XWT_FCR_BSS_SHIFT, XWT_FCR_SSTE_SHIFT,
*                   XWT_FCR_WM_SHIFT.
* 4.3 srm  01/30/18 Added doxygen tags
* </pre>
*
******************************************************************************/
#ifndef XWDTTB_HW_H_
#define XWDTTB_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register offsets for the AXI Timebase WDT core. Each register is 32
*   bits.
*  @{
*/
#define XWT_TWCSR0_OFFSET	0x00U	/**< Control/Status Register 0
                                          *  Offset */
#define XWT_TWCSR1_OFFSET	0x04U	/**< Control/Status Register 1
                                          *  Offset */
#define XWT_TBR_OFFSET		0x08U	/**< Timebase Register Offset */
/* @} */

/** @name Register offsets for the AXI Timebase WDT core with windowing
*   feature with basic mode. Each register is 32 bits.
*  @{
*/
#define XWT_MWR_OFFSET		0x0CU	/**< Master Write Control Register
                                          *  Offset */
#define XWT_ESR_OFFSET		0x10U	/**< Enable & Status Register Offset */
#define XWT_FCR_OFFSET		0x14U	/**< Function Control Register
                                          *  Offset */
#define XWT_FWR_OFFSET		0x18U	/**< First Window Configuration
                                          *  Register Offset */
#define XWT_SWR_OFFSET		0x1CU	/**< Second Window Configuration
                                          *  Register Offset */
#define XWT_TSR0_OFFSET		0x20U	/**< Task Signature Register 0
                                          *  Offset */
#define XWT_TSR1_OFFSET		0x24U	/**< Task Signature Register 1
                                          *  Offset */
#define XWT_STR_OFFSET		0x28U	/**< Second Sequence Timer Register
                                          *  Offset */
/* @} */

/** @name Control/Status Register 0 bits
*  @{
*/
#define XWT_CSR0_WRS_MASK	0x00000008U	/**< Reset status Mask */
#define XWT_CSR0_WDS_MASK	0x00000004U	/**< Timer state Mask */
#define XWT_CSR0_EWDT1_MASK	0x00000002U	/**< Enable bit 1 Mask */
/* @} */

/** @name Control/Status Register 0/1 bits
*  @{
*/
#define XWT_CSRX_EWDT2_MASK	0x00000001U	/**< Enable bit 2 Mask */
/* @} */

/** @name Master Write Control bits
*  @{
*/
#define XWT_MWR_AEN_MASK	0x00000002U	/**< Always Enable Mask */
#define XWT_MWR_MWC_MASK	0x00000001U	/**< Master Write Control
						   * Mask */
/* @} */

/** @name Enable & Status Register bits
*  @{
*/
#define XWT_ESR_LBE_MASK	0x07000000U	/**< Last Bad Event Mask */
#define XWT_ESR_FCV_MASK	0x00700000U	/**< Fail Counter Value Mask */
#define XWT_ESR_WRP_MASK	0x00020000U	/**< Watchdog Reset Pending
						  *  Mask */
#define XWT_ESR_WINT_MASK	0x00010000U	/**< Watchdog Interrupt Mask */
#define XWT_ESR_WSW_MASK	0x00000100U	/**< Watchdog Second Window
						  *  Mask */
#define XWT_ESR_WCFG_MASK	0x00000002U	/**< Wrong Configuration
						  *  Mask */
#define XWT_ESR_WEN_MASK	0x00000001U	/**< Window WDT Enable Mask */
#define XWT_ESR_LBE_SHIFT	24		/**< Last Bad Event Shift */
#define XWT_ESR_FCV_SHIFT	20		/**< Fail Counter Value
						  *  Shift */
#define XWT_ESR_WRP_SHIFT	17		/**< Watchdog Reset Pending
						  *  Shift */
#define XWT_ESR_WINT_SHIFT	16		/**< Watchdog Interrupt
						  *  Shift */
#define XWT_ESR_WSW_SHIFT	8		/**< Watchdog Second Window
						  *  Shift */
#define XWT_ESR_WCFG_SHIFT	1		/**< Wrong Configuration
						  *  Shift */
/* @} */

/** @name Function Control Register bits
*  @{
*/
#define XWT_FCR_SBC_MASK	0x0000FF00U	/**< Selected Byte Count
						  *  Mask */
#define XWT_FCR_BSS_MASK	0x000000C0U	/**< Byte Segment Selection
						  *  Mask */
#define XWT_FCR_SSTE_MASK	0x00000010U	/**< Second Sequence Timer
						  *  Enable Mask */
#define XWT_FCR_PSME_MASK	0x00000008U	/**< Program Sequence Monitor
						  *  Enable Mask */
#define XWT_FCR_FCE_MASK	0x00000004U	/**< Fail Counter Enable
						  *  Mask */
#define XWT_FCR_WM_MASK		0x00000002U	/**< Window WDT Mode Mask */
#define XWT_FCR_WDP_MASK	0x00000001U	/**< Window WDT Disable
						  *  Protection Mask */
#define XWT_FCR_SBC_SHIFT	8		/**< Selected Byte Count
						  *  Shift */
#define XWT_FCR_BSS_SHIFT	6		/**< Byte Segment Selection
						  *  Shift */
#define XWT_FCR_SSTE_SHIFT	4		/**< Second Sequence Timer
						  *  Enable Shift */
#define XWT_FCR_WM_SHIFT	1		/**< Window WDT Mode Shift */
/* @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XWdtTb_In32		Xil_In32	/**< Input Operations */
#define XWdtTb_Out32		Xil_Out32	/**< Output Operations */

/****************************************************************************/
/**
*
* Read from the specified WdtTb core's register.
*
* @param	BaseAddress contains the base address of the core.
* @param	RegOffset contains the offset from the 1st register of the
*		core to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XWdtTb_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XWdtTb_ReadReg(BaseAddress, RegOffset) \
	XWdtTb_In32((BaseAddress) + ((u32)RegOffset))

/***************************************************************************/
/**
*
* Write to the specified WdtTb core's register.
*
* @param	BaseAddress contains the base address of the core.
* @param	RegOffset contains the offset from the 1st register of the
*		core to select the specific register.
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
	XWdtTb_Out32((BaseAddress) + ((u32)RegOffset), (u32)(RegisterValue))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
