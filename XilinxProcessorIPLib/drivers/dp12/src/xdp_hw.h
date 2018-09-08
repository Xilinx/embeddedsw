/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdp_hw.h
 * @addtogroup dp_v6_0
 * @{
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xdp.h.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Added MST registers, masks, and values for RX.
 *                     Added HDCP registers and masks.
 * 5.0   als  05/16/16 Added MISC0 and MISC1 definitions.
 * 5.1   aad  08/16/16 Updated MISC0 definitions.
 * 5.2   aad  01/21/17 Added timeout macro for training timeout
 * 6.0   tu   05/14/17 Added AUX defer shift mask
 * 6.0   tu   08/03/17 Enabled video packing for bpc > 10
 * 6.0   tu   08/24/17 Modify #define for YCBCR422 and YCBCR444
 * </pre>
 *
*******************************************************************************/

#ifndef XDP_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XDP_HW_H_

/***************************** Include Files **********************************/

#include "xil_io.h"

/************************** Constant Definitions ******************************/

/** @name DP generic definitions: Link bandwith and lane count.
  * @{
  */
/* 0x000: LINK_BW_SET */
#define XDP_LINK_BW_SET_162GBPS	0x06	/**< 1.62 Gbps link rate. */
#define XDP_LINK_BW_SET_270GBPS	0x0A	/**< 2.70 Gbps link rate. */
#define XDP_LINK_BW_SET_540GBPS	0x14	/**< 5.40 Gbps link rate. */
/* 0x001: LANE_COUNT_SET */
#define XDP_LANE_COUNT_SET_1		0x01	/**< Lane count of 1. */
#define XDP_LANE_COUNT_SET_2		0x02	/**< Lane count of 2. */
#define XDP_LANE_COUNT_SET_4		0x04	/**< Lane count of 4. */
/* @} */

/** @name DP generic definitions: Bits per color components.
  * @{
  */
#define XDP_MAIN_STREAMX_MISC0_BDC_MASK \
				0x000000E0	/**< Bit depth per color
							component (BDC). */
#define XDP_MAIN_STREAMX_MISC0_BDC_SHIFT \
				5		/**< Shift bits for BDC.*/
#define XDP_MAIN_STREAMX_MISC0_BDC_6BPC \
				0x0		/**< 6 bits per component.*/
#define XDP_MAIN_STREAMX_MISC0_BDC_8BPC \
				0x1		/**< 8 bits per component.*/
#define XDP_MAIN_STREAMX_MISC0_BDC_10BPC \
				0x2		/**< 10 bits per component.*/
#define XDP_MAIN_STREAMX_MISC0_BDC_12BPC \
				0x3		/**< 12 bits per component.*/
#define XDP_MAIN_STREAMX_MISC0_BDC_16BPC \
				0x4		/**< 16 bits per component.*/
/* @} */

/** @name DP generic definitions: Miscellaneous components; color format.
  * @{
  */
#define XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_MASK \
				0x00000006	/**< Component format. */
#define XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT \
				1               /**< Shift bits for component
							format. */
#define XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB \
				0x0		/**< Stream's component format
							is RGB. */
#define XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 \
				0x5		/**< Stream's component format
							is YcbCr 4:2:2. */
#define XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 \
				0x6		/**< Stream's component format
							is YcbCr 4:4:4. */
/* @} */

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * Address mapping for the DisplayPort core in TX mode.
 *
*******************************************************************************/
/** @name DPTX core registers: Link configuration field.
  * @{
  */
#define XDP_TX_LINK_BW_SET		0x000	/**< Set main link bandwidth
							setting. */
#define XDP_TX_LANE_COUNT_SET		0x004	/**< Set lane count setting. */
#define XDP_TX_ENHANCED_FRAME_EN	0x008	/**< Enable enhanced framing
							symbol sequence. */
#define XDP_TX_TRAINING_PATTERN_SET	0x00C	/**< Set the link training
							pattern. */
#define XDP_TX_LINK_QUAL_PATTERN_SET	0x010	/**< Transmit the link quality
							pattern. */
#define XDP_TX_SCRAMBLING_DISABLE	0x014	/**< Disable scrambler and
							transmit all symbols. */
#define XDP_TX_DOWNSPREAD_CTRL		0x018	/**< Enable a 0.5% spreading of
							the clock. */
#define XDP_TX_SOFT_RESET		0x01C	/**< Software reset. */
/* @} */

/** @name DPTX core registers: Core enables.
  * @{
  */
#define XDP_TX_ENABLE			0x080	/**< Enable the basic operations
							of the DisplayPort TX
							core or output stuffing
							symbols if disabled. */
#define XDP_TX_ENABLE_MAIN_STREAM	0x084	/**< Enable transmission of main
							link video info. */
#define XDP_TX_ENABLE_SEC_STREAM	0x088	/**< Enable the transmission of
							secondary link info. */
#define XDP_TX_FORCE_SCRAMBLER_RESET	0x0C0	/**< Force a scrambler reset. */
#define XDP_TX_MST_CONFIG		0x0D0	/**< Enable MST. */
#define XDP_TX_LINE_RESET_DISABLE	0x0F0	/**< TX line reset disable. */
/* @} */

/** @name DPTX core registers: Core ID.
  * @{
  */
#define XDP_TX_VERSION			0x0F8	/**< Version and revision of the
							DisplayPort core. */
#define XDP_TX_CORE_ID			0x0FC	/**< DisplayPort protocol
							version and revision. */
/* @} */

/** @name DPTX core registers: AUX channel interface.
  * @{
  */
#define XDP_TX_AUX_CMD			0x100	/**< Initiates AUX commands. */
#define XDP_TX_AUX_WRITE_FIFO		0x104	/**< Write data for the current
							AUX command. */
#define XDP_TX_AUX_ADDRESS		0x108	/**< Specifies the address of
							current AUX command. */
#define XDP_TX_AUX_CLK_DIVIDER		0x10C	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDP_TX_USER_FIFO_OVERFLOW	0x110	/**< Indicates an overflow in
							user FIFO. */
#define XDP_TX_INTERRUPT_SIG_STATE	0x130	/**< The raw signal values for
							interrupt events. */
#define XDP_TX_AUX_REPLY_DATA		0x134	/**< Reply data received during
							the AUX reply. */
#define XDP_TX_AUX_REPLY_CODE		0x138	/**< Reply code received from
							the most recent AUX
							command. */
#define XDP_TX_AUX_REPLY_COUNT		0x13C	/**< Number of reply
							transactions received
							over AUX. */
#define XDP_TX_INTERRUPT_STATUS		0x140	/**< Status for interrupt
							events. */
#define XDP_TX_INTERRUPT_MASK		0x144	/**< Masks the specified
							interrupt sources. */
#define XDP_TX_REPLY_DATA_COUNT		0x148	/**< Total number of data bytes
							actually received during
							a transaction. */
#define XDP_TX_REPLY_STATUS		0x14C	/**< Reply status of most recent
							AUX transaction. */
#define XDP_TX_HPD_DURATION		0x150	/**< Duration of the HPD pulse
							in microseconds. */
/* @} */

/** @name DPTX core registers: Main stream attributes for SST / MST STREAM1.
  * @{
  */
#define XDP_TX_STREAM1_MSA_START	0x180	/**< Start of the MSA registers
							for stream 1. */
#define XDP_TX_MAIN_STREAM_HTOTAL	0x180	/**< Total number of clocks in
							the horizontal framing
							period. */
#define XDP_TX_MAIN_STREAM_VTOTAL	0x184	/**< Total number of lines in
							the video frame. */
#define XDP_TX_MAIN_STREAM_POLARITY	0x188	/**< Polarity for the video
							sync signals. */
#define XDP_TX_MAIN_STREAM_HSWIDTH	0x18C	/**< Width of the horizontal
							sync pulse. */
#define XDP_TX_MAIN_STREAM_VSWIDTH	0x190	/**< Width of the vertical sync
							pulse. */
#define XDP_TX_MAIN_STREAM_HRES		0x194	/**< Number of active pixels per
							line (the horizontal
							resolution). */
#define XDP_TX_MAIN_STREAM_VRES		0x198	/**< Number of active lines (the
							vertical resolution). */
#define XDP_TX_MAIN_STREAM_HSTART	0x19C	/**< Number of clocks between
							the leading edge of the
							horizontal sync and the
							start of active data. */
#define XDP_TX_MAIN_STREAM_VSTART	0x1A0	/**< Number of lines between the
							leading edge of the
							vertical sync and the
							first line of active
							data. */
#define XDP_TX_MAIN_STREAM_MISC0	0x1A4	/**< Miscellaneous stream
							attributes. */
#define XDP_TX_MAIN_STREAM_MISC1	0x1A8	/**< Miscellaneous stream
							attributes. */
#define XDP_TX_M_VID			0x1AC	/**< M value for the video
							stream as computed by
							the source core in
							asynchronous clock
							mode. Must be written
							in synchronous mode. */
#define XDP_TX_TU_SIZE			0x1B0	/**< Size of a transfer unit in
							the framing logic. */
#define XDP_TX_N_VID			0x1B4	/**< N value for the video
							stream as computed by
							the source core in
							asynchronous clock mode.
							Must be written in
							synchronous mode. */
#define XDP_TX_USER_PIXEL_WIDTH		0x1B8	/**< Selects the width of the
							user data input port. */
#define XDP_TX_USER_DATA_COUNT_PER_LANE	0x1BC	/**< Used to translate the
							number of pixels per
							line to the native
							internal 16-bit
							datapath. */
#define XDP_TX_MAIN_STREAM_INTERLACED	0x1C0	/**< Video is interlaced. */
#define XDP_TX_MIN_BYTES_PER_TU		0x1C4	/**< The minimum number of bytes
							per transfer unit. */
#define XDP_TX_FRAC_BYTES_PER_TU	0x1C8	/**< The fractional component
							when calculated the
							XDP_TX_MIN_BYTES_PER_TU
							register value. */
#define XDP_TX_INIT_WAIT		0x1CC	/**< Number of initial wait
							cycles at the start of a
							new line by the framing
							logic, allowing enough
							data to be buffered in
							the input FIFO. */
#define XDP_TX_STREAM1			0x1D0	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDP_TX_STREAM2			0x1D4	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDP_TX_STREAM3			0x1D8	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDP_TX_STREAM4			0x1DC	/**< Average stream symbol
							timeslots per MTP
							config. */
/* @} */

/** @name DPTX core registers: PHY configuration status.
  * @{
  */
#define XDP_TX_PHY_CONFIG		0x200	/**< Transceiver PHY reset and
							configuration. */
#define XDP_TX_PHY_VOLTAGE_DIFF_LANE_0	0x220	/**< Controls the differential
							voltage swing. */
#define XDP_TX_PHY_VOLTAGE_DIFF_LANE_1	0x224	/**< Controls the differential
							voltage swing. */
#define XDP_TX_PHY_VOLTAGE_DIFF_LANE_2	0x228	/**< Controls the differential
							voltage swing. */
#define XDP_TX_PHY_VOLTAGE_DIFF_LANE_3	0x22C	/**< Controls the differential
							voltage swing. */
#define XDP_TX_PHY_TRANSMIT_PRBS7	0x230	/**< Enable pseudo random bit
							sequence 7 pattern
							transmission for link
							quality assessment. */
#define XDP_TX_PHY_CLOCK_SELECT		0x234	/**< Instructs the PHY PLL to
							generate the proper
							clock frequency for the
							required link rate. */
#define XDP_TX_PHY_POWER_DOWN		0x238	/**< Controls PHY power down. */
#define XDP_TX_PHY_PRECURSOR_LANE_0	0x23C	/**< Controls the pre-cursor
							level. */
#define XDP_TX_PHY_PRECURSOR_LANE_1	0x240	/**< Controls the pre-cursor
							level. */
#define XDP_TX_PHY_PRECURSOR_LANE_2	0x244	/**< Controls the pre-cursor
							level. */
#define XDP_TX_PHY_PRECURSOR_LANE_3	0x248	/**< Controls the pre-cursor
							level. */
#define XDP_TX_PHY_POSTCURSOR_LANE_0	0x24C	/**< Controls the post-cursor
							level. */
#define XDP_TX_PHY_POSTCURSOR_LANE_1	0x250	/**< Controls the post-cursor
							level. */
#define XDP_TX_PHY_POSTCURSOR_LANE_2	0x254	/**< Controls the post-cursor
							level. */
#define XDP_TX_PHY_POSTCURSOR_LANE_3	0x258	/**< Controls the post-cursor
							level. */
#define XDP_TX_PHY_STATUS		0x280	/**< Current PHY status. */
#define XDP_TX_GT_DRP_COMMAND		0x2A0	/**< Provides access to the GT
							DRP ports. */
#define XDP_TX_GT_DRP_READ_DATA		0x2A4	/**< Provides access to GT DRP
							read data. */
#define XDP_TX_GT_DRP_CHANNEL_STATUS	0x2A8	/**< Provides access to GT DRP
							channel status. */
/* @} */

/** @name DPTX core registers: DisplayPort audio.
  * @{
  */
#define XDP_TX_AUDIO_CONTROL		0x300	/**< Enables audio stream
							packets in main link and
							buffer control. */
#define XDP_TX_AUDIO_CHANNELS		0x304	/**< Used to input active
							channel count. */
#define XDP_TX_AUDIO_INFO_DATA(NUM)	(0x308 + 4 * (NUM - 1)) /**< Word
							formatted as per CEA
							861-C info frame. */
#define XDP_TX_AUDIO_MAUD		0x328	/**< M value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDP_TX_AUDIO_NAUD		0x32C	/**< N value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDP_TX_AUDIO_EXT_DATA(NUM)	(0x330 + 4 * (NUM - 1)) /**< Word
							formatted as per
							extension packet. */
/* @} */

/** @name DPTX core registers: DisplayPort video.
 * @{
 */
#define XDP_TX_VIDEO_PACKING_CLOCK_CONTROL 0x90
/* @} */

/** @name DPTX core registers: HDCP.
  * @{
  */
#define XDP_TX_HDCP_ENABLE		0x400	/**< Enables HDCP core. */
/* @} */

/** @name DPTX core registers: Main stream attributes for MST STREAM2, 3, and 4.
  * @{
  */
#define XDP_TX_STREAM2_MSA_START	0x500	/**< Start of the MSA registers
							for stream 2. */
#define XDP_TX_STREAM2_MSA_START_OFFSET	(XDP_TX_STREAM2_MSA_START - \
		XDP_TX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 2 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDP_TX_STREAM3_MSA_START	0x550	/**< Start of the MSA registers
							for stream 3. */
#define XDP_TX_STREAM3_MSA_START_OFFSET	(XDP_TX_STREAM3_MSA_START - \
		XDP_TX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 3 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDP_TX_STREAM4_MSA_START	0x5A0	/**< Start of the MSA registers
							for stream 4. */
#define XDP_TX_STREAM4_MSA_START_OFFSET	(XDP_TX_STREAM4_MSA_START - \
		XDP_TX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 4 are at an
							offset from the
							corresponding registers
							of stream 1. */
/* @} */

#define XDP_TX_VC_PAYLOAD_BUFFER_ADDR	0x800	/**< Virtual channel payload
							table (0xFF bytes). */

/******************************************************************************/

/** @name DPTX core masks, shifts, and register values.
  * @{
  */
/* 0x000: LINK_BW_SET */
#define XDP_TX_LINK_BW_SET_162GBPS	0x06	/**< 1.62 Gbps link rate. */
#define XDP_TX_LINK_BW_SET_270GBPS	0x0A	/**< 2.70 Gbps link rate. */
#define XDP_TX_LINK_BW_SET_540GBPS	0x14	/**< 5.40 Gbps link rate. */
/* 0x001: LANE_COUNT_SET */
#define XDP_TX_LANE_COUNT_SET_1		0x01	/**< Lane count of 1. */
#define XDP_TX_LANE_COUNT_SET_2		0x02	/**< Lane count of 2. */
#define XDP_TX_LANE_COUNT_SET_4		0x04	/**< Lane count of 4. */
/* 0x00C: TRAINING_PATTERN_SET */
#define XDP_TX_TRAINING_PATTERN_SET_OFF	0x0	/**< Training off. */
#define XDP_TX_TRAINING_PATTERN_SET_TP1	0x1	/**< Training pattern 1 used for
							clock recovery. */
#define XDP_TX_TRAINING_PATTERN_SET_TP2	0x2	/**< Training pattern 2 used for
							channel equalization. */
