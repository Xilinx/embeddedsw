/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* @file xv_sdirx_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx SDI RX core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xv_sdirx.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   jsr    07/17/17 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_SDIRX_HW_H_
#define XV_SDIRX_HW_H_     /**< Prevent circular inclusions
					  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

#define XV_SDIRX_REGISTER_SIZE					27
#define XV_SDIRX_BASE						(0*64)
#define XV_SDIRX_RST_CTRL_OFFSET				((XV_SDIRX_BASE)+(0*4))
#define XV_SDIRX_MDL_CTRL_OFFSET				((XV_SDIRX_BASE)+(1*4))
#define XV_SDIRX_GIER_OFFSET					((XV_SDIRX_BASE)+(3*4))
#define XV_SDIRX_ISR_OFFSET					((XV_SDIRX_BASE)+(4*4))
#define XV_SDIRX_IER_OFFSET					((XV_SDIRX_BASE)+(5*4))

#define XV_SDIRX_RX_ST352_VLD_OFFSET				((XV_SDIRX_BASE)+(6*4))
#define XV_SDIRX_RX_ST352_0_OFFSET				((XV_SDIRX_BASE)+(7*4))
#define XV_SDIRX_RX_ST352_1_OFFSET				((XV_SDIRX_BASE)+(8*4))
#define XV_SDIRX_RX_ST352_2_OFFSET				((XV_SDIRX_BASE)+(9*4))
#define XV_SDIRX_RX_ST352_3_OFFSET				((XV_SDIRX_BASE)+(10*4))
#define XV_SDIRX_RX_ST352_4_OFFSET				((XV_SDIRX_BASE)+(11*4))
#define XV_SDIRX_RX_ST352_5_OFFSET				((XV_SDIRX_BASE)+(12*4))
#define XV_SDIRX_RX_ST352_6_OFFSET				((XV_SDIRX_BASE)+(13*4))
#define XV_SDIRX_RX_ST352_7_OFFSET				((XV_SDIRX_BASE)+(14*4))


#define XV_SDIRX_VER_OFFSET					((XV_SDIRX_BASE)+(15*4))
#define XV_SDIRX_SYS_CFG_OFFSET					((XV_SDIRX_BASE)+(16*4))

#define XV_SDIRX_MODE_DET_STS_OFFSET				((XV_SDIRX_BASE)+(17*4))
#define XV_SDIRX_TS_DET_STS_OFFSET				((XV_SDIRX_BASE)+(18*4))

#define XV_SDIRX_RX_EDH_STS_OFFSET				((XV_SDIRX_BASE)+(19*4))
#define XV_SDIRX_EDH_ERRCNT_EN_OFFSET				((XV_SDIRX_BASE)+(20*4))
#define XV_SDIRX_RX_EDH_ERRCNT_OFFSET				((XV_SDIRX_BASE)+(21*4))
#define XV_SDIRX_RX_ERR_OFFSET					((XV_SDIRX_BASE)+(22*4))


#define XV_SDIRX_VIDLCK_WINDOW_OFFSET				((XV_SDIRX_BASE)+(23*4))
#define XV_SDIRX_STS_SB_RX_TDATA_OFFSET				((XV_SDIRX_BASE)+(24*4))


/* RST_CTRL register masks */
#define XV_SDIRX_RST_CTRL_SDIRX_SS_EN_MASK			(1<<0)
#define XV_SDIRX_RST_CTRL_SRST_MASK				(1<<1)
#define XV_SDIRX_RST_CTRL_RST_CLR_ERR_MASK			(1<<2)
#define XV_SDIRX_RST_CTRL_RST_CLR_EDH_MASK			(1<<3)
#define XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_MASK			(1<<8)
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_MASK		(1<<9)
#define XV_SDIRX_RST_CTRL_SRST_SHIFT				1
#define XV_SDIRX_RST_CTRL_RST_CLR_ERR_SHIFT			2
#define XV_SDIRX_RST_CTRL_RST_CLR_EDH_SHIFT			3
#define XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_SHIFT			8
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_SHIFT		9
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_EN_SHIFT			10

/* MODULE_CTRL register masks */
#define XV_SDIRX_MDL_CTRL_FRM_EN_MASK				(1<<4)
#define XV_SDIRX_MDL_CTRL_MODE_DET_EN_MASK			(1<<5)
#define XV_SDIRX_MDL_CTRL_MODE_EN_MASK				0x3F00
#define XV_SDIRX_MDL_CTRL_FORCED_MODE_MASK			0x70000
#define XV_SDIRX_MDL_CTRL_FRM_EN_SHIFT				4
#define XV_SDIRX_MDL_CTRL_MODE_DET_EN_SHIFT			5
#define XV_SDIRX_MDL_CTRL_MODE_EN_SHIFT				8
#define XV_SDIRX_MDL_CTRL_FORCED_MODE_SHIFT			16

