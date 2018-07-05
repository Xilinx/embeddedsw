/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdppsu_hw.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xdppsu.h.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  23/01/17 Initial release.
 * 1.1   aad  10/04/17 Removed un-applicable registers
 * </pre>
 *
*******************************************************************************/

#ifndef XDPPSU_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPPSU_HW_H_

/***************************** Include Files **********************************/

#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions ******************************/

/******************************************************************************/
/**
 * Address mapping for the DisplayPort TX core.
 *
*******************************************************************************/
/** @name DPPSU core registers: Link configuration field.
  * @{
  */
#define XDPPSU_LINK_BW_SET		0x0000	/**< Set main link bandwidth
							setting. */
#define XDPPSU_LANE_COUNT_SET		0x0004	/**< Set lane count setting. */
#define XDPPSU_ENHANCED_FRAME_EN		0x0008	/**< Enable enhanced framing
							symbol sequence. */
#define XDPPSU_TRAINING_PATTERN_SET	0x000C	/**< Set the link training
							pattern. */
#define XDPPSU_LINK_QUAL_PATTERN_SET	0x0010	/**< Transmit the link quality
							pattern. */
#define XDPPSU_SCRAMBLING_DISABLE	0x0014	/**< Disable scrambler and
							transmit all symbols. */
#define XDPPSU_DOWNSPREAD_CTRL		0x0018	/**< Enable a 0.5% spreading of
							the clock. */
#define XDPPSU_SOFT_RESET		0x001C	/**< Software reset. */
/* @} */

/** @name DPPSU core registers: 80-bit custom patterns for link quality test.
  * @{
  */
#define XDPPSU_COMP_PATTERN_80BIT_1	0x0020	/**< Bits [31:0] of the 80-bit
							custom pattern. */
#define XDPPSU_COMP_PATTERN_80BIT_2	0x0024	/**< Bits [63:32] of the 80-bit
							custom pattern. */
#define XDPPSU_COMP_PATTERN_80BIT_3	0x0028	/**< Bits [79:64] of the 80-bit
							custom pattern. */
/* @} */

/** @name DPPSU core registers: Core enables.
  * @{
  */
#define XDPPSU_ENABLE			0x0080	/**< Enable the basic operations
							of the DisplayPort TX
							core or output stuffing
							symbols if disabled. */
#define XDPPSU_ENABLE_MAIN_STREAM	0x0084	/**< Enable transmission of main
							link video info. */
#define XDPPSU_FORCE_SCRAMBLER_RESET	0x00C0	/**< Force a scrambler reset. */
/* @} */

/** @name DPPSU core registers: Core ID.
  * @{
  */
#define XDPPSU_VERSION			0x00F8	/**< Core version. */
#define XDPPSU_CORE_ID			0x00FC	/**< DisplayPort revision. */
/* @} */

/** @name DPPSU core registers: AUX channel interface.
  * @{
  */
#define XDPPSU_AUX_CMD			0x0100	/**< Initiates AUX commands. */
#define XDPPSU_AUX_WRITE_FIFO		0x0104	/**< Write data for the current
							AUX command. */
#define XDPPSU_AUX_ADDRESS		0x0108	/**< Specifies the address of
							current AUX command. */
#define XDPPSU_AUX_CLK_DIVIDER		0x010C	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDPPSU_TX_USER_FIFO_OVERFLOW	0x0110	/**< Indicates an overflow in
							user FIFO. */
#define XDPPSU_INTERRUPT_SIG_STATE	0x0130	/**< The raw signal values for
							interupt events. */
#define XDPPSU_AUX_REPLY_DATA		0x0134	/**< Reply data received during
							the AUX reply. */
#define XDPPSU_AUX_REPLY_CODE		0x0138	/**< Reply code received from
							the most recent AUX
							command. */
#define XDPPSU_AUX_REPLY_COUNT		0x013C	/**< Number of reply
							transactions receieved
							over AUX. */
#define XDPPSU_REPLY_DATA_COUNT		0x0148	/**< Total number of data bytes
							actually received during
							a transaction. */
#define XDPPSU_REPLY_STATUS		0x014C	/**< Reply status of most recent
							AUX transaction. */
#define XDPPSU_HPD_DURATION		0x0150	/**< Duration of the HPD pulse
							in microseconds. */
/* @} */

/** @name DPPSU core registers: Main stream attributes.
  * @{
  */
#define XDPPSU_MAIN_STREAM_HTOTAL	0x0180	/**< Total number of clocks in
							the horizontal framing
							period. */
#define XDPPSU_MAIN_STREAM_VTOTAL	0x0184	/**< Total number of lines in
							the video frame. */
#define XDPPSU_MAIN_STREAM_POLARITY	0x0188	/**< Polarity for the video
							sync signals. */
#define XDPPSU_MAIN_STREAM_HSWIDTH	0x018C	/**< Width of the horizontal
							sync pulse. */
#define XDPPSU_MAIN_STREAM_VSWIDTH	0x0190	/**< Width of the vertical sync
							pulse. */
#define XDPPSU_MAIN_STREAM_HRES		0x0194	/**< Number of active pixels per
							line (the horizontal
							resolution). */
#define XDPPSU_MAIN_STREAM_VRES		0x0198	/**< Number of active lines (the
							vertical resolution). */
#define XDPPSU_MAIN_STREAM_HSTART	0x019C	/**< Number of clocks between
							the leading edge of the
							horizontal sync and the
							start of active data. */
#define XDPPSU_MAIN_STREAM_VSTART	0x01A0	/**< Number of lines between the
							leading edge of the
							vertical sync and the
							first line of active
							data. */
#define XDPPSU_MAIN_STREAM_MISC0		0x01A4	/**< Miscellaneous stream
							attributes. */
#define XDPPSU_MAIN_STREAM_MISC1		0x01A8	/**< Miscellaneous stream
							attributes. */
#define XDPPSU_M_VID			0x01AC	/**< M value for the video
							stream as computed by
							the source core in
							asynchronous clock
							mode. Must be written
							in synchronous mode. */
#define XDPPSU_TU_SIZE			0x01B0	/**< Size of a transfer unit in
							the framing logic. */
#define XDPPSU_N_VID			0x01B4	/**< N value for the video
							stream as computed by
							the source core in
							asynchronous clock mode.
							Must be written in
							synchronous mode. */
#define XDPPSU_USER_PIXEL_WIDTH		0x01B8	/**< Selects the width of the
							user data input port. */
#define XDPPSU_USER_DATA_COUNT_PER_LANE	0x01BC	/**< Used to translate the
							number of pixels per
							line to the native
							internal 16-bit
							datapath. */
#define XDPPSU_MIN_BYTES_PER_TU		0x01C4	/**< The minimum number of bytes
							per transfer unit. */
#define XDPPSU_FRAC_BYTES_PER_TU		0x01C8	/**< The fractional component
							when calculated the
							XDPPSU_MIN_BYTES_PER_TU
							register value. */
#define XDPPSU_INIT_WAIT			0x01CC	/**< Number of initial wait
							cycles at the start of a
							new line by the framing
							logic, allowing enough
							data to be buffered in
							the input FIFO. */
/* @} */

/** @name DPPSU core registers: PHY configuration status.
  * @{
  */
#define XDPPSU_PHY_CONFIG		0x0200	/**< Transceiver PHY reset and
							configuration. */
#define XDPPSU_PHY_TRANSMIT_PRBS7	0x0230	/**< Enable pseudo random bit
							sequence 7 pattern
							transmission for link
							quality assessment. */
#define XDPPSU_PHY_CLOCK_SELECT		0x0234	/**< Instructs the PHY PLL to
							generate the proper
							clock frequency for the
							required link rate. */
#define XDPPSU_TX_PHY_POWER_DOWN		0x0238	/**< Controls PHY power down. */
#define XDPPSU_PHY_PRECURSOR_LANE_0	0x024C	/**< Controls the pre-cursor
							level. */
#define XDPPSU_PHY_PRECURSOR_LANE_1	0x0250	/**< Controls the pre-cursor
							level. */
