/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
 * @file xsdiaud_hw.h
 * @addtogroup sdiaud_v1_1
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    02/14/18  Initial release.
 * 1.1   kar    04/25/18  Removed version register offset.
 *                        Added rate control enable shift and mask.
 * </pre>
 *
 ******************************************************************************/

#ifndef XSDIAUD_HW_H
#define XSDIAUD_HW_H
/* Prevent circular inclusions by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
/************************** Constant Definitions *****************************/
/** @name Register Map
 *
 * Register offsets for the XSdiAud Embed/Extract device
 * @{
 */
#define XSDIAUD_CNTRL_REG_OFFSET 0x00
	//!< SDI Audio Module control register offset

#define XSDIAUD_SOFT_RST_REG_OFFSET 0x04
	//!< SDI Audio Soft reset register offset

#define XSDIAUD_INT_EN_REG_OFFSET 0x0C
	//!< SDI Audio Interrupt enable register offset

#define XSDIAUD_INT_STS_REG_OFFSET 0x10
	//!< SDI Audio Interrupt status register offset

#define XSDIAUD_EMB_VID_CNTRL_REG_OFFSET 0X14
	//!< SDI Audio Embed Video control register offset

#define XSDIAUD_AUD_CNTRL_REG_OFFSET 0x18
	//!< SDI Audio Audio control register offset

#define XSDIAUD_AXIS_CHCOUNT_REG_OFFSET 0x1C
	//!< SDI Audio AXI stream channel count register offset

#define XSDIAUD_MUX1_OR_DMUX1_CNTRL_REG_OFFSET 0x20
	//!< SDI Audio Mux1 or Dmux1 control register offset

#define XSDIAUD_DMUX1_CNTRL_REG_OFFSET 0x20
	//!< SDI Audio DMux1 control register offset

#define XSDIAUD_GRP_PRES_REG_OFFSET 0X40
	//!< SDI Audio group presence register offset

#define XSDIAUD_EXT_CNTRL_PKTSTAT_REG_OFFSET 0X44
	//!< SDI Audio Extract control packet status register offset

#define XSDIAUD_EXT_CH_STAT0_REG_OFFSET 0X48
	//!< SDI Audio Extract channel status register0 offset

#define XSDIAUD_GUI_PARAM_REG_OFFSET 0XFC
	//!< SDI Audio GUI parameters register offset
/* @} */

/**
 * @name Core Configuration register masks and shifts of XSdiAud device
 * @{
 */
#define XSDIAUD_CNTRL_EN_SHIFT (0)
	//!< SDI Audio Embed enable or Extract enable shift
#define XSDIAUD_CNTRL_EN_MASK  (1 << XSDIAUD_CNTRL_EN_SHIFT)
	//!< SDI Audio Embed enable or Extract enable mask

#define XSDIAUD_SOFT_RST_ACLK_SHIFT (0)
	//!< SDI Audio Embed soft reset aclk shift
#define XSDIAUD_SOFT_RST_ACLK_MASK (1 << XSDIAUD_SOFT_RST_ACLK_SHIFT)
	//!< SDI Audio Embed soft reset aclk mask

#define XSDIAUD_SOFT_RST_SCLK_SHIFT (1)
	//!< SDI Audio soft reset sclk shift
#define XSDIAUD_SOFT_RST_SCLK_MASK (1 << XSDIAUD_SOFT_RST_SCLK_SHIFT)
	//!< SDI Audio soft reset sclk mask

#define XSDIAUD_VER_SHIFT (0)
	//!< SDI Audio version shift
#define XSDIAUD_VER_MASK (0xFF << XSDIAUD_VER_SHIFT)
	//!< SDI Audio version mask

#define XSDIAUD_EMB_VID_CNT_STD_SHIFT (0)
	//!< SDI Audio Embed video control line standard shift
#define XSDIAUD_EMB_VID_CNT_STD_MASK (0x1F << XSDIAUD_EMB_VID_CNT_STD_SHIFT)
	//!< SDI Audio Embed video control line standard mask

#define XSDIAUD_EMB_VID_CNT_ELE_SHIFT (5)
	//!< SDI Audio Embed video control external line enable shift
#define XSDIAUD_EMB_VID_CNT_ELE_MASK (1 << XSDIAUD_EMB_VID_CNT_ELE_SHIFT)
	//!< SDI Audio Embed video control external line enable mask

