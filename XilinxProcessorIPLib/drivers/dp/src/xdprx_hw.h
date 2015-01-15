/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdprx_hw.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xdprx.h.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * </pre>
 *
*******************************************************************************/

#ifndef XDPRX_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPRX_HW_H_

/***************************** Include Files **********************************/

#include "xil_io.h"

/************************** Constant Definitions ******************************/

/** @name DPRX core registers: Receiver core configuration.
  * @{
  */
#define XDPRX_LINK_ENABLE		0x000	/**< Enable the receiver
							core. */
#define XDPRX_AUX_CLK_DIVIDER		0x004	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDPRX_DTG_ENABLE		0x00C	/**< Enables the display timing
							generator (DTG). */
#define XDPRX_USER_PIXEL_WIDTH		0x010	/**< Selects the width of the
							user data input port. */
#define XDPRX_INTERRUPT_MASK		0x014	/**< Masks the specified
							interrupt sources. */
#define XDPRX_MISC_CTRL			0x018	/**< Miscellaneous control of
							RX behavior. */
#define XDPRX_SOFT_RESET		0x01C	/**< Software reset. */
/* @} */

/** @name DPRX core registers: AUX channel status.
  * @{
  */
#define XDPRX_AUX_REQ_IN_PROGRESS	0x020	/**< Indicates the receipt of an
							AUX channel request. */
#define XDPRX_REQ_ERROR_COUNT		0x024	/**< Provides a running total of
							errors detected on
							inbound AUX channel
							requests. */
#define XDPRX_REQ_COUNT			0x028	/**< Provides a running total of
							the number of AUX
							requests received. */
#define XDPRX_HPD_INTERRUPT		0x02C	/**< Instructs the DisplayPort
							RX core to assert an
							interrupt to the TX
							using the HPD signal. */
#define XDPRX_REQ_CLK_WIDTH		0x030	/**< Holds the half period of
							the recovered AUX
							clock. */
#define XDPRX_REQ_CMD			0x034	/**< Provides the most recent
							AUX command received. */
#define XDPRX_REQ_ADDRESS		0x038	/**< Contains the address field
							of the most recent AUX
							request. */
#define XDPRX_REQ_LENGTH		0x03C	/**< Contains length of the most
							recent AUX request. */
/* @} */

/** @name DPRX core registers: Interrupt registers.
  * @{
  */
#define XDPRX_INTERRUPT_CAUSE		0x040	/**< Indicates the cause of
							pending host interrupts
							for stream 1, training,
							payload allocation, and
							for the AUX channel. */
#define XDPRX_INTERRUPT_MASK_1		0x044	/**< Masks the specified
							interrupt sources. */
#define XDPRX_INTERRUPT_CAUSE_1		0x048	/**< Indicates the cause of a
							pending host interrupts
							for streams 2, 3, 4. */
#define XDPRX_HSYNC_WIDTH		0x050	/**< Controls the timing of the
							active-high horizontal
							sync pulse generated
							by the display timing
							generator (DTG). */
#define XDPRX_FAST_I2C_DIVIDER		0x060	/**< Fast I2C mode clock divider
							value. */
/* @} */

/** @name DPRX core registers: DPCD fields.
  * @{
  */
#define XDPRX_LOCAL_EDID_VIDEO		0x084	/**< Indicates the presence of
							EDID information for the
							video stream. */
#define XDPRX_LOCAL_EDID_AUDIO		0x088	/**< Indicates the presence of
							EDID information for the
							audio stream. */
#define XDPRX_REMOTE_CMD		0x08C	/**< Used for passing remote
							information to the
							DisplayPort TX. */
#define XDPRX_DEVICE_SERVICE_IRQ	0x090	/**< Indicates the DPCD
							DEVICE_SERVICE_IRQ_
							VECTOR state. */
#define XDPRX_VIDEO_UNSUPPORTED		0x094	/**< DPCD register bit to inform
							the DisplayPort TX that
							video data is not
							supported. */
#define XDPRX_AUDIO_UNSUPPORTED		0x098	/**< DPCD register bit to inform
							the DisplayPort TX that
							audio data is not
							supported. */
