/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_hw.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to access the device. High-level driver functions
 * are defined in xdptx.h.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  05/17/14 Initial release.
 *       als  08/03/14 Initial MST addition.
 * 3.0   als  12/16/14 Stream naming now starts at 1 to follow IP.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPTX_HW_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPTX_HW_H_

/***************************** Include Files **********************************/

#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions ******************************/

/******************************************************************************/
/**
 * Address mapping for the DisplayPort TX core.
 *
*******************************************************************************/
/** @name DPTX core registers: Link configuration field.
  * @{
  */
#define XDPTX_LINK_BW_SET		0x0000	/**< Set main link bandwidth
							setting. */
#define XDPTX_LANE_COUNT_SET		0x0004	/**< Set lane count setting. */
#define XDPTX_ENHANCED_FRAME_EN		0x0008	/**< Enable enhanced framing
							symbol sequence. */
#define XDPTX_TRAINING_PATTERN_SET	0x000C	/**< Set the link training
							pattern. */
#define XDPTX_LINK_QUAL_PATTERN_SET	0x0010	/**< Transmit the link quality
							pattern. */
#define XDPTX_SCRAMBLING_DISABLE	0x0014	/**< Disable scrambler and
							transmit all symbols. */
#define XDPTX_DOWNSPREAD_CTRL		0x0018	/**< Enable a 0.5% spreading of
							the clock. */
#define XDPTX_SOFT_RESET		0x001C	/**< Software reset. */
/* @} */

/** @name DPTX core registers: Core enables.
  * @{
  */
#define XDPTX_ENABLE			0x0080	/**< Enable the basic operations
							of the DisplayPort TX
							core or output stuffing
							symbols if disabled. */
#define XDPTX_ENABLE_MAIN_STREAM	0x0084	/**< Enable transmission of main
							link video info. */
#define XDPTX_ENABLE_SEC_STREAM		0x0088	/**< Enable the transmission of
							secondary link info. */
#define XDPTX_FORCE_SCRAMBLER_RESET	0x00C0	/**< Force a scrambler reset. */
#define XDPTX_TX_MST_CONFIG		0x00D0	/**< Enable MST. */
/* @} */

/** @name DPTX core registers: Core ID.
  * @{
  */
#define XDPTX_VERSION			0x00F8	/**< Core version. */
#define XDPTX_CORE_ID			0x00FC	/**< DisplayPort revision. */
/* @} */

/** @name DPTX core registers: AUX channel interface.
  * @{
  */
#define XDPTX_AUX_CMD			0x0100	/**< Initiates AUX commands. */
#define XDPTX_AUX_WRITE_FIFO		0x0104	/**< Write data for the current
							AUX command. */
#define XDPTX_AUX_ADDRESS		0x0108	/**< Specifies the address of
							current AUX command. */
#define XDPTX_AUX_CLK_DIVIDER		0x010C	/**< Clock divider value for
							generating the internal
							1MHz clock. */
#define XDPTX_TX_USER_FIFO_OVERFLOW	0x0110	/**< Indicates an overflow in
							user FIFO. */
#define XDPTX_INTERRUPT_SIG_STATE	0x0130	/**< The raw signal values for
							interupt events. */
#define XDPTX_AUX_REPLY_DATA		0x0134	/**< Reply data received during
							the AUX reply. */
#define XDPTX_AUX_REPLY_CODE		0x0138	/**< Reply code received from
							the most recent AUX
							command. */
#define XDPTX_AUX_REPLY_COUNT		0x013C	/**< Number of reply
							transactions receieved
							over AUX. */
#define XDPTX_INTERRUPT_STATUS		0x0140	/**< Status for interrupt
							events. */
#define XDPTX_INTERRUPT_MASK		0x0144	/**< Masks the specified
							interrupt sources. */
#define XDPTX_REPLY_DATA_COUNT		0x0148	/**< Total number of data bytes
							actually received during
							a transaction. */
#define XDPTX_REPLY_STATUS		0x014C	/**< Reply status of most recent
							AUX transaction. */
#define XDPTX_HPD_DURATION		0x0150	/**< Duration of the HPD pulse
							in microseconds. */
/* @} */

/** @name DPTX core registers: Main stream attributes for SST / MST STREAM1.
  * @{
  */
#define XDPTX_STREAM1_MSA_START		0x0180	/**< Start of the MSA registers
							for stream 1. */
#define XDPTX_MAIN_STREAM_HTOTAL	0x0180	/**< Total number of clocks in
							the horizontal framing
							period. */
#define XDPTX_MAIN_STREAM_VTOTAL	0x0184	/**< Total number of lines in
							the video frame. */
#define XDPTX_MAIN_STREAM_POLARITY	0x0188	/**< Polarity for the video
							sync signals. */
#define XDPTX_MAIN_STREAM_HSWIDTH	0x018C	/**< Width of the horizontal
							sync pulse. */
#define XDPTX_MAIN_STREAM_VSWIDTH	0x0190	/**< Width of the vertical sync
							pulse. */
#define XDPTX_MAIN_STREAM_HRES		0x0194	/**< Number of active pixels per
							line (the horizontal
							resolution). */
#define XDPTX_MAIN_STREAM_VRES		0x0198	/**< Number of active lines (the
							vertical resolution). */
#define XDPTX_MAIN_STREAM_HSTART	0x019C	/**< Number of clocks between
							the leading edge of the
							horizontal sync and the
							start of active data. */
#define XDPTX_MAIN_STREAM_VSTART	0x01A0	/**< Number of lines between the
							leading edge of the
							vertical sync and the
							first line of active
							data. */
#define XDPTX_MAIN_STREAM_MISC0		0x01A4	/**< Miscellaneous stream
							attributes. */
#define XDPTX_MAIN_STREAM_MISC1		0x01A8	/**< Miscellaneous stream
							attributes. */
#define XDPTX_M_VID			0x01AC	/**< M value for the video
							stream as computed by
							the source core in
							asynchronous clock
							mode. Must be written
							in synchronous mode. */
#define XDPTX_TU_SIZE			0x01B0	/**< Size of a transfer unit in
							the framing logic. */
#define XDPTX_N_VID			0x01B4	/**< N value for the video
							stream as computed by
							the source core in
							asynchronous clock mode.
							Must be written in
							synchronous mode. */
#define XDPTX_USER_PIXEL_WIDTH		0x01B8	/**< Selects the width of the
							user data input port. */
#define XDPTX_USER_DATA_COUNT_PER_LANE	0x01BC	/**< Used to translate the
							number of pixels per
							line to the native
							internal 16-bit
							datapath. */
#define XDPTX_MAIN_STREAM_INTERLACED	0x01C0	/**< Video is interlaced. */
#define XDPTX_MIN_BYTES_PER_TU		0x01C4	/**< The minimum number of bytes
							per transfer unit. */
#define XDPTX_FRAC_BYTES_PER_TU		0x01C8	/**< The fractional component
							when calculated the
							XDPTX_MIN_BYTES_PER_TU
							register value. */
#define XDPTX_INIT_WAIT			0x01CC	/**< Number of initial wait
							cycles at the start of a
							new line by the framing
							logic, allowing enough
							data to be buffered in
							the input FIFO. */
#define XDPTX_STREAM1			0x01D0	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDPTX_STREAM2			0x01D4	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDPTX_STREAM3			0x01D8	/**< Average stream symbol
							timeslots per MTP
							config. */
#define XDPTX_STREAM4			0x01DC	/**< Average stream symbol
							timeslots per MTP
							config. */
/* @} */

/** @name DPTX core registers: PHY configuration status.
  * @{
  */
#define XDPTX_PHY_CONFIG		0x0200	/**< Transceiver PHY reset and
							configuration. */
#define XDPTX_PHY_VOLTAGE_DIFF_LANE_0	0x0220	/**< Controls the differential
							voltage swing. */
#define XDPTX_PHY_VOLTAGE_DIFF_LANE_1	0x0224	/**< Controls the differential
							voltage swing. */
#define XDPTX_PHY_VOLTAGE_DIFF_LANE_2	0x0228	/**< Controls the differential
							voltage swing. */
#define XDPTX_PHY_VOLTAGE_DIFF_LANE_3	0x022C	/**< Controls the differential
							voltage swing. */
#define XDPTX_PHY_TRANSMIT_PRBS7	0x0230	/**< Enable pseudo random bit
							sequence 7 pattern
							transmission for link
							quality assessment. */
#define XDPTX_PHY_CLOCK_SELECT		0x0234	/**< Instructs the PHY PLL to
							generate the proper
							clock frequency for the
							required link rate. */
#define XDPTX_TX_PHY_POWER_DOWN		0x0238	/**< Controls PHY power down. */
#define XDPTX_PHY_PRECURSOR_LANE_0	0x023C	/**< Controls the pre-cursor
							level. */
