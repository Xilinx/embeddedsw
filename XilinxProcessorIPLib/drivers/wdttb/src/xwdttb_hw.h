/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_hw.h
* @addtogroup wdttb Overview
* @{
*
* The xwdttb_hw.h header file contains identifiers and register-level core
* functions (or macros) that can be used to access the AXI Timebase and
* Window Watchdog Timer core.
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
* 4.4 sne  03/04/19 Added Support for Versal (Generic Watchdog and Window
*                   Watchdog Timer).
*                   Added following macros for Generic WDT:
*                   XWT_TFR_OFFSET,XWT_TRR_OFFSET,XWT_IENR_OFFSET,
*                   XWT_IDR_OFFSET,XWT_IMR_OFFSET,XWT_GWRR_OFFSET,
*                   XWT_GWCSR_OFFSET,XWT_GWOR_OFFSET,XWT_GWCVR0_OFFSET,
*                   XWT_GWCVR1_OFFSET,XWT_GW_WR_OFFSET,XWT_GWCSR_GWEN_MASK,
*                   XWT_GWCSR_GWS1_MASK,XWT_GWCSR_GWS2_MASK,XWT_GWCSR_MASK,
*                   XWT_GW_WR_MASK,XWT_GWRR_MASK.
*                   Added following macro for Win WDT:
*                   XWT_SSTWR_OFFSET.
* 4.5 sne  09/27/19 Added common driver support for WWDT and AXI Timebase WDT.
*		    Removed versal flag. Removed AXI Timebase WDT offsets.
*		    Added following macros for WWDT
*		    XWT_TSR0_WWDT_OFFSET,XWT_TSR1_WWDT_OFFSET,
*		    XWT_STR_WWDT_OFFSET.
* 5.0 sne  03/11/20 Added XWT_GWDT and XWT_WWDT macros.
*
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
/** @name Register offsets for the WWDT core with Generic & windowing
*   feature with basic mode. Each register is 32 bits.
*  @{
*/

#define XWT_MWR_OFFSET          0x0000U    /**< Master Write Control Register
						Offset */
#define XWT_ESR_OFFSET          0x0004U    /**< Enable & Status Register
						Offset */
#define XWT_FCR_OFFSET          0x0008U    /**< Function Control Register
						Register Offset */
#define XWT_FWR_OFFSET          0x000CU    /**< First Window Configuration
						Register Offset */
#define XWT_SWR_OFFSET          0x0010U    /**< Second Window Configuration
						Register Offset */
#define XWT_TSR0_WWDT_OFFSET	0x0018U    /**< Task Signature Register 0
						Offset for WWDT */
#define XWT_TSR1_WWDT_OFFSET	0x001CU    /**< Task Signature Register 1
						Offset for WWDT */
#define XWT_STR_WWDT_OFFSET	0x0020U    /**< Second Sequence Timer Register
						Offset for WWDT */
#define XWT_TSR0_OFFSET         0x0014U    /**< Task Signature Register 0
						Offset */
#define XWT_TSR1_OFFSET         0x0018U    /**< Task Signature Register 1
						Offset */
#define XWT_STR_OFFSET          0x001CU    /**< Second Sequence Timer Register
						Offset */
#define XWT_SSTWR_OFFSET        0x0014U    /**< Second Sequence Timer Window
						Configuration Register Offset
						 */
#define XWT_TFR_OFFSET          0x0024U    /**< Token Feedback Register
						Offset */
#define XWT_TRR_OFFSET          0x0028U    /**< Token Response Register
						offset */
#define XWT_IENR_OFFSET         0x0030U    /**< Interrupt Enable Register
						Offset */
#define XWT_IDR_OFFSET          0x0034U    /**< Interrupt Disable Register
						Offset */
#define XWT_IMR_OFFSET          0x0038U    /**< Interrupt Mask Register
						Offset */
#define XWT_GWRR_OFFSET         0x1000U    /**< Generic Watchdog Refresh
						Register Offset */
#define XWT_GWCSR_OFFSET        0x2000U    /**< Generic Watchdog Control and
						Status Register Offset */
#define XWT_GWOR_OFFSET         0x2008U    /**< Generic Watchdog Offset
						Register Offset */
#define XWT_GWCVR0_OFFSET       0x2010U    /**< Generic Watchdog Compare
						Value Register 0 Offset */
#define XWT_GWCVR1_OFFSET       0x2014U    /**< Generic Watchdog Compare
						Value Register 1 Offset */
#define XWT_GW_WR_OFFSET        0x2FD0U    /**< Generic Watchdog Warm Reset
						Register Offset */

/** @} */

/** @name Register offsets for the AXI Timebase WDT core. Each register is 32
*   bits.
*  @{
*/
#define XWT_TWCSR0_OFFSET	0x00U	/**< Control/Status Register 0
                                          *  Offset */
#define XWT_TWCSR1_OFFSET	0x04U	/**< Control/Status Register 1
                                          *  Offset */
#define XWT_TBR_OFFSET		0x08U	/**< Timebase Register Offset */
/** @} */

/** @name Control/Status Register 0 bits
*  @{
*/
#define XWT_CSR0_WRS_MASK	0x00000008U	/**< Reset status Mask */
#define XWT_CSR0_WDS_MASK	0x00000004U	/**< Timer state Mask */
#define XWT_CSR0_EWDT1_MASK	0x00000002U	/**< Enable bit 1 Mask */
/** @} */

/** @name Control/Status Register 0/1 bits
*  @{
*/
#define XWT_CSRX_EWDT2_MASK	0x00000001U	/**< Enable bit 2 Mask */
/** @} */

/** @name Master Write Control bits
*  @{
*/
#define XWT_MWR_AEN_MASK	0x00000002U	/**< Always Enable Mask */
#define XWT_MWR_MWC_MASK	0x00000001U	/**< Master Write Control Mask */

/** @} */

/** @name Enable & Status Register bits
*  @{
*/
#define XWT_ESR_LBE_MASK	0x07000000U	/**< Last Bad Event Mask */
#define XWT_ESR_FCV_MASK	0x00700000U	/**< Fail Counter Value Mask */
#define XWT_ESR_WRP_MASK	0x00020000U	/**< Watchdog Reset Pending Mask */
#define XWT_ESR_WINT_MASK	0x00010000U	/**< Watchdog Interrupt Mask */
#define XWT_ESR_ACNT_MASK	0x0000C000U	/**< Answer Count Mask */
#define XWT_ESR_WSW_MASK	0x00000100U	/**< Watchdog Second Window Mask */
#define XWT_TOKEN_VAL_MASK	0x0000003CU /**< Token Value Mask */
#define XWT_ESR_WCFG_MASK	0x00000002U	/**< Wrong Configuration Mask */
#define XWT_ESR_WEN_MASK	0x00000001U	/**< Window WDT Enable Mask */
#define XWT_ESR_LBE_SHIFT	24U		/**< Last Bad Event Shift */
#define XWT_ESR_FCV_SHIFT	20U		/**< Fail Counter Value Shift */
#define XWT_ESR_WRP_SHIFT	17U		/**< Watchdog Reset Pending Shift */
#define XWT_ESR_WINT_SHIFT	16U		/**< Watchdog Interrupt Shift */
#define XWT_ESR_ACNT_SHIFT	14U		/**< Watchdog Answer Cnt Shift */
#define XWT_ESR_WSW_SHIFT	8U		/**< Watchdog Second Window Shift */
#define XWT_ESR_TOKENVAL_SHIFT	2U	/**< Token Value Shift */
#define XWT_ESR_WCFG_SHIFT	1U		/**< Wrong Configuration Shift */
/** @} */

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
#define XWT_FCR_SBC_SHIFT	8U		/**< Selected Byte Count
						  *  Shift */
#define XWT_FCR_BSS_SHIFT	6U		/**< Byte Segment Selection
						  *  Shift */
#define XWT_FCR_SSTE_SHIFT	4U		/**< Second Sequence Timer
						  *  Enable Shift */
#define XWT_FCR_WM_SHIFT	1U		/**< Window WDT Mode Shift */
/** @} */

/** @name Token feedback register bits
*  @{
*/
#define XWT_TFR_FDBK_MASK	0x00000F00U	/**< feedback bit */
#define XWT_TFR_SEED_MASK	0x0000000FU	/**< seed bit */

#define XWT_TFR_FDBK_SHIFT	8U	/**< feedback bit shift*/
/** @} */

/** @name Token response register bits
*  @{
*/
#define XWT_TRR_ANS_MASK		0x000000FFU	/**< ANS bit */

/** @} */

/** @name Generic Watchdog Control and Status Register  bits
 *  @{
 */
#define XWT_GWCSR_GWEN_MASK     0x00000001U     /**< Watchdog enable bit */
#define XWT_GWCSR_GWS1_MASK     0x00000002U     /**< Generic_wdt_interrupt bit */
#define XWT_GWCSR_GWS2_MASK     0x00000004U     /**< Generic_wdt_reset bit  */
/** @} */

/* @name Generic Watchdog Warm Reset Register bits
 * @{
 */
#define XWT_GW_WR_MASK          0x00000001U         /**< Enable Generic Watchdog Warm Reset Register */
#define XWT_GWRR_MASK           0x00000001U         /**< Generic watchdog Refresh Register */
/* @} */

/* @name Useful macros
 * @{
 */
#define XWT_START_VALUE         8U                  /**< Width of Win WDT values between 8-31 */
#define XWT_END_VALUE           31U                 /**< Width of Win WDT values between 8-31 */
#define XWT_ZERO                0U                  /**< Flag for 0 value*/
#define XWT_ONE                 1U                  /**< Flag for 1 value */
#define XWT_MAX_BYTE_SEGMENT    4U                  /**< Max Byte segment value */
#define XWT_GWDT				0U    				/**< Flag for GWDT */
#define XWT_WWDT				1U   				/**< Flag for WWDT */
#define XWT_ENABLE_QA_MODE		0x2		    		/**< Flag for WWDT */
#define XWT_MAX_TIMEOUT		30000U		    		/**< GWDT max timeout */
#define XWT_KILO_HZ		1000U		    /**< Flag for Kilo Hz */
/* @} */
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XWdtTb_In32		Xil_In32	/**< Input Operations */
#define XWdtTb_Out32		Xil_Out32	/**< Output Operations */
/** @} */

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

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