#define XDPPSU_PHY_STATUS		0x0280	/**< Current PHY status. */
/* @} */

/** @name DPPSU core registers: DisplayPort audio.
  * @{
  */
#define XDPPSU_TX_AUDIO_CONTROL		0x0300	/**< Enables audio stream
							packets in main link and
							buffer control. */
#define XDPPSU_TX_AUDIO_CHANNELS		0x0304	/**< Used to input active
							channel count. */
#define XDPPSU_TX_AUDIO_INFO_DATA	0x0308	/**< Word formatted as per
							CEA 861-C info frame. */
#define XDPPSU_TX_AUDIO_MAUD		0x0328	/**< M value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDPPSU_TX_AUDIO_NAUD		0x032C	/**< N value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDPPSU_TX_AUDIO_EXT_DATA		0x0330	/**< Word formatted as per
							extension packet. */
/* @} */

/** @name DPPSU core registers: Interrupts.
  * @{
  */
#define XDPPSU_INTR_STATUS		0x03A0	/**< Status for interrupt
							events. */
#define XDPPSU_INTR_MASK			0x03A4	/**< Masks the specified
							interrupt sources. */
#define XDPPSU_INTR_EN			0x03A8	/**< Interrupt enable
							register. */
#define XDPPSU_INTR_DIS			0x03AC	/**< Interrupt disable
							register. */
/******************************************************************************/

/** @name DPPSU core masks, shifts, and register values.
  * @{
  */
/* 0x000: LINK_BW_SET */
#define XDPPSU_LINK_BW_SET_162GBPS	0x06	/**< 1.62 Gbps link rate. */
#define XDPPSU_LINK_BW_SET_270GBPS	0x0A	/**< 2.70 Gbps link rate. */
#define XDPPSU_LINK_BW_SET_540GBPS	0x14	/**< 5.40 Gbps link rate. */
/* 0x001: LANE_COUNT_SET */
#define XDPPSU_LANE_COUNT_SET_1		0x01	/**< Lane count of 1. */
#define XDPPSU_LANE_COUNT_SET_2		0x02	/**< Lane count of 2. */
/* 0x00C: TRAINING_PATTERN_SET */
#define XDPPSU_TRAINING_PATTERN_SET_OFF	0x0	/**< Training off. */
#define XDPPSU_TRAINING_PATTERN_SET_TP1	0x1	/**< Training pattern 1 used for
							clock recovery. */
#define XDPPSU_TRAINING_PATTERN_SET_TP2	0x2	/**< Training pattern 2 used for
							channel equalization. */
#define XDPPSU_TRAINING_PATTERN_SET_TP3	0x3	/**< Training pattern 3 used for
							channel equalization for
							cores with DP v1.2. */
/* 0x010: LINK_QUAL_PATTERN_SET */
#define XDPPSU_LINK_QUAL_PATTERN_SET_OFF	0x0	/**< Link quality test pattern
							not transmitted. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_D102_TEST \
					0x1	/**< D10.2 unscrambled test
							pattern transmitted. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_SER_MES \
					0x2	/**< Symbol error rate
							measurement pattern
							transmitted. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_PRBS7 \
					0x3	/**< Pseudo random bit sequence
							7 transmitted. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_80B_CUSTOM \
					0x4	/**< 80-bit custom pattern. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_HBR2_COMP \
					0x5	/**< HBR2 compliance pattern. */
#define XDPPSU_LINK_QUAL_PATTERN_SET_EXT_MASK \
					0x4	/**< Used for HBR2 compliance
							and 80-bit custom
							patterns. */
/* 0x0F8 : VERSION_REGISTER */
#define XDPPSU_VERSION_INTER_REV_MASK \
				0x0000000F	/**< Internal revision. */
#define XDPPSU_VERSION_CORE_PATCH_MASK \
				0x00000030	/**< Core patch details. */
#define XDPPSU_VERSION_CORE_PATCH_SHIFT \
				8		/**< Shift bits for core patch
							details. */
#define XDPPSU_VERSION_CORE_VER_REV_MASK \
				0x000000C0	/**< Core version revision. */
#define XDPPSU_VERSION_CORE_VER_REV_SHIFT \
				12		/**< Shift bits for core version
							revision. */
#define XDPPSU_VERSION_CORE_VER_MNR_MASK \
				0x00000F00	/**< Core minor version. */
#define XDPPSU_VERSION_CORE_VER_MNR_SHIFT \
				16		/**< Shift bits for core minor
							version. */
#define XDPPSU_VERSION_CORE_VER_MJR_MASK \
				0x0000F000	/**< Core major version. */
#define XDPPSU_VERSION_CORE_VER_MJR_SHIFT \
				24		/**< Shift bits for core major
							version. */
/* 0x0FC : CORE_ID */
#define XDPPSU_CORE_ID_TYPE_MASK	0x0000000F	/**< Core type. */
#define XDPPSU_CORE_ID_TYPE_TX	0x0		/**< Core is a transmitter. */
#define XDPPSU_CORE_ID_TYPE_RX	0x1		/**< Core is a receiver. */
#define XDPPSU_CORE_ID_DP_REV_MASK \
				0x000000F0	/**< DisplayPort protocol
							revision. */
#define XDPPSU_CORE_ID_DP_REV_SHIFT \
				8		/**< Shift bits for DisplayPort
							protocol revision. */
#define XDPPSU_CORE_ID_DP_MNR_VER_MASK \
				0x00000F00	/**< DisplayPort protocol minor
							version. */
#define XDPPSU_CORE_ID_DP_MNR_VER_SHIFT \
				16		/**< Shift bits for DisplayPort
							protocol major
							version. */
#define XDPPSU_CORE_ID_DP_MJR_VER_MASK \
				0x0000F000	/**< DisplayPort protocol major
							version. */
#define XDPPSU_CORE_ID_DP_MJR_VER_SHIFT \
				24		/**< Shift bits for DisplayPort
							protocol major
							version. */
/* 0x100 AUX_CMD */
#define XDPPSU_AUX_CMD_NBYTES_TRANSFER_MASK \
				0x0000000F	/**< Number of bytes to transfer
							with the current AUX
							command. */
#define XDPPSU_AUX_CMD_MASK	0x00000F00	/**< AUX command. */
#define XDPPSU_AUX_CMD_SHIFT		8	/**< Shift bits for command. */
#define XDPPSU_AUX_CMD_I2C_WRITE		0x0	/**< I2C-over-AUX write
							command. */
#define XDPPSU_AUX_CMD_I2C_READ		0x1	/**< I2C-over-AUX read
							command. */
#define XDPPSU_AUX_CMD_I2C_WRITE_STATUS	0x2	/**< I2C-over-AUX write status
							command. */
#define XDPPSU_AUX_CMD_I2C_WRITE_MOT	0x4	/**< I2C-over-AUX write MOT
							(middle-of-transaction)
							command. */
#define XDPPSU_AUX_CMD_I2C_READ_MOT	0x5	/**< I2C-over-AUX read MOT
							(middle-of-transaction)
							command. */
#define XDPPSU_AUX_CMD_I2C_WRITE_STATUS_MOT \
					0x6	/**< I2C-over-AUX write status
							MOT (middle-of-
							transaction) command. */
#define XDPPSU_AUX_CMD_WRITE		0x8	/**< AUX write command. */
#define XDPPSU_AUX_CMD_READ		0x9	/**< AUX read command. */
#define XDPPSU_AUX_CMD_ADDR_ONLY_TRANSFER_EN \
				0x00001000	/**< Address only transfer
							enable (STOP will be
							sent after command). */
/* 0x10C: AUX_CLK_DIVIDER */
#define XDPPSU_AUX_CLK_DIVIDER_VAL_MASK \
				0x000000FF	/**< Clock divider value. */
#define XDPPSU_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK \
				0x0000FF00	/**< AUX (noise) signal width
							filter. */
#define XDPPSU_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT \
				8		/**< Shift bits for AUX signal
							width filter. */
