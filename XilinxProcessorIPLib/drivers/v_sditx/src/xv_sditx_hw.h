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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_sditx_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx SDI TX core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xv_sditx.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   jsr    07/17/17 Initial release.
# 2.0   vve    10/03/18 Add support for ST352 in C Stream
* </pre>
*
******************************************************************************/
#ifndef XV_SDITX_HW_H_
#define XV_SDITX_HW_H_     /**< Prevent circular inclusions
				*  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

#define XV_SDITX_REGISTER_SIZE				27
#define XV_SDITX_BASE					(0*64)
#define XV_SDITX_RST_CTRL_OFFSET			((XV_SDITX_BASE)+(0*4))
#define XV_SDITX_MDL_CTRL_OFFSET			((XV_SDITX_BASE)+(1*4))
#define XV_SDITX_GIER_OFFSET				((XV_SDITX_BASE)+(3*4))
#define XV_SDITX_ISR_OFFSET				((XV_SDITX_BASE)+(4*4))
#define XV_SDITX_IER_OFFSET				((XV_SDITX_BASE)+(5*4))

#define XV_SDITX_TX_ST352_LINE_OFFSET			((XV_SDITX_BASE)+(6*4))
#define XV_SDITX_TX_ST352_DATA_CH0_OFFSET		((XV_SDITX_BASE)+(7*4))
#define XV_SDITX_TX_ST352_DATA_CH1_OFFSET		((XV_SDITX_BASE)+(8*4))
#define XV_SDITX_TX_ST352_DATA_CH2_OFFSET		((XV_SDITX_BASE)+(9*4))
#define XV_SDITX_TX_ST352_DATA_CH3_OFFSET		((XV_SDITX_BASE)+(10*4))
#define XV_SDITX_TX_ST352_DATA_CH4_OFFSET		((XV_SDITX_BASE)+(11*4))
#define XV_SDITX_TX_ST352_DATA_CH5_OFFSET		((XV_SDITX_BASE)+(12*4))
#define XV_SDITX_TX_ST352_DATA_CH6_OFFSET		((XV_SDITX_BASE)+(13*4))
#define XV_SDITX_TX_ST352_DATA_CH7_OFFSET		((XV_SDITX_BASE)+(14*4))

#define XV_SDITX_VER_OFFSET				((XV_SDITX_BASE)+(15*4))
#define XV_SDITX_SYS_CFG_OFFSET				((XV_SDITX_BASE)+(16*4))

#define XV_SDITX_SB_TX_STS_TDATA_OFFSET			((XV_SDITX_BASE)+(24*4))

#define XV_SDITX_BRIDGE_STS_OFFSET			((XV_SDITX_BASE)+(26*4))
#define XV_SDITX_AXI4S_VID_OUT_STS_OFFSET		((XV_SDITX_BASE)+(27*4))

#define XV_SDITX_TX_ST352_DATA_CH0_C_OFFSET		((XV_SDITX_BASE)+(28*4))
#define XV_SDITX_TX_ST352_DATA_CH1_C_OFFSET		((XV_SDITX_BASE)+(29*4))
#define XV_SDITX_TX_ST352_DATA_CH2_C_OFFSET		((XV_SDITX_BASE)+(30*4))
#define XV_SDITX_TX_ST352_DATA_CH3_C_OFFSET		((XV_SDITX_BASE)+(31*4))
#define XV_SDITX_TX_ST352_DATA_CH4_C_OFFSET		((XV_SDITX_BASE)+(32*4))
#define XV_SDITX_TX_ST352_DATA_CH5_C_OFFSET		((XV_SDITX_BASE)+(33*4))
#define XV_SDITX_TX_ST352_DATA_CH6_C_OFFSET		((XV_SDITX_BASE)+(34*4))
#define XV_SDITX_TX_ST352_DATA_CH7_C_OFFSET		((XV_SDITX_BASE)+(35*4))

/* RST_CTRL register masks */
#define XV_SDITX_RST_CTRL_SDITX_SS_EN_MASK		(1<<0)
#define XV_SDITX_RST_CTRL_SRST_MASK			(1<<1)
#define XV_SDITX_RST_CTRL_SDITX_BRIDGE_EN_MASK		(1<<8)
#define XV_SDITX_RST_CTRL_AXI4S_VID_OUT_EN_MASK		(1<<9)
#define XV_SDITX_RST_CTRL_SRST_SHIFT			1
#define XV_SDITX_RST_CTRL_SDITX_BRIDGE_EN_SHIFT		8
#define XV_SDITX_RST_CTRL_AXI4S_VID_OUT_EN_SHIFT	9