#define XDP_TX_TRAINING_PATTERN_SET_TP3	0x3	/**< Training pattern 3 used for
							channel equalization for
							cores with DP v1.2. */
/* 0x010: LINK_QUAL_PATTERN_SET */
#define XDP_TX_LINK_QUAL_PATTERN_SET_OFF 0x0	/**< Link quality test pattern
							not transmitted. */
#define XDP_TX_LINK_QUAL_PATTERN_SET_D102_TEST \
					0x1	/**< D10.2 unscrambled test
							pattern transmitted. */
#define XDP_TX_LINK_QUAL_PATTERN_SET_SER_MES \
					0x2	/**< Symbol error rate
							measurement pattern
							transmitted. */
#define XDP_TX_LINK_QUAL_PATTERN_SET_PRBS7 \
					0x3	/**< Pseudo random bit sequence
							7 transmitted. */
/* 0x01C: SOFTWARE_RESET */
#define XDP_TX_SOFT_RESET_VIDEO_STREAM1_MASK \
				0x00000001	/**< Reset video logic. */
#define XDP_TX_SOFT_RESET_VIDEO_STREAM2_MASK \
				0x00000002	/**< Reset video logic. */
#define XDP_TX_SOFT_RESET_VIDEO_STREAM3_MASK \
				0x00000004	/**< Reset video logic. */
#define XDP_TX_SOFT_RESET_VIDEO_STREAM4_MASK \
				0x00000008	/**< Reset video logic. */
#define XDP_TX_SOFT_RESET_AUX_MASK \
				0x00000080	/**< Reset AUX logic. */
#define XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK \
				0x0000000F	/**< Reset video logic for all
							streams. */
/* 0x0D0: TX_MST_CONFIG */
#define XDP_TX_MST_CONFIG_MST_EN_MASK \
				0x00000001	/**< Enable MST. */
#define XDP_TX_MST_CONFIG_VCP_UPDATED_MASK \
				0x00000002	/**< The VC payload has been
							updated in the sink. */
/* 0x0F0: TX_LINE_RESET_DISABLE */
#define XDP_TX_LINE_RESET_DISABLE_MASK(Stream) \
	(1 << ((Stream) - XDP_TX_STREAM_ID1))	/**< Used to disable the end of
							the line reset to the
							internal video pipe. */
/* 0x0F8: VERSION */
#define XDP_TX_VERSION_INTER_REV_MASK \
				0x0000000F	/**< Internal revision. */
#define XDP_TX_VERSION_CORE_PATCH_MASK \
				0x00000030	/**< Core patch details. */
#define XDP_TX_VERSION_CORE_PATCH_SHIFT \
				8		/**< Shift bits for core patch
							details. */
#define XDP_TX_VERSION_CORE_VER_REV_MASK \
				0x000000C0	/**< Core version revision. */
#define XDP_TX_VERSION_CORE_VER_REV_SHIFT \
				12		/**< Shift bits for core version
							revision. */
#define XDP_TX_VERSION_CORE_VER_MNR_MASK \
				0x00000F00	/**< Core minor version. */
#define XDP_TX_VERSION_CORE_VER_MNR_SHIFT \
				16		/**< Shift bits for core minor
							version. */
#define XDP_TX_VERSION_CORE_VER_MJR_MASK \
				0x0000F000	/**< Core major version. */
#define XDP_TX_VERSION_CORE_VER_MJR_SHIFT \
				24		/**< Shift bits for core major
							version. */
/* 0x0FC: CORE_ID */
#define XDP_TX_CORE_ID_TYPE_MASK 0x0000000F	/**< Core type. */
#define XDP_TX_CORE_ID_TYPE_TX	0x0		/**< Core is a transmitter. */
#define XDP_TX_CORE_ID_TYPE_RX	0x1		/**< Core is a receiver. */
#define XDP_TX_CORE_ID_DP_REV_MASK \
				0x000000F0	/**< DisplayPort protocol
							revision. */
#define XDP_TX_CORE_ID_DP_REV_SHIFT \
				8		/**< Shift bits for DisplayPort
							protocol revision. */
#define XDP_TX_CORE_ID_DP_MNR_VER_MASK \
				0x00000F00	/**< DisplayPort protocol minor
							version. */
#define XDP_TX_CORE_ID_DP_MNR_VER_SHIFT \
				16		/**< Shift bits for DisplayPort
							protocol major
							version. */
#define XDP_TX_CORE_ID_DP_MJR_VER_MASK \
				0x0000F000	/**< DisplayPort protocol major
							version. */
#define XDP_TX_CORE_ID_DP_MJR_VER_SHIFT \
				24		/**< Shift bits for DisplayPort
							protocol major
							version. */
/* 0x100: AUX_CMD */
#define XDP_TX_AUX_CMD_NBYTES_TRANSFER_MASK \
				0x0000000F	/**< Number of bytes to transfer
							with the current AUX
							command. */
#define XDP_TX_AUX_CMD_MASK	0x00000F00	/**< AUX command. */
#define XDP_TX_AUX_CMD_SHIFT		8	/**< Shift bits for command. */
#define XDP_TX_AUX_CMD_I2C_WRITE		0x0	/**< I2C-over-AUX write
							command. */
#define XDP_TX_AUX_CMD_I2C_READ		0x1	/**< I2C-over-AUX read
							command. */
#define XDP_TX_AUX_CMD_I2C_WRITE_STATUS	0x2	/**< I2C-over-AUX write status
							command. */
#define XDP_TX_AUX_CMD_I2C_WRITE_MOT	0x4	/**< I2C-over-AUX write MOT
							(middle-of-transaction)
							command. */
#define XDP_TX_AUX_CMD_I2C_READ_MOT	0x5	/**< I2C-over-AUX read MOT
							(middle-of-transaction)
							command. */
#define XDP_TX_AUX_CMD_I2C_WRITE_STATUS_MOT \
					0x6	/**< I2C-over-AUX write status
							MOT (middle-of-
							transaction) command. */
#define XDP_TX_AUX_CMD_WRITE		0x8	/**< AUX write command. */
#define XDP_TX_AUX_CMD_READ		0x9	/**< AUX read command. */
#define XDP_TX_AUX_CMD_ADDR_ONLY_TRANSFER_EN \
				0x00001000	/**< Address only transfer
							enable (STOP will be
							sent after command). */
/* 0x10C: AUX_CLK_DIVIDER */
#define XDP_TX_AUX_CLK_DIVIDER_VAL_MASK	0x00FF	/**< Clock divider value. */
#define XDP_TX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK \
					0xFF00	/**< AUX (noise) signal width
							filter. */
#define XDP_TX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT \
					8	/**< Shift bits for AUX signal
							width filter. */
/* 0x130: INTERRUPT_SIG_STATE */
#define XDP_TX_INTERRUPT_SIG_STATE_HPD_STATE_MASK \
				0x00000001	/**< Raw state of the HPD pin on
							the DP connector. */
#define XDP_TX_INTERRUPT_SIG_STATE_REQUEST_STATE_MASK \
				0x00000002	/**< A request is currently
							being sent. */
#define XDP_TX_INTERRUPT_SIG_STATE_REPLY_STATE_MASK \
				0x00000004	/**< A reply is currently being
							received. */
#define XDP_TX_INTERRUPT_SIG_STATE_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
/* 0x138: AUX_REPLY_CODE */
#define XDP_TX_AUX_REPLY_CODE_ACK	0x0	/**< AUX command ACKed. */
#define XDP_TX_AUX_REPLY_CODE_I2C_ACK	0x0	/**< I2C-over-AUX command
							not ACKed. */
#define XDP_TX_AUX_REPLY_CODE_NACK	0x1	/**< AUX command not ACKed. */
#define XDP_TX_AUX_REPLY_CODE_DEFER	0x2	/**< AUX command deferred. */
#define XDP_TX_AUX_REPLY_CODE_I2C_NACK	0x4	/**< I2C-over-AUX command not
							ACKed. */
#define XDP_TX_AUX_REPLY_CODE_I2C_DEFER	0x8	/**< I2C-over-AUX command
							deferred. */
/* 0x140: INTERRUPT_STATUS */
#define XDP_TX_INTERRUPT_STATUS_HPD_IRQ_MASK \
				0x00000001	/**< Detected an IRQ framed with
							the proper timing on the
							HPD signal. */
#define XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK \
				0x00000002	/**< Detected the presence of
							the HPD signal. */
#define XDP_TX_INTERRUPT_STATUS_REPLY_RECEIVED_MASK \
				0x00000004	/**< An AUX reply transaction
							has been detected. */
#define XDP_TX_INTERRUPT_STATUS_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
#define XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK \
				0x00000010	/**< A pulse on the HPD line was
							detected. */
#define XDP_TX_INTERRUPT_STATUS_EXT_PKT_TXD_MASK \
				0x00000020	/**< Extended packet has been
							transmitted and the core
							is ready to accept a new
							packet. */
/* 0x144: INTERRUPT_MASK */
#define XDP_TX_INTERRUPT_MASK_HPD_IRQ_MASK \
				0x00000001	/**< Mask HPD IRQ interrupt. */
#define XDP_TX_INTERRUPT_MASK_HPD_EVENT_MASK \
				0x00000002	/**< Mask HPD event
							interrupt. */
#define XDP_TX_INTERRUPT_MASK_REPLY_RECEIVED_MASK \
				0x00000004	/**< Mask reply received
							interrupt. */
#define XDP_TX_INTERRUPT_MASK_REPLY_TIMEOUT_MASK \
				0x00000008	/**< Mask reply received
							interrupt. */
#define XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK \
				0x00000010	/**< Mask HPD pulse detected
							interrupt. */
#define XDP_TX_INTERRUPT_MASK_EXT_PKT_TXD_MASK \
				0x00000020	/**< Mask extended packet
							transmit interrupt. */
/* 0x14C: REPLY_STATUS */
#define XDP_TX_REPLY_STATUS_REPLY_RECEIVED_MASK \
				0x00000001	/**< AUX transaction is complete
							and a valid reply
							transaction received. */
#define XDP_TX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK \
				0x00000002	/**< AUX reply is currently
							being received. */
#define XDP_TX_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK \
				0x00000004	/**< AUX request is currently
							being transmitted. */
#define XDP_TX_REPLY_STATUS_REPLY_ERROR_MASK \
				0x00000008	/**< Detected an error in the
							AUX reply of the most
							recent transaction. */
#define XDP_TX_REPLY_STATUS_REPLY_STATUS_STATE_MASK \
				0x00000FF0	/**< Internal AUX reply state
							machine status bits. */
#define XDP_TX_REPLY_STATUS_REPLY_STATUS_STATE_SHIFT \
				4		/**< Shift bits for the internal
							AUX reply state machine
							status. */
/* 0x188, 0x508, 0x558, 0x5A8: MAIN_STREAM[1-4]_POLARITY */
#define XDP_TX_MAIN_STREAMX_POLARITY_HSYNC_POL_MASK \
				0x00000001	/**< Polarity of the horizontal
							sync pulse. */
#define XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_MASK \
				0x00000002	/**< Polarity of the vertical
							sync pulse. */
#define XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT \
				1		/**< Shift bits for polarity of
							the vertical sync
							pulse. */
/* 0x1A4, 0x524, 0x574, 0x5C4: MAIN_STREAM[1-4]_MISC0 */
#define XDP_TX_MAIN_STREAMX_MISC0_SYNC_CLK_MASK \
				0x00000001	/**< Synchronous clock. */
#define XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_MASK \
				0x00000006	/**< Component format. */
#define XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT \
				1               /**< Shift bits for component
							format. */
#define XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB \
				0x0		/**< Stream's component format
							is RGB. */
#define XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 \
				0x1		/**< Stream's component format
							is YcbCr 4:2:2. */
#define XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 \
				0x2		/**< Stream's component format
							is YcbCr 4:4:4. */
#define XDP_TX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_MASK \
				0x00000008	/**< Dynamic range. */
#define XDP_TX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_SHIFT \
				3		/**< Shift bits for dynamic
							range. */
#define XDP_TX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_VESA \
				0		/**< VESA range. */
#define XDP_TX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_CEA \
				1		/**< CEA range. */
#define XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_MASK \
				0x00000010	/**< YCbCr colorimetry. */
#define XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_SHIFT \
				4		/**< Shift bits for YCbCr
							colorimetry. */
#define XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_BT601 \
				0	   /**< ITU BT601 YCbCr coefficients. */
#define XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_BT709 \
				1	   /**< ITU BT709 YCbCr coefficients. */
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_MASK \
				0x000000E0	/**< Bit depth per color
							component (BDC). */
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_SHIFT \
				5		/**< Shift bits for BDC.*/
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_6BPC \
				0x0		/**< 6 bits per component.*/
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_8BPC \
				0x1		/**< 8 bits per component.*/
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_10BPC \
				0x2		/**< 10 bits per component.*/
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_12BPC \
				0x3		/**< 12 bits per component.*/
#define XDP_TX_MAIN_STREAMX_MISC0_BDC_16BPC \
				0x4		/**< 16 bits per component.*/
#define XDP_TX_MAIN_STREAMX_MISC0_OVERRIDE_CLOCKING_MODE_MASK \
				0x00000100	/**<Override Audio clk Mode.*/
#define XDP_TX_MAIN_STREAMX_MISC0_AUD_MODE_MASK \
				0x00000200	/**< Audio clock modes,
						     Setting this bit to 1
						     enables sync mode */
#define XDP_TX_MAIN_STREAMX_MISC0_AUD_INSERT_TIMESTAMP_MASK \
				0x00000400	/**< Inserts info/timestamp
						     every 512 BS symbols. */
#define XDP_TX_MAIN_STREAMX_MISC0_AUD_UNMASK_LOWER_MAUD_BITS_MASK \
				0x00000800	/**< Unmasks lower 2-bits of
						     Maud value. Masked by
						     default */
/* 0x1A8, 0x528, 0x578, 0x5C8: MAIN_STREAM[1-4]_MISC1 */
#define XDP_TX_MAIN_STREAMX_MISC1_INTERLACED_VTOTAL_GIVEN_MASK \
				0x00000001	/**< Interlaced vertical total
							even. */
#define XDP_TX_MAIN_STREAMX_MISC1_STEREO_VID_ATTR_MASK \
				0x00000006	/**< Stereo video attribute. */
#define XDP_TX_MAIN_STREAMX_MISC1_STEREO_VID_ATTR_SHIFT \
				1		/**< Shift bits for stereo video
							attribute. */
#define XDP_TX_MAIN_STREAMX_MISC1_Y_ONLY_EN_MASK \
				0x00000080	/* Y-only enable. */
/* 0x200: PHY_CONFIG */
#define XDP_TX_PHY_CONFIG_PHY_RESET_ENABLE_MASK \
				0x0000000	/**< Release reset. */
#define XDP_TX_PHY_CONFIG_PHY_RESET_MASK \
				0x0000001	/**< Hold the PHY in reset. */
#define XDP_TX_PHY_CONFIG_GTTX_RESET_MASK \
				0x0000002	/**< Hold GTTXRESET in reset. */
#define XDP_TX_PHY_CONFIG_TX_PHY_PMA_RESET_MASK \
				0x0000100	/**< Hold TX_PHY_PMA reset. */
#define XDP_TX_PHY_CONFIG_TX_PHY_PCS_RESET_MASK \
				0x0000200	/**< Hold TX_PHY_PCS reset. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_MASK \
				0x0000800	/**< Set TX_PHY_POLARITY. */
#define XDP_TX_PHY_CONFIG_TX_PHY_PRBSFORCEERR_MASK \
				0x0001000	/**< Set TX_PHY_PRBSFORCEERR. */
#define XDP_TX_PHY_CONFIG_TX_PHY_LOOPBACK_MASK \
				0x000E000	/**< Set TX_PHY_LOOPBACK. */
#define XDP_TX_PHY_CONFIG_TX_PHY_LOOPBACK_SHIFT 13 /**< Shift bits for
							TX_PHY_LOOPBACK. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_IND_LANE_MASK \
				0x0010000	/**< Set to enable individual
							lane polarity. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE0_MASK \
				0x0020000	/**< Set TX_PHY_POLARITY for
							lane 0. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE1_MASK \
				0x0040000	/**< Set TX_PHY_POLARITY for
							lane 1. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE2_MASK \
				0x0080000	/**< Set TX_PHY_POLARITY for
							lane 2. */
#define XDP_TX_PHY_CONFIG_TX_PHY_POLARITY_LANE3_MASK \
				0x0100000	/**< Set TX_PHY_POLARITY for
							lane 3. */