#define XDPTX_PHY_PRECURSOR_LANE_1	0x0240	/**< Controls the pre-cursor
							level. */
#define XDPTX_PHY_PRECURSOR_LANE_2	0x0244	/**< Controls the pre-cursor
							level. */
#define XDPTX_PHY_PRECURSOR_LANE_3	0x0248	/**< Controls the pre-cursor
							level. */
#define XDPTX_PHY_POSTCURSOR_LANE_0	0x024C	/**< Controls the post-cursor
							level. */
#define XDPTX_PHY_POSTCURSOR_LANE_1	0x0250	/**< Controls the post-cursor
							level. */
#define XDPTX_PHY_POSTCURSOR_LANE_2	0x0254	/**< Controls the post-cursor
							level. */
#define XDPTX_PHY_POSTCURSOR_LANE_3	0x0258	/**< Controls the post-cursor
							level. */
#define XDPTX_PHY_STATUS		0x0280	/**< Current PHY status. */
#define XDPTX_GT_DRP_COMMAND		0x02A0	/**< Provides acces to the GT
							DRP ports. */
#define XDPTX_GT_DRP_READ_DATA		0x02A4	/**< Provides access to GT DRP
							read data. */
#define XDPTX_GT_DRP_CHANNEL_STATUS	0x02A8	/**< Provides access to GT DRP
							channel status. */
/* @} */

/** @name DPTX core registers: DisplayPort audio.
  * @{
  */
#define XDPTX_TX_AUDIO_CONTROL		0x0300	/**< Enables audio stream
							packets in main link and
							buffer control. */
#define XDPTX_TX_AUDIO_CHANNELS		0x0304	/**< Used to input active
							channel count. */
#define XDPTX_TX_AUDIO_INFO_DATA	0x0308	/**< Word formatted as per
							CEA 861-C info frame. */
#define XDPTX_TX_AUDIO_MAUD		0x0328	/**< M value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDPTX_TX_AUDIO_NAUD		0x032C	/**< N value of audio stream
							as computed by the
							DisplayPort TX core when
							audio and link clocks
							are synchronous. */
#define XDPTX_TX_AUDIO_EXT_DATA		0x0330	/**< Word formatted as per
							extension packet. */
/* @} */

/** @name DPTX core registers: Main stream attributes for MST STREAM2, 3, and 4.
  * @{
  */
#define XDPTX_STREAM2_MSA_START		0x0500	/**< Start of the MSA registers
							for stream 2. */
#define XDPTX_STREAM2_MSA_START_OFFSET	(XDPTX_STREAM2_MSA_START - \
		XDPTX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 2 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDPTX_STREAM3_MSA_START		0x0550	/**< Start of the MSA registers
							for stream 3. */
#define XDPTX_STREAM3_MSA_START_OFFSET	(XDPTX_STREAM3_MSA_START - \
		XDPTX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 3 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDPTX_STREAM4_MSA_START		0x05A0	/**< Start of the MSA registers
							for stream 4. */
#define XDPTX_STREAM4_MSA_START_OFFSET	(XDPTX_STREAM4_MSA_START - \
		XDPTX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 4 are at an
							offset from the
							corresponding registers
							of stream 1. */
/* @} */

#define XDPTX_VC_PAYLOAD_BUFFER_ADDR	0x0800	/**< Virtual channel payload
							table (0xFF bytes). */

/******************************************************************************/

/** @name DPTX core masks, shifts, and register values.
  * @{
  */
/* 0x000: LINK_BW_SET */
#define XDPTX_LINK_BW_SET_162GBPS	0x06	/**< 1.62 Gbps link rate. */
#define XDPTX_LINK_BW_SET_270GBPS	0x0A	/**< 2.70 Gbps link rate. */
#define XDPTX_LINK_BW_SET_540GBPS	0x14	/**< 5.40 Gbps link rate. */
/* 0x001: LANE_COUNT_SET */
#define XDPTX_LANE_COUNT_SET_1		0x01	/**< Lane count of 1. */
#define XDPTX_LANE_COUNT_SET_2		0x02	/**< Lane count of 2. */
#define XDPTX_LANE_COUNT_SET_4		0x04	/**< Lane count of 4. */
/* 0x00C: TRAINING_PATTERN_SET */
#define XDPTX_TRAINING_PATTERN_SET_OFF	0x0	/**< Training off. */
#define XDPTX_TRAINING_PATTERN_SET_TP1	0x1	/**< Training pattern 1 used for
							clock recovery. */
#define XDPTX_TRAINING_PATTERN_SET_TP2	0x2	/**< Training pattern 2 used for
							channel equalization. */
#define XDPTX_TRAINING_PATTERN_SET_TP3	0x3	/**< Training pattern 3 used for
							channel equalization for
							cores with DP v1.2. */
/* 0x010: LINK_QUAL_PATTERN_SET */
#define XDPTX_LINK_QUAL_PATTERN_SET_OFF	0x0	/**< Link quality test pattern
							not transmitted. */
#define XDPTX_LINK_QUAL_PATTERN_SET_D102_TEST \
					0x1	/**< D10.2 unscrambled test
							pattern transmitted. */
#define XDPTX_LINK_QUAL_PATTERN_SET_SER_MES \
					0x2	/**< Symbol error rate
							measurement pattern
							transmitted. */
#define XDPTX_LINK_QUAL_PATTERN_SET_PRBS7 \
					0x3	/**< Pseudo random bit sequence
							7 transmitted. */
/* 0x01C: SOFTWARE_RESET */
#define XDPTX_SOFT_RESET_VIDEO_STREAM1_MASK \
				0x00000001	/**< Reset video logic. */
#define XDPTX_SOFT_RESET_VIDEO_STREAM2_MASK \
				0x00000002	/**< Reset video logic. */
#define XDPTX_SOFT_RESET_VIDEO_STREAM3_MASK \
				0x00000004	/**< Reset video logic. */
#define XDPTX_SOFT_RESET_VIDEO_STREAM4_MASK \
				0x00000008	/**< Reset video logic. */
#define XDPTX_SOFT_RESET_AUX_MASK \
				0x00000080	/**< Reset AUX logic. */
#define XDPTX_SOFT_RESET_VIDEO_STREAM_ALL_MASK \
				0x0000000F	/**< Reset video logic for all
							streams. */
/* 0x0D0: TX_MST_CONFIG */
#define XDPTX_TX_MST_CONFIG_MST_EN_MASK \
				0x00000001	/**< Enable MST. */
#define XDPTX_TX_MST_CONFIG_VCP_UPDATED_MASK \
				0x00000002	/**< The VC payload has been
							updated in the sink. */
/* 0x0F8 : VERSION_REGISTER */
#define XDPTX_VERSION_INTER_REV_MASK \
				0x0000000F	/**< Internal revision. */
#define XDPTX_VERSION_CORE_PATCH_MASK \
				0x00000030	/**< Core patch details. */
#define XDPTX_VERSION_CORE_PATCH_SHIFT \
				8		/**< Shift bits for core patch
							details. */
#define XDPTX_VERSION_CORE_VER_REV_MASK \
				0x000000C0	/**< Core version revision. */
#define XDPTX_VERSION_CORE_VER_REV_SHIFT \
				12		/**< Shift bits for core version
							revision. */
#define XDPTX_VERSION_CORE_VER_MNR_MASK \
				0x00000F00	/**< Core minor version. */
#define XDPTX_VERSION_CORE_VER_MNR_SHIFT \
				16		/**< Shift bits for core minor
							version. */
#define XDPTX_VERSION_CORE_VER_MJR_MASK \
				0x0000F000	/**< Core major version. */
#define XDPTX_VERSION_CORE_VER_MJR_SHIFT \
				24		/**< Shift bits for core major
							version. */
/* 0x0FC : CORE_ID */
#define XDPTX_CORE_ID_TYPE_MASK	0x0000000F	/**< Core type. */
#define XDPTX_CORE_ID_TYPE_TX	0x0		/**< Core is a transmitter. */
#define XDPTX_CORE_ID_TYPE_RX	0x1		/**< Core is a receiver. */
#define XDPTX_CORE_ID_DP_REV_MASK \
				0x000000F0	/**< DisplayPort protocol
							revision. */
#define XDPTX_CORE_ID_DP_REV_SHIFT \
				8		/**< Shift bits for DisplayPort
							protocol revision. */
#define XDPTX_CORE_ID_DP_MNR_VER_MASK \
				0x00000F00	/**< DisplayPort protocol minor
							version. */
#define XDPTX_CORE_ID_DP_MNR_VER_SHIFT \
				16		/**< Shift bits for DisplayPort
							protocol major
							version. */
#define XDPTX_CORE_ID_DP_MJR_VER_MASK \
				0x0000F000	/**< DisplayPort protocol major
							version. */
#define XDPTX_CORE_ID_DP_MJR_VER_SHIFT \
				24		/**< Shift bits for DisplayPort
							protocol major
							version. */
