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
 *
 * @file xspdif_hw.h
 * @addtogroup spdif_v1_0
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18  Initial release.
 * </pre>
 *
 ******************************************************************************/

#ifndef XSPDIF_HW_H
#define XSPDIF_HW_H
/* Prevent circular inclusions by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
/************************** Constant Definitions *****************************/
/** @name Register Map
*
* Register offsets for the XSpdif device.
* @{
*/
#define XSPDIF_GLOBAL_INTERRUPT_ENABLE_OFFSET 0x1C
	//!< Device Global interrupt enable register

#define XSPDIF_INTERRUPT_STATUS_REGISTER_OFFSET 0x20
	//!< IP Interrupt Status Register

#define XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET 0x28
	//!< IP interrupt enable Register

#define XSPDIF_SOFT_RESET_REGISTER_OFFSET   0x40
	//!< Soft Reset Register

#define XSPDIF_CONTROL_REGISTER_OFFSET   0x44
	//!< Control Register

#define XSPDIF_STATUS_REGISTER_OFFSET    0x48
	//!< Status Register

#define XSPDIF_CHANNEL_STATUS_REGISTER0_OFFSET      0x4C
	//!< Audio Channel Status bits 0 to 31

#define XSPDIF_CHANNEL_A_USER_DATA_REGISTER0_OFFSET      0x64
	//!< Channel A user data bits 0 to 31

#define XSPDIF_CHANNEL_B_USER_DATA_REGISTER0_OFFSET      0x7C
//!< Channel B user data bits 0 to 31
/* @} */

/**
* @name Core Configuration Register masks and shifts
* @{
*/
#define XSPDIF_CORE_ENABLE_SHIFT (0)
	//!< Is XSPDIF Core Enable bit shift

#define XSPDIF_CORE_ENABLE_MASK  (1 << XSPDIF_CORE_ENABLE_SHIFT)
	//!< Is XSPDIF Core Enable bit mask

#define XSPDIF_FIFO_FLUSH_SHIFT (1)
	//!< Is XSPDIF Reset FIFO bit shift

#define XSPDIF_FIFO_FLUSH_MASK  (1 << XSPDIF_FIFO_FLUSH_SHIFT)
	//!< Is XSPDIF Reset FIFO bit mask

#define XSPDIF_CLOCK_CONFIG_BITS_SHIFT (2)
	//!< Is XSPDIF clock configuration bits shift

#define XSPDIF_CLOCK_CONFIG_BITS_MASK ((0xF) << XSPDIF_CLOCK_CONFIG_BITS_SHIFT)
    //!< Is XSPDIF clock configuration bits mask

#define XSPDIF_SAMPLE_CLOCK_COUNT_SHIFT (0)
    //!< XSPDIF sample clock count shift.
#define XSPDIF_SAMPLE_CLOCK_COUNT_MASK ((0X3FF) << XSPDIF_SAMPLE_CLOCK_COUNT_SHIFT)
    //!< XSPDIF sample clock count mask.
/**
* @name Interrupt masks and shifts
* @{
*/
#define XSPDIF_TX_OR_RX_FIFO_FULL_SHIFT (0)
	//!< Transmitter or Receiver FIFO Full Interrupt bit shift

#define XSPDIF_TX_OR_RX_FIFO_FULL_MASK  (1 << XSPDIF_TX_OR_RX_FIFO_FULL_SHIFT)
	//!< Transmitter or Receiver FIFO Full Interrupt bit mask

#define XSPDIF_TX_OR_RX_FIFO_EMPTY_SHIFT (1)
	//!< Transmitter or Receiver FIFO Empty Interrupt bit shift

#define XSPDIF_TX_OR_RX_FIFO_EMPTY_MASK  (1 << XSPDIF_TX_OR_RX_FIFO_EMPTY_SHIFT)
	//!< Transmitter or Receiver FIFO Empty Interrupt bit mask

#define XSPDIF_START_OF_BLOCK_SHIFT (2)
	//!< Start of Block Interrupt bit mask ( in Receive mode)

#define XSPDIF_START_OF_BLOCK_MASK  (1 << XSPDIF_START_OF_BLOCK_SHIFT)
	//!< Transmitter or Receiver FIFO Full Interrupt bit shift

#define XSPDIF_BMC_ERROR_SHIFT (3)
	//!< BMC Error Interrupt bit shift

#define XSPDIF_BMC_ERROR_MASK (1 << XSPDIF_BMC_ERROR_SHIFT)
	//!< BMC Error Interrupt bit mask

#define XSPDIF_PREAMBLE_ERROR_SHIFT (4)
	//!< Preamble error Interrupt bit shift

#define XSPDIF_PREAMBLE_ERROR_MASK (1 << XSPDIF_PREAMBLE_ERROR_SHIFT)
	//!< Preamble error Interrupt bit mask

#define XSPDIF_GINTR_ENABLE_SHIFT (31)
	//!< Global interrupt enable bit shift

#define XSPDIF_GINTR_ENABLE_MASK (1 << XSPDIF_GINTR_ENABLE_SHIFT)
//!< Global interrupt enable bit mask
/* @} */
#define XSPDIF_CLK_4 4
       //!< Clock divide by 4
#define XSPDIF_CLK_8 8
       //!< Clock divide by 8
#define XSPDIF_CLK_16 16
       //!< Clock divide by 16
#define XSPDIF_CLK_24 24
       //!< Clock divide by 24
#define XSPDIF_CLK_32 32
       //!< Clock divide by 32
#define XSPDIF_CLK_48 48
       //!< Clock divide by 48
#define XSPDIF_CLK_64 64
       //!< Clock divide by 64
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XSpdif_In32   Xil_In32        //!< Input Operations
#define XSpdif_Out32  Xil_Out32       //!< Output Operations

/*****************************************************************************/
/**
*
* This macro reads a value from a XSpdif register.
* A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param  BaseAddress is the base address of the XSpdif core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file).
*
* @return The 32-bit value of the register.
*
* @note   C-style signature:
*         u32 XSpdif_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XSpdif_ReadReg(BaseAddress, RegOffset) \
	XSpdif_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a XSpdif register.
* A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param  BaseAddress is the base address of the XSpdif core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file) to be written.
* @param  Data is the 32-bit value to write into the register.
*
* @return None.
*
* @note   C-style signature:
*         void XSpdif_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XSpdif_WriteReg(BaseAddress, RegOffset, Data) \
	XSpdif_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

/*@}*/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XSPDIF_HW_H */
/** @} */