#define XDP_TX_PHY_CONFIG_TX_PHY_8B10BEN_MASK \
				0x0200000	/**< 8B10B encoding enable. */
#define XDP_TX_PHY_CONFIG_GT_ALL_RESET_MASK \
				0x0000003	/**< Reset GT and PHY. */
/* 0x234: PHY_CLOCK_SELECT */
#define XDP_TX_PHY_CLOCK_SELECT_162GBPS	0x1	/**< 1.62 Gbps link. */
#define XDP_TX_PHY_CLOCK_SELECT_270GBPS	0x3	/**< 2.70 Gbps link. */
#define XDP_TX_PHY_CLOCK_SELECT_540GBPS	0x5	/**< 5.40 Gbps link. */
/* 0x0220, 0x0224, 0x0228, 0x022C: XDP_TX_PHY_VOLTAGE_DIFF_LANE_[0-3] */
#define XDP_TX_VS_LEVEL_0		0x2	/**< Voltage swing level 0. */
#define XDP_TX_VS_LEVEL_1		0x5	/**< Voltage swing level 1. */
#define XDP_TX_VS_LEVEL_2		0x8	/**< Voltage swing level 2. */
#define XDP_TX_VS_LEVEL_3		0xF	/**< Voltage swing level 3. */
#define XDP_TX_VS_LEVEL_OFFSET		0x4	/**< Voltage swing compensation
							offset used when there's
							no redriver in display
							path. */
/* 0x024C, 0x0250, 0x0254, 0x0258: XDP_TX_PHY_POSTCURSOR_LANE_[0-3] */
#define XDP_TX_PE_LEVEL_0		0x00	/**< Pre-emphasis level 0. */
#define XDP_TX_PE_LEVEL_1		0x0E	/**< Pre-emphasis level 1. */
#define XDP_TX_PE_LEVEL_2		0x14	/**< Pre-emphasis level 2. */
#define XDP_TX_PE_LEVEL_3		0x1B	/**< Pre-emphasis level 3. */
/* 0x280: PHY_STATUS */
#define XDP_TX_PHY_STATUS_RESET_LANE_0_DONE_MASK \
				0x00000001	/**< Reset done for lane 0. */
#define XDP_TX_PHY_STATUS_RESET_LANE_1_DONE_MASK \
				0x00000002	/**< Reset done for lane 1. */
#define XDP_TX_PHY_STATUS_RESET_LANE_2_3_DONE_MASK \
				0x0000000C	/**< Reset done for lanes
							2 and 3. */
#define XDP_TX_PHY_STATUS_RESET_LANE_2_3_DONE_SHIFT \
				2		/**< Shift bits for reset done
							for lanes 2 and 3. */
#define XDP_TX_PHY_STATUS_PLL_LANE0_1_LOCK_MASK \
				0x00000010	/**< PLL locked for lanes
							0 and 1. */
#define XDP_TX_PHY_STATUS_PLL_LANE2_3_LOCK_MASK \
				0x00000020	/**< PLL locked for lanes
							2 and 3. */
#define XDP_TX_PHY_STATUS_PLL_FABRIC_LOCK_MASK \
				0x00000040	/**< FPGA fabric clock PLL
							locked. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_0_MASK \
				0x00030000	/**< TX buffer status lane 0. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_0_SHIFT \
				16		/**< Shift bits for TX buffer
							status lane 0. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_0_MASK \
				0x000C0000	/**< TX error on lane 0. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_0_SHIFT \
				18		/**< Shift bits for TX error on
							lane 0. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_1_MASK \
				0x00300000	/**< TX buffer status lane 1. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_1_SHIFT \
				20		/**< Shift bits for TX buffer
							status lane 1. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_1_MASK \
				0x00C00000	/**< TX error on lane 1. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_1_SHIFT \
				22		/**< Shift bits for TX error on
							lane 1. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_2_MASK \
				0x03000000	/**< TX buffer status lane 2. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_2_SHIFT \
				24		/**< Shift bits for TX buffer
							status lane 2. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_2_MASK \
				0x0C000000	/**< TX error on lane 2. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_2_SHIFT \
				26		/**< Shift bits for TX error on
							lane 2. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_3_MASK \
				0x30000000	/**< TX buffer status lane 3. */
#define XDP_TX_PHY_STATUS_TX_BUFFER_STATUS_LANE_3_SHIFT \
				28		/**< Shift bits for TX buffer
							status lane 3. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_3_MASK \
				0xC0000000	/**< TX error on lane 3. */
#define XDP_TX_PHY_STATUS_TX_ERROR_LANE_3_SHIFT \
				30		/**< Shift bits for TX error on
							lane 3. */
#define XDP_TX_PHY_STATUS_LANE_0_READY_MASK \
	(XDP_TX_PHY_STATUS_RESET_LANE_0_DONE_MASK | \
	XDP_TX_PHY_STATUS_PLL_LANE0_1_LOCK_MASK) /**< Lane 0 is ready. */
#define XDP_TX_PHY_STATUS_LANES_0_1_READY_MASK \
	(XDP_TX_PHY_STATUS_LANE_0_READY_MASK | \
	XDP_TX_PHY_STATUS_RESET_LANE_1_DONE_MASK) /**< Lanes 0-1 are ready. */
#define XDP_TX_PHY_STATUS_ALL_LANES_READY_MASK \
	(XDP_TX_PHY_STATUS_RESET_LANE_2_3_DONE_MASK | \
	XDP_TX_PHY_STATUS_PLL_LANE2_3_LOCK_MASK) /**< Lanes 0-3 are ready. */
#define XDP_TX_PHY_STATUS_LANES_READY_MASK(n) \
	(((n) > 2) ? XDP_TX_PHY_STATUS_ALL_LANES_READY_MASK : \
	((n) == 2) ? XDP_TX_PHY_STATUS_LANES_0_1_READY_MASK : \
	XDP_TX_PHY_STATUS_LANE_0_READY_MASK)	/**< Macro for lanes ready mask
							with number of lanes as
							the argument. */
/* 0x2A0: XDP_TX_GT_DRP_COMMAND */
#define XDP_TX_GT_DRP_COMMAND_DRP_ADDR_MASK \
				0x000F		/**< DRP address. */
#define XDP_TX_GT_DRP_COMMAND_DRP_RW_CMD_MASK \
				0x0080		/**< DRP read/write command
							(Read=0, Write=1). */
#define XDP_TX_GT_DRP_COMMAND_DRP_W_DATA_MASK \
				0xFF00		/**< DRP write data. */
#define XDP_TX_GT_DRP_COMMAND_DRP_W_DATA_SHIFT \
				16		/**< Shift bits for DRP write
							data. */
/* 0x400: XDP_TX_HDCP_ENABLE */
#define XDP_TX_HDCP_ENABLE_BYPASS_DISABLE_MASK \
				0x0001		/**< Disables bypass of the
							HDCP core. */

/* @} */

#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/******************************************************************************/

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * Address mapping for the DisplayPort core in RX mode.
 *
*******************************************************************************/

/** @name DPRX core registers: Receiver core configuration.
  * @{
  */
#define XDP_RX_LINK_ENABLE		0x000	/**< Enable the receiver
							core. */
#define XDP_RX_AUX_CLK_DIVIDER		0x004	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDP_RX_AUX_DEFER_SHIFT		24	/**< Aux defer. */
#define XDP_RX_LINE_RESET_DISABLE	0x008	/**< RX line reset disable. */
#define XDP_RX_DTG_ENABLE		0x00C	/**< Enables the display timing
							generator (DTG). */
#define XDP_RX_USER_PIXEL_WIDTH		0x010	/**< Selects the width of the
							user data input port. */
#define XDP_RX_INTERRUPT_MASK		0x014	/**< Masks the specified
							interrupt sources for
							stream 1. */
#define XDP_RX_MISC_CTRL		0x018	/**< Miscellaneous control of
							RX behavior. */
#define XDP_RX_SOFT_RESET		0x01C	/**< Software reset. */
/* @} */

/** @name DPRX core registers: AUX channel status.
  * @{
  */
#define XDP_RX_AUX_REQ_IN_PROGRESS	0x020	/**< Indicates the receipt of an
							AUX channel request. */
#define XDP_RX_REQ_ERROR_COUNT		0x024	/**< Provides a running total of
							errors detected on
							inbound AUX channel
							requests. */
#define XDP_RX_REQ_COUNT		0x028	/**< Provides a running total of
							the number of AUX
							requests received. */
#define XDP_RX_HPD_INTERRUPT		0x02C	/**< Instructs the DisplayPort
							RX core to assert an
							interrupt to the TX
							using the HPD signal. */
#define XDP_RX_REQ_CLK_WIDTH		0x030	/**< Holds the half period of
							the recovered AUX
							clock. */
#define XDP_RX_REQ_CMD			0x034	/**< Provides the most recent
							AUX command received. */
#define XDP_RX_REQ_ADDRESS		0x038	/**< Contains the address field
							of the most recent AUX
							request. */
#define XDP_RX_REQ_LENGTH		0x03C	/**< Contains length of the most
							recent AUX request. */
/* @} */

/** @name DPRX core registers: Interrupt registers.
  * @{
  */
#define XDP_RX_INTERRUPT_CAUSE		0x040	/**< Indicates the cause of
							pending host interrupts
							for stream 1, training,
							payload allocation, and
							for the AUX channel. */
#define XDP_RX_INTERRUPT_MASK_1		0x044	/**< Masks the specified
							interrupt sources for
							streams 2, 3, 4. */
#define XDP_RX_INTERRUPT_CAUSE_1	0x048	/**< Indicates the cause of a
							pending host interrupts
							for streams 2, 3, 4. */
/* @} */

#define XDP_RX_HSYNC_WIDTH		0x050	/**< Controls the timing of the
							active-high horizontal
							sync pulse generated
							by the display timing
							generator (DTG). */
#define XDP_RX_VSYNC_WIDTH		0x058	/**< Controls the timing of the
							active-high vertical
							sync pulse generated
							by the display timing
							generator (DTG). */
#define XDP_RX_FAST_I2C_DIVIDER		0x060	/**< Fast I2C mode clock divider
							value. */
#define XDP_RX_MST_ALLOC		0x06C	/**< Represents the content from
							the DPCD registers
							related to payload
							allocation. */

/** @name DPRX core registers: DPCD fields.
  * @{
  */
#define XDP_RX_LOCAL_EDID_VIDEO		0x084	/**< Indicates the presence of
							EDID information for the
							video stream. */
#define XDP_RX_LOCAL_EDID_AUDIO		0x088	/**< Indicates the presence of
							EDID information for the
							audio stream. */
#define XDP_RX_REMOTE_CMD		0x08C	/**< Used for passing remote
							information to the
							DisplayPort TX. */
#define XDP_RX_DEVICE_SERVICE_IRQ	0x090	/**< Indicates the DPCD
							DEVICE_SERVICE_IRQ_
							VECTOR state. */
#define XDP_RX_VIDEO_UNSUPPORTED	0x094	/**< DPCD register bit to inform
							the DisplayPort TX that
							video data is not
							supported. */
#define XDP_RX_AUDIO_UNSUPPORTED	0x098	/**< DPCD register bit to inform
							the DisplayPort TX that
							audio data is not
							supported. */
#define XDP_RX_OVER_LINK_BW_SET		0x09C	/**< Used to override the main
							link bandwidth setting
							in the DPCD. */
#define XDP_RX_OVER_LANE_COUNT_SET	0x0A0	/**< Used to override the lane
							count setting in the
							DPCD. */
#define XDP_RX_OVER_TP_SET		0x0A4	/**< Used to override the link
							training pattern in the
							DPCD. */
#define XDP_RX_OVER_TRAINING_LANE0_SET	0x0A8	/**< Used to override the
							TRAINING_LANE0_SET
							register in the DPCD. */
#define XDP_RX_OVER_TRAINING_LANE1_SET	0x0AC	/**< Used to override the
							TRAINING_LANE1_SET
							register in the DPCD. */
#define XDP_RX_OVER_TRAINING_LANE2_SET	0x0B0	/**< Used to override the
							TRAINING_LANE2_SET
							register in the DPCD. */
#define XDP_RX_OVER_TRAINING_LANE3_SET	0x0B4	/**< Used to override the
							TRAINING_LANE3_SET
							register in the DPCD. */
#define XDP_RX_OVER_CTRL_DPCD		0x0B8	/**< Used to enable AXI/APB
							write access to the DPCD
							capability structure. */
#define XDP_RX_OVER_DOWNSPREAD_CTRL	0x0BC	/**< Used to override downspread
							control in the DPCD. */
#define XDP_RX_OVER_LINK_QUAL_LANE0_SET	0x0C0	/**< Used to override the
							LINK_QUAL_LANE0_SET
							register in the DPCD. */
#define XDP_RX_OVER_LINK_QUAL_LANE1_SET	0x0C4	/**< Used to override the
							LINK_QUAL_LANE1_SET
							register in the DPCD. */
#define XDP_RX_OVER_LINK_QUAL_LANE2_SET	0x0C8	/**< Used to override the
							LINK_QUAL_LANE2_SET
							register in the DPCD. */
#define XDP_RX_OVER_LINK_QUAL_LANE3_SET	0x0CC	/**< Used to override the
							LINK_QUAL_LANE3_SET
							register in the DPCD. */
#define XDP_RX_MST_CAP			0x0D0	/**< Used to enable or disable
							MST capability. */
#define XDP_RX_SINK_COUNT		0x0D4	/**< The sink device count. */
#define XDP_RX_GUID0			0x0E0	/**< Lower 4 bytes of the DPCD's
							GUID field. */
#define XDP_RX_GUID1			0x0E4	/**< Bytes 4 to 7 of the DPCD's
							GUID field. */
#define XDP_RX_GUID2			0x0E8	/**< Bytes 8 to 11 of the DPCD's
							GUID field. */
#define XDP_RX_GUID3			0x0EC	/**< Upper 4 bytes of the DPCD's
							GUID field. */
#define XDP_RX_OVER_GUID		0x0F0	/**< Used to override the GUID
							field in the DPCD with
							what is stored in
							XDP_RX_GUID[0-3]. */
/* @} */

/** @name DPRX core registers: Core ID.
  * @{
  */
#define XDP_RX_VERSION			0x0F8	/**< Version and revision of the
							DisplayPort core. */
#define XDP_RX_CORE_ID			0x0FC	/**< DisplayPort protocol
							version and revision. */
/* @} */

/** @name DPRX core registers: User video status.
  * @{
  */
#define XDP_RX_USER_FIFO_OVERFLOW	0x110	/**< Indicates an overflow in
							user FIFO. */
#define XDP_RX_USER_VSYNC_STATE		0x114	/**< Provides a mechanism for
							the host processor to
							monitor the state of the
							video data path. */
/* @} */

/** @name DPRX core registers: PHY configuration and status.
  * @{
  */
#define XDP_RX_PHY_CONFIG		0x200	/**< Transceiver PHY reset and
							configuration. */
#define XDP_RX_PHY_STATUS		0x208	/**< Current PHY status. */
#define XDP_RX_PHY_POWER_DOWN		0x210	/**< Control PHY power down. */
#define XDP_RX_MIN_VOLTAGE_SWING	0x214	/**< Specifies the minimum
							voltage swing required
							during training before
							a link can be reliably
							established and advanced
							configuration for link
							training. */
#define XDP_RX_CDR_CONTROL_CONFIG	0x21C	/**< Control the configuration
							for clock and data
							recovery. */
#define XDP_RX_BS_IDLE_TIME		0x220	/**< Blanking start symbol idle
							time - this value is
							loaded as a timeout
							counter for detecting
							cable disconnect or
							unplug events. */
#define XDP_RX_GT_DRP_COMMAND		0x2A0	/**< Provides access to the GT
							DRP ports. */
#define XDP_RX_GT_DRP_READ_DATA		0x2A4	/**< Provides access to GT DRP
							read data. */
#define XDP_RX_GT_DRP_CH_STATUS		0x2A8	/**< Provides access to GT DRP
							channel status. */
/* @} */

/** @name DPRX core registers: Audio.
  * @{
  */
#define XDP_RX_AUDIO_CONTROL		0x300	/**< Enables audio stream
							packets in main link. */
#define XDP_RX_AUDIO_INFO_DATA(NUM)	(0x304 + 4 * (NUM - 1)) /**< Word
							formatted as per CEA
							861-C info frame. */
#define XDP_RX_AUDIO_MAUD		0x324	/**< M value of audio stream
							as decoded from audio
							time stamp packet. */
#define XDP_RX_AUDIO_NAUD		0x328	/**< N value of audio stream
							as decoded from audio
							time stamp packet. */