/* Global interrupt Enable regiser masks */
#define XV_SDIRX_GIER_GIE_MASK					(1<<0)
#define XV_SDIRX_GIER_GIE_SHIFT					0
#define XV_SDIRX_GIER_SET					1
#define XV_SDIRX_GIER_RESET					0


/* Interrupt status register masks */
#define XV_SDIRX_ISR_VIDEO_LOCK_MASK				(1<<0)
#define XV_SDIRX_ISR_VIDEO_UNLOCK_MASK				(1<<1)
#define XV_SDIRX_ISR_OVERFLOW_MASK				(1<<9)
#define XV_SDIRX_ISR_UNDERFLOW_MASK				(1<<10)
#define XV_SDIRX_ISR_VIDEO_LOCK_SHIFT				0
#define XV_SDIRX_ISR_VIDEO_UNLOCK_SHIFT				1
#define XV_SDIRX_ISR_OVERFLOW_SHIFT				9
#define XV_SDIRX_ISR_UNDERFLOW_SHIFT				10

/* All interrupts status mask */
#define XV_SDIRX_ISR_ALLINTR_MASK				0x00000603

/* Interrupt Enable Register masks */
#define XV_SDIRX_IER_VIDEO_LOCK_MASK				(1<<0)
#define XV_SDIRX_IER_VIDEO_UNLOCK_MASK				(1<<1)
#define XV_SDIRX_IER_OVERFLOW_MASK				(1<<9)
#define XV_SDIRX_IER_UNDERFLOW_MASK				(1<<10)
#define XV_SDIRX_IER_VIDEO_LOCK_SHIFT				0
#define XV_SDIRX_IER_VIDEO_UNLOCK_SHIFT				1
#define XV_SDIRX_IER_OVERFLOW_SHIFT				9
#define XV_SDIRX_IER_UNDERFLOW_SHIFT				10

/* All interrupts enable mask */
#define XV_SDIRX_IER_ALLINTR_MASK				0x00000603

/* RX_ST352_VALID register masks */
#define XV_SDIRX_RX_ST352_VLD_ST352_0				(1<<0)
#define XV_SDIRX_RX_ST352_VLD_ST352_1				(1<<1)
#define XV_SDIRX_RX_ST352_VLD_ST352_2				(1<<2)
#define XV_SDIRX_RX_ST352_VLD_ST352_3				(1<<3)
#define XV_SDIRX_RX_ST352_VLD_ST352_4				(1<<4)
#define XV_SDIRX_RX_ST352_VLD_ST352_5				(1<<5)
#define XV_SDIRX_RX_ST352_VLD_ST352_6				(1<<6)
#define XV_SDIRX_RX_ST352_VLD_ST352_7				(1<<7)

/* RX_ST352_ register masks */
#define XV_SDIRX_RX_ST352_MASK					0x0

/* Version register masks */
#define XV_SDIRX_VER_MASK					0x0

/* SYS_CONFIG register masks */
#define XV_SDIRX_SYS_CFG_AXI4LITE_EN_MASK			(1<<0)
#define XV_SDIRX_SYS_CFG_INC_RX_EDH_PROC_MASK			(1<<1)

/* MODE_DET_STS register masks */
#define XV_SDIRX_MODE_DET_STS_MODE_MASK				0x7
#define XV_SDIRX_MODE_DET_STS_MODE_LOCKED_MASK			(1<<3)
#define XV_SDIRX_MODE_DET_STS_ACT_STRM_MASK			0x70
#define XV_SDIRX_MODE_DET_STS_LVL_B_3G_MASK			(1<<7)
#define XV_SDIRX_MODE_DET_STS_ACT_STRM_SHIFT			4
#define XV_SDIRX_MODE_DET_STS_LVL_B_3G_SHIFT			7

