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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2stx_hw.h
 * @addtogroup i2stx_v1_0
 * @{
 *
 * Hardware register & masks definition file. It defines the register interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 2.0   kar    09/28/18 Added justication enable masks and shifts.
 *                       Added left and right justification masks and shifts.
 * </pre>
 *
 *****************************************************************************/

#ifndef XI2STX_HW_H
#define XI2STX_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
*
* Register offsets for the XI2S_Transmitter device.
* @{
*/
#define XI2S_TX_CORE_VER_OFFSET  0x00 /**< Core Version Register */
#define XI2S_TX_CORE_CFG_OFFSET  0x04 /**< Core Configuration Register */
#define XI2S_TX_CORE_CTRL_OFFSET 0x08 /**< Core Control Register */

#define XI2S_TX_IRQCTRL_OFFSET   0x10 /**< Interrupt Control Register */
#define XI2S_TX_IRQSTS_OFFSET    0x14 /**< Interrupt Status Register */

#define XI2S_TX_TMR_CTRL_OFFSET   0x20 /**< I2S Timing Control Register */

#define XI2S_TX_CH01_OFFSET      0x30 /**< Audio Channel 0/1 Control Register */
#define XI2S_TX_CH23_OFFSET      0x34 /**< Audio Channel 2/3 Control Register */
#define XI2S_TX_CH45_OFFSET      0x38 /**< Audio Channel 4/5 Control Register */
#define XI2S_TX_CH67_OFFSET      0x3C /**< Audio Channel 6/7 Control Register */

#define XI2S_TX_AES_CHSTS0_OFFSET 0x50 /**< AES Channel Status 0 Register */
#define XI2S_TX_AES_CHSTS1_OFFSET 0x54 /**< AES Channel Status 1 Register */
#define XI2S_TX_AES_CHSTS2_OFFSET 0x58 /**< AES Channel Status 2 Register */
#define XI2S_TX_AES_CHSTS3_OFFSET 0x5C /**< AES Channel Status 3 Register */
#define XI2S_TX_AES_CHSTS4_OFFSET 0x60 /**< AES Channel Status 4 Register */
#define XI2S_TX_AES_CHSTS5_OFFSET 0x64 /**< AES Channel Status 5 Register */

/* @} */
/**
* @name Core Configuration Register masks and shifts
* @{
*/
#define XI2S_TX_REG_CFG_MSTR_SHIFT (0) /**< Is I2S Master bit shift */
#define XI2S_TX_REG_CFG_MSTR_MASK  \
	(1 << XI2S_TX_REG_CFG_MSTR_SHIFT) /**< Is I2S Master mask */

#define XI2S_TX_REG_CFG_NUM_CH_SHIFT    (8) /**< Maximum number of
					      channels bit shift */
#define XI2S_TX_REG_CFG_NUM_CH_MASK     \
	(0xF << XI2S_TX_REG_CFG_NUM_CH_SHIFT) /**< Maximum number
						of channels mask */

#define XI2S_TX_REG_CFG_DWDTH_SHIFT (16) /**< I2S Data Width bit shift */
#define XI2S_TX_REG_CFG_DWDTH_MASK  \
	(1 << XI2S_TX_REG_CFG_DWDTH_SHIFT) /**< I2S Data Width mask */
/* @} */
/**
* @name Core Control Register masks and shifts
* @{
*/
#define XI2S_TX_REG_CTRL_EN_SHIFT (0) /**< Module Enable bit shift */
#define XI2S_TX_REG_CTRL_EN_MASK  \
	(1 << XI2S_TX_REG_CTRL_EN_SHIFT) /**< Module Enable mask */
#define XI2S_TX_REG_CTRL_JFE_SHIFT (1)
//!< Justification Enable or Disable shift
#define XI2S_TX_REG_CTRL_JFE_MASK (1 << XI2S_TX_REG_CTRL_JFE_SHIFT)
//!< Justification Enable or Disable mask
#define XI2S_TX_REG_CTRL_LORJF_SHIFT (2)
//!< Left or Right Justification shift
#define XI2S_TX_REG_CTRL_LORJF_MASK (1 << XI2S_TX_REG_CTRL_LORJF_SHIFT)
//!< Left or Right Justification mask
/* @} */
/**
* @name Interrupt masks and shifts
* @{
*/
#define XI2S_TX_INTR_AES_BLKCMPLT_SHIFT    (0) /**< AES Block Complete
						 Interrupt bit shift */
#define XI2S_TX_INTR_AES_BLKCMPLT_MASK     \
	(1 << XI2S_TX_INTR_AES_BLKCMPLT_SHIFT) /**< AES Block Complete
						 Interrupt mask */

#define XI2S_TX_INTR_AES_BLKSYNCERR_SHIFT  (1) /**< AES Block Synchronization
						 Error Interrupt bit shift */
#define XI2S_TX_INTR_AES_BLKSYNCERR_MASK   \
	(1 << XI2S_TX_INTR_AES_BLKSYNCERR_SHIFT) /**< AES Block Synchronization
						   Error Interrupt mask */

#define XI2S_TX_INTR_AES_CHSTSUPD_SHIFT    (2) /**< AES Channel Status Updated
						 Interrupt bit shift */
#define XI2S_TX_INTR_AES_CHSTSUPD_MASK     \
	(1 << XI2S_TX_INTR_AES_CHSTSUPD_SHIFT) /**< AES Channel Status Updated
						 Interrupt mask */

#define XI2S_TX_INTR_AUDUNDRFLW_SHIFT    (3) /**< Audio Underflow Detected
					       Interrupt bit shift */
#define XI2S_TX_INTR_AUDUNDRFLW_MASK     \
	(1 << XI2S_TX_INTR_AUDUNDRFLW_SHIFT) /**< Audio Underflow Detected
					       Interrupt mask */

#define XI2S_TX_GINTR_EN_SHIFT         (31) /**< Global Interrupt
					      Enable bit shift */
#define XI2S_TX_GINTR_EN_MASK          \
	(1 << XI2S_TX_GINTR_EN_SHIFT) /**< Global Interrupt Enable mask */
/* @} */
/**
* @name I2S Timing Control Register masks and shifts
* @{
*/
#define XI2S_TX_REG_TMR_SCLKDIV_SHIFT (0) /**< SClk Divider bit shift */
#define XI2S_TX_REG_TMR_SCLKDIV_MASK  \
	(0xF << XI2S_TX_REG_TMR_SCLKDIV_SHIFT) /**< SClk Divider mask */
/* @} */

/**
* @name Audio Channel Control Register masks and shifts
* @{
*/
#define XI2S_TX_REG_CHCTRL_CHMUX_SHIFT (0) /**< Channel MUX bit shift */
#define XI2S_TX_REG_CHCTRL_CHMUX_MASK  \
	(0x7 << XI2S_TX_REG_CHCTRL_CHMUX_SHIFT) /**< Channel MUX mask */
/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XI2s_Tx_In32   Xil_In32        /**< Input Operations */
#define XI2s_Tx_Out32  Xil_Out32       /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a I2S Transmitter register.
* A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param  BaseAddress is the base address of the I2S Transmitter core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file).
*
* @return The 32-bit value of the register.
*
* @note   C-style signature:
*         u32 XI2S_Tx_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XI2s_Tx_ReadReg(BaseAddress, RegOffset) \
	XI2s_Tx_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a I2S Transmitter register.
* A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param  BaseAddress is the base address of the I2S Transmitter core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file) to be written.
* @param  Data is the 32-bit value to write into the register.
*
* @return None.
*
* @note   C-style signature:
*         void XI2S_Tx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XI2s_Tx_WriteReg(BaseAddress, RegOffset, Data) \
	XI2s_Tx_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

/*@}*/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2STX_HW_H */
/** @} */