/* 0x130: INTERRUPT_SIG_STATE */
#define XDPPSU_INTERRUPT_SIG_STATE_HPD_STATE_MASK \
				0x00000001	/**< Raw state of the HPD pin on
							the DP connector. */
#define XDPPSU_INTERRUPT_SIG_STATE_REQUEST_STATE_MASK \
				0x00000002	/**< A request is currently
							being sent. */
#define XDPPSU_INTERRUPT_SIG_STATE_REPLY_STATE_MASK \
				0x00000004	/**< A reply is currently being
							received. */
#define XDPPSU_INTERRUPT_SIG_STATE_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
/* 0x138: AUX_REPLY_CODE */
#define XDPPSU_AUX_REPLY_CODE_ACK	0x0	/**< AUX command ACKed. */
#define XDPPSU_AUX_REPLY_CODE_I2C_ACK	0x0	/**< I2C-over-AUX command
							not ACKed. */
#define XDPPSU_AUX_REPLY_CODE_NACK	0x1	/**< AUX command not ACKed. */
#define XDPPSU_AUX_REPLY_CODE_DEFER	0x2	/**< AUX command deferred. */
#define XDPPSU_AUX_REPLY_CODE_I2C_NACK	0x4	/**< I2C-over-AUX command not
							ACKed. */
#define XDPPSU_AUX_REPLY_CODE_I2C_DEFER	0x8	/**< I2C-over-AUX command
							deferred. */
/* 0x14C: REPLY_STATUS */
#define XDPPSU_REPLY_STATUS_REPLY_RECEIVED_MASK \
				0x00000001	/**< AUX transaction is complete
							and a valid reply
							transaction received. */
#define XDPPSU_REPLY_STATUS_REPLY_IN_PROGRESS_MASK \
				0x00000002	/**< AUX reply is currently
							being received. */
#define XDPPSU_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK \
				0x00000004	/**< AUX request is currently
							being transmitted. */
#define XDPPSU_REPLY_STATUS_REPLY_ERROR_MASK \
				0x00000008	/**< Detected an error in the
							AUX reply of the most
							recent transaction. */
#define XDPPSU_REPLY_STATUS_REPLY_STATUS_STATE_MASK \
				0x00000FF0	/**< Internal AUX reply state
							machine status bits. */
#define XDPPSU_REPLY_STATUS_REPLY_STATUS_STATE_SHIFT \
				4		/**< Shift bits for the internal
							AUX reply state machine
							status. */
/* 0x188: MAIN_STREAM_POLARITY */
#define XDPPSU_MAIN_STREAM_POLARITY_HSYNC_POL_MASK \
				0x00000001	/**< Polarity of the horizontal
							sync pulse. */
#define XDPPSU_MAIN_STREAM_POLARITY_VSYNC_POL_MASK \
				0x00000002	/**< Polarity of the vertical
							sync pulse. */
#define XDPPSU_MAIN_STREAM_POLARITY_VSYNC_POL_SHIFT \
				1		/**< Shift bits for polarity of
							the vertical sync
							pulse. */
/* 0x1A4: MAIN_STREAM_MISC0 */
#define XDPPSU_MAIN_STREAM_MISC0_SYNC_CLK_MASK \
				0x00000001	/**< Synchronous clock. */
#define XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_MASK \
				0x00000006	/**< Component format. */
#define XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_SHIFT \
				1               /**< Shift bits for component
							format. */
#define XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_RGB \
				0x0		/**< Stream's component format
							is RGB. */
#define XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR422 \
				0x1		/**< Stream's component format
							is YcbCr 4:2:2. */
#define XDPPSU_MAIN_STREAM_MISC0_COMPONENT_FORMAT_YCBCR444 \
				0x2		/**< Stream's component format
							is YcbCr 4:4:4. */
#define XDPPSU_MAIN_STREAM_MISC0_DYNAMIC_RANGE_MASK \
				0x00000008	/**< Dynamic range. */
#define XDPPSU_MAIN_STREAM_MISC0_DYNAMIC_RANGE_SHIFT \
				3		/**< Shift bits for dynamic
							range. */
#define XDPPSU_MAIN_STREAM_MISC0_DYNAMIC_RANGE_CEA \
				0x00000008	/**< CEA range. */
#define XDPPSU_MAIN_STREAM_MISC0_DYNAMIC_RANGE_VESA \
				0x00000000	/**< VESA range. */
#define XDPPSU_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_MASK \
				0x00000010	/**< YCbCr colorimetry. */
#define XDPPSU_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_SHIFT \
				4		/**< Shift bits for YCbCr
							colorimetry. */
#define XDPPSU_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_ITU_BT709 \
				0x00000010    /**< ITU709 YCbCr coefficients. */
#define XDPPSU_MAIN_STREAM_MISC0_YCBCR_COLORIMETRY_ITU_BT601 \
				0x00000010    /**< ITU601 YCbCr coefficients. */
#define XDPPSU_MAIN_STREAM_MISC0_BDC_MASK \
				0x000000E0	/**< Bit depth per color
							component (BDC). */
#define XDPPSU_MAIN_STREAM_MISC0_BDC_SHIFT \
				5		/**< Shift bits for BDC.*/
#define XDPPSU_MAIN_STREAM_MISC0_BDC_6BPC \
				0x0		/**< 6 bits per component.*/
#define XDPPSU_MAIN_STREAM_MISC0_BDC_8BPC \
				0x1		/**< 8 bits per component.*/
#define XDPPSU_MAIN_STREAM_MISC0_BDC_10BPC \
				0x2		/**< 10 bits per component.*/
#define XDPPSU_MAIN_STREAM_MISC0_BDC_12BPC \
				0x3		/**< 12 bits per component.*/
#define XDPPSU_MAIN_STREAM_MISC0_BDC_16BPC \
				0x4		/**< 16 bits per component.*/
/* 0x1A8: MAIN_STREAM_MISC1 */
#define XDPPSU_MAIN_STREAM_MISC1_STEREO_VID_ATTR_MASK \
				0x00000006	/**< Stereo video attribute. */
#define XDPPSU_MAIN_STREAM_MISC1_STEREO_VID_ATTR_SHIFT \
				1		/**< Shift bits for stereo video
							attribute. */
#define XDPPSU_MAIN_STREAM_MISC1_Y_ONLY_EN_MASK \
				0x00000080	/**< Y only enable. */
/* 0x200: PHY_CONFIG */
#define XDPPSU_PHY_CONFIG_PHY_RESET_ENABLE_MASK \
				0x0000000	/**< Release reset. */
#define XDPPSU_PHY_CONFIG_PHY_RESET_MASK \
				0x0000001	/**< Hold the PHY in reset. */
#define XDPPSU_PHY_CONFIG_GTTX_RESET_MASK \
				0x0000002	/**< Hold GTTXRESET in reset. */
#define XDPPSU_PHY_CONFIG_TX_PHY_8B10BEN_MASK \
				0x0010000	/**< 8B10B encoding enable. */
#define XDPPSU_PHY_CONFIG_GT_ALL_RESET_MASK \
				0x0000003	/**< Reset GT and PHY. */
/* 0x234: PHY_CLOCK_SELECT */
#define XDPPSU_PHY_CLOCK_SELECT_162GBPS	0x1	/**< 1.62 Gbps link. */
#define XDPPSU_PHY_CLOCK_SELECT_270GBPS	0x3	/**< 2.70 Gbps link. */
#define XDPPSU_PHY_CLOCK_SELECT_540GBPS	0x5	/**< 5.40 Gbps link. */
/* 0x0220, 0x0224: XDPPSU_PHY_VOLTAGE_DIFF_LANE_[0-1] */
#define XDPPSU_VS_LEVEL_0		0x3	/**< Voltage swing level 0. */
#define XDPPSU_VS_LEVEL_1		0x7	/**< Voltage swing level 1. */
#define XDPPSU_VS_LEVEL_2		0XB	/**< Voltage swing level 2. */
#define XDPPSU_VS_LEVEL_3		0xF	/**< Voltage swing level 3. */
#define XDPPSU_VS_LEVEL_OFFSET		0x4	/**< Voltage swing
						  compensation. */