/* TS_DET_STS register masks */
#define XV_SDIRX_TS_DET_STS_T_LOCKED_MASK			(1<<0)
#define XV_SDIRX_TS_DET_STS_T_SCAN_MASK				(1<<1)
#define XV_SDIRX_TS_DET_STS_T_FAMILY_MASK			0xF0
#define XV_SDIRX_TS_DET_STS_T_RATE_MASK				0xF00
#define XV_SDIRX_TS_DET_STS_T_SCAN_SHIFT			1
#define XV_SDIRX_TS_DET_STS_T_FAMILY_SHIFT			4
#define XV_SDIRX_TS_DET_STS_T_RATE_SHIFT			8

/* RX_EDH_STS register masks */
#define XV_SDIRX_RX_EDH_STS_EDH_AP_MASK				(1<<0)
#define XV_SDIRX_RX_EDH_STS_EDH_FF_MASK				(1<<1)
#define XV_SDIRX_RX_EDH_STS_EDH_ANC_MASK			(1<<2)
#define XV_SDIRX_RX_EDH_STS_EDH_AP_FLAGS_MASK			0x1F0
#define XV_SDIRX_RX_EDH_STS_EDH_FF_FLAGS_MASK			0x3E00
#define XV_SDIRX_RX_EDH_STS_EDH_ANC_FLAGS_MASK			0x7C000
#define XV_SDIRX_RX_EDH_STS_EDH_PACKET_FLAGS_MASK		0x780000
#define XV_SDIRX_RX_EDH_STS_EDH_AP_FLAGS_SHIFT			4
#define XV_SDIRX_RX_EDH_STS_EDH_FF_FLAGS_SHIFT			9
#define XV_SDIRX_RX_EDH_STS_EDH_ANC_FLAGS_SHIFT			14
#define XV_SDIRX_RX_EDH_STS_EDH_PACKET_FLAGS_SHIFT		19

/* RX_EDH_ERRCNT register masks */
#define XV_SDIRX_RX_EDH_ERRCNT_EDH_ERRCNT_MASK			0xFFFF

/* RX_ERR register masks */
#define XV_SDIRX_RX_ERR_MASK					0xFFFF

/* RX_EDH_ERRCNT_EN masks */
#define XV_SDIRX_EDH_ERRCNT_EN_MASK				0xFFFF

/* STS_SB_RX_TDATA masks */
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_RX_CHANGE_DONE_MASK		(1<<0)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_RX_CHANGE_FAIL_MASK		(1<<1)
#define XV_SDIRX_STS_SB_RX_TDATA_GT_RX_RESETDONE_MASK			(1<<2)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_MASK			(1<<3)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_RXPLLCLKSEL_MASK		0x30
#define XV_SDIRX_STS_SB_RX_TDATA_GT_RXSYSCLKSEL_MASK			0xc0
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_FABRIC_RST_MASK		(1<<8)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_DRP_FAIL_MASK			(1<<9)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_RX_CHANGE_FAIL_CODE_MASK	0x7000
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_DRP_FAIL_CNT_MASK		0xFF0000
#define XV_SDIRX_STS_SB_RX_TDATA_GT_CMN_QPLL0LOCK_MASK			(1<<24)
#define XV_SDIRX_STS_SB_RX_TDATA_GT_CMN_QPLL1LOCK_MASK			(1<<25)
#define XV_SDIRX_STS_SB_RX_TDATA_GT_CH_CPLLLOCK_MASK			(1<<26)
#define XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_SHIFT			3


/* XV_SDIRX_BRIDGE_STS masks */
#define XV_SDIRX_BRIDGE_STS_SELECT_MASK					(1<<0)
#define XV_SDIRX_BRIDGE_STS_MODE_LOCKED_MASK				(1<<1)
#define XV_SDIRX_BRIDGE_STS_MODE_MASK					((0x7)<<4)
#define XV_SDIRX_BRIDGE_STS_3G_LEVELB_MASK				(1<<7)
#define XV_SDIRX_BRIDGE_STS_MODE_LOCKED_SHIFT				1
#define XV_SDIRX_BRIDGE_STS_MODE_SHIFT					4

/* Peripheral ID and General shift values. */
#define XV_SDIRX_SHIFT_16      16  /**< 16 shift value */
#define XV_SDIRX_MASK_16       0xFFFF  /**< 16 bit mask value */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XV_SdiRx_In32  Xil_In32    /**< Input Operations */
#define XV_SdiRx_Out32 Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a SDI RX register. A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the SDI RX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XV_SdiRx_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XV_SdiRx_ReadReg(BaseAddress, RegOffset) \
    XV_SdiRx_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a SDI RX register. A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the SDI RX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_SdiRx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XV_SdiRx_WriteReg(BaseAddress, RegOffset, Data) \
    XV_SdiRx_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