#define XSDIAUD_EMB_AUD_CNT_SR_SHIFT (0)
	//!< SDI Audio Embed audio control SD sample rate shift
#define XSDIAUD_EMB_AUD_CNT_SR_MASK (0x7 << XSDIAUD_EMB_AUD_CNT_SR_SHIFT)
	//!< SDI Audio Embed audio control SD sample rate mask

#define XSDIAUD_EMB_AUD_CNT_SS_SHIFT (3)
	//!< SDI Audio Embed audio control SD sample size shift
#define XSDIAUD_EMB_AUD_CNT_SS_MASK (1 << XSDIAUD_EMB_AUD_CNT_SS_SHIFT)
	//!< SDI Audio Embed audio control SD sample size mask

#define XSDIAUD_EMB_AUD_CNT_RCE_SHIFT (4)
	//!< SDI Audio Embed audio control Rate Control Enable shift
#define XSDIAUD_EMB_AUD_CNT_RCE_MASK (1 << XSDIAUD_EMB_AUD_CNT_RCE_SHIFT)
	//!< SDI Audio Embed audio control Rate Control Enable mask

#define XSDIAUD_EMB_AXIS_CHCOUNT_SHIFT (0)
	//!< SDI Audio Embed AXIS channel count shift
#define XSDIAUD_EMB_AXIS_CHCOUNT_MASK (0x1F << XSDIAUD_EMB_AXIS_CHCOUNT_SHIFT)
	//!< SDI Audio Embed AXIS channel count mask

#define XSDIAUD_EMD_MUX_CNT_GS_SHIFT (0)
	//!< SDI Audio Embed Mux group select shift
#define XSDIAUD_EMD_MUX_CNT_GS_MASK (0x3 << XSDIAUD_EMD_MUX_CNT_GS_SHIFT)
	//!< SDI Audio Embed Mux group select mask

#define XSDIAUD_GRP_PRESNT_SHIFT (0)
	//!< SDI Audio group present value shift
#define XSDIAUD_GRP_PRESNT_MASK (0xF << XSDIAUD_GRP_PRESNT_SHIFT)
	//!< SDI Audio group present value mask

#define XSDIAUD_GRP_PRESNTV_SHIFT (4)
	//!< SDI Audio group present valid shift
#define XSDIAUD_GRP_PRESNTV_MASK (1 << XSDIAUD_GRP_PRESNTV_SHIFT)
	//!< SDI Audio group present valid mask

#define XSDIAUD_INT_EN_GRP_CHG_SHIFT (0)
	//!< SDI Audio interrupt group change detect shift
#define XSDIAUD_INT_EN_GRP_CHG_MASK (1 << XSDIAUD_INT_EN_GRP_CHG_SHIFT)
	//!< SDI Audio interrupt group change detect mask

#define XSDIAUD_EXT_INT_EN_PKT_CHG_SHIFT (1)
	//!< SDI Audio Extract interrupt control packet change detect shift
#define XSDIAUD_EXT_INT_EN_PKT_CHG_MASK (1 << XSDIAUD_EXT_INT_EN_PKT_CHG_SHIFT)
	//!< SDI Audio Extract interrupt control packet change detect mask

#define XSDIAUD_EXT_INT_EN_STS_CHG_SHIFT (2)
	//!< SDI Audio Extract interrupt status change detect shift
#define XSDIAUD_EXT_INT_EN_STS_CHG_MASK (1 << XSDIAUD_EXT_INT_EN_STS_CHG_SHIFT)
	//!< SDI Audio Extract interrupt status change detect mask

#define XSDIAUD_EXT_INT_EN_FIFO_OF_SHIFT (3)
	//!< SDI Audio Extract interrupt FIFO overflow detect shift
#define XSDIAUD_EXT_INT_EN_FIFO_OF_MASK (1 << XSDIAUD_EXT_INT_EN_FIFO_OF_SHIFT)
	//!< SDI Audio Extract interrupt FIFO overflow detect mask

#define XSDIAUD_EXT_INT_EN_PERR_SHIFT (4)
	//!< SDI Audio Extract interrupt parity error detect shift
#define XSDIAUD_EXT_INT_EN_PERR_MASK (1 << XSDIAUD_EXT_INT_EN_PERR_SHIFT)
	//!< SDI Audio Extract interrupt parity error detect mask