/* 0x280: PHY_STATUS */
#define XDPPSU_PHY_STATUS_RESET_LANE_0_DONE_MASK \
				0x00000001	/**< Reset done for lane 0. */
#define XDPPSU_PHY_STATUS_RESET_LANE_1_DONE_MASK \
				0x00000002	/**< Reset done for lane 1. */
#define XDPPSU_PHY_STATUS_RATE_CHANGE_LANE_0_DONE_MASK \
				0x00000004	/**< Received PHYSTATUS pulse
							from GT after rate
							change request from
							lane 0. */
#define XDPPSU_PHY_STATUS_RATE_CHANGE_LANE_1_DONE_MASK \
				0x00000008	/**< Received PHYSTATUS pulse
							from GT after rate
							change request from
							lane 1. */
#define XDPPSU_PHY_STATUS_GT_PLL_LOCK_MASK \
				0x00000010	/**< GT PLL locked status. */
#define XDPPSU_PHY_STATUS_ALL_LANES_READY_MASK \
				0x00000013	/**< All lanes are ready. */
/* 0x3A0, 0x3A4, 0x3A8, 0x3AC: INTR_[STATUS,MASK,EN,DIS] */
#define XDPPSU_INTR_HPD_IRQ_MASK \
				0x00000001	/**< Detected an IRQ framed with
							the proper timing on the
							HPD signal. */
#define XDPPSU_INTR_HPD_EVENT_MASK \
				0x00000002	/**< Detected the presence of
							the HPD signal. */
#define XDPPSU_INTR_REPLY_RECEIVED_MASK \
				0x00000004	/**< An AUX reply transaction
							has been detected. */
#define XDPPSU_INTR_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
#define XDPPSU_INTR_HPD_PULSE_DETECTED_MASK \
				0x00000010	/**< A pulse on the HPD line was
							detected. */
#define XDPPSU_INTR_EXT_PKT_TXD_MASK \
				0x00000020	/**< Extended packet has been
							transmitted and the core
							is ready to accept a new
							packet. */
#define XDPPSU_INTR_LIV_ABUF_UNDRFLW_MASK \
				0x00001000	/**< Interrupt asserted when
							live audio is enabled at
							subsystem, but the input
							does not match audio
							sample rate. */
#define XDPPSU_INTR_VBLNK_START_MASK \
				0x00002000	/**< Interrupt at start of early
							vertical blanking. */
#define XDPPSU_INTR_PIXEL1_MATCH_MASK \
				0x00004000	/**< When VCOUNT and HCOUNT from
							AV buffer manager
							register 0x078 matches
							early VCOUNT. */
#define XDPPSU_INTR_PIXEL0_MATCH_MASK \
				0x00008000	/**< When VCOUNT and HCOUNT from
							AV buffer manager
							register 0x074 matches
							early VCOUNT. */
#define XDPPSU_INTR_CHBUF5_UNDERFLW_MASK \
				0x00010000	/**< AV buffer manager channel
							buffer 5 underflow. */
#define XDPPSU_INTR_CHBUF4_UNDERFLW_MASK \
				0x00020000	/**< AV buffer manager channel
							buffer 4 underflow. */
#define XDPPSU_INTR_CHBUF3_UNDERFLW_MASK \
				0x00040000	/**< AV buffer manager channel
							buffer 3 underflow. */
#define XDPPSU_INTR_CHBUF2_UNDERFLW_MASK \
				0x00080000	/**< AV buffer manager channel
							buffer 2 underflow. */
#define XDPPSU_INTR_CHBUF1_UNDERFLW_MASK \
				0x00100000	/**< AV buffer manager channel
							buffer 1 underflow. */
#define XDPPSU_INTR_CHBUF0_UNDERFLW_MASK \
				0x00200000	/**< AV buffer manager channel
							buffer 0 underflow. */
#define XDPPSU_INTR_CHBUF5_OVERFLW_MASK \
				0x00400000	/**< AV buffer manager channel
							buffer 5 overflow. */
#define XDPPSU_INTR_CHBUF4_OVERFLW_MASK \
				0x00800000	/**< AV buffer manager channel
							buffer 4 overflow. */
#define XDPPSU_INTR_CHBUF3_OVERFLW_MASK \
				0x01000000	/**< AV buffer manager channel
							buffer 3 overflow. */
#define XDPPSU_INTR_CHBUF2_OVERFLW_MASK \
				0x02000000	/**< AV buffer manager channel
							buffer 2 overflow. */
#define XDPPSU_INTR_CHBUF1_OVERFLW_MASK \
				0x04000000	/**< AV buffer manager channel
							buffer 1 overflow. */
#define XDPPSU_INTR_CHBUF0_OVERFLW_MASK \
				0x08000000	/**< AV buffer manager channel
							buffer 0 overflow. */
#define XDPPSU_INTR_CUST_TS_2_MASK \
				0x10000000	/**< Indicates that a user
							defined custom event 2
							has triggered a
							timestamp. */
#define XDPPSU_INTR_CUST_TS_MASK \
				0x20000000	/**< Indicates that a user
							defined custom event has
							triggered a
							timestamp. */
#define XDPPSU_INTR_EXT_VSYNC_TS_MASK \
				0x40000000	/**< Indicates that an external
							VSYNC has triggered a
							timestamp. This is
							generated on every
							posedge of the external
							VSYNC signal. */
#define XDPPSU_INTR_VSYNC_TS_MASK \
				0x80000000	/**< Indicates that a VSYNC
							timestamp is available.
							This is generated on
							every VSYNC event. */
#define XDPPSU_SOFT_RESET_EN	0x1		/**< Indicates that the soft
							reset has been set */
#define XDPPSU_DP_DISABLE		0x0		/**< This field disables the
							DisplayPort core. */

#define XDPPSU_DP_ENABLE	0x1		/**< This field enables the
							DisplayPort core. */
/* @} */

/******************************************************************************/

/******************************************************************************/
/**
 * Address mapping for the DisplayPort Configuration Data (DPCD) of the
 * downstream device.
 *
*******************************************************************************/
/** @name DisplayPort Configuration Data: Receiver capability field.
  * @{
  */
