/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
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
#       vve    10/03/18 Add support for ST352 in C-Stream
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

#define XV_SDIRX_RX_ST352_0_C_OFFSET				((XV_SDITX_BASE)+(28*4))
#define XV_SDIRX_RX_ST352_1_C_OFFSET				((XV_SDITX_BASE)+(29*4))
#define XV_SDIRX_RX_ST352_2_C_OFFSET				((XV_SDITX_BASE)+(30*4))
#define XV_SDIRX_RX_ST352_3_C_OFFSET				((XV_SDITX_BASE)+(31*4))
#define XV_SDIRX_RX_ST352_4_C_OFFSET				((XV_SDITX_BASE)+(32*4))
#define XV_SDIRX_RX_ST352_5_C_OFFSET				((XV_SDITX_BASE)+(33*4))
#define XV_SDIRX_RX_ST352_6_C_OFFSET				((XV_SDITX_BASE)+(34*4))
#define XV_SDIRX_RX_ST352_7_C_OFFSET				((XV_SDITX_BASE)+(35*4))

/* RST_CTRL register masks */
#define XV_SDIRX_RST_CTRL_SDIRX_SS_EN_MASK			(1<<0)
#define XV_SDIRX_RST_CTRL_SRST_MASK				(1<<1)
#define XV_SDIRX_RST_CTRL_RST_CLR_ERR_MASK			(1<<2)
#define XV_SDIRX_RST_CTRL_RST_CLR_EDH_MASK			(1<<3)
#define XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_MASK			(1<<8)
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_MASK		(1<<9)
#define XV_SDIRX_RST_CTRL_CH_FORMAT_AXI_EN_MASK			(1<<10)
#define XV_SDIRX_RST_CTRL_SRST_SHIFT				1
#define XV_SDIRX_RST_CTRL_RST_CLR_ERR_SHIFT			2
#define XV_SDIRX_RST_CTRL_RST_CLR_EDH_SHIFT			3
#define XV_SDIRX_RST_CTRL_SDIRX_BRIDGE_EN_SHIFT			8
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_MDL_EN_SHIFT		9
#define XV_SDIRX_RST_CTRL_VID_IN_AXI4S_EN_SHIFT			10

/* MODULE_CTRL register masks */
#define XV_SDIRX_MDL_CTRL_FRM_EN_MASK				(1<<4)
#define XV_SDIRX_MDL_CTRL_MODE_DET_EN_MASK			(1<<5)
#define XV_SDIRX_MDL_CTRL_VPID_MASK				(1 << 6)
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
#define XV_SDIRX_ISR_VSYNC_MASK					(1<<2)
#define XV_SDIRX_ISR_OVERFLOW_MASK				(1<<9)
#define XV_SDIRX_ISR_UNDERFLOW_MASK				(1<<10)
#define XV_SDIRX_ISR_VIDEO_LOCK_SHIFT				0
#define XV_SDIRX_ISR_VIDEO_UNLOCK_SHIFT				1
#define XV_SDIRX_ISR_OVERFLOW_SHIFT				9
#define XV_SDIRX_ISR_UNDERFLOW_SHIFT				10

/* All interrupts status mask */
#define XV_SDIRX_ISR_ALLINTR_MASK				0x00000607

/* Interrupt Enable Register masks */
#define XV_SDIRX_IER_VIDEO_LOCK_MASK				(1<<0)
#define XV_SDIRX_IER_VIDEO_UNLOCK_MASK				(1<<1)
#define XV_SDIRX_IER_VSYNC_MASK					(1<<2)
#define XV_SDIRX_IER_OVERFLOW_MASK				(1<<9)
#define XV_SDIRX_IER_UNDERFLOW_MASK				(1<<10)
#define XV_SDIRX_IER_VIDEO_LOCK_SHIFT				0
#define XV_SDIRX_IER_VIDEO_UNLOCK_SHIFT				1
#define XV_SDIRX_IER_OVERFLOW_SHIFT				9
#define XV_SDIRX_IER_UNDERFLOW_SHIFT				10

/* All interrupts enable mask */
#define XV_SDIRX_IER_ALLINTR_MASK				0x00000607

/* RX_ST352_VALID register masks */
#define XV_SDIRX_RX_ST352_VLD_ST352_0				(1<<0)
#define XV_SDIRX_RX_ST352_VLD_ST352_1				(1<<1)
#define XV_SDIRX_RX_ST352_VLD_ST352_2				(1<<2)
#define XV_SDIRX_RX_ST352_VLD_ST352_3				(1<<3)
#define XV_SDIRX_RX_ST352_VLD_ST352_4				(1<<4)
#define XV_SDIRX_RX_ST352_VLD_ST352_5				(1<<5)
#define XV_SDIRX_RX_ST352_VLD_ST352_6				(1<<6)
#define XV_SDIRX_RX_ST352_VLD_ST352_7				(1<<7)
#define XV_SDIRX_RX_ST352_VLD_ST352_8				(1<<8)
#define XV_SDIRX_RX_ST352_VLD_ST352_9				(1<<9)
#define XV_SDIRX_RX_ST352_VLD_ST352_10				(1<<10)
#define XV_SDIRX_RX_ST352_VLD_ST352_11				(1<<11)
#define XV_SDIRX_RX_ST352_VLD_ST352_12				(1<<12)
#define XV_SDIRX_RX_ST352_VLD_ST352_13				(1<<13)
#define XV_SDIRX_RX_ST352_VLD_ST352_14				(1<<14)
#define XV_SDIRX_RX_ST352_VLD_ST352_15				(1<<15)

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