/* 0x100 AUX_CMD */
#define XDPTX_AUX_CMD_NBYTES_TRANSFER_MASK \
				0x0000000F	/**< Number of bytes to transfer
							with the current AUX
							command. */
#define XDPTX_AUX_CMD_MASK	0x00000F00	/**< AUX command. */
#define XDPTX_AUX_CMD_SHIFT		8	/**< Shift bits for command. */
#define XDPTX_AUX_CMD_I2C_WRITE		0x0	/**< I2C-over-AUX write
							command. */
#define XDPTX_AUX_CMD_I2C_READ		0x1	/**< I2C-over-AUX read
							command. */
#define XDPTX_AUX_CMD_I2C_WRITE_STATUS	0x2	/**< I2C-over-AUX write status
							command. */
#define XDPTX_AUX_CMD_I2C_WRITE_MOT	0x4	/**< I2C-over-AUX write MOT
							(middle-of-transaction)
							command. */
#define XDPTX_AUX_CMD_I2C_READ_MOT	0x5	/**< I2C-over-AUX read MOT
							(middle-of-transaction)
							command. */
#define XDPTX_AUX_CMD_I2C_WRITE_STATUS_MOT \
					0x6	/**< I2C-over-AUX write status
							MOT (middle-of-
							transaction) command. */
#define XDPTX_AUX_CMD_WRITE		0x8	/**< AUX write command. */
#define XDPTX_AUX_CMD_READ		0x9	/**< AUX read command. */
#define XDPTX_AUX_CMD_ADDR_ONLY_TRANSFER_EN \
				0x00001000	/**< Address only transfer
							enable (STOP will be
							sent after command). */
/* 0x10C: AUX_CLK_DIVIDER */
#define XDPTX_AUX_CLK_DIVIDER_VAL_MASK \
				0x0000000F	/**< Clock divider value. */
#define XDPTX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK \
				0x00000F00	/**< AUX (noise) signal width
							filter. */
#define XDPTX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT \
				8		/**< Shift bits for AUX signal
							width filter. */
/* 0x130: INTERRUPT_SIG_STATE */
#define XDPTX_INTERRUPT_SIG_STATE_HPD_STATE_MASK \
				0x00000001	/**< Raw state of the HPD pin on
							the DP connector. */
#define XDPTX_INTERRUPT_SIG_STATE_REQUEST_STATE_MASK \
				0x00000002	/**< A request is currently
							being sent. */
#define XDPTX_INTERRUPT_SIG_STATE_REPLY_STATE_MASK \
				0x00000004	/**< A reply is currently being
							received. */
#define XDPTX_INTERRUPT_SIG_STATE_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
/* 0x138: AUX_REPLY_CODE */
#define XDPTX_AUX_REPLY_CODE_ACK	0x0	/**< AUX command ACKed. */
#define XDPTX_AUX_REPLY_CODE_I2C_ACK	0x0	/**< I2C-over-AUX command
							not ACKed. */
#define XDPTX_AUX_REPLY_CODE_NACK	0x1	/**< AUX command not ACKed. */
#define XDPTX_AUX_REPLY_CODE_DEFER	0x2	/**< AUX command deferred. */
#define XDPTX_AUX_REPLY_CODE_I2C_NACK	0x4	/**< I2C-over-AUX command not
							ACKed. */
#define XDPTX_AUX_REPLY_CODE_I2C_DEFER	0x8	/**< I2C-over-AUX command
							deferred. */
/* 0x140: INTERRUPT_STATUS */
#define XDPTX_INTERRUPT_STATUS_HPD_IRQ_MASK \
				0x00000001	/**< Detected an IRQ framed with
							the proper timing on the
							HPD signal. */
#define XDPTX_INTERRUPT_STATUS_HPD_EVENT_MASK \
				0x00000002	/**< Detected the presence of
							the HPD signal. */
#define XDPTX_INTERRUPT_STATUS_REPLY_RECEIVED_MASK \
				0x00000004	/**< An AUX reply transaction
							has been detected. */
#define XDPTX_INTERRUPT_STATUS_REPLY_TIMEOUT_MASK \
				0x00000008	/**< A reply timeout has
							occurred. */
#define XDPTX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK \
				0x00000010	/**< A pulse on the HPD line was
							detected. */
#define XDPTX_INTERRUPT_STATUS_EXT_PKT_TXD_MASK \
				0x00000020	/**< Extended packet has been
							transmitted and the core
							is ready to accept a new
							packet. */
/* 0x144: INTERRUPT_MASK */
#define XDPTX_INTERRUPT_MASK_HPD_IRQ_MASK \
				0x00000001	/**< Mask HPD IRQ interrupt. */
#define XDPTX_INTERRUPT_MASK_HPD_EVENT_MASK \
				0x00000002	/**< Mask HPD event
							interrupt. */
#define XDPTX_INTERRUPT_MASK_REPLY_RECEIVED_MASK \
				0x00000004	/**< Mask reply received
							interrupt. */
#define XDPTX_INTERRUPT_MASK_REPLY_TIMEOUT_MASK \
				0x00000008	/**< Mask reply received
							interrupt. */
#define XDPTX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK \
				0x00000010	/**< Mask HPD pulse detected
							interrupt. */
#define XDPTX_INTERRUPT_MASK_EXT_PKT_TXD_MASK \
				0x00000020	/**< Mask extended packet
							transmit interrupt. */
/* 0x14C: REPLY_STATUS */
#define XDPTX_REPLY_STATUS_REPLY_RECEIVED_MASK \
				0x00000001	/**< AUX transaction is complete
							and a valid reply
							transaction received. */
#define XDPTX_REPLY_STATUS_REPLY_IN_PROGRESS_MASK \
				0x00000002	/**< AUX reply is currently
							being received. */
#define XDPTX_REPLY_STATUS_REQUEST_IN_PROGRESS_MASK \
				0x00000004	/**< AUX request is currently
							being transmitted. */
#define XDPTX_REPLY_STATUS_REPLY_ERROR_MASK \
				0x00000008	/**< Detected an error in the
							AUX reply of the most
							recent transaction. */
#define XDPTX_REPLY_STATUS_REPLY_STATUS_STATE_MASK \
				0x00000FF0	/**< Internal AUX reply state
							machine status bits. */
#define XDPTX_REPLY_STATUS_REPLY_STATUS_STATE_SHIFT \
				4		/**< Shift bits for the internal
							AUX reply state machine
							status. */
/* 0x188, 0x508, 0x558, 0x5A8: MAIN_STREAM[1-4]_POLARITY */
#define XDPTX_MAIN_STREAMX_POLARITY_HSYNC_POL_MASK \
				0x00000001	/**< Polarity of the horizontal
							sync pulse. */
#define XDPTX_MAIN_STREAMX_POLARITY_VSYNC_POL_MASK \
				0x00000002	/**< Polarity of the vertical
							sync pulse. */
#define XDPTX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT \
				1		/**< Shift bits for polarity of
							the vertical sync
							pulse. */
/* 0x1A4, 0x524, 0x574, 0x5C4: MAIN_STREAM[1-4]_MISC0 */
#define XDPTX_MAIN_STREAMX_MISC0_SYNC_CLK_MASK \
				0x00000001	/**< Synchronous clock. */
#define XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_MASK \
				0x00000006	/**< Component format. */
#define XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_SHIFT \
				1               /**< Shift bits for component
							format. */
#define XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB \
				0x0		/**< Stream's component format
							is RGB. */
#define XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 \
				0x1		/**< Stream's component format
							is YcbCr 4:2:2. */
#define XDPTX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 \
				0x2		/**< Stream's component format
							is YcbCr 4:4:4. */
#define XDPTX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_MASK \
				0x00000008	/**< Dynamic range. */
#define XDPTX_MAIN_STREAMX_MISC0_DYNAMIC_RANGE_SHIFT \
				3		/**< Shift bits for dynamic
							range. */
#define XDPTX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_MASK \
				0x00000010	/**< YCbCr colorimetry. */
#define XDPTX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_SHIFT \
				4		/**< Shift bits for YCbCr
							colorimetry. */
#define XDPTX_MAIN_STREAMX_MISC0_BDC_MASK \
				0x000000E0	/**< Bit depth per color
							component (BDC). */
#define XDPTX_MAIN_STREAMX_MISC0_BDC_SHIFT \
				5		/**< Shift bits for BDC.*/
#define XDPTX_MAIN_STREAMX_MISC0_BDC_6BPC \
				0x0		/**< 6 bits per component.*/
#define XDPTX_MAIN_STREAMX_MISC0_BDC_8BPC \
				0x1		/**< 8 bits per component.*/
#define XDPTX_MAIN_STREAMX_MISC0_BDC_10BPC \
				0x2		/**< 10 bits per component.*/