/* MODULE_CTRL register masks */
#define XV_SDITX_MDL_CTRL_MODE_MASK			0x70
#define XV_SDITX_MDL_CTRL_M_MASK			(1<<7)
#define XV_SDITX_MDL_CTRL_MUX_PATTERN_MASK		0x700
#define XV_SDITX_MDL_CTRL_INS_CRC_MASK			(1<<12)
#define XV_SDITX_MDL_CTRL_INS_ST352_MASK		(1<<13)
#define XV_SDITX_MDL_CTRL_OVR_ST352_MASK		(1<<14)
#define XV_SDITX_MDL_CTRL_ST352_F2_EN_MASK		(1<<15)
#define XV_SDITX_MDL_CTRL_INS_SYNC_BIT_MASK		(1<<16)
#define XV_SDITX_MDL_CTRL_SD_BITREP_BYPASS_MASK		(1<<17)
#define XV_SDITX_MDL_CTRL_USE_ANC_IN_MASK		(1<<18)
#define XV_SDITX_MDL_CTRL_INS_LN_MASK			(1<<19)
#define XV_SDITX_MDL_CTRL_INS_EDH_MASK			(1<<20)
#define XV_SDITX_MDL_CTRL_VID_FRMTYUV444_MASK		(1<<22)
#define XV_SDITX_MDL_CTRL_VID_FRMT_MASK			0x600000
#define XV_SDITX_MDL_CTRL_C_ST352_MASK			(1<<23)
#define XV_SDITX_MDL_CTRL_C_ST352_SWITCH_3GA_MASK	(1<<24)
#define XV_SDITX_MDL_CTRL_MODE_SHIFT			4
#define XV_SDITX_MDL_CTRL_M_SHIFT			7
#define XV_SDITX_MDL_CTRL_MUX_PATTERN_SHIFT		8
#define XV_SDITX_MDL_CTRL_INS_CRC_SHIFT			12
#define XV_SDITX_MDL_CTRL_INS_ST352_SHIFT		13
#define XV_SDITX_MDL_CTRL_OVR_ST352_SHIFT		14
#define XV_SDITX_MDL_CTRL_ST352_F2_EN_SHIFT		15
#define XV_SDITX_MDL_CTRL_INS_SYNC_BIT_SHIFT		16
#define XV_SDITX_MDL_CTRL_SD_BITREP_BYPASS_SHIFT	17
#define XV_SDITX_MDL_CTRL_USE_ANC_IN_SHIFT		18
#define XV_SDITX_MDL_CTRL_INS_LN_SHIFT			19
#define XV_SDITX_MDL_CTRL_INS_EDH_SHIFT			20
#define XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT		21
#define XV_SDITX_MDL_CTRL_VID_FRMTYUV444_SHIFT		22

/* Global interrupt Enable regiser masks */
#define XV_SDITX_GIER_GIE_MASK				(1<<0)
#define XV_SDITX_GIER_GIE_SHIFT				0
#define XV_SDITX_GIER_SET				1
#define XV_SDITX_GIER_RESET				0

/* Interrupt status register masks */
#define XV_SDITX_ISR_GTTX_RSTDONE_MASK			(1<<0)
#define XV_SDITX_ISR_TX_CE_ALIGN_ERR_MASK		(1<<1)
#define XV_SDITX_ISR_AXI4S_VID_LOCK_MASK		(1<<8)
#define XV_SDITX_ISR_OVERFLOW_MASK			(1<<9)
#define XV_SDITX_ISR_UNDERFLOW_MASK			(1<<10)
#define XV_SDITX_ISR_TX_CE_ALIGN_ERR_SHIFT		1
#define XV_SDITX_ISR_AXI4S_VID_LOCK_SHIFT		8
#define XV_SDITX_ISR_OVERFLOW_SHIFT			9
#define XV_SDITX_ISR_UNDERFLOW_SHIFT			10

/* All interrupts status mask */
#define XV_SDITX_ISR_ALLINTR_MASK			0x00000703