#define XSDIAUD_EXT_INT_EN_CERR_SHIFT (5)
	//!< SDI Audio Extract interrupt checksum error detect shift
#define XSDIAUD_EXT_INT_EN_CERR_MASK (1 << XSDIAUD_EXT_INT_EN_CERR_SHIFT)
	//!< SDI Audio Extract interrupt checksum error detect mask

#define XSDIAUD_INT_ST_GRP_CHG_SHIFT (0)
	//!< SDI Audio interrupt status group change detect shift
#define XSDIAUD_INT_ST_GRP_CHG_MASK (1 << XSDIAUD_INT_ST_GRP_CHG_SHIFT)
	//!< SDI Audio interrupt status group change detect mask

#define XSDIAUD_EXT_INT_ST_PKT_CHG_SHIFT (1)
	//!< SDI Audio Extract interrupt status control pkt change detect shift
#define XSDIAUD_EXT_INT_ST_PKT_CHG_MASK (1 << XSDIAUD_EXT_INT_ST_PKT_CHG_SHIFT)
	//!< SDI Audio Extract interrupt status control pkt change detect mask

#define XSDIAUD_EXT_INT_ST_STS_CHG_SHIFT (2)
	//!< SDI Audio Extract interrupt status status change detect shift
#define XSDIAUD_EXT_INT_ST_STS_CHG_MASK (1 << XSDIAUD_EXT_INT_ST_STS_CHG_SHIFT)
	//!< SDI Audio Extract status change detect mask

#define XSDIAUD_EXT_INT_ST_FIFO_OF_SHIFT (3)
	//!< SDI Audio Extract interrupt status FIFO overflow detect shift
#define XSDIAUD_EXT_INT_ST_FIFO_OF_MASK (1 << XSDIAUD_EXT_INT_ST_FIFO_OF_SHIFT)
	//!< SDI Audio Extract interrupt status FIFO overflow detect mask

#define XSDIAUD_EXT_INT_ST_PERR_SHIFT (4)
	//!< SDI Audio Extract interrupt status parity error detect shift
#define XSDIAUD_EXT_INT_ST_PERR_MASK (1 << XSDIAUD_EXT_INT_ST_PERR_SHIFT)
	//!< SDI Audio Extract interrupt status parity error detect mask

#define XSDIAUD_EXT_INT_ST_CERR_SHIFT (5)
	//!< SDI Audio Extract interrupt status checksum error detect shift
#define XSDIAUD_EXT_INT_ST_CERR_MASK (1 << XSDIAUD_EXT_INT_ST_CERR_SHIFT)
	//!< SDI Audio Extract interrupt status checksum error detect mask

#define XSDIAUD_EXT_AUD_CNT_CP_EN_SHIFT (0)
	//!< SDI Audio Extract audio clock phase enable shift
#define XSDIAUD_EXT_AUD_CNT_CP_EN_MASK (1 << XSDIAUD_EXT_AUD_CNT_CP_EN_SHIFT)
	//!< SDI Audio Extract audio clock phase enable mask

#define XSDIAUD_EXT_AXIS_CHCOUNT_SHIFT (0)
	//!< SDI Audio Extract AXIS channel count shift
#define XSDIAUD_EXT_AXIS_CHCOUNT_MASK (0x1F << XSDIAUD_EXT_AXIS_CHCOUNT_SHIFT)
	//!< SDI Audio Extract AXIS channel count mask

#define XSDIAUD_EXT_DMUX_GRPS_SHIFT (0)
	//!< SDI Audio Extract DMux group select shift
#define XSDIAUD_EXT_DMUX_GRPS_MASK (0x3 << XSDIAUD_EXT_DMUX_GRPS_SHIFT)
	//!< SDI Audio Extract DMux group select mask

#define XSDIAUD_EXT_DMUX_MUTE_SHIFT (2)
	//!< SDI Audio Extract DMux mute channel shift
#define XSDIAUD_EXT_DMUX_MUTEALL_MASK (0xF << XSDIAUD_EXT_DMUX_MUTE_SHIFT)
	//!< SDI Audio Extract DMux mute all channels mask
#define XSDIAUD_EXT_DMUX_MUTE1_MASK (0x1 << XSDIAUD_EXT_DMUX_MUTE_SHIFT)
    //!< SDI Audio Extract Dmux mute channel 1 mask