#define XDPTX_MAIN_STREAMX_MISC0_BDC_12BPC \
				0x3		/**< 12 bits per component.*/
#define XDPTX_MAIN_STREAMX_MISC0_BDC_16BPC \
				0x4		/**< 16 bits per component.*/
/* 0x1A8, 0x528, 0x578, 0x5C8: MAIN_STREAM[1-4]_MISC1 */
#define XDPTX_MAIN_STREAMX_MISC1_INTERLACED_VTOTAL_GIVEN_MASK \
				0x00000001	/**< Interlaced vertical total
							even. */
#define XDPTX_MAIN_STREAMX_MISC1_STEREO_VID_ATTR_MASK \
				0x00000006	/**< Stereo video attribute. */
#define XDPTX_MAIN_STREAMX_MISC1_STEREO_VID_ATTR_SHIFT \
				1		/**< Shift bits for stereo video
							attribute. */
/* 0x200: PHY_CONFIG */
#define XDPTX_PHY_CONFIG_PHY_RESET_ENABLE_MASK \
				0x0000000	/**< Release reset. */
#define XDPTX_PHY_CONFIG_PHY_RESET_MASK \
				0x0000001	/**< Hold the PHY in reset. */
#define XDPTX_PHY_CONFIG_GTTX_RESET_MASK \
				0x0000002	/**< Hold GTTXRESET in reset. */
#define XDPTX_PHY_CONFIG_TX_PHY_PMA_RESET_MASK \
				0x0000100	/**< Hold TX_PHY_PMA reset. */
#define XDPTX_PHY_CONFIG_TX_PHY_PCS_RESET_MASK \
				0x0000200	/**< Hold TX_PHY_PCS reset. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_MASK \
				0x0000800	/**< Set TX_PHY_POLARITY. */
#define XDPTX_PHY_CONFIG_TX_PHY_PRBSFORCEERR_MASK \
				0x0001000	/**< Set TX_PHY_PRBSFORCEERR. */
#define XDPTX_PHY_CONFIG_TX_PHY_LOOPBACK_MASK \
				0x000E000	/**< Set TX_PHY_LOOPBACK. */
#define XDPTX_PHY_CONFIG_TX_PHY_LOOPBACK_SHIFT 13 /**< Shift bits for
							TX_PHY_LOOPBACK. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_IND_LANE_MASK \
				0x0010000	/**< Set to enable individual
							lane polarity. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_LANE0_MASK \
				0x0020000	/**< Set TX_PHY_POLARITY for
							lane 0. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_LANE1_MASK \
				0x0040000	/**< Set TX_PHY_POLARITY for
							lane 1. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_LANE2_MASK \
				0x0080000	/**< Set TX_PHY_POLARITY for
							lane 2. */
#define XDPTX_PHY_CONFIG_TX_PHY_POLARITY_LANE3_MASK \
				0x0100000	/**< Set TX_PHY_POLARITY for
							lane 3. */
#define XDPTX_PHY_CONFIG_TX_PHY_8B10BEN_MASK \
				0x0200000	/**< 8B10B encoding enable. */
#define XDPTX_PHY_CONFIG_GT_ALL_RESET_MASK \
				0x0000003	/**< Reset GT and PHY. */
/* 0x234: PHY_CLOCK_SELECT */
#define XDPTX_PHY_CLOCK_SELECT_162GBPS	0x1	/**< 1.62 Gbps link. */
#define XDPTX_PHY_CLOCK_SELECT_270GBPS	0x3	/**< 2.70 Gbps link. */
#define XDPTX_PHY_CLOCK_SELECT_540GBPS	0x5	/**< 5.40 Gbps link. */
/* 0x0220, 0x0224, 0x0228, 0x022C: XDPTX_PHY_VOLTAGE_DIFF_LANE_[0-3] */
#define XDPTX_VS_LEVEL_0		0x2	/**< Voltage swing level 0. */
#define XDPTX_VS_LEVEL_1		0x5	/**< Voltage swing level 1. */
#define XDPTX_VS_LEVEL_2		0x8	/**< Voltage swing level 2. */
#define XDPTX_VS_LEVEL_3		0xF	/**< Voltage swing level 3. */
#define XDPTX_VS_LEVEL_OFFSET		0x4	/**< Voltage swing compensation
							offset used when there's
							no redriver in display
							path. */
/* 0x024C, 0x0250, 0x0254, 0x0258: XDPTX_PHY_POSTCURSOR_LANE_[0-3] */
#define XDPTX_PE_LEVEL_0		0x00	/**< Pre-emphasis level 0. */
#define XDPTX_PE_LEVEL_1		0x0E	/**< Pre-emphasis level 1. */
#define XDPTX_PE_LEVEL_2		0x14	/**< Pre-emphasis level 2. */
#define XDPTX_PE_LEVEL_3		0x1B	/**< Pre-emphasis level 3. */
/* 0x280: PHY_STATUS */
#define XDPTX_PHY_STATUS_RESET_LANE_0_1_DONE_MASK \
				0x00000003	/**< Reset done for lanes
							0 and 1. */
#define XDPTX_PHY_STATUS_RESET_LANE_2_3_DONE_MASK \
				0x0000000C	/**< Reset done for lanes
							2 and 3. */
#define XDPTX_PHY_STATUS_PLL_LANE0_1_LOCK_MASK \
				0x00000010	/**< PLL locked for lanes
							0 and 1. */
#define XDPTX_PHY_STATUS_PLL_LANE2_3_LOCK_MASK \
				0x00000020	/**< PLL locked for lanes
							2 and 3. */
#define XDPTX_PHY_STATUS_PLL_FABRIC_LOCK_MASK \
				0x00000040	/**< FPGA fabric clock PLL
							locked. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_0_MASK \
				0x00030000	/**< TX buffer status lane 0. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_0_SHIFT \
				16		/**< Shift bits for TX buffer
							status lane 0. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_0_MASK \
				0x000C0000	/**< TX error on lane 0. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_0_SHIFT \
				18		/**< Shift bits for TX error on
							lane 0. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_1_MASK \
				0x00300000	/**< TX buffer status lane 1. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_1_SHIFT \
				20		/**< Shift bits for TX buffer
							status lane 1. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_1_MASK \
				0x00C00000	/**< TX error on lane 1. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_1_SHIFT \
				22		/**< Shift bits for TX error on
							lane 1. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_2_MASK \
				0x03000000	/**< TX buffer status lane 2. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_2_SHIFT \
				24		/**< Shift bits for TX buffer
							status lane 2. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_2_MASK \
				0x0C000000	/**< TX error on lane 2. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_2_SHIFT \
				26		/**< Shift bits for TX error on
							lane 2. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_3_MASK \
				0x30000000	/**< TX buffer status lane 3. */
#define XDPTX_PHY_STATUS_TX_BUFFER_STATUS_LANE_3_SHIFT \
				28		/**< Shift bits for TX buffer
							status lane 3. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_3_MASK \
				0xC0000000	/**< TX error on lane 3. */
#define XDPTX_PHY_STATUS_TX_ERROR_LANE_3_SHIFT \
				30		/**< Shift bits for TX error on
							lane 3. */
#define XDPTX_PHY_STATUS_LANES_0_1_READY_MASK \
				0x00000013	/**< Lanes 0 and 1 are ready. */
#define XDPTX_PHY_STATUS_ALL_LANES_READY_MASK \
				0x0000003F	/**< All lanes are ready. */
/* 0x2A0: XDPTX_GT_DRP_COMMAND */
#define XDPTX_GT_DRP_COMMAND_DRP_ADDR_MASK \
				0x000F		/**< DRP address. */
#define XDPTX_GT_DRP_COMMAND_DRP_RW_CMD_MASK \
				0x0080		/**< DRP read/write command
							(Read=0, Write=1). */
#define XDPTX_GT_DRP_COMMAND_DRP_W_DATA_MASK \
				0xFF00		/**< DRP write data. */