/* ST352 related macros */
#define XST352_PAYLOAD_BYTE_MASK	0xFF
#define XST352_PAYLOAD_BYTE1_SHIFT	0
#define XST352_PAYLOAD_BYTE2_SHIFT	8
#define XST352_PAYLOAD_BYTE3_SHIFT	16
#define XST352_PAYLOAD_BYTE4_SHIFT	24

#define XST352_BYTE1_ST292_1x720L_1_5G		0x84
#define XST352_BYTE1_ST292_1x1080L_1_5G		0x85
#define XST352_BYTE1_ST425_2008_750L_3GB	0x88
#define XST352_BYTE1_ST425_2008_1125L_3GA	0x89
#define XST352_BYTE1_ST372_DL_3GB		0x8A
#define XST352_BYTE1_ST372_2x720L_3GB		0x8B
#define XST352_BYTE1_ST372_2x1080L_3GB		0x8C
#define XST352_BYTE1_ST2081_10_2160L_6G		0xC0
#define XST352_BYTE1_ST2081_10_2_1080L_6G	0xC1
#define XST352_BYTE1_ST2081_10_DL_2160L_6G	0xC2
#define XST352_BYTE1_ST2082_10_2160L_12G	0xCE

#define XST352_BYTE2_TS_TYPE_MASK		1 << 15
#define XST352_BYTE2_TS_TYPE_OFFSET		15
#define XST352_BYTE2_PIC_TYPE_MASK		1 << 14
#define XST352_BYTE2_PIC_TYPE_OFFSET		14
#define XST352_BYTE2_TS_PIC_TYPE_INTERLACED	0
#define XST352_BYTE2_TS_PIC_TYPE_PROGRESSIVE	1

#define XST352_BYTE2_FPS_MASK			0xF
#define XST352_BYTE2_FPS_SHIFT			8
#define XST352_BYTE2_FPS_96F			0x1
#define XST352_BYTE2_FPS_24F			0x2
#define XST352_BYTE2_FPS_24			0x3
#define XST352_BYTE2_FPS_48F			0x4
#define XST352_BYTE2_FPS_25			0x5
#define XST352_BYTE2_FPS_30F			0x6
#define XST352_BYTE2_FPS_30			0x7
#define XST352_BYTE2_FPS_48			0x8
#define XST352_BYTE2_FPS_50			0x9
#define XST352_BYTE2_FPS_60F			0xA
#define XST352_BYTE2_FPS_60			0xB
/* Table 4 ST 2081-10:2015 */
#define XST352_BYTE2_FPS_96			0xC
#define XST352_BYTE2_FPS_100			0xD
#define XST352_BYTE2_FPS_120F			0xE
#define XST352_BYTE2_FPS_120			0xF

/* Electro Optical Transfer Function bit[5:4] */
#define XST352_BYTE2_EOTF_MASK			(0x3 << 12)
#define XST352_BYTE2_EOTF_SHIFT			12
#define XST352_BYTE2_EOTF_SDRTV			0x0
#define XST352_BYTE2_EOTF_HLG			0x1
#define XST352_BYTE2_EOTF_SMPTE2084		0x2

#define XST352_BYTE3_COLOR_FORMAT_MASK		0xF
#define XST352_BYTE3_COLOR_FORMAT_420		0x3
#define XST352_BYTE3_COLOR_FORMAT_422		0x0
#define XST352_BYTE3_COLOR_FORMAT_444		0x1
#define XST352_BYTE3_COLOR_FORMAT_444_RGB	0x2

#define XST352_BYTE3_ACT_LUMA_COUNT_MASK	1 << 22
#define XST352_BYTE3_ACT_LUMA_COUNT_OFFSET	22

#define XST352_BYTE3_COLORIMETRY_MASK		(0x3 << 20)
#define XST352_BYTE3_COLORIMETRY_SHIFT		20
#define XST352_BYTE3_COLORIMETRY_BT709		0
#define XST352_BYTE3_COLORIMETRY_VANC		1
#define XST352_BYTE3_COLORIMETRY_UHDTV		2
#define XST352_BYTE3_COLORIMETRY_UNKNOWN	3

#define XST352_BYTE4_BIT_DEPTH_MASK		0x03
#define XST352_BYTE4_BIT_DEPTH_8		0x00
#define XST352_BYTE4_BIT_DEPTH_10		0x01
#define XST352_BYTE4_BIT_DEPTH_12		0x02

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