#define XDP_RX_AUDIO_STATUS		0x32C	/**< Status of audio stream. */
#define XDP_RX_AUDIO_EXT_DATA(NUM)	(0x330 + 4 * (NUM - 1)) /**< Word
							formatted as per
							extension packet. */
/* @} */

/** @name DPRX core registers: DPCD configuration space.
  * @{
  */
#define XDP_RX_DPCD_LINK_BW_SET		0x400	/**< Current link bandwidth
							setting as exposed in
							the RX DPCD. */
#define XDP_RX_DPCD_LANE_COUNT_SET	0x404	/**< Current lane count
							setting as exposed in
							the RX DPCD. */
#define XDP_RX_DPCD_ENHANCED_FRAME_EN	0x408	/**< Current setting for
							enhanced framing symbol
							mode as exposed in the
							RX DPCD. */
#define XDP_RX_DPCD_TRAINING_PATTERN_SET 0x40C	/**< Current training pattern
							setting as exposed in
							the RX DPCD. */
#define XDP_RX_DPCD_LINK_QUALITY_PATTERN_SET 0x410 /**< Current value of the
							link quality pattern
							field as exposed in the
							RX DPCD. */
#define XDP_RX_DPCD_RECOVERED_CLOCK_OUT_EN 0x414 /**< Value of the output clock
							enable field as exposed
							in the RX DPCD. */
#define XDP_RX_DPCD_SCRAMBLING_DISABLE	0x418	/**< Value of the scrambling
							disable field as exposed
							in the RX DPCD. */
#define XDP_RX_DPCD_SYMBOL_ERROR_COUNT_SELECT 0x41C /**< Current value of the
							symbol error count
							select field as exposed
							in the RX DPCD. */
#define XDP_RX_DPCD_TRAINING_LANE_0_SET	0x420	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 0. */
#define XDP_RX_DPCD_TRAINING_LANE_1_SET	0x424	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 1. */
#define XDP_RX_DPCD_TRAINING_LANE_2_SET	0x428	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 2. */
#define XDP_RX_DPCD_TRAINING_LANE_3_SET	0x42C	/**< The RX DPCD value Used by
							the TX during link
							training to configure
							the RX PHY lane 3. */
#define XDP_RX_DPCD_DOWNSPREAD_CONTROL	0x430	/**< The RX DPCD value that
							is used by the TX to
							inform the RX that
							downspreading has been
							enabled. */
#define XDP_RX_DPCD_MAIN_LINK_CHANNEL_CODING_SET 0x434 /**< 8B/10B encoding
							setting as exposed in
							the RX DPCD. */
#define XDP_RX_DPCD_SET_POWER_STATE	0x438	/**< Power state requested by
							the TX as exposed in the
							RX DPCD. */
#define XDP_RX_DPCD_LANE01_STATUS	0x43C	/**< Link training status for
							lanes 0 and 1 as exposed
							in the RX DPCD. */
#define XDP_RX_DPCD_LANE23_STATUS	0x440	/**< Link training status for
							lanes 2 and 3 as exposed
							in the RX DPCD. */
#define XDP_RX_DPCD_SOURCE_OUI_VALUE	0x444	/** The RX DPCD value used by
							the TX to set the
							organizationally unique
							identifier (OUI). */
#define XDP_RX_DPCD_SYM_ERR_CNT01	0x448	/** The symbol error counter
							values for lanes 0 and 1
							as exposed in the RX
							DPCD. */
#define XDP_RX_DPCD_SYM_ERR_CNT23	0x44C	/** The symbol error counter
							values for lanes 2 and 3
							as exposed in the RX
							DPCD. */
/* @} */

/** @name DPRX core registers: Main stream attributes for SST / MST STREAM1.
  * @{
  */
#define XDP_RX_STREAM1_MSA_START	0x500	/**< Start of the MSA registers
							for stream 1. */
#define XDP_RX_MSA_HRES			0x500	/**< Number of active pixels per
							line (the horizontal
							resolution). */
#define XDP_RX_MSA_HSPOL		0x504	/**< The horizontal sync
							polarity. */
#define XDP_RX_MSA_HSWIDTH		0x508	/**< Width of the horizontal
							sync pulse. */
#define XDP_RX_MSA_HSTART		0x50C	/**< Number of clocks between
							the leading edge of the
							horizontal sync and the
							start of active data. */
#define XDP_RX_MSA_HTOTAL		0x510	/**< Total number of clocks in
							the horizontal framing
							period. */
#define XDP_RX_MSA_VHEIGHT		0x514	/**< Number of active lines (the
							vertical resolution). */
#define XDP_RX_MSA_VSPOL		0x518	/**< The vertical sync
							polarity. */
#define XDP_RX_MSA_VSWIDTH		0x51C	/**< Width of the vertical
							sync pulse. */
#define XDP_RX_MSA_VSTART		0x520	/**< Number of lines between the
							leading edge of the
							vertical sync and the
							first line of active
							data. */
#define XDP_RX_MSA_VTOTAL		0x524	/**< Total number of lines in
							the video frame. */
#define XDP_RX_MSA_MISC0		0x528	/**< Miscellaneous stream
							attributes. */
#define XDP_RX_MSA_MISC1		0x52C	/**< Miscellaneous stream
							attributes. */
#define XDP_RX_MSA_MVID			0x530	/**< Used to recover the video
							clock from the link
							clock. */
#define XDP_RX_MSA_NVID			0x534	/**< Used to recover the video
							clock from the link
							clock. */
#define XDP_RX_MSA_VBID			0x538	/**< The most recently received
							VB-ID value. */
/* @} */

/** @name DPRX core registers: Main stream attributes for MST STREAM2, 3, and 4.
  * @{
  */
#define XDP_RX_STREAM2_MSA_START	0x540	/**< Start of the MSA registers
							for stream 2. */
#define XDP_RX_STREAM2_MSA_START_OFFSET	(XDP_RX_STREAM2_MSA_START - \
		XDP_RX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 2 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDP_RX_STREAM3_MSA_START	0x580	/**< Start of the MSA registers
							for stream 3. */
#define XDP_RX_STREAM3_MSA_START_OFFSET	(XDP_RX_STREAM3_MSA_START - \
		XDP_RX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 3 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDP_RX_STREAM4_MSA_START	0x5C0	/**< Start of the MSA registers
							for stream 4. */
#define XDP_RX_STREAM4_MSA_START_OFFSET	(XDP_RX_STREAM4_MSA_START - \
		XDP_RX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 4 are at an
							offset from the
							corresponding registers
							of stream 1. */
/* @} */

/** @name DPRX core registers: DPCD registers for HDCP.
  * @{
  */
#define XDP_RX_DPCD_HDCP_TABLE		0x900	/**< HDCP register table
							(0x100 bytes). */
/* @} */

/** @name DPRX core registers: MST field for sideband message buffers and the
  *	  virtual channel payload table.
  * @{
  */
#define XDP_RX_DOWN_REQ			0xA00	/**< Down request buffer address
							space. */
#define XDP_RX_DOWN_REP			0xB00	/**< Down reply buffer address
							space. */
#define XDP_RX_VC_PAYLOAD_TABLE		0x800	/**< Virtual channel payload
							table (0xFF bytes). */
#define XDP_RX_VC_PAYLOAD_TABLE_ID_SLOT(SlotNum) \
			(XDP_RX_VC_PAYLOAD_TABLE + SlotNum)
/* @} */

/** @name DPRX core registers: Vendor specific DPCD.
  * @{
  */
#define XDP_RX_SOURCE_DEVICE_SPECIFIC_FIELD 0xE00 /**< User access to the source
							specific field as
							exposed in the RX
							DPCD (0xFF bytes). */
#define XDP_RX_SOURCE_DEVICE_SPECIFIC_FIELD_REG(RegNum) \
			(XDP_RX_SOURCE_DEVICE_SPECIFIC_FIELD + (4 * RegNum))
#define XDP_RX_SINK_DEVICE_SPECIFIC_FIELD 0xF00	/**< User access to the sink
							specific field as
							exposed in the RX
							DPCD (0xFF bytes). */
#define XDP_RX_SINK_DEVICE_SPECIFIC_FIELD_REG(RegNum) \
			(XDP_RX_SINK_DEVICE_SPECIFIC_FIELD + (4 * RegNum))
/* @} */

/******************************************************************************/

/** @name DPRX core masks, shifts, and register values.
  * @{
  */
/* 0x004: AUX_CLK_DIVIDER */
#define XDP_RX_AUX_CLK_DIVIDER_VAL_MASK	0x00FF	/**< Clock divider value. */
#define XDP_RX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK \
					0xFF00	/**< AUX (noise) signal width
							filter. */
#define XDP_RX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT \
					8	/**< Shift bits for AUX signal
							width filter. */
/* 0x008: RX_LINE_RESET_DISABLE */
#define XDP_RX_LINE_RESET_DISABLE_MASK(Stream) \
	(1 << ((Stream) - XDP_TX_STREAM_ID1))	/**< Used to disable the end of
							the line reset to the
							internal video pipe. */
/* 0x010: USER_PIXEL_WIDTH */
#define XDP_RX_USER_PIXEL_WIDTH_1	0x1	/**< Single pixel wide
							interface. */
#define XDP_RX_USER_PIXEL_WIDTH_2	0x2	/**< Dual pixel output mode. */
#define XDP_RX_USER_PIXEL_WIDTH_4	0x4	/**< Quad pixel output mode. */
/* 0x014: INTERRUPT_MASK */
#define XDP_RX_INTERRUPT_MASK_VM_CHANGE_MASK \
					0x00000001 /**< Mask the interrupt
							assertion for a
							resolution change, as
							detected from the MSA
							fields. */
#define XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK \
					0x00000002 /**< Mask the interrupt
							assertion for a power
							state change. */
#define XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK \
					0x00000004 /**< Mask the interrupt
							assertion for the
							no-video condition being
							detected after active
							video received. */
#define XDP_RX_INTERRUPT_MASK_VBLANK_MASK \
					0x00000008 /**< Mask the interrupt
							assertion for the start
							of the blanking
							interval. */
#define XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK \
					0x00000010 /**< Mask the interrupt
							assertion for training
							loss on active lanes. */
#define XDP_RX_INTERRUPT_MASK_VIDEO_MASK 0x00000040 /**< Mask the interrupt
							assertion for a valid
							video frame being
							detected on the main
							link. Video interrupt is
							set after a delay of 8
							video frames following a
							valid scrambler reset
							character. */
#define XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK \
					0x00000100 /**< Mask the interrupt
							assertion for an audio
							info packet being
							received. */
#define XDP_RX_INTERRUPT_MASK_EXT_PKT_MASK \
					0x00000200 /**< Mask the interrupt
							assertion for an audio
							extension packet being
							received. */
#define XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK \
					0x00000400 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK \
					0x00000800 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK \
					0x00001000 /**< Mask the interrupt
							assertion for a
							downstream reply being
							ready. */
#define XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK \
					0x00002000 /**< Mask the interrupt
							assertion for a
							downstream request being
							ready. */
#define XDP_RX_INTERRUPT_MASK_TRAINING_DONE_MASK \
					0x00004000 /**< Mask the interrupt
							assertion for link
							training completion. */
#define XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK \
					0x00008000 /**< Mask the interrupt
							assertion for a change
							in bandwidth. */
#define XDP_RX_INTERRUPT_MASK_TP1_MASK	0x00010000 /**< Mask the interrupt
							assertion for start of
							training pattern 1. */
#define XDP_RX_INTERRUPT_MASK_TP2_MASK	0x00020000 /**< Mask the interrupt
							assertion for start of
							training pattern 2. */
#define XDP_RX_INTERRUPT_MASK_TP3_MASK	0x00040000 /**< Mask the interrupt
							assertion for start of
							training pattern 3. */
#define XDP_RX_INTERRUPT_MASK_HDCP_DEBUG_WRITE_MASK \
					0x00080000 /**< Mask the interrupt
							for a write to any HDCP
							debug register. */
#define XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK \
					0x00100000 /**< Mask the interrupt
							for a write to the HDCP
							AKSV MSB register. */
#define XDP_RX_INTERRUPT_MASK_HDCP_AN_WRITE_MASK \
					0x00200000 /**< Mask the interrupt
							for a write to the HDCP
							An MSB register. */
#define XDP_RX_INTERRUPT_MASK_HDCP_AINFO_WRITE_MASK \
					0x00400000 /**< Mask the interrupt
							for a write to the HDCP
							AInfo register. */
#define XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK \
					0x00800000 /**< Mask the interrupt
							for a read of the HDCP
							Ro register. */
#define XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK \
					0x01000000 /**< Mask the interrupt
							for a read of the HDCP
							BInfo register. */
#define XDP_RX_INTERRUPT_MASK_AUDIO_OVER_MASK \
					0x08000000 /**< Mask the interrupt
							assertion caused for an
							audio packet
							overflow. */
#define XDP_RX_INTERRUPT_MASK_PAYLOAD_ALLOC_MASK \
					0x10000000 /**< Mask the interrupt
							assertion for the RX's
							DPCD payload allocation
							registers that have been
							updated as part of
							(de-)allocation or
							partial deletion. */
#define XDP_RX_INTERRUPT_MASK_ACT_RX_MASK \
					0x20000000 /**< Mask the interrupt
							assertion for the ACT
							sequence being
							received. */
#define XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK \
					0x40000000 /**< Mask the interrupt
							assertion for the start
							of a CRC test. */
#define XDP_RX_INTERRUPT_MASK_UNPLUG_MASK \
					0x80000000 /**< Mask the unplug event
							interrupt. */
#define XDP_RX_INTERRUPT_MASK_ALL_MASK	0xF9FFFFFF /**< Mask all interrupts. */
/* 0x018: MISC_CTRL */
#define XDP_RX_MISC_CTRL_USE_FILT_MSA_MASK \
					0x1	/**< When set, two matching
							values must be detected
							for each field of the
							MSA values before the
							associated register is
							updated internally. */
#define XDP_RX_MISC_CTRL_LONG_I2C_USE_DEFER_MASK \
					0x2	/**< When set, the long I2C
							write data transfers
							are responded to using
							DEFER instead of partial
							ACKs. */
#define XDP_RX_MISC_CTRL_I2C_USE_AUX_DEFER_MASK \
					0x4	/**< When set, I2C DEFERs will
							be sent as AUX DEFERs to
							the source device. */
/* 0x01C: SOFT_RESET */
#define XDP_RX_SOFT_RESET_VIDEO_MASK	0x01	/**< Reset the video logic. */
#define XDP_RX_SOFT_RESET_AUX_MASK	0x80	/**< Reset the AUX logic. */
/* 0x02C: HPD_INTERRUPT */
#define XDP_RX_HPD_INTERRUPT_ASSERT_MASK \
				0x00000001	/**< Instructs the RX core to
							assert an interrupt to
							the TX using the HPD
							signal. */
#define XDP_RX_HPD_INTERRUPT_LENGTH_US_MASK \
				0xFFFF0000	/**< The length of the HPD pulse
							to generate (in
							microseconds). */
#define XDP_RX_HPD_INTERRUPT_LENGTH_US_SHIFT 16	/**< Shift bits for the HPD
							pulse length. */
/* 0x040: INTERRUPT_CAUSE */
#define XDP_RX_INTERRUPT_CAUSE_VM_CHANGE_MASK \
	XDP_RX_INTERRUPT_MASK_VM_CHANGE_MASK	/**< Interrupt caused by a
							resolution change, as
							detected from the MSA
							fields. */
#define XDP_RX_INTERRUPT_CAUSE_POWER_STATE_MASK \
	XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK	/**< Interrupt caused by a
							power state change. */
#define XDP_RX_INTERRUPT_CAUSE_NO_VIDEO_MASK \
	XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK	/**< Interrupt caused by the
							no-video condition being
							detected after active
							video received. */
#define XDP_RX_INTERRUPT_CAUSE_VBLANK_MASK \
		XDP_RX_INTERRUPT_MASK_VBLANK_MASK /**< Interrupt caused by the
							start of the blanking
							interval. */
#define XDP_RX_INTERRUPT_CAUSE_TRAINING_LOST_MASK \
	XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK	/**< Interrupt caused by
							training loss on active
							lanes. */
#define XDP_RX_INTERRUPT_CAUSE_VIDEO_MASK \
		XDP_RX_INTERRUPT_MASK_VIDEO_MASK /**< Interrupt caused by a
							valid video frame being
							detected on the main
							link. Video interrupt is
							set after a delay of 8
							video frames following a
							valid scrambler reset
							character. */