#define XSDIAUD_EXT_DMUX_MUTE2_MASK (0x2 << XSDIAUD_EXT_DMUX_MUTE_SHIFT)
    //!< SDI Audio Extract Dmux mute channel 2 mask
#define XSDIAUD_EXT_DMUX_MUTE3_MASK (0x4 << XSDIAUD_EXT_DMUX_MUTE_SHIFT)
    //!< SDI Audio Extract Dmux mute channel 3 mask
#define XSDIAUD_EXT_DMUX_MUTE4_MASK (0x8 << XSDIAUD_EXT_DMUX_MUTE_SHIFT)
    //!< SDI Audio Extract Dmux mute channel 4 mask

#define XSDIAUD_EXT_PKTST_SR_SHIFT (0)
	//!< SDI Audio Extract control packet status sampling rate shift
#define XSDIAUD_EXT_PKTST_SR_MASK (0xFFF << XSDIAUD_EXT_PKTST_SR_SHIFT)
	//!< SDI Audio Extract control packet status sampling rate mask

#define XSDIAUD_EXT_PKTST_AC_SHIFT (12)
	//!< SDI Audio Extract control packet status group active channel shift
#define XSDIAUD_EXT_PKTST_AC_MASK (0xFFFF << XSDIAUD_EXT_PKTST_AC_SHIFT)
	//!< SDI Audio Extract control packet status group active channel mask

/* GUI Masks and Shifts */
#define XSDIAUD_GUI_CHAN_SHIFT (0)
	//!< SDI Audio GUI Channels parameter shift
#define XSDIAUD_GUI_CHAN_MASK (0xF << XSDIAUD_GUI_CHAN_SHIFT)
	//!< SDI Audio GUI Channels parameter mask

#define XSDIAUD_GUI_STD_SHIFT (4)
	//!< SDI Audio GUI UHD SDI standard parameter shift
#define XSDIAUD_GUI_STD_MASK (0x3 << XSDIAUD_GUI_STD_SHIFT)
	//!< SDI Audio GUI UHD Standard parameter mask

#define XSDIAUD_GUI_AUDF_SHIFT (6)
	//!< SDI Audio GUI audio function parameter shift
#define XSDIAUD_GUI_AUDF_MASK (1 << XSDIAUD_GUI_AUDF_SHIFT)
	//!< SDI Audio GUI audio function parameter mask

#define XSDIAUD_GUI_AXIE_SHIFT (7)
	//!< SDI Audio GUI AXI-lite enable parameter shift
#define XSDIAUD_GUI_AXIE_MASK (1 << XSDIAUD_GUI_AXIE_SHIFT)
	//!< SDI Audio GUI AXI-lite enable parameter mask

#define XSDIAUD_GUI_CHSTAT_EXTR_SHIFT (8)
	//!< SDI Audio GUI Channel status extract parameter shift
#define XSDIAUD_GUI_CHSTAT_EXTR_MASK (1 << XSDIAUD_GUI_CHSTAT_EXTR_SHIFT)
	//!< SDI Audio GUI channel status extract parameter mask

/**************************** Type Definitions *******************************/

/***************** Macros (In-line Functions) Definitions *********************/

/** @name Register access macro definition
 * @{
 */
#define XSdiAud_In32   Xil_In32        //!< Input Operations
#define XSdiAud_Out32  Xil_Out32       //!< Output Operations

/*****************************************************************************
 *
 * This macro reads a value from a XSdiAud register.
 * A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param  BaseAddress is the base address of the XSdiAud core instance.
 * @param  RegOffset is the register offset of the register (defined at
 *         the top of this file).
 *
 * @return The 32-bit value of the register.
 *
 * @note   C-style signature:
 *         u32 XSdiAud_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 *****************************************************************************/
#define XSdiAud_ReadReg(BaseAddress, RegOffset) \
	XSdiAud_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************
 *****************************************************************************
 *
 *
 * This macro writes a value to a XSdiAud register.
 * A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param  BaseAddress is the base address of the XSdiAud core instance.
 * @param  RegOffset is the register offset of the register (defined at
 *         the top of this file) to be written.
 * @param  Data is the 32-bit value to write into the register.
 *
 * @return None.
 *
 * @note   C-style signature:
 *         void XSdiAud_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
#define XSdiAud_WriteReg(BaseAddress, RegOffset, Data) \
	XSdiAud_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XSDIAUD_HW_H */
/** @} */