#define XDPTX_GT_DRP_COMMAND_DRP_W_DATA_SHIFT \
				16		/**< Shift bits for DRP write
							data. */
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
#define XDPTX_DPCD_REV					0x00000
#define XDPTX_DPCD_MAX_LINK_RATE			0x00001
#define XDPTX_DPCD_MAX_LANE_COUNT			0x00002
#define XDPTX_DPCD_MAX_DOWNSPREAD			0x00003
#define XDPTX_DPCD_NORP_PWR_V_CAP			0x00004
#define XDPTX_DPCD_DOWNSP_PRESENT			0x00005
#define XDPTX_DPCD_ML_CH_CODING_CAP			0x00006
#define XDPTX_DPCD_DOWNSP_COUNT_MSA_OUI			0x00007
#define	XDPTX_DPCD_RX_PORT0_CAP_0			0x00008
#define	XDPTX_DPCD_RX_PORT0_CAP_1			0x00009
#define	XDPTX_DPCD_RX_PORT1_CAP_0			0x0000A
#define	XDPTX_DPCD_RX_PORT1_CAP_1			0x0000B
#define XDPTX_DPCD_I2C_SPEED_CTL_CAP			0x0000C
#define XDPTX_DPCD_EDP_CFG_CAP				0x0000D
#define XDPTX_DPCD_TRAIN_AUX_RD_INTERVAL		0x0000E
#define XDPTX_DPCD_ADAPTER_CAP				0x0000F
#define XDPTX_DPCD_FAUX_CAP				0x00020
#define XDPTX_DPCD_MSTM_CAP				0x00021
#define XDPTX_DPCD_NUM_AUDIO_EPS			0x00022
#define	XDPTX_DPCD_AV_GRANULARITY			0x00023
#define XDPTX_DPCD_AUD_DEC_LAT_7_0			0x00024
#define XDPTX_DPCD_AUD_DEC_LAT_15_8			0x00025
#define XDPTX_DPCD_AUD_PP_LAT_7_0			0x00026
#define XDPTX_DPCD_AUD_PP_LAT_15_8			0x00027
#define XDPTX_DPCD_VID_INTER_LAT			0x00028
#define XDPTX_DPCD_VID_PROG_LAT				0x00029
#define XDPTX_DPCD_REP_LAT				0x0002A
#define XDPTX_DPCD_AUD_DEL_INS_7_0			0x0002B
#define XDPTX_DPCD_AUD_DEL_INS_15_8			0x0002C
#define XDPTX_DPCD_AUD_DEL_INS_23_16			0x0002D
#define XDPTX_DPCD_GUID					0x00030
#define XDPTX_DPCD_RX_GTC_VALUE_7_0			0x00054
#define XDPTX_DPCD_RX_GTC_VALUE_15_8			0x00055
#define XDPTX_DPCD_RX_GTC_VALUE_23_16			0x00056
#define XDPTX_DPCD_RX_GTC_VALUE_31_24			0x00057
#define XDPTX_DPCD_RX_GTC_MSTR_REQ			0x00058
#define XDPTX_DPCD_RX_GTC_FREQ_LOCK_DONE		0x00059
#define XDPTX_DPCD_DOWNSP_0_CAP				0x00080
#define XDPTX_DPCD_DOWNSP_1_CAP				0x00081
#define XDPTX_DPCD_DOWNSP_2_CAP				0x00082
#define XDPTX_DPCD_DOWNSP_3_CAP				0x00083
#define XDPTX_DPCD_DOWNSP_0_DET_CAP			0x00080
#define XDPTX_DPCD_DOWNSP_1_DET_CAP			0x00084
#define XDPTX_DPCD_DOWNSP_2_DET_CAP			0x00088
#define XDPTX_DPCD_DOWNSP_3_DET_CAP			0x0008C
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field.
  * @{
  */
#define XDPTX_DPCD_LINK_BW_SET				0x00100
#define XDPTX_DPCD_LANE_COUNT_SET			0x00101
#define XDPTX_DPCD_TP_SET				0x00102
#define XDPTX_DPCD_TRAINING_LANE0_SET			0x00103
#define XDPTX_DPCD_TRAINING_LANE1_SET			0x00104
#define XDPTX_DPCD_TRAINING_LANE2_SET			0x00105
#define XDPTX_DPCD_TRAINING_LANE3_SET			0x00106
#define XDPTX_DPCD_DOWNSPREAD_CTRL			0x00107
#define XDPTX_DPCD_ML_CH_CODING_SET			0x00108
#define XDPTX_DPCD_I2C_SPEED_CTL_SET			0x00109
#define XDPTX_DPCD_EDP_CFG_SET				0x0010A
#define XDPTX_DPCD_LINK_QUAL_LANE0_SET			0x0010B
#define XDPTX_DPCD_LINK_QUAL_LANE1_SET			0x0010C
#define XDPTX_DPCD_LINK_QUAL_LANE2_SET			0x0010D
#define XDPTX_DPCD_LINK_QUAL_LANE3_SET			0x0010E
#define XDPTX_DPCD_TRAINING_LANE0_1_SET2		0x0010F
#define XDPTX_DPCD_TRAINING_LANE2_3_SET2		0x00110
#define XDPTX_DPCD_MSTM_CTRL				0x00111
#define XDPTX_DPCD_AUDIO_DELAY_7_0			0x00112
#define XDPTX_DPCD_AUDIO_DELAY_15_8			0x00113
#define XDPTX_DPCD_AUDIO_DELAY_23_6			0x00114
#define XDPTX_DPCD_UPSTREAM_DEVICE_DP_PWR_NEED		0x00118
#define XDPTX_DPCD_FAUX_MODE_CTRL			0x00120
#define XDPTX_DPCD_FAUX_FORWARD_CH_DRIVE_SET		0x00121
#define XDPTX_DPCD_BACK_CH_STATUS			0x00122
#define XDPTX_DPCD_FAUX_BACK_CH_SYMBOL_ERROR_COUNT	0x00123
#define XDPTX_DPCD_FAUX_BACK_CH_TRAINING_PATTERN_TIME	0x00125
#define XDPTX_DPCD_TX_GTC_VALUE_7_0			0x00154
#define XDPTX_DPCD_TX_GTC_VALUE_15_8			0x00155
#define XDPTX_DPCD_TX_GTC_VALUE_23_16			0x00156
#define XDPTX_DPCD_TX_GTC_VALUE_31_24			0x00157
#define XDPTX_DPCD_RX_GTC_VALUE_PHASE_SKEW_EN		0x00158
#define XDPTX_DPCD_TX_GTC_FREQ_LOCK_DONE		0x00159
#define XDPTX_DPCD_ADAPTER_CTRL				0x001A0
#define XDPTX_DPCD_BRANCH_DEVICE_CTRL			0x001A1
#define XDPTX_DPCD_PAYLOAD_ALLOCATE_SET			0x001C0
#define XDPTX_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT	0x001C1
#define XDPTX_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT	0x001C2
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field.
  * @{
  */
#define XDPTX_DPCD_SINK_COUNT				0x00200
#define XDPTX_DPCD_DEVICE_SERVICE_IRQ			0x00201
#define XDPTX_DPCD_STATUS_LANE_0_1			0x00202
#define XDPTX_DPCD_STATUS_LANE_2_3			0x00203
#define XDPTX_DPCD_LANE_ALIGN_STATUS_UPDATED		0x00204
#define XDPTX_DPCD_SINK_STATUS				0x00205
#define XDPTX_DPCD_ADJ_REQ_LANE_0_1			0x00206
#define XDPTX_DPCD_ADJ_REQ_LANE_2_3			0x00207
#define XDPTX_DPCD_TRAINING_SCORE_LANE_0		0x00208
#define XDPTX_DPCD_TRAINING_SCORE_LANE_1		0x00209
#define XDPTX_DPCD_TRAINING_SCORE_LANE_2		0x0020A
#define XDPTX_DPCD_TRAINING_SCORE_LANE_3		0x0020B
#define XDPTX_DPCD_ADJ_REQ_PC2				0x0020C
#define XDPTX_DPCD_FAUX_FORWARD_CH_SYMBOL_ERROR_COUNT	0x0020D
#define XDPTX_DPCD_SYMBOL_ERROR_COUNT_LANE_0		0x00210
#define XDPTX_DPCD_SYMBOL_ERROR_COUNT_LANE_1		0x00212
#define XDPTX_DPCD_SYMBOL_ERROR_COUNT_LANE_2		0x00214
#define XDPTX_DPCD_SYMBOL_ERROR_COUNT_LANE_3		0x00216
/* @} */

/** @name DisplayPort Configuration Data: Automated testing sub-field.
  * @{
  */
#define XDPTX_DPCD_FAUX_FORWARD_CH_STATUS		0x00280
#define XDPTX_DPCD_FAUX_BACK_CH_DRIVE_SET		0x00281
#define XDPTX_DPCD_FAUX_BACK_CH_SYM_ERR_COUNT_CTRL	0x00282
#define XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS		0x002C0
#define XDPTX_DPCD_VC_PAYLOAD_ID_SLOT(SlotNum) \
			(XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS + SlotNum)
/* @} */

/** @name DisplayPort Configuration Data: Sink control field.
  * @{
  */
#define XDPTX_DPCD_SET_POWER_DP_PWR_VOLTAGE		0x00600
/* @} */

/** @name DisplayPort Configuration Data: Sideband message buffers.
  * @{
  */
#define XDPTX_DPCD_DOWN_REQ				0x01000
#define XDPTX_DPCD_UP_REP				0x01200
#define XDPTX_DPCD_DOWN_REP				0x01400
#define XDPTX_DPCD_UP_REQ				0x01600
/* @} */

/** @name DisplayPort Configuration Data: Event status indicator field.
  * @{
  */
#define XDPTX_DPCD_SINK_COUNT_ESI			0x02002
#define XDPTX_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0	0x02003
#define XDPTX_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI1	0x02004
#define XDPTX_DPCD_SINK_LINK_SERVICE_IRQ_VECTOR_ESI0	0x02005
#define XDPTX_DPCD_SINK_LANE0_1_STATUS			0x0200C
#define XDPTX_DPCD_SINK_LANE2_3_STATUS			0x0200D
#define XDPTX_DPCD_SINK_ALIGN_STATUS_UPDATED_ESI	0x0200E
#define XDPTX_DPCD_SINK_STATUS_ESI			0x0200F
/* @} */

/** @name DisplayPort Configuration Data: Field addresses and sizes.
  * @{
  */
#define XDPTX_DPCD_RECEIVER_CAP_FIELD_START		XDPTX_DPCD_REV
#define XDPTX_DPCD_RECEIVER_CAP_FIELD_SIZE		0x100
#define XDPTX_DPCD_LINK_CFG_FIELD_START			XDPTX_DPCD_LINK_BW_SET
#define XDPTX_DPCD_LINK_CFG_FIELD_SIZE			0x100
#define XDPTX_DPCD_LINK_SINK_STATUS_FIELD_START		XDPTX_DPCD_SINK_COUNT
#define XDPTX_DPCD_LINK_SINK_STATUS_FIELD_SIZE		0x17
/* @} */

/******************************************************************************/

/** @name DisplayPort Configuration Data: Receiver capability field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00000: DPCD_REV */
#define XDPTX_DPCD_REV_MNR_MASK					0x0F
#define XDPTX_DPCD_REV_MJR_MASK					0xF0
#define XDPTX_DPCD_REV_MJR_SHIFT				4
/* 0x00001: MAX_LINK_RATE */
#define XDPTX_DPCD_MAX_LINK_RATE_162GBPS			0x06
#define XDPTX_DPCD_MAX_LINK_RATE_270GBPS			0x0A
#define XDPTX_DPCD_MAX_LINK_RATE_540GBPS			0x14
/* 0x00002: MAX_LANE_COUNT */
#define XDPTX_DPCD_MAX_LANE_COUNT_MASK				0x1F
#define XDPTX_DPCD_MAX_LANE_COUNT_1				0x01
#define XDPTX_DPCD_MAX_LANE_COUNT_2				0x02
#define XDPTX_DPCD_MAX_LANE_COUNT_4				0x04
#define XDPTX_DPCD_TPS3_SUPPORT_MASK				0x40
#define XDPTX_DPCD_ENHANCED_FRAME_SUPPORT_MASK			0x80
/* 0x00003: MAX_DOWNSPREAD */
#define XDPTX_DPCD_MAX_DOWNSPREAD_MASK				0x01
#define XDPTX_DPCD_NO_AUX_HANDSHAKE_LINK_TRAIN_MASK		0x40
/* 0x00005: DOWNSP_PRESENT */
#define XDPTX_DPCD_DOWNSP_PRESENT_MASK				0x01
#define XDPTX_DPCD_DOWNSP_TYPE_MASK				0x06
#define XDPTX_DPCD_DOWNSP_TYPE_SHIFT				1
#define XDPTX_DPCD_DOWNSP_TYPE_DP				0x0
#define XDPTX_DPCD_DOWNSP_TYPE_AVGA_ADVII			0x1
#define XDPTX_DPCD_DOWNSP_TYPE_DVI_HDMI_DPPP			0x2
#define XDPTX_DPCD_DOWNSP_TYPE_OTHERS				0x3
#define XDPTX_DPCD_DOWNSP_FORMAT_CONV_MASK			0x08
#define XDPTX_DPCD_DOWNSP_DCAP_INFO_AVAIL_MASK			0x10
/* 0x00006, 0x00108: ML_CH_CODING_SUPPORT, ML_CH_CODING_SET */
#define XDPTX_DPCD_ML_CH_CODING_MASK				0x01
/* 0x00007: DOWNSP_COUNT_MSA_OUI */
#define XDPTX_DPCD_DOWNSP_COUNT_MASK				0x0F
#define XDPTX_DPCD_MSA_TIMING_PAR_IGNORED_MASK			0x40
#define XDPTX_DPCD_OUI_SUPPORT_MASK				0x80
/* 0x00008, 0x0000A: RX_PORT[0-1]_CAP_0 */
#define XDPTX_DPCD_RX_PORTX_CAP_0_LOCAL_EDID_PRESENT_MASK	0x02
#define XDPTX_DPCD_RX_PORTX_CAP_0_ASSOC_TO_PRECEDING_PORT_MASK	0x04
/* 0x0000C, 0x00109: I2C_SPEED_CTL_CAP, I2C_SPEED_CTL_SET */
#define XDPTX_DPCD_I2C_SPEED_CTL_NONE				0x00
#define XDPTX_DPCD_I2C_SPEED_CTL_1KBIPS				0x01
#define XDPTX_DPCD_I2C_SPEED_CTL_5KBIPS				0x02
#define XDPTX_DPCD_I2C_SPEED_CTL_10KBIPS			0x04
#define XDPTX_DPCD_I2C_SPEED_CTL_100KBIPS			0x08
#define XDPTX_DPCD_I2C_SPEED_CTL_400KBIPS			0x10
#define XDPTX_DPCD_I2C_SPEED_CTL_1MBIPS				0x20
/* 0x0000E: TRAIN_AUX_RD_INTERVAL */
#define XDPTX_DPCD_TRAIN_AUX_RD_INT_100_400US			0x00
#define XDPTX_DPCD_TRAIN_AUX_RD_INT_4MS				0x01
#define XDPTX_DPCD_TRAIN_AUX_RD_INT_8MS				0x02
#define XDPTX_DPCD_TRAIN_AUX_RD_INT_12MS			0x03
#define XDPTX_DPCD_TRAIN_AUX_RD_INT_16MS			0x04
/* 0x00020: DPCD_FAUX_CAP */
#define XDPTX_DPCD_FAUX_CAP_MASK				0x01
/* 0x00021: MSTM_CAP */
#define XDPTX_DPCD_MST_CAP_MASK					0x01
/* 0x00080, 0x00081|4, 0x00082|8, 0x00083|C: DOWNSP_X_(DET_)CAP */
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_MASK			0x07
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_DP				0x0
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_AVGA			0x1
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_DVI			0x2
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_HDMI			0x3
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_OTHERS			0x4
#define XDPTX_DPCD_DOWNSP_X_CAP_TYPE_DPPP			0x5
#define XDPTX_DPCD_DOWNSP_X_CAP_HPD_MASK			0x80
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_MASK		0xF0
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_SHIFT		4
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_60	0x1
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_720_480_I_50	0x2
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_60	0x3
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1920_1080_I_50	0x4
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_60	0x5
#define XDPTX_DPCD_DOWNSP_X_CAP_NON_EDID_ATTR_1280_720_P_50	0x7
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDPTX_DPCD_DOWNSP_X_DCAP_MAX_BPC_MASK			0x03
#define XDPTX_DPCD_DOWNSP_X_DCAP_MAX_BPC_8			0x0
#define XDPTX_DPCD_DOWNSP_X_DCAP_MAX_BPC_10			0x1
#define XDPTX_DPCD_DOWNSP_X_DCAP_MAX_BPC_12			0x2
#define XDPTX_DPCD_DOWNSP_X_DCAP_MAX_BPC_16			0x3
/* 0x00082, 0x00086, 0x0008A, 0x0008E: DOWNSP_X_(DET_)CAP2 */
#define XDPTX_DPCD_DOWNSP_X_DCAP_HDMI_DPPP_FS2FP_MASK		0x01
#define XDPTX_DPCD_DOWNSP_X_DCAP_DVI_DL_MASK			0x02
#define XDPTX_DPCD_DOWNSP_X_DCAP_DVI_HCD_MASK			0x04
/* @} */

/** @name DisplayPort Configuration Data: Link configuration field masks,
  *       shifts, and register values.
  * @{
  */
/* 0x00100: XDPTX_DPCD_LINK_BW_SET */
#define XDPTX_DPCD_LINK_BW_SET_162GBPS				0x06
#define XDPTX_DPCD_LINK_BW_SET_270GBPS				0x0A
#define XDPTX_DPCD_LINK_BW_SET_540GBPS				0x14
/* 0x00101: LANE_COUNT_SET */
#define XDPTX_DPCD_LANE_COUNT_SET_MASK				0x1F
#define XDPTX_DPCD_LANE_COUNT_SET_1				0x01
#define XDPTX_DPCD_LANE_COUNT_SET_2				0x02
#define XDPTX_DPCD_LANE_COUNT_SET_4				0x04
#define XDPTX_DPCD_ENHANCED_FRAME_EN_MASK			0x80
/* 0x00102: TP_SET */
#define XDPTX_DPCD_TP_SEL_MASK					0x03
#define XDPTX_DPCD_TP_SEL_OFF					0x0
#define XDPTX_DPCD_TP_SEL_TP1					0x1
#define XDPTX_DPCD_TP_SEL_TP2					0x2
#define XDPTX_DPCD_TP_SEL_TP3					0x3
#define XDPTX_DPCD_TP_SET_LQP_MASK				0x06
#define XDPTX_DPCD_TP_SET_LQP_SHIFT				2
#define XDPTX_DPCD_TP_SET_LQP_OFF				0x0
#define XDPTX_DPCD_TP_SET_LQP_D102_TEST				0x1
#define XDPTX_DPCD_TP_SET_LQP_SER_MES				0x2
#define XDPTX_DPCD_TP_SET_LQP_PRBS7				0x3
#define XDPTX_DPCD_TP_SET_REC_CLK_OUT_EN_MASK			0x10
#define XDPTX_DPCD_TP_SET_SCRAMB_DIS_MASK			0x20
#define XDPTX_DPCD_TP_SET_SE_COUNT_SEL_MASK			0xC0
#define XDPTX_DPCD_TP_SET_SE_COUNT_SEL_SHIFT			6
#define XDPTX_DPCD_TP_SET_SE_COUNT_SEL_DE_ISE			0x0
#define XDPTX_DPCD_TP_SET_SE_COUNT_SEL_DE			0x1
#define XDPTX_DPCD_TP_SET_SE_COUNT_SEL_ISE			0x2
/* 0x00103-0x00106: TRAINING_LANE[0-3]_SET */
#define XDPTX_DPCD_TRAINING_LANEX_SET_VS_MASK			0x03
#define XDPTX_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK		0x04
#define XDPTX_DPCD_TRAINING_LANEX_SET_PE_MASK			0x18
#define XDPTX_DPCD_TRAINING_LANEX_SET_PE_SHIFT			3
#define XDPTX_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK		0x20
/* 0x00107: DOWNSPREAD_CTRL */
#define XDPTX_DPCD_SPREAD_AMP_MASK				0x10
#define XDPTX_DPCD_MSA_TIMING_PAR_IGNORED_EN_MASK		0x80
/* 0x00108: ML_CH_CODING_SET - Same as 0x00006: ML_CH_CODING_SUPPORT */
/* 0x00109: I2C_SPEED_CTL_SET - Same as 0x0000C: I2C_SPEED_CTL_CAP */
/* 0x0010F-0x00110: TRAINING_LANE[0_1-2_3]_SET2 */
#define XDPTX_DPCD_TRAINING_LANE_0_2_SET_PC2_MASK		0x03
#define XDPTX_DPCD_TRAINING_LANE_0_2_SET_MAX_PC2_MASK		0x04
#define XDPTX_DPCD_TRAINING_LANE_1_3_SET_PC2_MASK		0x30
#define XDPTX_DPCD_TRAINING_LANE_1_3_SET_PC2_SHIFT		4
#define XDPTX_DPCD_TRAINING_LANE_1_3_SET_MAX_PC2_MASK		0x40
/* 0x00111: MSTM_CTRL */
#define XDPTX_DPCD_MST_EN_MASK					0x01
#define XDPTX_DPCD_UP_REQ_EN_MASK				0x02
#define XDPTX_DPCD_UP_IS_SRC_MASK				0x03
/* @} */

/** @name DisplayPort Configuration Data: Link/sink status field masks, shifts,
  *       and register values.
  * @{
  */
/* 0x00202: STATUS_LANE_0_1 */
#define XDPTX_DPCD_STATUS_LANE_0_CR_DONE_MASK			0x01
#define XDPTX_DPCD_STATUS_LANE_0_CE_DONE_MASK			0x02
#define XDPTX_DPCD_STATUS_LANE_0_SL_DONE_MASK			0x04
#define XDPTX_DPCD_STATUS_LANE_1_CR_DONE_MASK			0x10
#define XDPTX_DPCD_STATUS_LANE_1_CE_DONE_MASK			0x20
#define XDPTX_DPCD_STATUS_LANE_1_SL_DONE_MASK			0x40
/* 0x00202: STATUS_LANE_2_3 */
#define XDPTX_DPCD_STATUS_LANE_2_CR_DONE_MASK			0x01
#define XDPTX_DPCD_STATUS_LANE_2_CE_DONE_MASK			0x02
#define XDPTX_DPCD_STATUS_LANE_2_SL_DONE_MASK			0x04
#define XDPTX_DPCD_STATUS_LANE_3_CR_DONE_MASK			0x10
#define XDPTX_DPCD_STATUS_LANE_3_CE_DONE_MASK			0x20
#define XDPTX_DPCD_STATUS_LANE_3_SL_DONE_MASK			0x40
/* 0x00204: LANE_ALIGN_STATUS_UPDATED */
#define XDPTX_DPCD_LANE_ALIGN_STATUS_UPDATED_IA_DONE_MASK \
								0x01
#define XDPTX_DPCD_LANE_ALIGN_STATUS_UPDATED_DOWNSP_STATUS_CHANGED_MASK \
								0x40
#define XDPTX_DPCD_LANE_ALIGN_STATUS_UPDATED_LINK_STATUS_UPDATED_MASK \
								0x80
/* 0x00205: SINK_STATUS */
#define XDPTX_DPCD_SINK_STATUS_RX_PORT0_SYNC_STATUS_MASK	0x01
#define XDPTX_DPCD_SINK_STATUS_RX_PORT1_SYNC_STATUS_MASK	0x02

/* 0x00206, 0x00207: ADJ_REQ_LANE_[0,2]_[1,3] */
#define XDPTX_DPCD_ADJ_REQ_LANE_0_2_VS_MASK			0x03
#define XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_MASK			0x0C
#define XDPTX_DPCD_ADJ_REQ_LANE_0_2_PE_SHIFT			2
#define XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_MASK			0x30
#define XDPTX_DPCD_ADJ_REQ_LANE_1_3_VS_SHIFT			4
#define XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_MASK			0xC0
#define XDPTX_DPCD_ADJ_REQ_LANE_1_3_PE_SHIFT			6
/* 0x0020C: ADJ_REQ_PC2 */
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_0_MASK			0x03
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_1_MASK			0x0C
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_1_SHIFT			2
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_2_MASK			0x30
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_2_SHIFT			4
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_3_MASK			0xC0
#define XDPTX_DPCD_ADJ_REQ_PC2_LANE_3_SHIFT			6
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
#define XDPTX_SEGPTR_ADDR				0x30
#define XDPTX_EDID_ADDR					0x50
#define XDPTX_EDID_BLOCK_SIZE				128
#define XDPTX_EDID_DTD_DD(Num)				(0x36 + (18 * Num))
#define XDPTX_EDID_PTM					XDPTX_EDID_DTD_DD(0)
#define XDPTX_EDID_EXT_BLOCK_COUNT			0x7E
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Detailed Timing Descriptor (DTD).
  * @{
  */
#define XDPTX_EDID_DTD_PIXEL_CLK_KHZ_LSB		0x00
#define XDPTX_EDID_DTD_PIXEL_CLK_KHZ_MSB		0x01
#define XDPTX_EDID_DTD_HRES_LSB				0x02
#define XDPTX_EDID_DTD_HBLANK_LSB			0x03
#define XDPTX_EDID_DTD_HRES_HBLANK_U4			0x04
#define XDPTX_EDID_DTD_VRES_LSB				0x05
#define XDPTX_EDID_DTD_VBLANK_LSB			0x06
#define XDPTX_EDID_DTD_VRES_VBLANK_U4			0x07
#define XDPTX_EDID_DTD_HFPORCH_LSB			0x08
#define XDPTX_EDID_DTD_HSPW_LSB				0x09
#define XDPTX_EDID_DTD_VFPORCH_VSPW_L4			0x0A
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2			0x0B
#define XDPTX_EDID_DTD_HIMGSIZE_MM_LSB			0x0C
#define XDPTX_EDID_DTD_VIMGSIZE_MM_LSB			0x0D
#define XDPTX_EDID_DTD_XIMGSIZE_MM_U4			0x0E
#define XDPTX_EDID_DTD_HBORDER				0x0F
#define XDPTX_EDID_DTD_VBORDER				0x10
#define XDPTX_EDID_DTD_SIGNAL				0x11

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values.
  * @{
  */
#define XDPTX_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK	0x0F
#define XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_MASK		0xF0
#define XDPTX_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT	4
#define XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK	0x0F
#define XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK	0xF0
#define XDPTX_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT	4
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK	0xC0
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK	0x30
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK	0x0C
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK	0x03
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT	6
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT	4
#define XDPTX_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT	2
#define XDPTX_EDID_DTD_XIMGSIZE_MM_U4_VIMGSIZE_MM_MASK	0x0F
#define XDPTX_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_MASK	0xF0
#define XDPTX_EDID_DTD_XIMGSIZE_MM_U4_HIMGSIZE_MM_SHIFT	4
#define XDPTX_EDID_DTD_SIGNAL_HPOLARITY_MASK		0x02
#define XDPTX_EDID_DTD_SIGNAL_VPOLARITY_MASK		0x04
#define XDPTX_EDID_DTD_SIGNAL_HPOLARITY_SHIFT		1
#define XDPTX_EDID_DTD_SIGNAL_VPOLARITY_SHIFT		2
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       DisplayID extension block.
  * @{
  */
#define XDPTX_EDID_EXT_BLOCK_TAG		0x00
#define XDPTX_DISPID_VER_REV			0x00
#define XDPTX_DISPID_SIZE			0x01
#define XDPTX_DISPID_TYPE			0x02
#define XDPTX_DISPID_EXT_COUNT			0x03
#define XDPTX_DISPID_PAYLOAD_START		0x04
#define XDPTX_DISPID_DB_SEC_TAG			0x00
#define XDPTX_DISPID_DB_SEC_REV			0x01
#define XDPTX_DISPID_DB_SEC_SIZE		0x02
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the DisplayID extension block.
  * @{
  */
#define XDPTX_EDID_EXT_BLOCK_TAG_DISPID		0x70
#define XDPTX_DISPID_TDT_TAG			0x12
/* @} */

/** @name Extended Display Identification Data: Register offsets for the
  *       Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDPTX_DISPID_TDT_TOP0			0x04
#define XDPTX_DISPID_TDT_TOP1			0x05
#define XDPTX_DISPID_TDT_TOP2			0x06
#define XDPTX_DISPID_TDT_HSIZE0			0x07
#define XDPTX_DISPID_TDT_HSIZE1			0x08
#define XDPTX_DISPID_TDT_VSIZE0			0x09
#define XDPTX_DISPID_TDT_VSIZE1			0x0A
#define XDPTX_DISPID_TDT_VENID0			0x10
#define XDPTX_DISPID_TDT_VENID1			0x11
#define XDPTX_DISPID_TDT_VENID2			0x12
#define XDPTX_DISPID_TDT_PCODE0			0x13
#define XDPTX_DISPID_TDT_PCODE1			0x14
#define XDPTX_DISPID_TDT_SN0			0x15
#define XDPTX_DISPID_TDT_SN1			0x16
#define XDPTX_DISPID_TDT_SN2			0x17
#define XDPTX_DISPID_TDT_SN3			0x18
/* @} */

/** @name Extended Display Identification Data: Masks, shifts, and register
  *       values for the Tiled Display Topology (TDT) section data block.
  * @{
  */
#define XDPTX_DISPID_TDT_TOP0_HTOT_L_SHIFT	4
#define XDPTX_DISPID_TDT_TOP0_HTOT_L_MASK	(0xF << 4)
#define XDPTX_DISPID_TDT_TOP0_VTOT_L_MASK	0xF
#define XDPTX_DISPID_TDT_TOP1_HLOC_L_SHIFT	4
#define XDPTX_DISPID_TDT_TOP1_HLOC_L_MASK	(0xF << 4)
#define XDPTX_DISPID_TDT_TOP1_VLOC_L_MASK	0xF
#define XDPTX_DISPID_TDT_TOP2_HTOT_H_SHIFT	6
#define XDPTX_DISPID_TDT_TOP2_HTOT_H_MASK	(0x3 << 6)
#define XDPTX_DISPID_TDT_TOP2_VTOT_H_SHIFT	4
#define XDPTX_DISPID_TDT_TOP2_VTOT_H_MASK	(0x3 << 4)
#define XDPTX_DISPID_TDT_TOP2_HLOC_H_SHIFT	2
#define XDPTX_DISPID_TDT_TOP2_HLOC_H_MASK	(0x3 << 2)
#define XDPTX_DISPID_TDT_TOP2_VLOC_H_MASK	0x3
/* @} */

/******************************************************************************/
/**
 * Multi-stream transport (MST) definitions.
 *
*******************************************************************************/
/** @name Stream identification.
  * @{
  */
#define XDPTX_STREAM_ID1			1
#define XDPTX_STREAM_ID2			2
#define XDPTX_STREAM_ID3			3
#define XDPTX_STREAM_ID4			4
/* @} */

/** @name Sideband message codes when the driver is in MST mode.
  * @{
  */
#define XDPTX_SBMSG_LINK_ADDRESS		0x01
#define XDPTX_SBMSG_ENUM_PATH_RESOURCES		0x10
#define XDPTX_SBMSG_ALLOCATE_PAYLOAD		0x11
#define XDPTX_SBMSG_CLEAR_PAYLOAD_ID_TABLE	0x14
#define XDPTX_SBMSG_REMOTE_DPCD_READ		0x20
#define XDPTX_SBMSG_REMOTE_DPCD_WRITE		0x21
#define XDPTX_SBMSG_REMOTE_I2C_READ		0x22
#define XDPTX_SBMSG_REMOTE_I2C_WRITE		0x23
/* @} */

/******************* Macros (Inline Functions) Definitions ********************/

/** @name Register access macro definitions.
  * @{
  */
#define XDptx_In32 Xil_In32
#define XDptx_Out32 Xil_Out32
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
 *		u32 XDptx_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
*******************************************************************************/
#define XDptx_ReadReg(BaseAddress, RegOffset) \
					XDptx_In32((BaseAddress) + (RegOffset))

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
 *		void XDptx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
*******************************************************************************/
#define XDptx_WriteReg(BaseAddress, RegOffset, Data) \
				XDptx_Out32((BaseAddress) + (RegOffset), (Data))


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
 *		u8 XDptx_IsEdidExtBlockDispId(u8 *Ext)
 *
*******************************************************************************/
#define XDptx_IsEdidExtBlockDispId(Ext) \
	(Ext[XDPTX_EDID_EXT_BLOCK_TAG] == XDPTX_EDID_EXT_BLOCK_TAG_DISPID)

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
 *		u8 XDptx_GetDispIdTdtHTotal(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtHTotal(Tdt) \
	(((((Tdt[XDPTX_DISPID_TDT_TOP2] & XDPTX_DISPID_TDT_TOP2_HTOT_H_MASK) \
	>> XDPTX_DISPID_TDT_TOP2_HTOT_H_SHIFT) << 4) | \
	((Tdt[XDPTX_DISPID_TDT_TOP0] & XDPTX_DISPID_TDT_TOP0_HTOT_L_MASK) >> \
	XDPTX_DISPID_TDT_TOP0_HTOT_L_SHIFT)) + 1)

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
 *		u8 XDptx_GetDispIdTdtVTotal(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtVTotal(Tdt) \
	(((((Tdt[XDPTX_DISPID_TDT_TOP2] & XDPTX_DISPID_TDT_TOP2_VTOT_H_MASK) \
	>> XDPTX_DISPID_TDT_TOP2_VTOT_H_SHIFT) << 4) | \
	(Tdt[XDPTX_DISPID_TDT_TOP0] & XDPTX_DISPID_TDT_TOP0_VTOT_L_MASK)) + 1)

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
 *		u8 XDptx_GetDispIdTdtHLoc(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtHLoc(Tdt) \
	((((Tdt[XDPTX_DISPID_TDT_TOP2] & XDPTX_DISPID_TDT_TOP2_HLOC_H_MASK) \
	>> XDPTX_DISPID_TDT_TOP2_HLOC_H_SHIFT) << 4) | \
	((Tdt[XDPTX_DISPID_TDT_TOP1] & XDPTX_DISPID_TDT_TOP1_HLOC_L_MASK) >> \
	XDPTX_DISPID_TDT_TOP1_HLOC_L_SHIFT))

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
 *		u8 XDptx_GetDispIdTdtVLoc(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtVLoc(Tdt) \
	(((Tdt[XDPTX_DISPID_TDT_TOP2] & XDPTX_DISPID_TDT_TOP2_VLOC_H_MASK) << \
	4) | (Tdt[XDPTX_DISPID_TDT_TOP1] & XDPTX_DISPID_TDT_TOP1_VLOC_L_MASK))

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
 *		u8 XDptx_GetDispIdTdtNumTiles(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtNumTiles(Tdt) \
	(XDptx_GetDispIdTdtHTotal(Tdt) * XDptx_GetDispIdTdtVTotal(Tdt))

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
 *		u8 XDptx_GetDispIdTdtTileOrder(u8 *Tdt)
 *
*******************************************************************************/
#define XDptx_GetDispIdTdtTileOrder(Tdt) \
	((XDptx_GetDispIdTdtVLoc(Tdt) * XDptx_GetDispIdTdtHTotal(Tdt)) + \
	XDptx_GetDispIdTdtHLoc(Tdt))

#endif /* XDPTX_HW_H_ */