#define XDP_RX_INTERRUPT_CAUSE_INFO_PKT_MASK \
	XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK	/**< Interrupt caused by an
							audio info packet being
							received. */
#define XDP_RX_INTERRUPT_CAUSE_EXT_PKT_MASK \
	XDP_RX_INTERRUPT_MASK_EXT_PKT_MASK	/**< Interrupt caused by an
							audio extension packet
							being received. */
#define XDP_RX_INTERRUPT_CAUSE_VCP_ALLOC_MASK \
	XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK	/**< Interrupt caused by a
							virtual channel payload
							being allocated. */
#define XDP_RX_INTERRUPT_CAUSE_VCP_DEALLOC_MASK \
	XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK	/**< Interrupt caused by a
							virtual channel payload
							being allocated. */
#define XDP_RX_INTERRUPT_CAUSE_DOWN_REPLY_MASK \
	XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK	/**< Interrupt caused by a
							downstream reply being
							ready. */
#define XDP_RX_INTERRUPT_CAUSE_DOWN_REQUEST_MASK \
	XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK	/**< Interrupt caused by a
							downstream request being
							ready. */
#define XDP_RX_INTERRUPT_CAUSE_TRAINING_DONE_MASK \
	XDP_RX_INTERRUPT_MASK_TRAINING_DONE_MASK /**< Interrupt caused by link
							training completion. */
#define XDP_RX_INTERRUPT_CAUSE_BW_CHANGE_MASK \
	XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK	/**< Interrupt caused by a
							change in bandwidth. */
#define XDP_RX_INTERRUPT_CAUSE_TP1_MASK \
	XDP_RX_INTERRUPT_MASK_TP1_MASK		/**< Interrupt caused by the
							start of training
							pattern 1. */
#define XDP_RX_INTERRUPT_CAUSE_TP2_MASK \
	XDP_RX_INTERRUPT_MASK_TP2_MASK		/**< Interrupt caused by the
							start of training
							pattern 2. */
#define XDP_RX_INTERRUPT_CAUSE_TP3_MASK \
	XDP_RX_INTERRUPT_MASK_TP3_MASK		/**< Interrupt caused by the
							start of training
							pattern 3. */
#define XDP_RX_INTERRUPT_CAUSE_AUDIO_OVER_MASK \
	XDP_RX_INTERRUPT_MASK_AUDIO_OVER_MASK	/**< Interrupt caused by an
							audio packet
							overflow. */
#define XDP_RX_INTERRUPT_CAUSE_PAYLOAD_ALLOC_MASK \
	XDP_RX_INTERRUPT_MASK_PAYLOAD_ALLOC_MASK /**< Interrupt caused by the
							RX's DPCD payload
							allocation registers has
							been updated as part of
							(de-)allocation or
							partial deletion. */
#define XDP_RX_INTERRUPT_CAUSE_ACT_RX_MASK \
	XDP_RX_INTERRUPT_MASK_ACT_RX_MASK	/**< Interrupt caused by the
							ACT sequence being
							received. */
#define XDP_RX_INTERRUPT_CAUSE_CRC_TEST_MASK \
	XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK	/**< Interrupt caused by the
							start of a CRC test. */
#define XDP_RX_INTERRUPT_CAUSE_UNPLUG_MASK \
	XDP_RX_INTERRUPT_MASK_UNPLUG_MASK	/**< Interrupt caused by the
							an unplug event. */
