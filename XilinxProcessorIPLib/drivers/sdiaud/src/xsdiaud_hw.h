/*******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 * @file xsdiaud_hw.h
 * @addtogroup sdiaud_v2_1
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
 * 2.0   vve    09/27/18  Add 32 channel support
 *                        Add support for channel status extraction logic both
 *                        on embed and extract side.
 *                        Add APIs to detect group change, sample rate change,
 *                        active channel change
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

#define XSDIAUD_CORE_VERSION_REG_OFFSET 0x08
	//!< SDI Audio Core version register offset

#define XSDIAUD_INT_STS_REG_OFFSET 0x10
	//!< SDI Audio Interrupt status register offset

#define XSDIAUD_EMB_VID_CNTRL_REG_OFFSET 0X14
	//!< SDI Audio Embed Video control register offset

#define XSDIAUD_AUD_CNTRL_REG_OFFSET 0x18
	//!< SDI Audio Audio control register offset

#define XSDIAUD_VALID_CH_REG_OFFSET 0x20
	//!< SDI Audio valid channels register offset

#define XSDIAUD_MUTE_CH_REG_OFFSET 0x30
	//!< SDI Audio mute channels register offset

#define XSDIAUD_ACT_GRP_PRES_REG_OFFSET 0X40
	//!< SDI Audio group presence register offset

#define XSDIAUD_EXT_FIFO_OVFLW_ST_REG_OFFSET 0X44
	//!< SDI Audio Extract FIFO overflow status register offset

#define XSDIAUD_EXT_CH_STAT0_REG_OFFSET 0X48
	//!< SDI Audio Extract channel status register0 offset

#define XSDIAUD_ACT_CH_STAT_REG_OFFSET 0X60
	//!< SDI Audio active channel status register offset

#define XSDIAUD_SR_STAT_REG_OFFSET 0X70
	//!< SDI Audio sample rate status register offset

#define XSDIAUD_ASX_STAT_REG_OFFSET 0X80
	//!< SDI Audio async channel pair status register offset

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

#define XSDIAUD_EMB_VID_CNT_FAMILY_SHIFT (0)
	//!< SDI Audio Embed video control family shift
#define XSDIAUD_EMB_VID_CNT_FAMILY_MASK \
		(0xF << XSDIAUD_EMB_VID_CNT_FAMILY_SHIFT)
	//!< SDI Audio Embed video control family mask

#define XSDIAUD_EMB_VID_CNT_RATE_SHIFT (4)
	//!< SDI Audio Embed video control rate shift
#define XSDIAUD_EMB_VID_CNT_RATE_MASK \
		(0xF << XSDIAUD_EMB_VID_CNT_RATE_SHIFT)
	//!< SDI Audio Embed video control rate mask

#define XSDIAUD_EMB_VID_CNT_SCAN_SHIFT (8)
	//!< SDI Audio Embed video control scan shift
#define XSDIAUD_EMB_VID_CNT_SCAN_MASK (1 << XSDIAUD_EMB_VID_CNT_SCAN_SHIFT)
	//!< SDI Audio Embed video control scan mask

#define XSDIAUD_EMB_VID_CNT_ELE_SHIFT (16)
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

#define XSDIAUD_EMB_AUD_CNT_ASX_SHIFT (4)
	//!< SDI Audio Embed audio control asynchronous data flag shift
#define XSDIAUD_EMB_AUD_CNT_ASX_MASK (1 << XSDIAUD_EMB_AUD_CNT_ASX_SHIFT)
	//!< SDI Audio Embed audio control asynchronous data flag mask

#define XSDIAUD_EMB_AUD_CNT_AES_CH_PAIR_SHIFT (16)
	//!< SDI Audio Embed audio control aes channel pair shift
#define XSDIAUD_EMB_AUD_CNT_AES_CH_PAIR_MASK \
		(0xF << XSDIAUD_EMB_AUD_CNT_AES_CH_PAIR_SHIFT)
	//!< SDI Audio Embed audio control aes channel pair mask

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
#define XSDIAUD_GRP_PRESNT_MASK (0xFF << XSDIAUD_GRP_PRESNT_SHIFT)
	//!< SDI Audio group present value mask

#define XSDIAUD_GRP_PRESNTV_SHIFT (4)
	//!< SDI Audio group present valid shift
#define XSDIAUD_GRP_PRESNTV_MASK (1 << XSDIAUD_GRP_PRESNTV_SHIFT)
	//!< SDI Audio group present valid mask

#define XSDIAUD_INT_EN_AUD_STAT_UPDATE_SHIFT (8)
	//!< SDI Audio interrupt status update shift
#define XSDIAUD_INT_EN_AUD_STAT_UPDATE_MASK (1 << XSDIAUD_INT_EN_AUD_STAT_UPDATE_SHIFT)
	//!< SDI Audio interrupt status update mask

#define XSDIAUD_INT_EN_GRP_CHANGE_SHIFT (9)
	//!< SDI Audio interrupt group change detect shift
#define XSDIAUD_INT_EN_GRP_CHANGE_MASK (1 << XSDIAUD_INT_EN_GRP_CHANGE_SHIFT)
	//!< SDI Audio interrupt group change detect mask

#define XSDIAUD_INT_EN_CH_CHANGE_SHIFT (10)
	//!< SDI Audio interrupt active channel change detect shift
#define XSDIAUD_INT_EN_CH_CHANGE_MASK (1 << XSDIAUD_INT_EN_CH_CHANGE_SHIFT)
	//!< SDI Audio interrupt active channel change detect mask

#define XSDIAUD_INT_EN_SMP_RATE_CHANGE_SHIFT (11)
	//!< SDI Audio interrupt sample rate change detect shift
#define XSDIAUD_INT_EN_SMP_RATE_CHANGE_MASK (1 << XSDIAUD_INT_EN_SMP_RATE_CHANGE_SHIFT)
	//!< SDI Audio interrupt sample rate change detect mask

#define XSDIAUD_INT_EN_ASX_CHANGE_SHIFT (12)
	//!< SDI Audio interrupt asynchronous data flag value change detect shift
#define XSDIAUD_INT_EN_ASX_CHANGE_MASK (1 << XSDIAUD_INT_EN_ASX_CHANGE_SHIFT)
	//!< SDI Audio interrupt asynchronous data flag value change detect mask

#define XSDIAUD_INT_EN_AES_CS_UPDATE_SHIFT (16)
	//!< SDI Audio interrupt aes channel status value update shift
#define XSDIAUD_INT_EN_AES_CS_UPDATE_MASK (1 << XSDIAUD_INT_EN_AES_CS_UPDATE_SHIFT)
	//!< SDI Audio interrupt aes channel status value update mask

#define XSDIAUD_INT_EN_AES_CS_CHANGE_SHIFT (17)
	//!< SDI Audio interrupt aes channel status value change shift
#define XSDIAUD_INT_EN_AES_CS_CHANGE_MASK (1 << XSDIAUD_INT_EN_AES_CS_CHANGE_SHIFT)
	//!< SDI Audio interrupt aes channel status value change mask

#define XSDIAUD_EXT_INT_EN_VID_PROP_CHANGE_SHIFT (3)
	//!< SDI Audio Extract interrupt video properties change detect shift
#define XSDIAUD_EXT_INT_EN_VID_PROP_CHANGE_MASK \
	(1 << XSDIAUD_EXT_INT_EN_VID_PROP_CHANGE_SHIFT)
	//!< SDI Audio Extract interrupt video properties change detect mask

#define XSDIAUD_EXT_INT_EN_FIFO_OF_SHIFT (2)
	//!< SDI Audio Extract interrupt FIFO overflow detect shift
#define XSDIAUD_EXT_INT_EN_FIFO_OF_MASK (1 << XSDIAUD_EXT_INT_EN_FIFO_OF_SHIFT)
	//!< SDI Audio Extract interrupt FIFO overflow detect mask

#define XSDIAUD_EXT_INT_EN_CERR_SHIFT (1)
	//!< SDI Audio Extract interrupt checksum error detect shift
#define XSDIAUD_EXT_INT_EN_CERR_MASK (1 << XSDIAUD_EXT_INT_EN_CERR_SHIFT)
	//!< SDI Audio Extract interrupt checksum error detect mask

#define XSDIAUD_EXT_INT_EN_PERR_SHIFT (0)
	//!< SDI Audio Extract interrupt parity error detect shift
#define XSDIAUD_EXT_INT_EN_PERR_MASK (1 << XSDIAUD_EXT_INT_EN_PERR_SHIFT)
	//!< SDI Audio Extract interrupt parity error detect mask

#define XSDIAUD_INT_ST_AUD_STAT_UPDATE_SHIFT (8)
	//!< SDI Audio interrupt status update shift
#define XSDIAUD_INT_ST_AUD_STAT_UPDATE_MASK (1 << XSDIAUD_INT_ST_AUD_STAT_UPDATE_SHIFT)
	//!< SDI Audio interrupt status update mask

#define XSDIAUD_INT_ST_GRP_CHANGE_SHIFT (9)
	//!< SDI Audio interrupt status group change detect shift
#define XSDIAUD_INT_ST_GRP_CHANGE_MASK (1 << XSDIAUD_INT_ST_GRP_CHANGE_SHIFT)
	//!< SDI Audio interrupt status group change detect mask

#define XSDIAUD_INT_ST_CH_CHANGE_SHIFT (10)
	//!< SDI Audio interrupt status active channel change detect shift
#define XSDIAUD_INT_ST_CH_CHANGE_MASK (1 << XSDIAUD_INT_ST_CH_CHANGE_SHIFT)
	//!< SDI Audio interrupt status active channel change detect mask

#define XSDIAUD_INT_ST_SMP_RATE_CHANGE_SHIFT (11)
	//!< SDI Audio interrupt status sample rate change detect shift
#define XSDIAUD_INT_ST_SMP_RATE_CHANGE_MASK (1 << XSDIAUD_INT_ST_SMP_RATE_CHANGE_SHIFT)
	//!< SDI Audio interrupt status sample rate change detect mask

#define XSDIAUD_INT_ST_ASX_CHANGE_SHIFT (12)
	//!< SDI Audio interrupt status asynchronous data flag value change detect shift
#define XSDIAUD_INT_ST_ASX_CHANGE_MASK (1 << XSDIAUD_INT_ST_ASX_CHANGE_SHIFT)
	//!< SDI Audio interrupt status asynchronous data flag value change detect mask

#define XSDIAUD_INT_ST_AES_CS_UPDATE_SHIFT (16)
	//!< SDI Audio interrupt status aes channel status value update shift
#define XSDIAUD_INT_ST_AES_CS_UPDATE_MASK (1 << XSDIAUD_INT_ST_AES_CS_UPDATE_SHIFT)
	//!< SDI Audio interrupt status aes channel status value update mask

#define XSDIAUD_INT_ST_AES_CS_CHANGE_SHIFT (17)
	//!< SDI Audio interrupt status aes channel status value change shift
#define XSDIAUD_INT_ST_AES_CS_CHANGE_MASK (1 << XSDIAUD_INT_ST_AES_CS_CHANGE_SHIFT)
	//!< SDI Audio interrupt status aes channel status value change mask

#define XSDIAUD_EXT_INT_ST_VID_PROP_CHANGE_SHIFT (3)
	//!< SDI Audio Extract interrupt status video properties change detect shift
#define XSDIAUD_EXT_INT_ST_VID_PROP_CHANGE_MASK \
	(1 << XSDIAUD_EXT_INT_ST_VID_PROP_CHANGE_SHIFT)
	//!< SDI Audio Extract interrupt status video properties change detect mask

#define XSDIAUD_EXT_INT_ST_FIFO_OF_SHIFT (2)
	//!< SDI Audio Extract interrupt status FIFO overflow detect shift
#define XSDIAUD_EXT_INT_ST_FIFO_OF_MASK (1 << XSDIAUD_EXT_INT_ST_FIFO_OF_SHIFT)
	//!< SDI Audio Extract interrupt status FIFO overflow detect mask

#define XSDIAUD_EXT_INT_ST_CERR_SHIFT (1)
	//!< SDI Audio Extract interrupt status checksum error detect shift
#define XSDIAUD_EXT_INT_ST_CERR_MASK (1 << XSDIAUD_EXT_INT_ST_CERR_SHIFT)
	//!< SDI Audio Extract interrupt status checksum error detect mask

#define XSDIAUD_EXT_INT_ST_PERR_SHIFT (0)
	//!< SDI Audio Extract interrupt status parity error detect shift
#define XSDIAUD_EXT_INT_ST_PERR_MASK (1 << XSDIAUD_EXT_INT_ST_PERR_SHIFT)
	//!< SDI Audio Extract interrupt status parity error detect mask

#define XSDIAUD_EXT_AUD_CNT_CP_EN_SHIFT (29)
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
