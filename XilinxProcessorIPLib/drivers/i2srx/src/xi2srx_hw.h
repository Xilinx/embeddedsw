/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi2srx_hw.h
* @addtogroup i2srx_v2_1
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   kar    01/25/18 Initial release.
* 2.0   kar    09/28/18 Added justication enable masks and shifts.
*                       Added left and right justification masks and shifts.
* </pre>
*
*****************************************************************************/

#ifndef XI2SRX_HW_H
#define XI2SRX_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
/************************** Constant Definitions *****************************/
/** @name Register Map
*
* Register offsets for the XI2srx device.
* @{
*/
#define XI2S_RX_CORE_VER_OFFSET  0x00 //!< Core Version Register
#define XI2S_RX_CORE_CFG_OFFSET  0x04 //!< Core Configuration Register
#define XI2S_RX_CORE_CTRL_OFFSET 0x08 //!< Core Control Register

#define XI2S_RX_IRQCTRL_OFFSET   0x10 //!< Interrupt Control Register
#define XI2S_RX_IRQSTS_OFFSET    0x14 //!< Interrupt Status Register

#define XI2S_RX_TMR_CTRL_OFFSET    0x20 //!< XI2S Timing Control Register

#define XI2S_RX_CH01_OFFSET      0x30 //!< Audio Channel 0/1 Control Register
#define XI2S_RX_CH23_OFFSET      0x34 //!< Audio Channel 2/3 Control Register
#define XI2S_RX_CH45_OFFSET      0x38 //!< Audio Channel 4/5 Control Register
#define XI2S_RX_CH67_OFFSET      0x3C //!< Audio Channel 6/7 Control Register

#define XI2S_RX_AES_CHSTS0_OFFSET 0x50 //!< AES Channel Status 0 Register
#define XI2S_RX_AES_CHSTS1_OFFSET 0x54 //!< AES Channel Status 1 Register
#define XI2S_RX_AES_CHSTS2_OFFSET 0x58 //!< AES Channel Status 2 Register
#define XI2S_RX_AES_CHSTS3_OFFSET 0x5C //!< AES Channel Status 3 Register
#define XI2S_RX_AES_CHSTS4_OFFSET 0x60 //!< AES Channel Status 4 Register
#define XI2S_RX_AES_CHSTS5_OFFSET 0x64 //!< AES Channel Status 5 Register
/* @} */
/**
* @name Core Configuration Register masks and shifts
* @{
*/
#define XI2S_RX_REG_CFG_MSTR_SHIFT (0)
//!< Is XI2S Master bit shift
#define XI2S_RX_REG_CFG_MSTR_MASK  (1 << XI2S_RX_REG_CFG_MSTR_SHIFT)
//!< Is XI2S Master mask
#define XI2S_RX_REG_CFG_NUM_CH_SHIFT    (8)
//!< Maximum number of channels bit shift
#define XI2S_RX_REG_CFG_NUM_CH_MASK     (0xF << XI2S_RX_REG_CFG_NUM_CH_SHIFT)
//!< Maximum number of channels mask
#define XI2S_RX_REG_CFG_DWDTH_SHIFT (16)
//!< XI2S Data Width bit shift
#define XI2S_RX_REG_CFG_DWDTH_MASK  (1 << XI2S_RX_REG_CFG_DWDTH_SHIFT)
//!< XI2S Data Width mask
/* @} */
/**
* @name Core Control Register masks and shifts
* @{
*/
#define XI2S_RX_REG_CTRL_EN_SHIFT (0)
//!< Module Enable bit shift
#define XI2S_RX_REG_CTRL_EN_MASK  (1 << XI2S_RX_REG_CTRL_EN_SHIFT)
//!< Module Enable mask
#define XI2S_RX_REG_CTRL_JFE_SHIFT (1)
//!< Justification Enable or Disable shift
#define XI2S_RX_REG_CTRL_JFE_MASK (1 << XI2S_RX_REG_CTRL_JFE_SHIFT)
//!< Justification Enable or Disable mask
#define XI2S_RX_REG_CTRL_LORJF_SHIFT (2)
//!< Left or Right Justification shift
#define XI2S_RX_REG_CTRL_LORJF_MASK (1 << XI2S_RX_REG_CTRL_LORJF_SHIFT)
//!< Left or Right Justification mask

#define XI2S_RX_REG_CTRL_LATCH_CHSTS_SHIFT (16)
//!< Latch AES Channel Status bit shift
#define XI2S_RX_REG_CTRL_LATCH_CHSTS_MASK  \
       	(1 << XI2S_RX_REG_CTRL_LATCH_CHSTS_SHIFT)
//!< Latch AES Channel Status mask
/* @} */
/**
* @name Interrupt masks and shifts
* @{
*/
#define XI2S_RX_INTR_AES_BLKCMPLT_SHIFT    (0)
//!< AES Block Complete Interrupt bit shift
#define XI2S_RX_INTR_AES_BLKCMPLT_MASK    (1 << XI2S_RX_INTR_AES_BLKCMPLT_SHIFT)
//!< AES Block Complete Interrupt mask

#define XI2S_RX_INTR_AUDOVRFLW_SHIFT     (1)
//!< Audio Overflow Detected Interrupt bit shift
#define XI2S_RX_INTR_AUDOVRFLW_MASK      (1 << XI2S_RX_INTR_AUDOVRFLW_SHIFT)
//!< Audio Overflow Detected Interrupt mask

#define XI2S_RX_GINTR_EN_SHIFT         (31)
//!< Global Interrupt Enable bit shift
#define XI2S_RX_GINTR_EN_MASK          (1 << XI2S_RX_GINTR_EN_SHIFT)
//!< Global Interrupt Enable mask
/* @} */
/**
* @name XI2S Timing Control Register masks and shifts
* @{
*/
#define XI2S_RX_REG_TMR_SCLKDIV_SHIFT (0)
//!< SClk Divider bit shift
#define XI2S_RX_REG_TMR_SCLKDIV_MASK  (0xF << XI2S_RX_REG_TMR_SCLKDIV_SHIFT)
//!< SClk Divider mask
/* @} */
/**
* @name Audio Channel Control Register masks and shifts
* @{
*/
#define XI2S_RX_REG_CHCTRL_CHMUX_SHIFT (0)
//!< Channel MUX bit shift

#define XI2S_RX_REG_CHCTRL_CHMUX_MASK  (0x7 << XI2S_RX_REG_CHCTRL_CHMUX_SHIFT)
//!< Channel MUX mask
/* @} */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XI2s_Rx_In32   Xil_In32        //!< Input Operations
#define XI2s_Rx_Out32  Xil_Out32       //!< Output Operations

/*****************************************************************************/
/**
*
* This macro reads a value from a XI2s Receiver register.
* A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param  BaseAddress is the base address of the XI2s Receiver core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file).
*
* @return The 32-bit value of the register.
*
* @note   C-style signature:
*         u32 XI2s_Rx_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XI2s_Rx_ReadReg(BaseAddress, RegOffset) \
XI2s_Rx_In32((BaseAddress) + ((u32)RegOffset))
/*****************************************************************************/
/**
*
* This macro writes a value to a XI2s Receiver register.
* A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param  BaseAddress is the base address of the XI2s Receiver core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file) to be written.
* @param  Data is the 32-bit value to write into the register.
*
* @return None.
*
* @note   C-style signature:
*         void XI2s_Rx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XI2s_Rx_WriteReg(BaseAddress, RegOffset, Data) \
XI2s_Rx_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2SRX_HW_H */
/** @} */