#define XDPPSU_DPCD_REV					0x00000
#define XDPPSU_DPCD_MAX_LINK_RATE			0x00001
#define XDPPSU_DPCD_MAX_LANE_COUNT			0x00002
#define XDPPSU_DPCD_MAX_DOWNSPREAD			0x00003
#define XDPPSU_DPCD_NORP_PWR_V_CAP			0x00004
#define XDPPSU_DPCD_DOWNSP_PRESENT			0x00005
#define XDPPSU_DPCD_ML_CH_CODING_CAP			0x00006
#define XDPPSU_DPCD_DOWNSP_COUNT_MSA_OUI			0x00007
#define	XDPPSU_DPCD_RX_PORT0_CAP_0			0x00008
#define	XDPPSU_DPCD_RX_PORT0_CAP_1			0x00009
#define	XDPPSU_DPCD_RX_PORT1_CAP_0			0x0000A
#define	XDPPSU_DPCD_RX_PORT1_CAP_1			0x0000B
#define XDPPSU_DPCD_I2C_SPEED_CTL_CAP			0x0000C
#define XDPPSU_DPCD_EDP_CFG_CAP				0x0000D
#define XDPPSU_DPCD_TRAIN_AUX_RD_INTERVAL		0x0000E
#define XDPPSU_DPCD_ADAPTER_CAP				0x0000F
#define XDPPSU_DPCD_FAUX_CAP				0x00020
#define XDPPSU_DPCD_MSTM_CAP				0x00021
#define XDPPSU_DPCD_NUM_AUDIO_EPS			0x00022
#define	XDPPSU_DPCD_AV_GRANULARITY			0x00023
#define XDPPSU_DPCD_AUD_DEC_LAT_7_0			0x00024
#define XDPPSU_DPCD_AUD_DEC_LAT_15_8			0x00025
#define XDPPSU_DPCD_AUD_PP_LAT_7_0			0x00026
#define XDPPSU_DPCD_AUD_PP_LAT_15_8			0x00027
#define XDPPSU_DPCD_VID_INTER_LAT			0x00028
#define XDPPSU_DPCD_VID_PROG_LAT				0x00029
#define XDPPSU_DPCD_REP_LAT				0x0002A
#define XDPPSU_DPCD_AUD_DEL_INS_7_0			0x0002B
#define XDPPSU_DPCD_AUD_DEL_INS_15_8			0x0002C
#define XDPPSU_DPCD_AUD_DEL_INS_23_16			0x0002D
#define XDPPSU_DPCD_GUID					0x00030
#define XDPPSU_DPCD_RX_GTC_VALUE_7_0			0x00054
#define XDPPSU_DPCD_RX_GTC_VALUE_15_8			0x00055
#define XDPPSU_DPCD_RX_GTC_VALUE_23_16			0x00056
#define XDPPSU_DPCD_RX_GTC_VALUE_31_24			0x00057
#define XDPPSU_DPCD_RX_GTC_MSTR_REQ			0x00058
#define XDPPSU_DPCD_RX_GTC_FREQ_LOCK_DONE		0x00059
#define XDPPSU_DPCD_DOWNSP_0_CAP				0x00080
#define XDPPSU_DPCD_DOWNSP_1_CAP				0x00081
#define XDPPSU_DPCD_DOWNSP_2_CAP				0x00082
#define XDPPSU_DPCD_DOWNSP_3_CAP				0x00083
#define XDPPSU_DPCD_DOWNSP_0_DET_CAP			0x00080
#define XDPPSU_DPCD_DOWNSP_1_DET_CAP			0x00084
#define XDPPSU_DPCD_DOWNSP_2_DET_CAP			0x00088
#define XDPPSU_DPCD_DOWNSP_3_DET_CAP			0x0008C
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field.
  * @{
  */
#define XDPPSU_DPCD_LINK_BW_SET				0x00100
#define XDPPSU_DPCD_LANE_COUNT_SET			0x00101
#define XDPPSU_DPCD_TP_SET				0x00102
#define XDPPSU_DPCD_TRAINING_LANE0_SET			0x00103
#define XDPPSU_DPCD_TRAINING_LANE1_SET			0x00104
#define XDPPSU_DPCD_TRAINING_LANE2_SET			0x00105
#define XDPPSU_DPCD_TRAINING_LANE3_SET			0x00106
#define XDPPSU_DPCD_DOWNSPREAD_CTRL			0x00107
#define XDPPSU_DPCD_ML_CH_CODING_SET			0x00108
#define XDPPSU_DPCD_I2C_SPEED_CTL_SET			0x00109
#define XDPPSU_DPCD_EDP_CFG_SET				0x0010A
#define XDPPSU_DPCD_LINK_QUAL_LANE0_SET			0x0010B
#define XDPPSU_DPCD_LINK_QUAL_LANE1_SET			0x0010C
#define XDPPSU_DPCD_LINK_QUAL_LANE2_SET			0x0010D
#define XDPPSU_DPCD_LINK_QUAL_LANE3_SET			0x0010E
#define XDPPSU_DPCD_TRAINING_LANE0_1_SET2		0x0010F
#define XDPPSU_DPCD_TRAINING_LANE2_3_SET2		0x00110
#define XDPPSU_DPCD_MSTM_CTRL				0x00111
#define XDPPSU_DPCD_AUDIO_DELAY_7_0			0x00112
#define XDPPSU_DPCD_AUDIO_DELAY_15_8			0x00113
#define XDPPSU_DPCD_AUDIO_DELAY_23_6			0x00114
#define XDPPSU_DPCD_UPSTREAM_DEVICE_DP_PWR_NEED		0x00118
#define XDPPSU_DPCD_FAUX_MODE_CTRL			0x00120
#define XDPPSU_DPCD_FAUX_FORWARD_CH_DRIVE_SET		0x00121
#define XDPPSU_DPCD_BACK_CH_STATUS			0x00122
#define XDPPSU_DPCD_FAUX_BACK_CH_SYMBOL_ERROR_COUNT	0x00123
#define XDPPSU_DPCD_FAUX_BACK_CH_TRAINING_PATTERN_TIME	0x00125
#define XDPPSU_DPCD_TX_GTC_VALUE_7_0			0x00154
#define XDPPSU_DPCD_TX_GTC_VALUE_15_8			0x00155
#define XDPPSU_DPCD_TX_GTC_VALUE_23_16			0x00156
#define XDPPSU_DPCD_TX_GTC_VALUE_31_24			0x00157
#define XDPPSU_DPCD_RX_GTC_VALUE_PHASE_SKEW_EN		0x00158
#define XDPPSU_DPCD_TX_GTC_FREQ_LOCK_DONE		0x00159
#define XDPPSU_DPCD_ADAPTER_CTRL			0x001A0
#define XDPPSU_DPCD_BRANCH_DEVICE_CTRL			0x001A1
#define XDPPSU_DPCD_PAYLOAD_ALLOCATE_SET		0x001C0
#define XDPPSU_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT	0x001C1
#define XDPPSU_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT	0x001C2
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field.
  * @{
  */
#define XDPPSU_DPCD_SINK_COUNT				0x00200
#define XDPPSU_DPCD_DEVICE_SERVICE_IRQ			0x00201
#define XDPPSU_DPCD_STATUS_LANE_0_1			0x00202
#define XDPPSU_DPCD_STATUS_LANE_2_3			0x00203
#define XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED		0x00204
#define XDPPSU_DPCD_SINK_STATUS				0x00205
#define XDPPSU_DPCD_ADJ_REQ_LANE_0_1			0x00206
#define XDPPSU_DPCD_ADJ_REQ_LANE_2_3			0x00207
#define XDPPSU_DPCD_TRAINING_SCORE_LANE_0		0x00208
#define XDPPSU_DPCD_TRAINING_SCORE_LANE_1		0x00209
#define XDPPSU_DPCD_TRAINING_SCORE_LANE_2		0x0020A
#define XDPPSU_DPCD_TRAINING_SCORE_LANE_3		0x0020B
#define XDPPSU_DPCD_ADJ_REQ_PC2				0x0020C
#define XDPPSU_DPCD_FAUX_FORWARD_CH_SYMBOL_ERROR_COUNT	0x0020D
#define XDPPSU_DPCD_SYMBOL_ERROR_COUNT_LANE_0		0x00210
#define XDPPSU_DPCD_SYMBOL_ERROR_COUNT_LANE_1		0x00212
#define XDPPSU_DPCD_SYMBOL_ERROR_COUNT_LANE_2		0x00214
#define XDPPSU_DPCD_SYMBOL_ERROR_COUNT_LANE_3		0x00216
/* @} */

/** @name DisplayPort Configuration Data: Automated testing sub-field.
  * @{
  */
#define XDPPSU_DPCD_FAUX_FORWARD_CH_STATUS		0x00280
#define XDPPSU_DPCD_FAUX_BACK_CH_DRIVE_SET		0x00281
#define XDPPSU_DPCD_FAUX_BACK_CH_SYM_ERR_COUNT_CTRL	0x00282
#define XDPPSU_DPCD_PAYLOAD_TABLE_UPDATE_STATUS		0x002C0
#define XDPPSU_DPCD_VC_PAYLOAD_ID_SLOT(SlotNum) \
			(XDPPSU_DPCD_PAYLOAD_TABLE_UPDATE_STATUS + SlotNum)
/* @} */

/** @name DisplayPort Configuration Data: Sink control field.
  * @{
  */
#define XDPPSU_DPCD_SET_POWER_DP_PWR_VOLTAGE		0x00600
/* @} */

/** @name DisplayPort Configuration Data: Sideband message buffers.
  * @{
  */
#define XDPPSU_DPCD_DOWN_REQ				0x01000
#define XDPPSU_DPCD_UP_REP				0x01200
#define XDPPSU_DPCD_DOWN_REP				0x01400
#define XDPPSU_DPCD_UP_REQ				0x01600
/* @} */

/** @name DisplayPort Configuration Data: Event status indicator field.
  * @{
  */
#define XDPPSU_DPCD_SINK_COUNT_ESI			0x02002
#define XDPPSU_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0	0x02003
#define XDPPSU_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI1	0x02004
#define XDPPSU_DPCD_SINK_LINK_SERVICE_IRQ_VECTOR_ESI0	0x02005
#define XDPPSU_DPCD_SINK_LANE0_1_STATUS			0x0200C
#define XDPPSU_DPCD_SINK_LANE2_3_STATUS			0x0200D
#define XDPPSU_DPCD_SINK_ALIGN_STATUS_UPDATED_ESI	0x0200E
#define XDPPSU_DPCD_SINK_STATUS_ESI			0x0200F
/* @} */

/** @name DisplayPort Configuration Data: Field addresses and sizes.
  * @{
  */
#define XDPPSU_DPCD_RECEIVER_CAP_FIELD_START		XDPPSU_DPCD_REV
#define XDPPSU_DPCD_RECEIVER_CAP_FIELD_SIZE		0x100
#define XDPPSU_DPCD_LINK_CFG_FIELD_START			XDPPSU_DPCD_LINK_BW_SET
#define XDPPSU_DPCD_LINK_CFG_FIELD_SIZE			0x100
#define XDPPSU_DPCD_LINK_SINK_STATUS_FIELD_START		XDPPSU_DPCD_SINK_COUNT
#define XDPPSU_DPCD_LINK_SINK_STATUS_FIELD_SIZE		0x17
/* @} */

/******************************************************************************/

/** @name DisplayPort Configuration Data: Receiver capability field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00000: DPCD_REV */
#define XDPPSU_DPCD_REV_MNR_MASK				0x0F
#define XDPPSU_DPCD_REV_MJR_MASK				0xF0
#define XDPPSU_DPCD_REV_MJR_SHIFT				4
/* 0x00001: MAX_LINK_RATE */
#define XDPPSU_DPCD_MAX_LINK_RATE_162GBPS			0x06
#define XDPPSU_DPCD_MAX_LINK_RATE_270GBPS			0x0A
#define XDPPSU_DPCD_MAX_LINK_RATE_540GBPS			0x14
/* 0x00002: MAX_LANE_COUNT */
#define XDPPSU_DPCD_MAX_LANE_COUNT_MASK				0x1F
#define XDPPSU_DPCD_MAX_LANE_COUNT_1				0x01
#define XDPPSU_DPCD_MAX_LANE_COUNT_2				0x02
#define XDPPSU_DPCD_MAX_LANE_COUNT_4				0x04
#define XDPPSU_DPCD_TPS3_SUPPORT_MASK				0x40
#define XDPPSU_DPCD_ENHANCED_FRAME_SUPPORT_MASK			0x80
/* 0x00003: MAX_DOWNSPREAD */
#define XDPPSU_DPCD_MAX_DOWNSPREAD_MASK				0x01
#define XDPPSU_DPCD_NO_AUX_HANDSHAKE_LINK_TRAIN_MASK		0x40
/* 0x00005: DOWNSP_PRESENT */
#define XDPPSU_DPCD_DOWNSP_PRESENT_MASK				0x01
#define XDPPSU_DPCD_DOWNSP_TYPE_MASK				0x06
#define XDPPSU_DPCD_DOWNSP_TYPE_SHIFT				1
#define XDPPSU_DPCD_DOWNSP_TYPE_DP				0x0
#define XDPPSU_DPCD_DOWNSP_TYPE_AVGA_ADVII			0x1
#define XDPPSU_DPCD_DOWNSP_TYPE_DVI_HDMI_DPPP			0x2
#define XDPPSU_DPCD_DOWNSP_TYPE_OTHERS				0x3
#define XDPPSU_DPCD_DOWNSP_FORMAT_CONV_MASK			0x08
#define XDPPSU_DPCD_DOWNSP_DCAP_INFO_AVAIL_MASK			0x10
/* 0x00006, 0x00108: ML_CH_CODING_SUPPORT, ML_CH_CODING_SET */
#define XDPPSU_DPCD_ML_CH_CODING_MASK				0x01
/* 0x00007: DOWNSP_COUNT_MSA_OUI */
#define XDPPSU_DPCD_DOWNSP_COUNT_MASK				0x0F
#define XDPPSU_DPCD_MSA_TIMING_PAR_IGNORED_MASK			0x40
#define XDPPSU_DPCD_OUI_SUPPORT_MASK				0x80
/* 0x00008, 0x0000A: RX_PORT[0-1]_CAP_0 */
#define XDPPSU_DPCD_RX_PORTX_CAP_0_LOCAL_EDID_PRESENT_MASK	0x02
#define XDPPSU_DPCD_RX_PORTX_CAP_0_ASSOC_TO_PRECEDING_PORT_MASK	0x04
/* 0x0000C, 0x00109: I2C_SPEED_CTL_CAP, I2C_SPEED_CTL_SET */
#define XDPPSU_DPCD_I2C_SPEED_CTL_NONE				0x00
#define XDPPSU_DPCD_I2C_SPEED_CTL_1KBIPS			0x01
#define XDPPSU_DPCD_I2C_SPEED_CTL_5KBIPS			0x02
#define XDPPSU_DPCD_I2C_SPEED_CTL_10KBIPS			0x04
#define XDPPSU_DPCD_I2C_SPEED_CTL_100KBIPS			0x08
#define XDPPSU_DPCD_I2C_SPEED_CTL_400KBIPS			0x10
#define XDPPSU_DPCD_I2C_SPEED_CTL_1MBIPS			0x20
/* 0x0000E: TRAIN_AUX_RD_INTERVAL */
#define XDPPSU_DPCD_TRAIN_AUX_RD_INT_100_400US			0x00
#define XDPPSU_DPCD_TRAIN_AUX_RD_INT_4MS			0x01
#define XDPPSU_DPCD_TRAIN_AUX_RD_INT_8MS			0x02
#define XDPPSU_DPCD_TRAIN_AUX_RD_INT_12MS			0x03
#define XDPPSU_DPCD_TRAIN_AUX_RD_INT_16MS			0x04
/* 0x00020: DPCD_FAUX_CAP */
#define XDPPSU_DPCD_FAUX_CAP_MASK				0x01
/* 0x00021: MSTM_CAP */
#define XDPPSU_DPCD_MST_CAP_MASK				0x01
/* 0x00080, 0x00081|4, 0x00082|8, 0x00083|C: DOWNSP_X_(DET_)CAP */
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_MASK			0x07
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_DP			0x0
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_AVGA			0x1
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_DVI			0x2
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_HDMI			0x3
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_OTHERS			0x4
#define XDPPSU_DPCD_DOWNSP_X_CAP_TYPE_DPPP			0x5
#define XDPPSU_DPCD_DOWNSP_X_CAP_HPD_MASK			0x80
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_MASK		0xF0
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_SHIFT		4
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_60	0x1
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_50	0x2
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_60	0x3
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_50	0x4
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_60	0x5
#define XDPPSU_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_50	0x7
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDPPSU_DPCD_DOWNSP_X_DCAP_MAX_BPC_MASK			0x03
#define XDPPSU_DPCD_DOWNSP_X_DCAP_MAX_BPC_8			0x0
#define XDPPSU_DPCD_DOWNSP_X_DCAP_MAX_BPC_10			0x1
#define XDPPSU_DPCD_DOWNSP_X_DCAP_MAX_BPC_12			0x2
#define XDPPSU_DPCD_DOWNSP_X_DCAP_MAX_BPC_16			0x3
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDPPSU_DPCD_DOWNSP_X_DCAP_HDMI_DPPP_FS2FP_MASK		0x01
#define XDPPSU_DPCD_DOWNSP_X_DCAP_DVI_DL_MASK			0x02
#define XDPPSU_DPCD_DOWNSP_X_DCAP_DVI_HCD_MASK			0x04
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00100: XDPPSU_DPCD_LINK_BW_SET */
#define XDPPSU_DPCD_LINK_BW_SET_162GBPS				0x06
#define XDPPSU_DPCD_LINK_BW_SET_270GBPS				0x0A
#define XDPPSU_DPCD_LINK_BW_SET_540GBPS				0x14
/* 0x00101: LANE_COUNT_SET */
#define XDPPSU_DPCD_LANE_COUNT_SET_MASK				0x1F
#define XDPPSU_DPCD_LANE_COUNT_SET_1				0x01
#define XDPPSU_DPCD_LANE_COUNT_SET_2				0x02
#define XDPPSU_DPCD_LANE_COUNT_SET_4				0x04
#define XDPPSU_DPCD_ENHANCED_FRAME_EN_MASK			0x80
/* 0x00102: TP_SET */
#define XDPPSU_DPCD_TP_SEL_MASK					0x03
#define XDPPSU_DPCD_TP_SEL_OFF					0x0
#define XDPPSU_DPCD_TP_SEL_TP1					0x1
#define XDPPSU_DPCD_TP_SEL_TP2					0x2
#define XDPPSU_DPCD_TP_SEL_TP3					0x3
#define XDPPSU_DPCD_TP_SET_LQP_MASK				0x06
#define XDPPSU_DPCD_TP_SET_LQP_SHIFT				2
#define XDPPSU_DPCD_TP_SET_LQP_OFF				0x0
#define XDPPSU_DPCD_TP_SET_LQP_D102_TEST			0x1
#define XDPPSU_DPCD_TP_SET_LQP_SER_MES				0x2
#define XDPPSU_DPCD_TP_SET_LQP_PRBS7				0x3
#define XDPPSU_DPCD_TP_SET_REC_CLK_OUT_EN_MASK			0x10
#define XDPPSU_DPCD_TP_SET_SCRAMB_DIS_MASK			0x20
#define XDPPSU_DPCD_TP_SET_SE_COUNT_SEL_MASK			0xC0
#define XDPPSU_DPCD_TP_SET_SE_COUNT_SEL_SHIFT			6
#define XDPPSU_DPCD_TP_SET_SE_COUNT_SEL_DE_ISE			0x0
#define XDPPSU_DPCD_TP_SET_SE_COUNT_SEL_DE			0x1
#define XDPPSU_DPCD_TP_SET_SE_COUNT_SEL_ISE			0x2
/* 0x00103-0x00106: TRAINING_LANE[0-3]_SET */
#define XDPPSU_DPCD_TRAINING_LANEX_SET_VS_MASK			0x03
#define XDPPSU_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK		0x04
#define XDPPSU_DPCD_TRAINING_LANEX_SET_PE_MASK			0x18
#define XDPPSU_DPCD_TRAINING_LANEX_SET_PE_SHIFT			3
#define XDPPSU_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK		0x20
/* 0x00107: DOWNSPREAD_CTRL */
#define XDPPSU_DPCD_SPREAD_AMP_MASK				0x10
#define XDPPSU_DPCD_MSA_TIMING_PAR_IGNORED_EN_MASK		0x80
/* 0x00108: ML_CH_CODING_SET - Same as 0x00006: ML_CH_CODING_SUPPORT */
/* 0x00109: I2C_SPEED_CTL_SET - Same as 0x0000C: I2C_SPEED_CTL_CAP */
/* 0x0010F-0x00110: TRAINING_LANE[0_1-2_3]_SET2 */
#define XDPPSU_DPCD_TRAINING_LANE_0_2_SET_PC2_MASK		0x03
#define XDPPSU_DPCD_TRAINING_LANE_0_2_SET_MAX_PC2_MASK		0x04
#define XDPPSU_DPCD_TRAINING_LANE_1_3_SET_PC2_MASK		0x30
#define XDPPSU_DPCD_TRAINING_LANE_1_3_SET_PC2_SHIFT		4
#define XDPPSU_DPCD_TRAINING_LANE_1_3_SET_MAX_PC2_MASK		0x40
/* 0x00111: MSTM_CTRL */
#define XDPPSU_DPCD_MST_EN_MASK					0x01
#define XDPPSU_DPCD_UP_REQ_EN_MASK				0x02
#define XDPPSU_DPCD_UP_IS_SRC_MASK				0x03
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field masks, shifts,
  *       and register values.
  * @{
  */
/* 0x00200: SINK_COUNT */
#define XDPPSU_DPCD_SINK_COUNT_LOW_MASK				0x3F
#define XDPPSU_DPCD_SINK_CP_READY_MASK				0x40
#define XDPPSU_DPCD_SINK_COUNT_HIGH_MASK			0x80
#define XDPPSU_DPCD_SINK_COUNT_HIGH_LOW_SHIFT			1
/* 0x00202: STATUS_LANE_0_1 */
#define XDPPSU_DPCD_STATUS_LANE_0_CR_DONE_MASK			0x01
#define XDPPSU_DPCD_STATUS_LANE_0_CE_DONE_MASK			0x02
#define XDPPSU_DPCD_STATUS_LANE_0_SL_DONE_MASK			0x04
#define XDPPSU_DPCD_STATUS_LANE_1_CR_DONE_MASK			0x10
#define XDPPSU_DPCD_STATUS_LANE_1_CE_DONE_MASK			0x20
#define XDPPSU_DPCD_STATUS_LANE_1_SL_DONE_MASK			0x40
/* 0x00202: STATUS_LANE_2_3 */
#define XDPPSU_DPCD_STATUS_LANE_2_CR_DONE_MASK			0x01
#define XDPPSU_DPCD_STATUS_LANE_2_CE_DONE_MASK			0x02
#define XDPPSU_DPCD_STATUS_LANE_2_SL_DONE_MASK			0x04
#define XDPPSU_DPCD_STATUS_LANE_3_CR_DONE_MASK			0x10
#define XDPPSU_DPCD_STATUS_LANE_3_CE_DONE_MASK			0x20
#define XDPPSU_DPCD_STATUS_LANE_3_SL_DONE_MASK			0x40
/* 0x00204: LANE_ALIGN_STATUS_UPDATED */
#define XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK \
								0x01
#define XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED_DOWNSP_STATUS_CHANGED_MASK \
								0x40
#define XDPPSU_DPCD_LANE_ALIGN_STATUS_UPDATED_LINK_STATUS_UPDATED_MASK \
								0x80
/* 0x00205: SINK_STATUS */
#define XDPPSU_DPCD_SINK_STATUS_RX_PORT0_SYNC_STATUS_MASK	0x01
#define XDPPSU_DPCD_SINK_STATUS_RX_PORT1_SYNC_STATUS_MASK	0x02

/* 0x00206, 0x00207: ADJ_REQ_LANE_[0,2]_[1,3] */
#define XDPPSU_DPCD_ADJ_REQ_LANE_0_2_VS_MASK			0x03
#define XDPPSU_DPCD_ADJ_REQ_LANE_0_2_PE_MASK			0x0C
#define XDPPSU_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT			2
#define XDPPSU_DPCD_ADJ_REQ_LANE_1_3_VS_MASK			0x30
#define XDPPSU_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT			4
#define XDPPSU_DPCD_ADJ_REQ_LANE_1_3_PE_MASK			0xC0
#define XDPPSU_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT			6
/* 0x0020C: ADJ_REQ_PC2 */
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_0_MASK			0x03
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_1_MASK			0x0C
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_1_SHIFT			2
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_2_MASK			0x30
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_2_SHIFT			4
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_3_MASK			0xC0
#define XDPPSU_DPCD_ADJ_REQ_PC2_LANE_3_SHIFT			6
/* @} */

/******************************************************************************/

/******************************************************************************/
/**
 * Address mapping for the Extended Display Identification Data (EDID) of the
 * downstream device.
 *
*******************************************************************************/
/** @name Extended Display Identification Data: Field addresses and sizes.
  * @{
  */
#define XDPPSU_SEGPTR_ADDR				0x30
#define XDPPSU_EDID_ADDR				0x50
#define XDPPSU_EDID_BLOCK_SIZE				128
#define XDPPSU_EDID_DTD_DD(Num)				(0x36 + (18 * Num))
#define XDPPSU_EDID_PTM					XDPPSU_EDID_DTD_DD(0)
#define XDPPSU_EDID_EXT_BLOCK_COUNT			0x7E
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Detailed Timing Descriptor (DTD).
  * @{
  */
#define XDPPSU_EDID_DTD_PIXEL_CLK_KHZ_LSB		0x00
#define XDPPSU_EDID_DTD_PIXEL_CLK_KHZ_MSB		0x01
#define XDPPSU_EDID_DTD_HRES_LSB			0x02
#define XDPPSU_EDID_DTD_HBLANK_LSB			0x03
#define XDPPSU_EDID_DTD_HRES_HBLANK_U4			0x04
#define XDPPSU_EDID_DTD_VRES_LSB			0x05
#define XDPPSU_EDID_DTD_VBLANK_LSB			0x06
#define XDPPSU_EDID_DTD_VRES_VBLANK_U4			0x07
#define XDPPSU_EDID_DTD_HFPORCH_LSB			0x08
#define XDPPSU_EDID_DTD_HSPW_LSB			0x09
#define XDPPSU_EDID_DTD_VFPORCH_VSPW_L4			0x0A
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2			0x0B
#define XDPPSU_EDID_DTD_HIMGSIZE_MM_LSB			0x0C
#define XDPPSU_EDID_DTD_VIMGSIZE_MM_LSB			0x0D
#define XDPPSU_EDID_DTD_XIMGSIZE_MM_U4			0x0E
#define XDPPSU_EDID_DTD_HBORDER				0x0F
#define XDPPSU_EDID_DTD_VBORDER				0x10
#define XDPPSU_EDID_DTD_SIGNAL				0x11

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values.
  * @{
  */
#define XDPPSU_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK		0x0F
#define XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_MASK		0xF0
#define XDPPSU_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT		4
#define XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK		0x0F
#define XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK		0xF0
#define XDPPSU_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT		4
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK		0xC0
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK		0x30
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK		0x0C
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK		0x03
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT		6
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT		4
#define XDPPSU_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT		2
#define XDPPSU_EDID_DTD_XIMGSIZE_MM_U4_VIMGSIZE_MM_MASK		0x0F
#define XDPPSU_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_MASK		0xF0
#define XDPPSU_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_SHIFT	4
#define XDPPSU_EDID_DTD_SIGNAL_HPOLARITY_MASK			0x02
#define XDPPSU_EDID_DTD_SIGNAL_VPOLARITY_MASK			0x04
#define XDPPSU_EDID_DTD_SIGNAL_HPOLARITY_SHIFT			1
#define XDPPSU_EDID_DTD_SIGNAL_VPOLARITY_SHIFT			2
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       DisplayID extension block.
  * @{
  */
#define XDPPSU_EDID_EXT_BLOCK_TAG		0x00
#define XDPPSU_DISPID_VER_REV			0x00
#define XDPPSU_DISPID_SIZE			0x01
#define XDPPSU_DISPID_TYPE			0x02
#define XDPPSU_DISPID_EXT_COUNT			0x03
#define XDPPSU_DISPID_PAYLOAD_START		0x04
#define XDPPSU_DISPID_DB_SEC_TAG		0x00
#define XDPPSU_DISPID_DB_SEC_REV		0x01
#define XDPPSU_DISPID_DB_SEC_SIZE		0x02
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the DisplayID extension block.
  * @{
  */
#define XDPPSU_EDID_EXT_BLOCK_TAG_DISPID	0x70
#define XDPPSU_DISPID_TDT_TAG			0x12
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDPPSU_DISPID_TDT_TOP0			0x04
#define XDPPSU_DISPID_TDT_TOP1			0x05
#define XDPPSU_DISPID_TDT_TOP2			0x06
#define XDPPSU_DISPID_TDT_HSIZE0		0x07
#define XDPPSU_DISPID_TDT_HSIZE1		0x08
#define XDPPSU_DISPID_TDT_VSIZE0		0x09
#define XDPPSU_DISPID_TDT_VSIZE1		0x0A
#define XDPPSU_DISPID_TDT_VENID0		0x10
#define XDPPSU_DISPID_TDT_VENID1		0x11
#define XDPPSU_DISPID_TDT_VENID2		0x12
#define XDPPSU_DISPID_TDT_PCODE0		0x13
#define XDPPSU_DISPID_TDT_PCODE1		0x14
#define XDPPSU_DISPID_TDT_SN0			0x15
#define XDPPSU_DISPID_TDT_SN1			0x16
#define XDPPSU_DISPID_TDT_SN2			0x17
#define XDPPSU_DISPID_TDT_SN3			0x18
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDPPSU_DISPID_TDT_TOP0_HTOT_L_SHIFT	4
#define XDPPSU_DISPID_TDT_TOP0_HTOT_L_MASK	(0xF << 4)
#define XDPPSU_DISPID_TDT_TOP0_VTOT_L_MASK	0xF
#define XDPPSU_DISPID_TDT_TOP1_HLOC_L_SHIFT	4
#define XDPPSU_DISPID_TDT_TOP1_HLOC_L_MASK	(0xF << 4)
#define XDPPSU_DISPID_TDT_TOP1_VLOC_L_MASK	0xF
#define XDPPSU_DISPID_TDT_TOP2_HTOT_H_SHIFT	6
#define XDPPSU_DISPID_TDT_TOP2_HTOT_H_MASK	(0x3 << 6)
#define XDPPSU_DISPID_TDT_TOP2_VTOT_H_SHIFT	4
#define XDPPSU_DISPID_TDT_TOP2_VTOT_H_MASK	(0x3 << 4)
#define XDPPSU_DISPID_TDT_TOP2_HLOC_H_SHIFT	2
#define XDPPSU_DISPID_TDT_TOP2_HLOC_H_MASK	(0x3 << 2)
#define XDPPSU_DISPID_TDT_TOP2_VLOC_H_MASK	0x3
/* @} */

#define XDPPSU_0_LANE_COUNT			2
#define XDPPSU_0_LINK_RATE			20
#define XDPPSU_0_MAX_BITS_PER_COLOR		12
#define XDPPSU_0_QUAD_PIXEL_ENABLE		0
#define XDPPSU_0_DUAL_PIXEL_ENABLE		0
#define XDPPSU_0_YCRCB_ENABLE			1
#define XDPPSU_0_YONLY_ENABLE			1
#define XDPPSU_0_GT_DATAWIDTH			2
#define XDPPSU_0_SECONDARY_SUPPORT		1
#define XDPPSU_0_AUDIO_CHANNELS			2
#define XDPPSU_0_MST_ENABLE			0
#define XDPPSU_0_NUMBER_OF_MST_STREAMS		0
#define XDPPSU_0_PROTOCOL_SELECTION		1
#define XDPPSU_0_S_AXI_ACLK			100000000

/* Serdes Register Address Space */
#define SERDES_BASEADDR		0xFD400000
#define SERDES_L0_TX_DEEMPHASIS	0x0048
#define SERDES_L0_TX_MARGININGF	0x0CC0
#define	SERDES_LANE_OFFSET	0x4000
/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XDpPsu_In32 Xil_In32
#define XDpPsu_Out32 Xil_Out32
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
 *		u32 XDpPsu_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XDpPsu_ReadReg(BaseAddress, RegOffset) \
					XDpPsu_In32((BaseAddress) + (RegOffset))

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
 *		void XDpPsu_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XDpPsu_WriteReg(BaseAddress, RegOffset, Data) \
				XDpPsu_Out32((BaseAddress) + (RegOffset), (Data))


#endif /* XDPPSU_HW_H_ */