/* Interrupt Enable Register masks */
#define XV_SDITX_IER_GTTX_RSTDONE_MASK			(1<<0)
#define XV_SDITX_IER_TX_CE_ALIGN_ERR_MASK		(1<<1)
#define XV_SDITX_IER_AXI4S_VID_LOCK_MASK		(1<<8)
#define XV_SDITX_IER_OVERFLOW_MASK			(1<<9)
#define XV_SDITX_IER_UNDERFLOW_MASK			(1<<10)

/* All interrupts Enable mask */
#define XV_SDITX_IER_ALLINTR_MASK			0x00000703

/* TX_ST352_LINE register masks */
#define XV_SDITX_TX_ST352_LINE_F1_MASK			0x7FF
#define XV_SDITX_TX_ST352_LINE_F2_MASK			0x7FF0000
#define XV_SDITX_TX_ST352_LINE_F1_SHIFT			0
#define XV_SDITX_TX_ST352_LINE_F2_SHIFT			16

/* TX_ST352_DATA_CH0 register masks */
#define SDITX_TX_ST352_DATA_CH_MASK			0xFFFFFFFF

/* Version register masks */
#define XV_SDITX_VER_MASK				0x0

/* SYS_CONFIG register masks */
#define XV_SDITX_SYS_CFG_AXI4LITE_EN_MASK		(1<<0)
#define XV_SDITX_SYS_CFG_INC_TX_EDH_PROC_MASK		(1<<1)
#define XV_SDITX_SYS_CFG_ADV_FEATURE_MASK		(1<<2)

/* STS_SB_TX_TDATA masks */
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_TX_CHANGE_DONE_MASK		(1<<0)
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_TX_CHANGE_FAIL_MASK		(1<<1)
#define XV_SDITX_SB_TX_STS_TDATA_GT_TX_RESETDONE_MASK			(1<<2)
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_SLEW_RATE_MASK			(1<<3)
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_TXPLLCLKSEL_MASK		0x30
#define XV_SDITX_SB_TX_STS_TDATA_GT_TXSYSCLKSEL_MASK			0xc0
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_FABRIC_RST_MASK		(1<<8)
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_DRP_FAIL_MASK			(1<<9)
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_TX_CHANGE_FAIL_CODE_MASK	0x7000
#define XV_SDITX_SB_TX_STS_TDATA_SDICTRL_DRP_FAIL_CNT_MASK		0xFF0000
#define XV_SDITX_SB_TX_STS_TDATA_GT_CMN_QPLL0LOCK_MASK			(1<<24)
#define XV_SDITX_SB_TX_STS_TDATA_GT_CMN_QPLL1LOCK_MASK			(1<<25)
#define XV_SDITX_SB_TX_STS_TDATA_GT_CH_CPLLLOCK_MASK			(1<<26)

/* XV_SDITX_BRIDGE_STS masks */
#define XV_SDITX_BRIDGE_STS_SELECT_MASK					(1<<0)
#define XV_SDITX_BRIDGE_STS_MODE_MASK					((0x3)<<4)
#define XV_SDITX_BRIDGE_STS_3G_LEVELB_MASK				(1<<6)
#define XV_SDITX_BRIDGE_STS_MODE_SHIFT					4

/* XV_SDITX_AXI4S_VID_OUT_STS1 masks */
#define XV_SDITX_AXI4S_VID_OUT_STS1_LOCKED_MASK				(1<<0)
#define XV_SDITX_AXI4S_VID_OUT_STS1_OVRFLOW_MASK			(1<<1)
#define XV_SDITX_AXI4S_VID_OUT_STS1_UNDERFLOW_MASK			(1<<2)
#define XV_SDITX_AXI4S_VID_OUT_STS1_OVRFLOW_SHIFT			1
#define XV_SDITX_AXI4S_VID_OUT_STS1_UNDERFLOW_SHIFT			2

/* XV_SDITX_AXI4S_VID_OUT_STS2_OFFSET */
#define XV_SDITX_AXI4S_VID_OUT_STS2_STATUS_MASK				0xFFFFFFFF


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XV_SdiTx_In32  Xil_In32    /**< Input Operations */
#define XV_SdiTx_Out32 Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a SDI TX register. A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the SDI TX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XV_SdiTx_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XV_SdiTx_ReadReg(BaseAddress, RegOffset) \
    XV_SdiTx_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a SDI TX register. A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the SDI TX core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XV_SdiTx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XV_SdiTx_WriteReg(BaseAddress, RegOffset, Data) \
    XV_SdiTx_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