/* 0x044: INTERRUPT_MASK_1 */
#define XDP_RX_INTERRUPT_MASK_1_EXT_PKT_STREAM234_MASK(Stream) \
		(0x00001 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for an audio
							extension packet being
							received for stream
							2, 3, or 4. */
#define XDP_RX_INTERRUPT_MASK_1_INFO_PKT_STREAM234_MASK(Stream) \
		(0x00002 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for an audio
							info packet being
							received for stream
							2, 3, or 4. */
#define XDP_RX_INTERRUPT_MASK_1_VM_CHANGE_STREAM234_MASK(Stream) \
		(0x00004 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for a
							resolution change, as
							detected from the MSA
							fields for stream 2, 3,
							or 4. */
#define XDP_RX_INTERRUPT_MASK_1_NO_VIDEO_STREAM234_MASK(Stream) \
		(0x00008 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for the
							no-video condition being
							detected after active
							video received for
							stream 2, 3, or 4. */
#define XDP_RX_INTERRUPT_MASK_1_VBLANK_STREAM234_MASK(Stream) \
		(0x00010 << ((Stream - 2) * 6)) /**< Mask the interrupt
							assertion for the start
							of the blanking interval
							for stream 2, 3, or
							4. */
#define XDP_RX_INTERRUPT_MASK_1_VIDEO_STREAM234_MASK(Stream) \
		(0x00020 << ((Stream - 2) * 6)) /**< Mask the interrupt
							assertion for a valid
							video frame being
							detected on the main
							link for stream 2, 3,
							or 4. */
/* 0x048: INTERRUPT_CAUSE_1 */
#define XDP_RX_INTERRUPT_CAUSE_1_EXT_PKT_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_EXT_PKT_STREAM234_MASK(Stream) /**< Interrupt
							caused by an audio
							extension packet being
							received for stream 2,
							3, or 4. */
#define XDP_RX_INTERRUPT_CAUSE_1_INFO_PKT_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_INFO_PKT_STREAM234_MASK(Stream) /**< Interrupt
							caused by an audio info
							packet being received
							for stream 2, 3, or
							4. */
#define XDP_RX_INTERRUPT_CAUSE_1_VM_CHANGE_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_VM_CHANGE_STREAM234_MASK(Stream) /**< Interrupt
							caused by a resolution
							change, as detected from
							the MSA fields for
							stream 2, 3, or 4. */
#define XDP_RX_INTERRUPT_CAUSE_1_NO_VIDEO_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_NO_VIDEO_STREAM234_MASK(Stream) /**< Interrupt
							caused by the no-video
							condition being detected
							after active video
							received for stream 2,
							3, or 4. */
#define XDP_RX_INTERRUPT_CAUSE_1_VBLANK_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_VBLANK_STREAM234_MASK(Stream) /**< Interrupt
							caused by the start of
							the blanking interval
							for stream 2, 3, or
							4. */
#define XDP_RX_INTERRUPT_CAUSE_1_VIDEO_STREAM234_MASK(Stream) \
	XDP_RX_INTERRUPT_CAUSE_1_VIDEO_STREAM234_MASK(Stream) /**< Interrupt
							caused by a valid video
							frame being detected on
							the main link for
							stream 2, 3, or 4. */
/* 0x050: HSYNC_WIDTH */
#define XDP_RX_HSYNC_WIDTH_PULSE_WIDTH_MASK \
					0x00FF	/**< Specifies the number of
							clock cycles the
							horizontal sync pulse is
							asserted. */
#define XDP_RX_HSYNC_WIDTH_FRONT_PORCH_MASK \
					0xFF00	/**< Defines the number of video
							clock cycles to place
							between the last pixel
							of active data and the
							start of the horizontal
							sync pulse (the front
							porch). */
#define XDP_RX_HSYNC_WIDTH_FRONT_PORCH_SHIFT 8	/**< Shift bits for the front
							porch. */
/* 0x06C: MST_ALLOC */
#define XDP_RX_MST_ALLOC_VCP_ID_MASK   0x00003F	/**< The virtual channel payload
							ID that was issued as
							part of the most recent
							ALLOCATE_PAYLOAD down
							request. */
#define XDP_RX_MST_ALLOC_START_TS_MASK 0x003F00	/**< The starting time slot that
							was issued as part of
							the most recent
							ALLOCATE_PAYLOAD down
							request. */
#define XDP_RX_MST_ALLOC_START_TS_SHIFT	8	/**< Shift bits for the starting
							time slot. */
#define XDP_RX_MST_ALLOC_COUNT_TS_MASK 0x3F0000	/**< The time slot count that
							was issued as part of
							part of the most recent
							ALLOCATE_PAYLOAD down
							request. */
#define XDP_RX_MST_ALLOC_COUNT_TS_SHIFT	16	/**< Shift bits for the time
							slot count. */
/* 0x090: DEVICE_SERVICE_IRQ */
#define XDP_RX_DEVICE_SERVICE_IRQ_NEW_REMOTE_CMD_MASK \
					0x01	/**< Indicates that a new
							command is present in
							the REMOTE_CMD
							register. */
#define XDP_RX_DEVICE_SERVICE_IRQ_SINK_SPECIFIC_IRQ_MASK \
					0x02	/**< Reflects the
							SINK_SPECIFIC_IRQ
							state. */
#define XDP_RX_DEVICE_SERVICE_IRQ_CP_IRQ_MASK \
					0x04	/**< Generates a CP IRQ
							event */
#define XDP_RX_DEVICE_SERVICE_IRQ_NEW_DOWN_REPLY_MASK \
					0x10	/**< Indicates a new DOWN_REPLY
							buffer message is
							ready. */
/* 0x09C: OVER_LINK_BW_SET */
#define XDP_RX_OVER_LINK_BW_SET_162GBPS	0x06    /**< 1.62 Gbps link rate. */
#define XDP_RX_OVER_LINK_BW_SET_270GBPS	0x0A    /**< 2.70 Gbps link rate. */
#define XDP_RX_OVER_LINK_BW_SET_540GBPS	0x14    /**< 5.40 Gbps link rate. */

/* 0x0A0: OVER_LANE_COUNT_SET */
#define XDP_RX_OVER_LANE_COUNT_SET_MASK	0x1F	/**< The lane count override
							value. */
#define XDP_RX_OVER_LANE_COUNT_SET_1	0x1	/**< Lane count of 1. */
#define XDP_RX_OVER_LANE_COUNT_SET_2	0x2	/**< Lane count of 2. */
#define XDP_RX_OVER_LANE_COUNT_SET_4	0x4	/**< Lane count of 4. */
#define XDP_RX_OVER_LANE_COUNT_SET_TPS3_SUPPORTED_MASK \
					0x40	/**< Capability override for
							training pattern 3. */
#define XDP_RX_OVER_LANE_COUNT_SET_ENHANCED_FRAME_CAP_MASK \
					0x80	/**< Capability override for
							enhanced framing. */
/* 0x0A4: OVER_TP_SET */
#define XDP_RX_OVER_TP_SET_TP_SELECT_MASK \
					0x0003	/**< Training pattern select
							override. */
#define XDP_RX_OVER_TP_SET_LQP_SET_MASK \
					0x000C	/**< Link quality pattern set
							override. */
#define XDP_RX_OVER_TP_SET_LQP_SET_SHIFT 2	/**< Shift bits for link quality
							pattern set override. */
#define XDP_RX_OVER_TP_SET_REC_CLK_OUT_EN_MASK \
					0x0010	/**< Recovered clock output
							enable override. */
#define XDP_RX_OVER_TP_SET_SCRAMBLER_DISABLE_MASK \
					0x0020	/**< Scrambling disable
							override. */
#define XDP_RX_OVER_TP_SET_SYMBOL_ERROR_COUNT_SEL_MASK \
					0x00C0	/**< Symbol error count
							override. */
#define XDP_RX_OVER_TP_SET_SYMBOL_ERROR_COUNT_SEL_SHIFT \
					6	/**< Shift bits for symbol error
							count override. */
#define XDP_RX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_MASK \
					0xFF00	/**< Training AUX read interval
							override. */
#define XDP_RX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_SHIFT \
					8	/**< Shift bits for training AUX
							read interval
							override. */
/* 0x0A8, 0x0AC, 0x0B0, 0x0B4: OVER_TRAINING_LANEX_SET */
#define XDP_RX_OVER_TRAINING_LANEX_SET_VS_SET_MASK \
					0x03	/**< Voltage swing set
							override. */
#define XDP_RX_OVER_TRAINING_LANEX_SET_MAX_VS_MASK \
					0x04	/**< Maximum voltage swing
							override. */
#define XDP_RX_OVER_TRAINING_LANEX_SET_PE_SET_MASK \
					0x18	/**< Pre-emphasis set
							override. */
#define XDP_RX_OVER_TRAINING_LANEX_SET_PE_SET_SHIFT \
					3	/**< Shift bits for pre-emphasis
							set override. */
#define XDP_RX_OVER_TRAINING_LANEX_SET_MAX_PE_MASK \
					0x20	/**< Maximum pre-emphasis
							override. */
/* 0x0D0 : MST_CAP */
#define XDP_RX_MST_CAP_ENABLE_MASK	0x001	/**< When set to 1, enables MST
							mode in the RX, or
							disables it when 0. */
#define XDP_RX_MST_CAP_SOFT_VCP_MASK	0x002	/**< When set to 1, enables
							software control over
							the virtual channel
							payload table. */
#define XDP_RX_MST_CAP_OVER_ACT_MASK	0x004	/**< When set to 1, overrides
							the ACT trigger. This
							is used when software
							controls the virtual
							channel payload
							table. */
#define XDP_RX_MST_CAP_VCP_UPDATE_MASK	0x010	/**< When set to 1, indicates to
							the upstream device that
							the virtual channel
							payload table has been
							updated. This is used
							when software controls
							the virtual channel
							payload table. */
#define XDP_RX_MST_CAP_VCP_CLEAR_MASK	0x100	/**< When set to 1, clears the
							virtual channel payload
							table. */
/* 0x0F8 : VERSION_REGISTER */
#define XDP_RX_VERSION_INTER_REV_MASK \
				0x0000000F	/**< Internal revision. */
#define XDP_RX_VERSION_CORE_PATCH_MASK \
				0x00000030	/**< Core patch details. */
#define XDP_RX_VERSION_CORE_PATCH_SHIFT \
				8		/**< Shift bits for core patch
							details. */
#define XDP_RX_VERSION_CORE_VER_REV_MASK \
				0x000000C0	/**< Core version revision. */
#define XDP_RX_VERSION_CORE_VER_REV_SHIFT \
				12		/**< Shift bits for core version
							revision. */
#define XDP_RX_VERSION_CORE_VER_MNR_MASK \
				0x00000F00	/**< Core minor version. */
#define XDP_RX_VERSION_CORE_VER_MNR_SHIFT \
				16		/**< Shift bits for core minor
							version. */
#define XDP_RX_VERSION_CORE_VER_MJR_MASK \
				0x0000F000	/**< Core major version. */
#define XDP_RX_VERSION_CORE_VER_MJR_SHIFT \
				24		/**< Shift bits for core major
							version. */
/* 0x0FC : CORE_ID */
#define XDP_RX_CORE_ID_TYPE_MASK 0x0000000F	/**< Core type. */
#define XDP_RX_CORE_ID_TYPE_TX	0x0		/**< Core is a transmitter. */
#define XDP_RX_CORE_ID_TYPE_RX	0x1		/**< Core is a receiver. */
#define XDP_RX_CORE_ID_DP_REV_MASK \
				0x000000F0	/**< DisplayPort protocol
							revision. */
#define XDP_RX_CORE_ID_DP_REV_SHIFT \
				8		/**< Shift bits for DisplayPort
							protocol revision. */
#define XDP_RX_CORE_ID_DP_MNR_VER_MASK \
				0x00000F00	/**< DisplayPort protocol minor
							version. */
#define XDP_RX_CORE_ID_DP_MNR_VER_SHIFT \
				16		/**< Shift bits for DisplayPort
							protocol major
							version. */
#define XDP_RX_CORE_ID_DP_MJR_VER_MASK \
				0x0000F000	/**< DisplayPort protocol major
							version. */
#define XDP_RX_CORE_ID_DP_MJR_VER_SHIFT \
				24		/**< Shift bits for DisplayPort
							protocol major
							version. */
/* 0x110: USER_FIFO_OVERFLOW */
#define XDP_RX_USER_FIFO_OVERFLOW_FLAG_STREAMX_MASK(Stream) \
				(Stream)	/**< Indicates that the internal
							FIFO has detected on
							overflow condition for
							the specified stream. */
#define XDP_RX_USER_FIFO_OVERFLOW_VID_UNPACK_STREAMX_MASK(Stream) \
				(Stream << 4)	/**< Indicates that the video
							unpack FIFO has
							overflown for the
							specified stream. */
#define XDP_RX_USER_FIFO_OVERFLOW_VID_TIMING_STREAMX_MASK(Stream) \
				(Stream << 8)	/**< Indicates that the video
							timing FIFO has
							overflown for the
							specified stream. */
/* 0x114: USER_VSYNC_STATE */
#define XDP_RX_USER_VSYNC_STATE_STREAMX_MASK(Stream) \
				(Stream)	/**< The state of the vertical
							sync pulse for the
							specified stream. */
/* 0x200: PHY_CONFIG */
#define XDP_RX_PHY_CONFIG_PHY_RESET_ENABLE_MASK \
				0x00000000	/**< Release reset. */
#define XDP_RX_PHY_CONFIG_GTPLL_RESET_MASK \
				0x00000001	/**< Hold the GTPLL in reset. */
#define XDP_RX_PHY_CONFIG_GTRX_RESET_MASK \
				0x00000002	/**< Hold GTRXRESET in reset. */
#define XDP_RX_PHY_CONFIG_RX_PHY_PMA_RESET_MASK \
				0x00000100	/**< Hold RX_PHY_PMA reset. */
#define XDP_RX_PHY_CONFIG_RX_PHY_PCS_RESET_MASK \
				0x00000200	/**< Hold RX_PHY_PCS reset. */
#define XDP_RX_PHY_CONFIG_RX_PHY_BUF_RESET_MASK \
				0x00000400	/**< Hold RX_PHY_BUF reset. */
#define XDP_RX_PHY_CONFIG_RX_PHY_DFE_LPM_RESET_MASK \
				0x00000800	/**< Hold RX_PHY_DFE_LPM
							reset. */
#define XDP_RX_PHY_CONFIG_RX_PHY_POLARITY_MASK \
				0x00001000	/**< Set RX_PHY_POLARITY. */
#define XDP_RX_PHY_CONFIG_RX_PHY_LOOPBACK_MASK \
				0x0000E000	/**< Set RX_PHY_LOOPBACK. */
#define XDP_RX_PHY_CONFIG_RX_PHY_EYESCANRESET_MASK \
				0x00010000	/**< Set RX_PHY_EYESCANRESET. */
#define XDP_RX_PHY_CONFIG_RX_PHY_EYESCANTRIGGER_MASK \
				0x00020000	/**< Set RX_PHY_
							EYESCANTRIGGER. */
#define XDP_RX_PHY_CONFIG_RX_PHY_PRBSCNTRESET_MASK \
				0x00040000	/**< Set RX_PHY_PRBSCNTRESET. */
#define XDP_RX_PHY_CONFIG_RX_PHY_RXLPMHFHOLD_MASK \
				0x00080000	/**< Set RX_PHY_RXLPMHFHOLD. */
#define XDP_RX_PHY_CONFIG_RX_PHY_RXLPMLFHOLD_MASK \
				0x00100000	/**< Set RX_PHY_RXLPMLFHOLD. */
#define XDP_RX_PHY_CONFIG_RX_PHY_RXLPMHFOVERDEN_MASK \
				0x00200000	/**< Set RX_PHY_
							RXLPMHFOVERDEN. */
#define XDP_RX_PHY_CONFIG_RX_PHY_CDRHOLD_MASK \
				0x00400000	/**< Set RX_PHY_CDRHOLD. */
#define XDP_RX_PHY_CONFIG_RESET_AT_TRAIN_ITER_MASK \
				0x00800000	/**< Issue reset at every
							training iteration. */
#define XDP_RX_PHY_CONFIG_RESET_AT_LINK_RATE_CHANGE_MASK \
				0x01000000	/**< Issue reset at every link
							rate change. */
#define XDP_RX_PHY_CONFIG_RESET_AT_TP1_START_MASK \
				0x02000000	/**< Issue reset at start of
							training pattern 1. */
#define XDP_RX_PHY_CONFIG_EN_CFG_RX_PHY_POLARITY_MASK \
				0x04000000	/**< Enable the individual lane
							polarity. */
#define XDP_RX_PHY_CONFIG_RX_PHY_POLARITY_LANE0_MASK \
				0x08000000	/**< Configure RX_PHY_POLARITY
							for lane 0. */
#define XDP_RX_PHY_CONFIG_RX_PHY_POLARITY_LANE1_MASK \
				0x10000000	/**< Configure RX_PHY_POLARITY
							for lane 1. */
#define XDP_RX_PHY_CONFIG_RX_PHY_POLARITY_LANE2_MASK \
				0x20000000	/**< Configure RX_PHY_POLARITY
							for lane 2. */
#define XDP_RX_PHY_CONFIG_RX_PHY_POLARITY_LANE3_MASK \
				0x40000000	/**< Configure RX_PHY_POLARITY
							for lane 3. */
#define XDP_RX_PHY_CONFIG_GT_ALL_RESET_MASK \
				0x00000003	/**< Reset GT and PHY. */
/* 0x208: PHY_STATUS */
#define XDP_RX_PHY_STATUS_RESET_LANE_0_1_DONE_MASK \
				0x00000003	/**< Reset done for lanes
							0 and 1. */
#define XDP_RX_PHY_STATUS_RESET_LANE_2_3_DONE_MASK \
				0x0000000C	/**< Reset done for lanes
							2 and 3. */
#define XDP_RX_PHY_STATUS_RESET_LANE_2_3_DONE_SHIFT \
				2		/**< Shift bits for reset done
							for lanes 2 and 3. */
#define XDP_RX_PHY_STATUS_PLL_LANE0_1_LOCK_MASK \
				0x00000010	/**< PLL locked for lanes
							0 and 1. */
#define XDP_RX_PHY_STATUS_PLL_LANE2_3_LOCK_MASK \
				0x00000020	/**< PLL locked for lanes
							2 and 3. */
#define XDP_RX_PHY_STATUS_PLL_FABRIC_LOCK_MASK \
				0x00000040	/**< FPGA fabric clock PLL
							locked. */
#define XDP_RX_PHY_STATUS_RX_CLK_LOCK_MASK \
				0x00000080	/**< Receiver clock locked. */
#define XDP_RX_PHY_STATUS_PRBSERR_LANE_0_MASK \
				0x00000100	/**< PRBS error on lane 0. */
#define XDP_RX_PHY_STATUS_PRBSERR_LANE_1_MASK \
				0x00000200	/**< PRBS error on lane 1. */
#define XDP_RX_PHY_STATUS_PRBSERR_LANE_2_MASK \
				0x00000400	/**< PRBS error on lane 2. */
#define XDP_RX_PHY_STATUS_PRBSERR_LANE_3_MASK \
				0x00000800	/**< PRBS error on lane 3. */
#define XDP_RX_PHY_STATUS_RX_VLOW_LANE_0_MASK \
				0x00001000	/**< RX voltage low on lane
							0. */
#define XDP_RX_PHY_STATUS_RX_VLOW_LANE_1_MASK \
				0x00002000	/**< RX voltage low on lane
							1. */
#define XDP_RX_PHY_STATUS_RX_VLOW_LANE_2_MASK \
				0x00004000	/**< RX voltage low on lane
							2. */
#define XDP_RX_PHY_STATUS_RX_VLOW_LANE_3_MASK \
				0x00008000	/**< RX voltage low on lane
							3. */
#define XDP_RX_PHY_STATUS_LANE_ALIGN_LANE_0_MASK \
				0x00010000	/**< Lane alignment status
							for lane 0. */
#define XDP_RX_PHY_STATUS_LANE_ALIGN_LANE_1_MASK \
				0x00020000	/**< Lane alignment status
							for lane 1. */
#define XDP_RX_PHY_STATUS_LANE_ALIGN_LANE_2_MASK \
				0x00040000	/**< Lane alignment status
							for lane 2. */
#define XDP_RX_PHY_STATUS_LANE_ALIGN_LANE_3_MASK \
				0x00080000	/**< Lane alignment status
							for lane 3. */
#define XDP_RX_PHY_STATUS_SYM_LOCK_LANE_0_MASK \
				0x00100000	/**< Symbol lock status for
							lane 0. */
#define XDP_RX_PHY_STATUS_SYM_LOCK_LANE_1_MASK \
				0x00200000	/**< Symbol lock status for
							lane 1. */
#define XDP_RX_PHY_STATUS_SYM_LOCK_LANE_2_MASK \
				0x00400000	/**< Symbol lock status for
							lane 2. */
#define XDP_RX_PHY_STATUS_SYM_LOCK_LANE_3_MASK \
				0x00800000	/**< Symbol lock status for
							lane 3. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_0_MASK \
				0x03000000	/**< RX buffer status lane 0. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_0_SHIFT \
				24		/**< Shift bits for RX buffer
							status lane 0. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_1_MASK \
				0x0C000000	/**< RX buffer status lane 1. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUE_LANE_1_SHIFT \
				26		/**< Shift bits for RX buffer
							status lane 1. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_2_MASK \
				0x30000000	/**< RX buffer status lane 2. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_2_SHIFT \
				28		/**< Shift bits for RX buffer
							status lane 2. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_3_MASK \
				0xC0000000	/**< RX buffer status lane 3. */
#define XDP_RX_PHY_STATUS_RX_BUFFER_STATUS_LANE_3_SHIFT \
				30		/**< Shift bits for RX buffer
							status lane 3. */
#define XDP_RX_PHY_STATUS_LANES_0_1_READY_MASK \
				0x00000013	/**< Lanes 0 and 1 are ready. */
#define XDP_RX_PHY_STATUS_ALL_LANES_READY_MASK \
				0x0000003F	/**< All lanes are ready. */
/* 0x210: PHY_POWER_DOWN */
#define XDP_RX_PHY_POWER_DOWN_LANE_0_MASK 0x1	/**< Power down the PHY for lane
							0. */
#define XDP_RX_PHY_POWER_DOWN_LANE_1_MASK 0x2	/**< Power down the PHY for lane
							1. */
#define XDP_RX_PHY_POWER_DOWN_LANE_2_MASK 0x4	/**< Power down the PHY for lane
							2. */
#define XDP_RX_PHY_POWER_DOWN_LANE_3_MASK 0x8	/**< Power down the PHY for lane
							3. */
/* 0x214: MIN_VOLTAGE_SWING */
#define XDP_RX_MIN_VOLTAGE_SWING_MIN_MASK \
				0x000003	/**< The minimum voltage swing
							level. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_MASK \
				0x00000C	/**< Clock recovery options. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_SHIFT 2	/**< Shift bits for clock
							recovery options. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_VS_INC \
				0x0		/**< Increment voltage swing
							adjust request every
							training iteration. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_VS_INC_4CNT \
				0x1		/**< Increment voltage swing
							adjust request every
							4 or VS_SWEEP_CNT
							iterations. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_VS_HOLD \
				0x2		/**< Hold adjust request to
							SET_VS. */
#define XDP_RX_MIN_VOLTAGE_SWING_CR_OPT_VS_NA \
				0x3		/**< Not applicable. */
#define XDP_RX_MIN_VOLTAGE_SWING_VS_SWEEP_CNT_MASK \
				0x000070	/**< Voltage swing sweep
							count. */

#define XDP_RX_MIN_VOLTAGE_SWING_VS_SWEEP_CNT_SHIFT \
				4		/**< Shift bits for voltage
							swing sweep count. */
#define XDP_RX_MIN_VOLTAGE_SWING_SET_VS_MASK \
				0x000300	/**< Set voltage swing level. */
#define XDP_RX_MIN_VOLTAGE_SWING_SET_VS_SHIFT 8	/**< Shift bits for voltage
							swing setting. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_MASK \
				0x000C00	/**< Channel equalization
							options. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_SHIFT 10 /**< Shift bits for channel
							equalization options. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_PE_INC \
				0x0		/**< Increment pre-emphasis
							adjust request every
							training iteration until
							maximum level, SET_PE,
							is reached. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_PE_HOLD \
				0x1		/**< Hold adjust request to
							SET_PE. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_PE_TABLE \
				0x3		/**< Pick pre-emphasis values
							from PE_TABLE. */
#define XDP_RX_MIN_VOLTAGE_SWING_CE_OPT_VS_NA \
				0x2		/**< Not applicable. */
#define XDP_RX_MIN_VOLTAGE_SWING_SET_PE_MASK \
				0x003000	/**< Set pre-emphasis level. */
#define XDP_RX_MIN_VOLTAGE_SWING_SET_PE_SHIFT 12 /**< Shift bits for
							pre-emphasis setting. */
#define XDP_RX_MIN_VOLTAGE_SWING_PE_TABLE_MASK(Iteration) \
	(0x3 << (14 + ((Iteration - 1) * 2)))	/**< Table specifying what
							pre-emphasis level to
							request for each
							training iteration. */
#define XDP_RX_MIN_VOLTAGE_SWING_PE_TABLE_SHIFT(Iteration) \
	(14 + ((Iteration - 1) * 2))		/**< Shift bits for
							pre-emphasis table. */
/* 0x21C: CDR_CONTROL_CONFIG */
#define XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_TO_MASK \
				0x000FFFFF	/**< Controls the CDR tDLOCK
							timeout value. */
#define XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159 \
				0x1388		/**< CDR tDLOCK calibration
							value using DP159. */
#define XDP_RX_CDR_CONTROL_CONFIG_DFE_CTRL_MASK \
				0x80000000	/**< Use DFE control. */
#define XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT \
				0X40000000	/**< Timeout for MST mode. */
/* @} */

#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

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
#define XDP_DPCD_REV						0x00000
#define XDP_DPCD_MAX_LINK_RATE					0x00001
#define XDP_DPCD_MAX_LANE_COUNT					0x00002
#define XDP_DPCD_MAX_DOWNSPREAD					0x00003
#define XDP_DPCD_NORP_PWR_V_CAP					0x00004
#define XDP_DPCD_DOWNSP_PRESENT					0x00005
#define XDP_DPCD_ML_CH_CODING_CAP				0x00006
#define XDP_DPCD_DOWNSP_COUNT_MSA_OUI				0x00007
#define	XDP_DPCD_RX_PORT0_CAP_0					0x00008
#define	XDP_DPCD_RX_PORT0_CAP_1					0x00009
#define	XDP_DPCD_RX_PORT1_CAP_0					0x0000A
#define	XDP_DPCD_RX_PORT1_CAP_1					0x0000B
#define XDP_DPCD_I2C_SPEED_CTL_CAP				0x0000C
#define XDP_DPCD_EDP_CFG_CAP					0x0000D
#define XDP_DPCD_TRAIN_AUX_RD_INTERVAL				0x0000E
#define XDP_DPCD_ADAPTER_CAP					0x0000F
#define XDP_DPCD_FAUX_CAP					0x00020
#define XDP_DPCD_MSTM_CAP					0x00021
#define XDP_DPCD_NUM_AUDIO_EPS					0x00022
#define	XDP_DPCD_AV_GRANULARITY					0x00023
#define XDP_DPCD_AUD_DEC_LAT_7_0				0x00024
#define XDP_DPCD_AUD_DEC_LAT_15_8				0x00025
#define XDP_DPCD_AUD_PP_LAT_7_0					0x00026
#define XDP_DPCD_AUD_PP_LAT_15_8				0x00027
#define XDP_DPCD_VID_INTER_LAT					0x00028
#define XDP_DPCD_VID_PROG_LAT					0x00029
#define XDP_DPCD_REP_LAT					0x0002A
#define XDP_DPCD_AUD_DEL_INS_7_0				0x0002B
#define XDP_DPCD_AUD_DEL_INS_15_8				0x0002C
#define XDP_DPCD_AUD_DEL_INS_23_16				0x0002D
#define XDP_DPCD_GUID						0x00030
#define XDP_DPCD_RX_GTC_VALUE_7_0				0x00054
#define XDP_DPCD_RX_GTC_VALUE_15_8				0x00055
#define XDP_DPCD_RX_GTC_VALUE_23_16				0x00056
#define XDP_DPCD_RX_GTC_VALUE_31_24				0x00057
#define XDP_DPCD_RX_GTC_MSTR_REQ				0x00058
#define XDP_DPCD_RX_GTC_FREQ_LOCK_DONE				0x00059
#define XDP_DPCD_DOWNSP_0_CAP					0x00080
#define XDP_DPCD_DOWNSP_1_CAP					0x00081
#define XDP_DPCD_DOWNSP_2_CAP					0x00082
#define XDP_DPCD_DOWNSP_3_CAP					0x00083
#define XDP_DPCD_DOWNSP_0_DET_CAP				0x00080
#define XDP_DPCD_DOWNSP_1_DET_CAP				0x00084
#define XDP_DPCD_DOWNSP_2_DET_CAP				0x00088
#define XDP_DPCD_DOWNSP_3_DET_CAP				0x0008C
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field.
  * @{
  */
#define XDP_DPCD_LINK_BW_SET					0x00100
#define XDP_DPCD_LANE_COUNT_SET					0x00101
#define XDP_DPCD_TP_SET						0x00102
#define XDP_DPCD_TRAINING_LANE0_SET				0x00103
#define XDP_DPCD_TRAINING_LANE1_SET				0x00104
#define XDP_DPCD_TRAINING_LANE2_SET				0x00105
#define XDP_DPCD_TRAINING_LANE3_SET				0x00106
#define XDP_DPCD_DOWNSPREAD_CTRL				0x00107
#define XDP_DPCD_ML_CH_CODING_SET				0x00108
#define XDP_DPCD_I2C_SPEED_CTL_SET				0x00109
#define XDP_DPCD_EDP_CFG_SET					0x0010A
#define XDP_DPCD_LINK_QUAL_LANE0_SET				0x0010B
#define XDP_DPCD_LINK_QUAL_LANE1_SET				0x0010C
#define XDP_DPCD_LINK_QUAL_LANE2_SET				0x0010D
#define XDP_DPCD_LINK_QUAL_LANE3_SET				0x0010E
#define XDP_DPCD_TRAINING_LANE0_1_SET2				0x0010F
#define XDP_DPCD_TRAINING_LANE2_3_SET2				0x00110
#define XDP_DPCD_MSTM_CTRL					0x00111
#define XDP_DPCD_AUDIO_DELAY_7_0				0x00112
#define XDP_DPCD_AUDIO_DELAY_15_8				0x00113
#define XDP_DPCD_AUDIO_DELAY_23_6				0x00114
#define XDP_DPCD_UPSTREAM_DEVICE_DP_PWR_NEED			0x00118
#define XDP_DPCD_FAUX_MODE_CTRL					0x00120
#define XDP_DPCD_FAUX_FORWARD_CH_DRIVE_SET			0x00121
#define XDP_DPCD_BACK_CH_STATUS					0x00122
#define XDP_DPCD_FAUX_BACK_CH_SYMBOL_ERROR_COUNT		0x00123
#define XDP_DPCD_FAUX_BACK_CH_TRAINING_PATTERN_TIME		0x00125
#define XDP_DPCD_TX_GTC_VALUE_7_0				0x00154
#define XDP_DPCD_TX_GTC_VALUE_15_8				0x00155
#define XDP_DPCD_TX_GTC_VALUE_23_16				0x00156
#define XDP_DPCD_TX_GTC_VALUE_31_24				0x00157
#define XDP_DPCD_RX_GTC_VALUE_PHASE_SKEW_EN			0x00158
#define XDP_DPCD_TX_GTC_FREQ_LOCK_DONE				0x00159
#define XDP_DPCD_ADAPTER_CTRL					0x001A0
#define XDP_DPCD_BRANCH_DEVICE_CTRL				0x001A1
#define XDP_DPCD_PAYLOAD_ALLOCATE_SET				0x001C0
#define XDP_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT		0x001C1
#define XDP_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT		0x001C2
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field.
  * @{
  */
#define XDP_DPCD_SINK_COUNT					0x00200
#define XDP_DPCD_DEVICE_SERVICE_IRQ				0x00201
#define XDP_DPCD_STATUS_LANE_0_1				0x00202
#define XDP_DPCD_STATUS_LANE_2_3				0x00203
#define XDP_DPCD_LANE_ALIGN_STATUS_UPDATED			0x00204
#define XDP_DPCD_SINK_STATUS					0x00205
#define XDP_DPCD_ADJ_REQ_LANE_0_1				0x00206
#define XDP_DPCD_ADJ_REQ_LANE_2_3				0x00207
#define XDP_DPCD_TRAINING_SCORE_LANE_0				0x00208
#define XDP_DPCD_TRAINING_SCORE_LANE_1				0x00209
#define XDP_DPCD_TRAINING_SCORE_LANE_2				0x0020A
#define XDP_DPCD_TRAINING_SCORE_LANE_3				0x0020B
#define XDP_DPCD_ADJ_REQ_PC2					0x0020C
#define XDP_DPCD_FAUX_FORWARD_CH_SYMBOL_ERROR_COUNT		0x0020D
#define XDP_DPCD_SYMBOL_ERROR_COUNT_LANE_0			0x00210
#define XDP_DPCD_SYMBOL_ERROR_COUNT_LANE_1			0x00212
#define XDP_DPCD_SYMBOL_ERROR_COUNT_LANE_2			0x00214
#define XDP_DPCD_SYMBOL_ERROR_COUNT_LANE_3			0x00216
/* @} */

/** @name DisplayPort Configuration Data: Automated testing sub-field.
  * @{
  */
#define XDP_DPCD_FAUX_FORWARD_CH_STATUS				0x00280
#define XDP_DPCD_FAUX_BACK_CH_DRIVE_SET				0x00281
#define XDP_DPCD_FAUX_BACK_CH_SYM_ERR_COUNT_CTRL		0x00282
#define XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS			0x002C0
#define XDP_DPCD_VC_PAYLOAD_ID_SLOT(SlotNum) \
			(XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS + SlotNum)
/* @} */

/** @name DisplayPort Configuration Data: Sink control field.
  * @{
  */
#define XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE			0x00600
/* @} */

/** @name DisplayPort Configuration Data: Sideband message buffers.
  * @{
  */
#define XDP_DPCD_DOWN_REQ					0x01000
#define XDP_DPCD_UP_REP						0x01200
#define XDP_DPCD_DOWN_REP					0x01400
#define XDP_DPCD_UP_REQ						0x01600
/* @} */

/** @name DisplayPort Configuration Data: Event status indicator field.
  * @{
  */
#define XDP_DPCD_SINK_COUNT_ESI					0x02002
#define XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0		0x02003
#define XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI1		0x02004
#define XDP_DPCD_SINK_LINK_SERVICE_IRQ_VECTOR_ESI0		0x02005
#define XDP_DPCD_SINK_LANE0_1_STATUS				0x0200C
#define XDP_DPCD_SINK_LANE2_3_STATUS				0x0200D
#define XDP_DPCD_SINK_ALIGN_STATUS_UPDATED_ESI			0x0200E
#define XDP_DPCD_SINK_STATUS_ESI				0x0200F
/* @} */

/** @name DisplayPort Configuration Data: Field addresses and sizes.
  * @{
  */
#define XDP_DPCD_RECEIVER_CAP_FIELD_START		XDP_DPCD_REV
#define XDP_DPCD_RECEIVER_CAP_FIELD_SIZE			0x100
#define XDP_DPCD_LINK_CFG_FIELD_START		XDP_DPCD_LINK_BW_SET
#define XDP_DPCD_LINK_CFG_FIELD_SIZE				0x100
#define XDP_DPCD_LINK_SINK_STATUS_FIELD_START	XDP_DPCD_SINK_COUNT
#define XDP_DPCD_LINK_SINK_STATUS_FIELD_SIZE			0x17
/* @} */

/******************************************************************************/

/** @name DisplayPort Configuration Data: Receiver capability field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00000: DPCD_REV */
#define XDP_DPCD_REV_MNR_MASK					0x0F
#define XDP_DPCD_REV_MJR_MASK					0xF0
#define XDP_DPCD_REV_MJR_SHIFT					4
/* 0x00001: MAX_LINK_RATE */
#define XDP_DPCD_MAX_LINK_RATE_162GBPS				0x06
#define XDP_DPCD_MAX_LINK_RATE_270GBPS				0x0A
#define XDP_DPCD_MAX_LINK_RATE_540GBPS				0x14
/* 0x00002: MAX_LANE_COUNT */
#define XDP_DPCD_MAX_LANE_COUNT_MASK				0x1F
#define XDP_DPCD_MAX_LANE_COUNT_1				0x01
#define XDP_DPCD_MAX_LANE_COUNT_2				0x02
#define XDP_DPCD_MAX_LANE_COUNT_4				0x04
#define XDP_DPCD_TPS3_SUPPORT_MASK				0x40
#define XDP_DPCD_ENHANCED_FRAME_SUPPORT_MASK			0x80
/* 0x00003: MAX_DOWNSPREAD */
#define XDP_DPCD_MAX_DOWNSPREAD_MASK				0x01
#define XDP_DPCD_NO_AUX_HANDSHAKE_LINK_TRAIN_MASK		0x40
/* 0x00005: DOWNSP_PRESENT */
#define XDP_DPCD_DOWNSP_PRESENT_MASK				0x01
#define XDP_DPCD_DOWNSP_TYPE_MASK				0x06
#define XDP_DPCD_DOWNSP_TYPE_SHIFT				1
#define XDP_DPCD_DOWNSP_TYPE_DP					0x0
#define XDP_DPCD_DOWNSP_TYPE_AVGA_ADVII				0x1
#define XDP_DPCD_DOWNSP_TYPE_DVI_HDMI_DPPP			0x2
#define XDP_DPCD_DOWNSP_TYPE_OTHERS				0x3
#define XDP_DPCD_DOWNSP_FORMAT_CONV_MASK			0x08
#define XDP_DPCD_DOWNSP_DCAP_INFO_AVAIL_MASK			0x10
/* 0x00006, 0x00108: ML_CH_CODING_SUPPORT, ML_CH_CODING_SET */
#define XDP_DPCD_ML_CH_CODING_MASK				0x01
/* 0x00007: DOWNSP_COUNT_MSA_OUI */
#define XDP_DPCD_DOWNSP_COUNT_MASK				0x0F
#define XDP_DPCD_MSA_TIMING_PAR_IGNORED_MASK			0x40
#define XDP_DPCD_OUI_SUPPORT_MASK				0x80
/* 0x00008, 0x0000A: RX_PORT[0-1]_CAP_0 */
#define XDP_DPCD_RX_PORTX_CAP_0_LOCAL_EDID_PRESENT_MASK		0x02
#define XDP_DPCD_RX_PORTX_CAP_0_ASSOC_TO_PRECEDING_PORT_MASK	0x04
/* 0x0000C, 0x00109: I2C_SPEED_CTL_CAP, I2C_SPEED_CTL_SET */
#define XDP_DPCD_I2C_SPEED_CTL_NONE				0x00
#define XDP_DPCD_I2C_SPEED_CTL_1KBIPS				0x01
#define XDP_DPCD_I2C_SPEED_CTL_5KBIPS				0x02
#define XDP_DPCD_I2C_SPEED_CTL_10KBIPS				0x04
#define XDP_DPCD_I2C_SPEED_CTL_100KBIPS				0x08
#define XDP_DPCD_I2C_SPEED_CTL_400KBIPS				0x10
#define XDP_DPCD_I2C_SPEED_CTL_1MBIPS				0x20
/* 0x0000E: TRAIN_AUX_RD_INTERVAL */
#define XDP_DPCD_TRAIN_AUX_RD_INT_100_400US			0x00
#define XDP_DPCD_TRAIN_AUX_RD_INT_4MS				0x01
#define XDP_DPCD_TRAIN_AUX_RD_INT_8MS				0x02
#define XDP_DPCD_TRAIN_AUX_RD_INT_12MS				0x03
#define XDP_DPCD_TRAIN_AUX_RD_INT_16MS				0x04
/* 0x00020: DPCD_FAUX_CAP */
#define XDP_DPCD_FAUX_CAP_MASK					0x01
/* 0x00021: MSTM_CAP */
#define XDP_DPCD_MST_CAP_MASK					0x01
/* 0x00080, 0x00081|4, 0x00082|8, 0x00083|C: DOWNSP_X_(DET_)CAP */
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_MASK				0x07
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_DP				0x0
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_AVGA				0x1
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_DVI				0x2
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_HDMI				0x3
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_OTHERS			0x4
#define XDP_DPCD_DOWNSP_X_CAP_TYPE_DPPP				0x5
#define XDP_DPCD_DOWNSP_X_CAP_HPD_MASK				0x80
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_MASK		0xF0
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_SHIFT		4
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_60	0x1
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_50	0x2
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_60	0x3
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_50	0x4
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_60	0x5
#define XDP_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_50	0x7
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDP_DPCD_DOWNSP_X_DCAP_MAX_BPC_MASK			0x03
#define XDP_DPCD_DOWNSP_X_DCAP_MAX_BPC_8			0x0
#define XDP_DPCD_DOWNSP_X_DCAP_MAX_BPC_10			0x1
#define XDP_DPCD_DOWNSP_X_DCAP_MAX_BPC_12			0x2
#define XDP_DPCD_DOWNSP_X_DCAP_MAX_BPC_16			0x3
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDP_DPCD_DOWNSP_X_DCAP_HDMI_DPPP_FS2FP_MASK		0x01
#define XDP_DPCD_DOWNSP_X_DCAP_DVI_DL_MASK			0x02
#define XDP_DPCD_DOWNSP_X_DCAP_DVI_HCD_MASK			0x04
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00100: XDP_DPCD_LINK_BW_SET */
#define XDP_DPCD_LINK_BW_SET_162GBPS				0x06
#define XDP_DPCD_LINK_BW_SET_270GBPS				0x0A
#define XDP_DPCD_LINK_BW_SET_540GBPS				0x14
/* 0x00101: LANE_COUNT_SET */
#define XDP_DPCD_LANE_COUNT_SET_MASK				0x1F
#define XDP_DPCD_LANE_COUNT_SET_1				0x01
#define XDP_DPCD_LANE_COUNT_SET_2				0x02
#define XDP_DPCD_LANE_COUNT_SET_4				0x04
#define XDP_DPCD_ENHANCED_FRAME_EN_MASK				0x80
/* 0x00102: TP_SET */
#define XDP_DPCD_TP_SEL_MASK					0x03
#define XDP_DPCD_TP_SEL_OFF					0x0
#define XDP_DPCD_TP_SEL_TP1					0x1
#define XDP_DPCD_TP_SEL_TP2					0x2
#define XDP_DPCD_TP_SEL_TP3					0x3
#define XDP_DPCD_TP_SET_LQP_MASK				0x06
#define XDP_DPCD_TP_SET_LQP_SHIFT				2
#define XDP_DPCD_TP_SET_LQP_OFF					0x0
#define XDP_DPCD_TP_SET_LQP_D102_TEST				0x1
#define XDP_DPCD_TP_SET_LQP_SER_MES				0x2
#define XDP_DPCD_TP_SET_LQP_PRBS7				0x3
#define XDP_DPCD_TP_SET_REC_CLK_OUT_EN_MASK			0x10
#define XDP_DPCD_TP_SET_SCRAMB_DIS_MASK				0x20
#define XDP_DPCD_TP_SET_SE_COUNT_SEL_MASK			0xC0
#define XDP_DPCD_TP_SET_SE_COUNT_SEL_SHIFT			6
#define XDP_DPCD_TP_SET_SE_COUNT_SEL_DE_ISE			0x0
#define XDP_DPCD_TP_SET_SE_COUNT_SEL_DE				0x1
#define XDP_DPCD_TP_SET_SE_COUNT_SEL_ISE			0x2
/* 0x00103-0x00106: TRAINING_LANE[0-3]_SET */
#define XDP_DPCD_TRAINING_LANEX_SET_VS_MASK			0x03
#define XDP_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK			0x04
#define XDP_DPCD_TRAINING_LANEX_SET_PE_MASK			0x18
#define XDP_DPCD_TRAINING_LANEX_SET_PE_SHIFT			3
#define XDP_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK			0x20
/* 0x00107: DOWNSPREAD_CTRL */
#define XDP_DPCD_SPREAD_AMP_MASK				0x10
#define XDP_DPCD_MSA_TIMING_PAR_IGNORED_EN_MASK			0x80
/* 0x00108: ML_CH_CODING_SET - Same as 0x00006: ML_CH_CODING_SUPPORT */
/* 0x00109: I2C_SPEED_CTL_SET - Same as 0x0000C: I2C_SPEED_CTL_CAP */
/* 0x0010F-0x00110: TRAINING_LANE[0_1-2_3]_SET2 */
#define XDP_DPCD_TRAINING_LANE_0_2_SET_PC2_MASK			0x03
#define XDP_DPCD_TRAINING_LANE_0_2_SET_MAX_PC2_MASK		0x04
#define XDP_DPCD_TRAINING_LANE_1_3_SET_PC2_MASK			0x30
#define XDP_DPCD_TRAINING_LANE_1_3_SET_PC2_SHIFT		4
#define XDP_DPCD_TRAINING_LANE_1_3_SET_MAX_PC2_MASK		0x40
/* 0x00111: MSTM_CTRL */
#define XDP_DPCD_MST_EN_MASK					0x01
#define XDP_DPCD_UP_REQ_EN_MASK					0x02
#define XDP_DPCD_UP_IS_SRC_MASK					0x03
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field masks, shifts,
  *       and register values.
  * @{
  */
/* 0x00200: SINK_COUNT */
#define XDP_DPCD_SINK_COUNT_LOW_MASK				0x3F
#define XDP_DPCD_SINK_CP_READY_MASK				0x40
#define XDP_DPCD_SINK_COUNT_HIGH_MASK				0x80
#define XDP_DPCD_SINK_COUNT_HIGH_LOW_SHIFT			1
/* 0x00202: STATUS_LANE_0_1 */
#define XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK			0x01
#define XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK			0x02
#define XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK			0x04
#define XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK			0x10
#define XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK			0x20
#define XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK			0x40
/* 0x00202: STATUS_LANE_2_3 */
#define XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK			0x01
#define XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK			0x02
#define XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK			0x04
#define XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK			0x10
#define XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK			0x20
#define XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK			0x40
/* 0x00204: LANE_ALIGN_STATUS_UPDATED */
#define XDP_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK \
								0x01
#define XDP_DPCD_LANE_ALIGN_STATUS_UPDATED_DOWNSP_STATUS_CHANGED_MASK \
								0x40
#define XDP_DPCD_LANE_ALIGN_STATUS_UPDATED_LINK_STATUS_UPDATED_MASK \
								0x80
/* 0x00205: SINK_STATUS */
#define XDP_DPCD_SINK_STATUS_RX_PORT0_SYNC_STATUS_MASK		0x01
#define XDP_DPCD_SINK_STATUS_RX_PORT1_SYNC_STATUS_MASK		0x02

/* 0x00206, 0x00207: ADJ_REQ_LANE_[0,2]_[1,3] */
#define XDP_DPCD_ADJ_REQ_LANE_0_2_VS_MASK			0x03
#define XDP_DPCD_ADJ_REQ_LANE_0_2_PE_MASK			0x0C
#define XDP_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT			2
#define XDP_DPCD_ADJ_REQ_LANE_1_3_VS_MASK			0x30
#define XDP_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT			4
#define XDP_DPCD_ADJ_REQ_LANE_1_3_PE_MASK			0xC0
#define XDP_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT			6
/* 0x0020C: ADJ_REQ_PC2 */
#define XDP_DPCD_ADJ_REQ_PC2_LANE_0_MASK			0x03
#define XDP_DPCD_ADJ_REQ_PC2_LANE_1_MASK			0x0C
#define XDP_DPCD_ADJ_REQ_PC2_LANE_1_SHIFT			2
#define XDP_DPCD_ADJ_REQ_PC2_LANE_2_MASK			0x30
#define XDP_DPCD_ADJ_REQ_PC2_LANE_2_SHIFT			4
#define XDP_DPCD_ADJ_REQ_PC2_LANE_3_MASK			0xC0
#define XDP_DPCD_ADJ_REQ_PC2_LANE_3_SHIFT			6
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
#define XDP_SEGPTR_ADDR						0x30
#define XDP_EDID_ADDR						0x50
#define XDP_EDID_BLOCK_SIZE					128
#define XDP_EDID_DTD_DD(Num)				(0x36 + (18 * Num))
#define XDP_EDID_PTM					XDP_EDID_DTD_DD(0)
#define XDP_EDID_EXT_BLOCK_COUNT				0x7E
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Detailed Timing Descriptor (DTD).
  * @{
  */
#define XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB				0x00
#define XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB				0x01
#define XDP_EDID_DTD_HRES_LSB					0x02
#define XDP_EDID_DTD_HBLANK_LSB					0x03
#define XDP_EDID_DTD_HRES_HBLANK_U4				0x04
#define XDP_EDID_DTD_VRES_LSB					0x05
#define XDP_EDID_DTD_VBLANK_LSB					0x06
#define XDP_EDID_DTD_VRES_VBLANK_U4				0x07
#define XDP_EDID_DTD_HFPORCH_LSB				0x08
#define XDP_EDID_DTD_HSPW_LSB					0x09
#define XDP_EDID_DTD_VFPORCH_VSPW_L4				0x0A
#define XDP_EDID_DTD_XFPORCH_XSPW_U2				0x0B
#define XDP_EDID_DTD_HIMGSIZE_MM_LSB				0x0C
#define XDP_EDID_DTD_VIMGSIZE_MM_LSB				0x0D
#define XDP_EDID_DTD_XIMGSIZE_MM_U4				0x0E
#define XDP_EDID_DTD_HBORDER					0x0F
#define XDP_EDID_DTD_VBORDER					0x10
#define XDP_EDID_DTD_SIGNAL					0x11

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values.
  * @{
  */
#define XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK			0x0F
#define XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK			0xF0
#define XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT			4
#define XDP_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK			0x0F
#define XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK		0xF0
#define XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT		4
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK		0xC0
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK			0x30
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK		0x0C
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK			0x03
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT		6
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT			4
#define XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT		2
#define XDP_EDID_DTD_XIMGSIZE_MM_U4_VIMGSIZE_MM_MASK		0x0F
#define XDP_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_MASK		0xF0
#define XDP_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_SHIFT		4
#define XDP_EDID_DTD_SIGNAL_HPOLARITY_MASK			0x02
#define XDP_EDID_DTD_SIGNAL_VPOLARITY_MASK			0x04
#define XDP_EDID_DTD_SIGNAL_HPOLARITY_SHIFT			1
#define XDP_EDID_DTD_SIGNAL_VPOLARITY_SHIFT			2
/* @} */


#if XPAR_XDPTXSS_NUM_INSTANCES
/** @name Extended Display Identification Data: Register offsets for the
  *       DisplayID extension block.
  * @{
  */
#define XDP_EDID_EXT_BLOCK_TAG			0x00
#define XDP_TX_DISPID_VER_REV			0x00
#define XDP_TX_DISPID_SIZE			0x01
#define XDP_TX_DISPID_TYPE			0x02
#define XDP_TX_DISPID_EXT_COUNT			0x03
#define XDP_TX_DISPID_PAYLOAD_START		0x04
#define XDP_TX_DISPID_DB_SEC_TAG		0x00
#define XDP_TX_DISPID_DB_SEC_REV		0x01
#define XDP_TX_DISPID_DB_SEC_SIZE		0x02
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the DisplayID extension block.
  * @{
  */
#define XDP_EDID_EXT_BLOCK_TAG_DISPID	0x70
#define XDP_TX_DISPID_TDT_TAG			0x12
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDP_TX_DISPID_TDT_TOP0			0x04
#define XDP_TX_DISPID_TDT_TOP1			0x05
#define XDP_TX_DISPID_TDT_TOP2			0x06
#define XDP_TX_DISPID_TDT_HSIZE0		0x07
#define XDP_TX_DISPID_TDT_HSIZE1		0x08
#define XDP_TX_DISPID_TDT_VSIZE0		0x09
#define XDP_TX_DISPID_TDT_VSIZE1		0x0A
#define XDP_TX_DISPID_TDT_VENID0		0x10
#define XDP_TX_DISPID_TDT_VENID1		0x11
#define XDP_TX_DISPID_TDT_VENID2		0x12
#define XDP_TX_DISPID_TDT_PCODE0		0x13
#define XDP_TX_DISPID_TDT_PCODE1		0x14
#define XDP_TX_DISPID_TDT_SN0			0x15
#define XDP_TX_DISPID_TDT_SN1			0x16
#define XDP_TX_DISPID_TDT_SN2			0x17
#define XDP_TX_DISPID_TDT_SN3			0x18
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDP_TX_DISPID_TDT_TOP0_HTOT_L_SHIFT	4
#define XDP_TX_DISPID_TDT_TOP0_HTOT_L_MASK	(0xF << 4)
#define XDP_TX_DISPID_TDT_TOP0_VTOT_L_MASK	0xF
#define XDP_TX_DISPID_TDT_TOP1_HLOC_L_SHIFT	4
#define XDP_TX_DISPID_TDT_TOP1_HLOC_L_MASK	(0xF << 4)
#define XDP_TX_DISPID_TDT_TOP1_VLOC_L_MASK	0xF
#define XDP_TX_DISPID_TDT_TOP2_HTOT_H_SHIFT	6
#define XDP_TX_DISPID_TDT_TOP2_HTOT_H_MASK	(0x3 << 6)
#define XDP_TX_DISPID_TDT_TOP2_VTOT_H_SHIFT	4
#define XDP_TX_DISPID_TDT_TOP2_VTOT_H_MASK	(0x3 << 4)
#define XDP_TX_DISPID_TDT_TOP2_HLOC_H_SHIFT	2
#define XDP_TX_DISPID_TDT_TOP2_HLOC_H_MASK	(0x3 << 2)
#define XDP_TX_DISPID_TDT_TOP2_VLOC_H_MASK	0x3
/* @} */
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/******************************************************************************/
/**
 * Multi-stream transport (MST) definitions.
 *
*******************************************************************************/
/** @name Stream identification.
  * @{
  */
#define XDP_TX_STREAM_ID1			1
#define XDP_TX_STREAM_ID2			2
#define XDP_TX_STREAM_ID3			3
#define XDP_TX_STREAM_ID4			4
/* @} */

/** @name Sideband message codes when the driver is in MST mode.
  * @{
  */
#define XDP_SBMSG_LINK_ADDRESS			0x01
#define XDP_SBMSG_CONNECTION_STATUS_NOTIFY	0x02
#define XDP_SBMSG_ENUM_PATH_RESOURCES		0x10
#define XDP_SBMSG_ALLOCATE_PAYLOAD		0x11
#define XDP_SBMSG_QUERY_PAYLOAD			0x12
#define XDP_SBMSG_RESOURCE_STATUS_NOTIFY	0x13
#define XDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE	0x14
#define XDP_SBMSG_REMOTE_DPCD_READ		0x20
#define XDP_SBMSG_REMOTE_DPCD_WRITE		0x21
#define XDP_SBMSG_REMOTE_I2C_READ		0x22
#define XDP_SBMSG_REMOTE_I2C_WRITE		0x23
#define XDP_SBMSG_POWER_UP_PHY			0x24
#define XDP_SBMSG_POWER_DOWN_PHY		0x25
#define XDP_SBMSG_SINK_EVENT_NOTIFY		0x30
#define XDP_SBMSG_QUERY_STREAM_ENCRYPT_STATUS	0x38
/* @} */

/** @name Sideband message codes when the driver is in MST mode.
  * @{
  */
#define XDP_SBMSG_NAK_REASON_WRITE_FAILURE	0x01
#define XDP_SBMSG_NAK_REASON_INVALID_RAD	0x02
#define XDP_SBMSG_NAK_REASON_CRC_FAILURE	0x03
#define XDP_SBMSG_NAK_REASON_BAD_PARAM		0x04
#define XDP_SBMSG_NAK_REASON_DEFER		0x05
#define XDP_SBMSG_NAK_REASON_LINK_FAILURE	0x06
#define XDP_SBMSG_NAK_REASON_NO_RESOURCES	0x07
#define XDP_SBMSG_NAK_REASON_DPCD_FAIL		0x08
#define XDP_SBMSG_NAK_REASON_I2C_NAK		0x09
#define XDP_SBMSG_NAK_REASON_ALLOCATE_FAIL	0x0A
/* @} */

#define XDP_RX_NUM_I2C_ENTRIES_PER_PORT		3 /**< The number of I2C user-
							defined entries in the
							I2C map of each port. */
#define XDP_GUID_NBYTES				16 /**< The number of bytes for
							the global unique ID. */
#define XDP_MAX_NPORTS				16 /**< The maximum number of
							ports connected to a
							DisplayPort device. */

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XDp_In32 Xil_In32
#define XDp_Out32 Xil_Out32
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
 *		u32 XDp_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XDp_ReadReg(BaseAddress, RegOffset) \
					XDp_In32((BaseAddress) + (RegOffset))

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
 *		void XDp_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XDp_WriteReg(BaseAddress, RegOffset, Data) \
				XDp_Out32((BaseAddress) + (RegOffset), (Data))


#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * Check if an Extended Display Identification Data (EDID) extension block is of
 * type DisplayID.
 *
 * @param	Ext is a pointer to the EDID extension block under comparison.
 *
 * @return
 *		- 1 if the extension block is of type DisplayID.
 *		- Otherwise.
 *
 * @note	C-style signature:
 *		u8 XDp_TxIsEdidExtBlockDispId(u8 *Ext)
 *
*******************************************************************************/
#define XDp_TxIsEdidExtBlockDispId(Ext) \
	(Ext[XDP_EDID_EXT_BLOCK_TAG] == XDP_EDID_EXT_BLOCK_TAG_DISPID)

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, retrieve the total number of
 * horizontal tiles in the tiled display. The TDT block is part of an Extended
 * Display Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The total number of horizontal tiles in the tiled display.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtHTotal(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtHTotal(Tdt) \
	(((((Tdt[XDP_TX_DISPID_TDT_TOP2] & XDP_TX_DISPID_TDT_TOP2_HTOT_H_MASK) \
	>> XDP_TX_DISPID_TDT_TOP2_HTOT_H_SHIFT) << 4) | \
	((Tdt[XDP_TX_DISPID_TDT_TOP0] & XDP_TX_DISPID_TDT_TOP0_HTOT_L_MASK) >> \
	XDP_TX_DISPID_TDT_TOP0_HTOT_L_SHIFT)) + 1)

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, retrieve the total number of
 * vertical tiles in the tiled display. The TDT block is part of an Extended
 * Display Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The total number of vertical tiles in the tiled display.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtVTotal(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtVTotal(Tdt) \
	(((((Tdt[XDP_TX_DISPID_TDT_TOP2] & XDP_TX_DISPID_TDT_TOP2_VTOT_H_MASK) \
	>> XDP_TX_DISPID_TDT_TOP2_VTOT_H_SHIFT) << 4) | \
	(Tdt[XDP_TX_DISPID_TDT_TOP0] & XDP_TX_DISPID_TDT_TOP0_VTOT_L_MASK)) + 1)

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, retrieve the horizontal tile
 * location in the tiled display. The TDT block is part of an Extended Display
 * Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The horizontal tile location in the tiled display represented by
 *		the specified TDT.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtHLoc(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtHLoc(Tdt) \
	((((Tdt[XDP_TX_DISPID_TDT_TOP2] & XDP_TX_DISPID_TDT_TOP2_HLOC_H_MASK) \
	>> XDP_TX_DISPID_TDT_TOP2_HLOC_H_SHIFT) << 4) | \
	((Tdt[XDP_TX_DISPID_TDT_TOP1] & XDP_TX_DISPID_TDT_TOP1_HLOC_L_MASK) >> \
	XDP_TX_DISPID_TDT_TOP1_HLOC_L_SHIFT))

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, retrieve the vertical tile
 * location in the tiled display. The TDT block is part of an Extended Display
 * Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The vertical tile location in the tiled display represented by
 *		the specified TDT.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtVLoc(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtVLoc(Tdt) \
	(((Tdt[XDP_TX_DISPID_TDT_TOP2] & XDP_TX_DISPID_TDT_TOP2_VLOC_H_MASK) << \
	4) | (Tdt[XDP_TX_DISPID_TDT_TOP1] & XDP_TX_DISPID_TDT_TOP1_VLOC_L_MASK))

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, retrieve the total number of
 * tiles in the tiled display. The TDT block is part of an Extended Display
 * Identification Data (EDID) extension block of type DisplayID.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The total number of tiles in the tiled display.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtNumTiles(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtNumTiles(Tdt) \
	(XDp_TxGetDispIdTdtHTotal(Tdt) * XDp_TxGetDispIdTdtVTotal(Tdt))

/******************************************************************************/
/**
 * Given a Tiled Display Topology (TDT) data block, calculate the tiling order
 * of the associated tile. The TDT block is part of an Extended Display
 * Identification Data (EDID) extension block of type DisplayID.
 * The tiling order starts at 0 for x,y coordinate 0,0 and increments as the
 * horizontal location increases. Once the last horizontal tile has been
 * reached, the next tile in the order is 0,y+1.
 *
 * @param	Tdt is a pointer to the TDT data block.
 *
 * @return	The total number of horizontal tiles in the tiled display.
 *
 * @note	C-style signature:
 *		u8 XDp_TxGetDispIdTdtTileOrder(u8 *Tdt)
 *
*******************************************************************************/
#define XDp_TxGetDispIdTdtTileOrder(Tdt) \
	((XDp_TxGetDispIdTdtVLoc(Tdt) * XDp_TxGetDispIdTdtHTotal(Tdt)) + \
	XDp_TxGetDispIdTdtHLoc(Tdt))

#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#endif /* XDP_HW_H_ */
/** @} */