#define XDPRX_OVER_LINK_BW_SET		0x09C	/**< Used to override the main
							link bandwidth setting
							in the DPCD. */
#define XDPRX_OVER_LANE_COUNT_SET	0x0A0	/**< Used to override the lane
							count setting in the
							DPCD. */
#define XDPRX_OVER_TP_SET		0x0A4	/**< Used to override the link
							training pattern in the
							DPCD. */
#define XDPRX_OVER_TRAINING_LANE0_SET	0x0A8	/**< Used to override the
							TRAINING_LANE0_SET
							register in the DPCD. */
#define XDPRX_OVER_TRAINING_LANE1_SET	0x0AC	/**< Used to override the
							TRAINING_LANE1_SET
							register in the DPCD. */
#define XDPRX_OVER_TRAINING_LANE2_SET	0x0B0	/**< Used to override the
							TRAINING_LANE2_SET
							register in the DPCD. */
#define XDPRX_OVER_TRAINING_LANE3_SET	0x0B4	/**< Used to override the
							TRAINING_LANE3_SET
							register in the DPCD. */
#define XDPRX_OVER_CTRL_DPCD		0x0B8	/**< Used to enable AXI/APB
							write access to the DPCD
							capability structure. */
#define XDPRX_OVER_DOWNSPREAD_CTRL	0x0BC	/**< Used to override downspread
							control in the DPCD. */
#define XDPRX_OVER_LINK_QUAL_LANE0_SET	0x0C0	/**< Used to override the
							LINK_QUAL_LANE0_SET
							register in the DPCD. */
#define XDPRX_OVER_LINK_QUAL_LANE1_SET	0x0C4	/**< Used to override the
							LINK_QUAL_LANE1_SET
							register in the DPCD. */
#define XDPRX_OVER_LINK_QUAL_LANE2_SET	0x0C8	/**< Used to override the
							LINK_QUAL_LANE2_SET
							register in the DPCD. */
#define XDPRX_OVER_LINK_QUAL_LANE3_SET	0x0CC	/**< Used to override the
							LINK_QUAL_LANE3_SET
							register in the DPCD. */
#define XDPRX_MST_CAP			0x0D0	/**< Used to enable or disable
							MST capability. */
#define XDPRX_SINK_COUNT		0x0D4	/**< The sink device count. */
#define XDPRX_GUID0			0x0E0	/**< Lower 4 bytes of the DPCD's
							GUID field. */
#define XDPRX_GUID1			0x0E4	/**< Bytes 4 to 7 of the DPCD's
							GUID field. */
#define XDPRX_GUID2			0x0E8	/**< Bytes 8 to 11 of the DPCD's
							GUID field. */
#define XDPRX_GUID3			0x0EC	/**< Upper 4 bytes of the DPCD's
							GUID field. */
#define XDPRX_OVER_GUID			0x0F0	/**< Used to override the GUID
							field in the DPCD with
							what is stored in
							XDPRX_GUID[0-3]. */
/* @} */

/** @name DPRX core registers: Core ID.
  * @{
  */
#define XDPRX_VERSION			0x0F8	/**< Version and revision of the
							DisplayPort core. */
#define XDPRX_CORE_ID			0x0FC	/**< DisplayPort protocol
							version and revision. */
/* @} */

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XDprx_In32 Xil_In32
#define XDprx_Out32 Xil_Out32
/* @} */

/******************************************************************************/
/**
 * This is a low-level function that reads from the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to be read from.
 *
 * @return	The 32-bit value of the specified register.
 *
 * @note	C-style signature:
 *		u32 XDprx_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XDprx_ReadReg(BaseAddress, RegOffset) \
					XDprx_In32((BaseAddress) + (RegOffset))

/******************************************************************************/
/**
 * This is a low-level function that writes to the specified register.
 *
 * @param	BaseAddress is the base address of the device.
 * @param	RegOffset is the register offset to write to.
 * @param	Data is the 32-bit data to write to the specified register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 *		void XDprx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XDprx_WriteReg(BaseAddress, RegOffset, Data) \
				XDprx_Out32((BaseAddress) + (RegOffset), (Data))

#endif /* XDPRX_HW_H_ */
