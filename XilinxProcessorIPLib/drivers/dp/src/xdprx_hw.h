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
 * 1.0   als  01/20/14 Initial release.
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
							interrupt sources for
							stream 1. */
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
							interrupt sources for
							streams 2, 3, 4. */
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

/** @name DPRX core registers: User video status.
  * @{
  */
#define XDPRX_USER_FIFO_OVERFLOW	0x110	/**< Indicates an overflow in
							user FIFO. */
#define XDPRX_USER_VSYNC_STATE		0x114	/**< Provides a mechanism for
							the host processor to
							monitor the state of the
							video data path. */
/* @} */

/** @name DPRX core registers: PHY configuration and status.
  * @{
  */
#define XDPRX_PHY_CONFIG		0x200	/**< Transceiver PHY reset and
							configuration. */
#define XDPRX_PHY_STATUS		0x208	/**< Current PHY status. */
#define XDPRX_PHY_POWER_DOWN		0x210	/**< Control PHY power down. */
#define XDPRX_MIN_VOLTAGE_SWING		0x214	/**< Specifies the minimum
							voltage swing required
							during training before
							a link can be reliably
							established and advanced
							configuration for link
							training. */
#define XDPRX_CDR_CONTROL_CONFIG	0x21C	/**< Control the configuration
							for clock and data
							recovery. */
#define XDPRX_GT_DRP_COMMAND		0x2A0	/**< Provides access to the GT
							DRP ports. */
#define XDPRX_GT_DRP_READ_DATA		0x2A4	/**< Provides access to GT DRP
							read data. */
#define XDPRX_GT_DRP_CH_STATUS		0x2A8	/**< Provides access to GT DRP
							channel status. */
/* @} */

/** @name DPRX core registers: Audio.
  * @{
  */
#define XDPRX_RX_AUDIO_CONTROL		0x300	/**< Enables audio stream
							packets in main link. */
#define XDPRX_RX_AUDIO_INFO_DATA(NUM)	(0x304 + 4 * (NUM - 1)) /**< Word
							formatted as per CEA
							861-C info frame. */
#define XDPRX_RX_AUDIO_MAUD		0x324	/**< M value of audio stream
							as decoded from audio
							time stamp packet. */
#define XDPRX_RX_AUDIO_NAUD		0x328	/**< N value of audio stream
							as decoded from audio
							time stamp packet. */
#define XDPRX_RX_AUDIO_STATUS		0x32C	/**< Status of audio stream. */
#define XDPRX_RX_AUDIO_EXT_DATA(NUM)	(0x330 + 4 * (NUM - 1)) /**< Word
							formatted as per
							extension packet. */
/* @} */

/** @name DPRX core registers: DPCD configuration space.
  * @{
  */
#define XDPRX_DPCD_LINK_BW_SET		0x400	/**< Current link bandwidth
							setting as exposed in
							the RX DPCD. */
#define XDPRX_DPCD_LANE_COUNT_SET	0x404	/**< Current lane count
							setting as exposed in
							the RX DPCD. */
#define XDPRX_DPCD_ENHANCED_FRAME_EN	0x408	/**< Current setting for
							enhanced framing symbol
							mode as exposed in the
							RX DPCD. */
#define XDPRX_DPCD_TRAINING_PATTERN_SET	0x40C	/**< Current training pattern
							setting as exposed in
							the RX DPCD. */
#define XDPRX_DPCD_LINK_QUALITY_PATTERN_SET 0x410 /**< Current value of the link
							quality pattern
							field as exposed in the
							RX DPCD. */
#define XDPRX_DPCD_RECOVERED_CLOCK_OUT_EN 0x414	/**< Value of the output clock
							enable field as exposed
							in the RX DPCD. */
#define XDPRX_DPCD_SCRAMBLING_DISABLE	0x418	/**< Value of the scrambling
							disable field as exposed
							in the RX DPCD. */
#define XDPRX_DPCD_SYMBOL_ERROR_COUNT_SELECT 0x41C /**< Current value of the
							symbol error count
							select field as exposed
							in the RX DPCD. */
#define XDPRX_DPCD_TRAINING_LANE_0_SET	0x420	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 0. */
#define XDPRX_DPCD_TRAINING_LANE_1_SET	0x424	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 1. */
#define XDPRX_DPCD_TRAINING_LANE_2_SET	0x428	/**< The RX DPCD value used by
							the TX during link
							training to configure
							the RX PHY lane 2. */
#define XDPRX_DPCD_TRAINING_LANE_3_SET	0x42C	/**< The RX DPCD value Used by
							the TX during link
							training to configure
							the RX PHY lane 3. */
#define XDPRX_DPCD_DOWNSPREAD_CONTROL	0x430	/**< The RX DPCD value that
							is used by the TX to
							inform the RX that
							downspreading has been
							enabled. */
#define XDPRX_DPCD_MAIN_LINK_CHANNEL_CODING_SET 0x434 /**< 8B/10B encoding
							setting as exposed in
							the RX DPCD. */
#define XDPRX_DPCD_SET_POWER_STATE	0x438	/**< Power state requested by
							the TX as exposed in the
							RX DPCD. */
#define XDPRX_DPCD_LANE01_STATUS	0x43C	/**< Link training status for
							lanes 0 and 1 as exposed
							in the RX DPCD. */
#define XDPRX_DPCD_LANE23_STATUS	0x440	/**< Link training status for
							lanes 2 and 3 as exposed
							in the RX DPCD. */
#define XDPRX_DPCD_SOURCE_OUI_VALUE	0x444	/** The RX DPCD value used by
							the TX to set the
							organizationally unique
							identifier (OUI). */
#define XDPRX_DPCD_SYM_ERR_CNT01	0x448	/** The symbol error counter
							values for lanes 0 and 1
							as exposed in the RX
							DPCD. */
#define XDPRX_DPCD_SYM_ERR_CNT23	0x44C	/** The symbol error counter
							values for lanes 2 and 3
							as exposed in the RX
							DPCD. */
/* @} */

/** @name DPRX core registers: Main stream attributes for SST / MST STREAM1.
  * @{
  */
#define XDPRX_STREAM1_MSA_START		0x500	/**< Start of the MSA registers
							for stream 1. */
#define XDPRX_MSA_HRES			0x500	/**< Number of active pixels per
							line (the horizontal
							resolution). */
#define XDPRX_MSA_HSPOL			0x504	/**< The horizontal sync
							polarity. */
#define XDPRX_MSA_HSWIDTH		0x508	/**< Width of the horizontal
							sync pulse. */
#define XDPRX_MSA_HSTART		0x50C	/**< Number of clocks between
							the leading edge of the
							horizontal sync and the
							start of active data. */
#define XDPRX_MSA_HTOTAL		0x510	/**< Total number of clocks in
							the horizontal framing
							period. */
#define XDPRX_MSA_VHEIGHT		0x514	/**< Number of active lines (the
							vertical resolution). */
#define XDPRX_MSA_VSPOL			0x518	/**< The vertical sync
							polarity. */
#define XDPRX_MSA_VSWIDTH		0x51C	/**< Width of the vertical
							sync pulse. */
#define XDPRX_MSA_VSTART		0x520	/**< Number of lines between the
							leading edge of the
							vertical sync and the
							first line of active
							data. */
#define XDPRX_MSA_VTOTAL		0x524	/**< Total number of lines in
							the video frame. */
#define XDPRX_MSA_MISC0			0x528	/**< Miscellaneous stream
							attributes. */
#define XDPRX_MSA_MISC1			0x52C	/**< Miscellaneous stream
							attributes. */
#define XDPRX_MSA_MVID			0x530	/**< Used to recover the video
							clock from the link
							clock. */
#define XDPRX_MSA_NVID			0x534	/**< Used to recover the video
							clock from the link
							clock. */
#define XDPRX_MSA_VBID			0x538	/**< The most recently received
							VB-ID value. */
/* @} */

/** @name DPRX core registers: Main stream attributes for MST STREAM2, 3, and 4.
  * @{
  */
#define XDPRX_STREAM2_MSA_START		0x540	/**< Start of the MSA registers
							for stream 2. */
#define XDPRX_STREAM2_MSA_START_OFFSET	(XDPRX_STREAM2_MSA_START - \
		XDPRX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 2 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDPRX_STREAM3_MSA_START		0x580	/**< Start of the MSA registers
							for stream 3. */
#define XDPRX_STREAM3_MSA_START_OFFSET	(XDPRX_STREAM3_MSA_START - \
		XDPRX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 3 are at an
							offset from the
							corresponding registers
							of stream 1. */
#define XDPRX_STREAM4_MSA_START		0x5C0	/**< Start of the MSA registers
							for stream 4. */
#define XDPRX_STREAM4_MSA_START_OFFSET	(XDPRX_STREAM4_MSA_START - \
		XDPRX_STREAM1_MSA_START)	/**< The MSA registers for
							stream 4 are at an
							offset from the
							corresponding registers
							of stream 1. */
/* @} */

/** @name DPRX core registers: MST field for sideband message buffers and the
  *	  virtual channel payload table.
  * @{
  */
#define XDPRX_DOWN_REQ			0xA00	/**< Down request buffer address
							space. */
#define XDPRX_DOWN_REP			0xB00	/**< Down reply buffer address
							space. */
#define XDPRX_VC_PAYLOAD_TABLE		0x800	/**< Virtual channel payload
							table (0xFF bytes). */
#define XDPRX_VC_PAYLOAD_TABLE_ID_SLOT(SlotNum) \
			(XDPRX_VC_PAYLOAD_TABLE + SlotNum)
/* @} */

/** @name DPRX core registers: Vendor specific DPCD.
  * @{
  */
#define XDPRX_SOURCE_DEVICE_SPECIFIC_FIELD 0xE00 /**< User access to the source
							specific field as
							exposed in the RX
							DPCD (0xFF bytes). */
#define XDPRX_SOURCE_DEVICE_SPECIFIC_FIELD_REG(RegNum) \
			(XDPRX_SOURCE_DEVICE_SPECIFIC_FIELD + (4 * RegNum))
#define XDPRX_SINK_DEVICE_SPECIFIC_FIELD 0xF00	/**< User access to the sink
							specific field as
							exposed in the RX
							DPCD (0xFF bytes). */
#define XDPRX_SINK_DEVICE_SPECIFIC_FIELD_REG(RegNum) \
			(XDPRX_SINK_DEVICE_SPECIFIC_FIELD + (4 * RegNum))
/* @} */

/******************************************************************************/

/** @name DPRX core masks, shifts, and register values.
  * @{
  */
/* 0x004: AUX_CLK_DIVIDER */
#define XDPRX_AUX_CLK_DIVIDER_VAL_MASK	0x00FF	/**< Clock divider value. */
#define XDPRX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_MASK \
					0xFF00	/**< AUX (noise) signal width
							filter. */
#define XDPRX_AUX_CLK_DIVIDER_AUX_SIG_WIDTH_FILT_SHIFT \
					8	/**< Shift bits for AUX signal
							width filter. */
/* 0x010: USER_PIXEL_WIDTH */
#define XDPRX_USER_PIXEL_WIDTH_1	0x1	/**< Single pixel wide
							interface. */
#define XDPRX_USER_PIXEL_WIDTH_2	0x2	/**< Dual pixel output mode. */
#define XDPRX_USER_PIXEL_WIDTH_4	0x4	/**< Quad pixel output mode. */
/* 0x014: INTERRUPT_MASK */
#define XDPRX_INTERRUPT_MASK_VM_CHANGE_MASK \
					0x00001	/**< Mask the interrupt
							assertion for a
							resolution change, as
							detected from the MSA
							fields. */
#define XDPRX_INTERRUPT_MASK_POWER_STATE_MASK \
					0x00002 /**< Mask the interrupt
							assertion for a power
							state change. */
#define XDPRX_INTERRUPT_MASK_NO_VIDEO_MASK \
					0x00004 /**< Mask the interrupt
							assertion for the
							no-video condition being
							detected after active
							video received. */
#define XDPRX_INTERRUPT_MASK_VBLANK_MASK \
					0x00008 /**< Mask the interrupt
							assertion for the start
							of the blanking
							interval. */
#define XDPRX_INTERRUPT_MASK_TRAINING_LOST_MASK \
					0x00010 /**< Mask the interrupt
							assertion for training
							loss on active lanes. */
#define XDPRX_INTERRUPT_MASK_VIDEO_MASK	0x00040 /**< Mask the interrupt
							assertion for a valid
							video frame being
							detected on the main
							link. Video interrupt is
							set after a delay of 8
							video frames following a
							valid scrambler reset
							character. */
#define XDPRX_INTERRUPT_MASK_INFO_PKT_MASK \
					0x00100 /**< Mask the interrupt
							assertion for an audio
							info packet being
							received. */
#define XDPRX_INTERRUPT_MASK_EXT_PKT_MASK \
					0x00200 /**< Mask the interrupt
							assertion for an audio
							extension packet being
							received. */
#define XDPRX_INTERRUPT_MASK_VCP_ALLOC_MASK \
					0x00400 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDPRX_INTERRUPT_MASK_VCP_DEALLOC_MASK \
					0x00800 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDPRX_INTERRUPT_MASK_DOWN_REPLY_MASK \
					0x01000 /**< Mask the interrupt
							assertion for a
							downstream reply being
							ready. */
#define XDPRX_INTERRUPT_MASK_DOWN_REQUEST_MASK \
					0x02000 /**< Mask the interrupt
							assertion for a
							downstream request being
							ready. */
#define XDPRX_INTERRUPT_MASK_TRAINING_DONE_MASK \
					0x04000 /**< Mask the interrupt
							assertion for link
							training completion. */
#define XDPRX_INTERRUPT_MASK_BW_CHANGE_MASK \
					0x08000 /**< Mask the interrupt
							assertion for a change
							in bandwidth. */
#define XDPRX_INTERRUPT_MASK_TP1_MASK	0x10000 /**< Mask the interrupt
							assertion for start of
							training pattern 1. */
#define XDPRX_INTERRUPT_MASK_TP2_MASK	0x20000 /**< Mask the interrupt
							assertion for start of
							training pattern 2. */
#define XDPRX_INTERRUPT_MASK_TP3_MASK	0x40000 /**< Mask the interrupt
							assertion for start of
							training pattern 3. */
#define XDPRX_INTERRUPT_MASK_ALL_MASK	0x7FFFF /**< Mask all interrupts. */
/* 0x018: MISC_CTRL */
#define XDPRX_MISC_CTRL_USE_FILT_MSA_MASK \
					0x1	/**< When set, two matching
							values must be detected
							for each field of the
							MSA values before the
							associated register is
							updated internally. */
#define XDPRX_MISC_CTRL_LONG_I2C_USE_DEFER_MASK \
					0x2	/**< When set, the long I2C
							write data transfwers
							are responded to using
							DEFER instead of partial
							ACKs. */
#define XDPRX_MISC_CTRL_I2C_USE_AUX_DEFER_MASK \
					0x4	/**< When set, I2C DEFERs will
							be sent as AUX DEFERs to
							the source device. */
/* 0x01C: SOFT_RESET */
#define XDPRX_SOFT_RESET_VIDEO_MASK	0x01	/**< Reset the video logic. */
#define XDPRX_SOFT_RESET_AUX_MASK	0x80	/**< Reset the AUX logic. */
/* 0x02C: HPD_INTERRUPT */
#define XDPRX_HPD_INTERRUPT_ASSERT_MASK \
				0x00000001	/**< Instructs the RX core to
							assert an interrupt to
							the TX using the HPD
							signal. */
#define XDPRX_HPD_INTERRUPT_LENGTH_US_MASK \
				0xFFFF0000	/**< The length of the HPD pulse
							to generate (in
							microseconds). */
#define XDPRX_HPD_INTERRUPT_LENGTH_US_SHIFT 16	/**< Shift bits for the HPD
							pulse length. */
/* 0x040: INTERRUPT_CAUSE */
#define XDPRX_INTERRUPT_CAUSE_VM_CHANGE_MASK \
	XDPRX_INTERRUPT_MASK_VM_CHANGE_MASK	/**< Interrupt caused by a
							resolution change, as
							detected from the MSA
							fields. */
#define XDPRX_INTERRUPT_CAUSE_POWER_STATE_MASK \
	XDPRX_INTERRUPT_MASK_POWER_STATE_MASK	/**< Interrupt caused by a
							power state change. */
#define XDPRX_INTERRUPT_CAUSE_NO_VIDEO_MASK \
	XDPRX_INTERRUPT_MASK_NO_VIDEO_MASK	/**< Interrupt caused by the
							no-video condition being
							detected after active
							video received. */
#define XDPRX_INTERRUPT_CAUSE_VBLANK_MASK \
		XDPRX_INTERRUPT_MASK_VBLANK_MASK /**< Interrupt caused by the
							start of the blanking
							interval. */
#define XDPRX_INTERRUPT_CAUSE_TRAINING_LOST_MASK \
	XDPRX_INTERRUPT_MASK_TRAINING_LOST_MASK	/**< Interrupt caused by
							training loss on active
							lanes. */
#define XDPRX_INTERRUPT_CAUSE_VIDEO_MASK \
		XDPRX_INTERRUPT_MASK_VIDEO_MASK	/**< Interrupt caused by a valid
							video frame being
							detected on the main
							link. Video interrupt is
							set after a delay of 8
							video frames following a
							valid scrambler reset
							character. */
#define XDPRX_INTERRUPT_CAUSE_INFO_PKT_MASK \
	XDPRX_INTERRUPT_MASK_INFO_PKT_MASK	/**< Interrupt caused by an
							audio info packet being
							received. */
#define XDPRX_INTERRUPT_CAUSE_EXT_PKT_MASK \
	XDPRX_INTERRUPT_MASK_EXT_PKT_MASK	/**< Interrupt caused by an
							audio extension packet
							being received. */
#define XDPRX_INTERRUPT_CAUSE_VCP_ALLOC_MASK \
	XDPRX_INTERRUPT_MASK_VCP_ALLOC_MASK	/**< Interrupt caused by a
							virtual channel payload
							being allocated. */
#define XDPRX_INTERRUPT_CAUSE_VCP_DEALLOC_MASK \
	XDPRX_INTERRUPT_MASK_VCP_DEALLOC_MASK	/**< Interrupt caused by a
							virtual channel payload
							being allocated. */
#define XDPRX_INTERRUPT_CAUSE_DOWN_REPLY_MASK \
	XDPRX_INTERRUPT_MASK_DOWN_REPLY_MASK	/**< Interrupt caused by a
							downstream reply being
							ready. */
#define XDPRX_INTERRUPT_CAUSE_DOWN_REQUEST_MASK \
	XDPRX_INTERRUPT_MASK_DOWN_REQUEST_MASK	/**< Interrupt caused by a
							downstream request being
							ready. */
#define XDPRX_INTERRUPT_CAUSE_TRAINING_DONE_MASK \
	XDPRX_INTERRUPT_MASK_TRAINING_DONE_MASK	/**< Interrupt caused by link
							training completion. */
#define XDPRX_INTERRUPT_CAUSE_BW_CHANGE_MASK \
	XDPRX_INTERRUPT_MASK_BW_CHANGE_MASK	/**< Interrupt caused by a
							change in bandwidth. */
#define XDPRX_INTERRUPT_CAUSE_TP1_MASK \
		XDPRX_INTERRUPT_MASK_TP1_MASK	/**< Interrupt caused by the
							start of training
							pattern 1. */
#define XDPRX_INTERRUPT_CAUSE_TP2_MASK \
		XDPRX_INTERRUPT_MASK_TP2_MASK	/**< Interrupt caused by the
							start of training
							pattern 2. */
#define XDPRX_INTERRUPT_CAUSE_TP3_MASK \
		XDPRX_INTERRUPT_MASK_TP3_MASK	/**< Interrupt caused by the
							start of training
							pattern 3. */
/* 0x044: INTERRUPT_MASK_1 */
#define XDPRX_INTERRUPT_MASK_1_EXT_PKT_STREAM234_MASK(Stream) \
		(0x00001 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for an audio
							extension packet being
							received for stream
							2, 3, or 4. */
#define XDPRX_INTERRUPT_MASK_1_INFO_PKT_STREAM234_MASK(Stream) \
		(0x00002 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for an audio
							info packet being
							received for stream
							2, 3, or 4. */
#define XDPRX_INTERRUPT_MASK_1_VM_CHANGE_STREAM234_MASK(Stream) \
		(0x00004 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for a
							resolution change, as
							detected from the MSA
							fields for stream 2, 3,
							or 4. */
#define XDPRX_INTERRUPT_MASK_1_NO_VIDEO_STREAM234_MASK(Stream) \
		(0x00008 << ((Stream - 2) * 6))	/**< Mask the interrupt
							assertion for the
							no-video condition being
							detected after active
							video received for
							stream 2, 3, or 4. */
#define XDPRX_INTERRUPT_MASK_1_VBLANK_STREAM234_MASK(Stream) \
		(0x00010 << ((Stream - 2) * 6)) /**< Mask the interrupt
							assertion for the start
							of the blanking interval
							for stream 2, 3, or
							4. */
#define XDPRX_INTERRUPT_MASK_1_VIDEO_STREAM234_MASK(Stream) \
		(0x00020 << ((Stream - 2) * 6)) /**< Mask the interrupt
							assertion for a valid
							video frame being
							detected on the main
							link for stream 2, 3,
							or 4. */
/* 0x048: INTERRUPT_CAUSE_1 */
#define XDPRX_INTERRUPT_CAUSE_1_EXT_PKT_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_EXT_PKT_STREAM234_MASK(Stream) /**< Interrupt
							caused by an audio
							extension packet being
							received for stream 2,
							3, or 4. */
#define XDPRX_INTERRUPT_CAUSE_1_INFO_PKT_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_INFO_PKT_STREAM234_MASK(Stream) /**< Interrupt
							caused by an audio info
							packet being received
							for stream 2, 3, or
							4. */
#define XDPRX_INTERRUPT_CAUSE_1_VM_CHANGE_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_VM_CHANGE_STREAM234_MASK(Stream) /**< Interrupt
							caused by a resolution
							change, as detected from
							the MSA fields for
							stream 2, 3, or 4. */
#define XDPRX_INTERRUPT_CAUSE_1_NO_VIDEO_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_NO_VIDEO_STREAM234_MASK(Stream) /**< Interrupt
							caused by the no-video
							condition being detected
							after active video
							received for stream 2,
							3, or 4. */
#define XDPRX_INTERRUPT_CAUSE_1_VBLANK_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_VBLANK_STREAM234_MASK(Stream) /**< Interrupt
							caused by the start of
							the blanking interval
							for stream 2, 3, or
							4. */
#define XDPRX_INTERRUPT_CAUSE_1_VIDEO_STREAM234_MASK(Stream) \
	XDPRX_INTERRUPT_CAUSE_1_VIDEO_STREAM234_MASK(Stream) /**< Interrupt
							caused by a valid video
							frame being detected on
							the main link for
							stream 2, 3, or 4. */
/* 0x050: HSYNC_WIDTH */
#define XDPRX_HSYNC_WIDTH_PULSE_WIDTH_MASK \
					0x00FF	/**< Specifies the number of
							clock cycles the
							horizontal sync pulse is
							asserted. */
#define XDPRX_HSYNC_WIDTH_FRONT_PORCH_MASK \
					0xFF00	/**< Defines the number of video
							clock cycles to place
							between the last pixel
							of active data and the
							start of the horizontal
							sync pulse (the front
							porch). */
#define XDPRX_HSYNC_WIDTH_FRONT_PORCH_SHIFT 8	/**< Shift bits for the front
							porch. */
/* 0x090: DEVICE_SERVICE_IRQ */
#define XDPRX_DEVICE_SERVICE_IRQ_NEW_REMOTE_CMD_MASK \
					0x01	/**< Indicates that a new
							command is present in
							the REMOTE_CMD
							register. */
#define XDPRX_DEVICE_SERVICE_IRQ_SINK_SPECIFIC_IRQ_MASK \
					0x02	/**< Reflects the
							SINK_SPECIFIC_IRQ
							state. */
#define XDPRX_DEVICE_SERVICE_IRQ_NEW_DOWN_REPLY_MASK \
					0x10	/**< Indicates a new DOWN_REPLY
							buffer message is
							ready. */
/* 0x09C: OVER_LINK_BW_SET */
#define XDPRX_OVER_LINK_BW_SET_162GBPS	0x06    /**< 1.62 Gbps link rate. */
#define XDPRX_OVER_LINK_BW_SET_270GBPS	0x0A    /**< 2.70 Gbps link rate. */
#define XDPRX_OVER_LINK_BW_SET_540GBPS	0x14    /**< 5.40 Gbps link rate. */

/* 0x0A0: OVER_LANE_COUNT_SET */
#define XDPRX_OVER_LANE_COUNT_SET_MASK	0x1F	/**< The lane count override
							value. */
#define XDPRX_OVER_LANE_COUNT_SET_1	0x1	/**< Lane count of 1. */
#define XDPRX_OVER_LANE_COUNT_SET_2	0x2	/**< Lane count of 2. */
#define XDPRX_OVER_LANE_COUNT_SET_4	0x4	/**< Lane count of 4. */
#define XDPRX_OVER_LANE_COUNT_SET_TPS3_SUPPORTED_MASK \
					0x20	/**< Capability override for
							training pattern 3. */
#define XDPRX_OVER_LANE_COUNT_SET_ENHANCED_FRAME_CAP_MASK \
					0x80	/**< Capability override for
							enhanced framing. */
/* 0x0A4: OVER_TP_SET */
#define XDPRX_OVER_TP_SET_TP_SELECT_MASK \
					0x0003	/**< Training pattern select
							override. */
#define XDPRX_OVER_TP_SET_LQP_SET_MASK \
					0x000C	/**< Link quality pattern set
							override. */
#define XDPRX_OVER_TP_SET_LQP_SET_SHIFT	2	/**< Shift bits for link quality
							pattern set override. */
#define XDPRX_OVER_TP_SET_REC_CLK_OUT_EN_MASK \
					0x0010	/**< Recovered clock output
							enable override. */
#define XDPRX_OVER_TP_SET_SCRAMBLER_DISABLE_MASK \
					0x0020	/**< Scrambling disable
							override. */
#define XDPRX_OVER_TP_SET_SYMBOL_ERROR_COUNT_SEL_MASK \
					0x00C0	/**< Symbol error count
							override. */
#define XDPRX_OVER_TP_SET_SYMBOL_ERROR_COUNT_SEL_SHIFT \
					6	/**< Shift bits for symbol error
							count override. */
#define XDPRX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_MASK \
					0xFF00	/**< Training AUX read interval
							override. */
#define XDPRX_OVER_TP_SET_TRAINING_AUX_RD_INTERVAL_SHIFT \
					8	/**< Shift bits for training AUX
							read interval
							override. */
/* 0x0A8, 0x0AC, 0x0B0, 0x0B4: OVER_TRAINING_LANEX_SET */
#define XDPRX_OVER_TRAINING_LANEX_SET_VS_SET_MASK \
					0x03	/**< Voltage swing set
							override. */
#define XDPRX_OVER_TRAINING_LANEX_SET_MAX_VS_MASK \
					0x04	/**< Maximum voltage swing
							override. */
#define XDPRX_OVER_TRAINING_LANEX_SET_PE_SET_MASK \
					0x18	/**< Pre-emphasis set
							override. */
#define XDPRX_OVER_TRAINING_LANEX_SET_PE_SET_SHIFT \
					3	/**< Shift bits for pre-emphasis
							set override. */
#define XDPRX_OVER_TRAINING_LANEX_SET_MAX_PE_MASK \
					0x20	/**< Maximum pre-emphasis
							override. */
/* 0x0F8 : VERSION_REGISTER */
#define XDPRX_VERSION_INTER_REV_MASK \
				0x0000000F	/**< Internal revision. */
#define XDPRX_VERSION_CORE_PATCH_MASK \
				0x00000030	/**< Core patch details. */
#define XDPRX_VERSION_CORE_PATCH_SHIFT \
				8		/**< Shift bits for core patch
							details. */
#define XDPRX_VERSION_CORE_VER_REV_MASK \
				0x000000C0	/**< Core version revision. */
#define XDPRX_VERSION_CORE_VER_REV_SHIFT \
				12		/**< Shift bits for core version
							revision. */
#define XDPRX_VERSION_CORE_VER_MNR_MASK \
				0x00000F00	/**< Core minor version. */
#define XDPRX_VERSION_CORE_VER_MNR_SHIFT \
				16		/**< Shift bits for core minor
							version. */
#define XDPRX_VERSION_CORE_VER_MJR_MASK \
				0x0000F000	/**< Core major version. */
#define XDPRX_VERSION_CORE_VER_MJR_SHIFT \
				24		/**< Shift bits for core major
							version. */
/* 0x0FC : CORE_ID */
#define XDPRX_CORE_ID_TYPE_MASK	0x0000000F	/**< Core type. */
#define XDPRX_CORE_ID_TYPE_TX	0x0		/**< Core is a transmitter. */
#define XDPRX_CORE_ID_TYPE_RX	0x1		/**< Core is a receiver. */
#define XDPRX_CORE_ID_DP_REV_MASK \
				0x000000F0	/**< DisplayPort protocol
							revision. */
#define XDPRX_CORE_ID_DP_REV_SHIFT \
				8		/**< Shift bits for DisplayPort
							protocol revision. */
#define XDPRX_CORE_ID_DP_MNR_VER_MASK \
				0x00000F00	/**< DisplayPort protocol minor
							version. */
#define XDPRX_CORE_ID_DP_MNR_VER_SHIFT \
				16		/**< Shift bits for DisplayPort
							protocol major
							version. */
#define XDPRX_CORE_ID_DP_MJR_VER_MASK \
				0x0000F000	/**< DisplayPort protocol major
							version. */
#define XDPRX_CORE_ID_DP_MJR_VER_SHIFT \
				24		/**< Shift bits for DisplayPort
							protocol major
							version. */
/* 0x110: USER_FIFO_OVERFLOW */
#define XDPRX_USER_FIFO_OVERFLOW_FLAG_STREAMX_MASK(Stream) \
				(Stream)	/**< Indicates that the internal
							FIFO has detected on
							overflow condition for
							the specified stream. */
#define XDPRX_USER_FIFO_OVERFLOW_VID_UNPACK_STREAMX_MASK(Stream) \
				(Stream << 4)	/**< Indicates that the video
							unpack FIFO has
							overflown for the
							specified stream. */
#define XDPRX_USER_FIFO_OVERFLOW_VID_TIMING_STREAMX_MASK(Stream) \
				(Stream << 8)	/**< Indicates that the video
							timing FIFO has
							overflown for the
							specified stream. */
/* 0x114: USER_VSYNC_STATE */
#define XDPRX_USER_VSYNC_STATE_STREAMX_MASK(Stream) \
				(Stream)	/**< The state of the vertical
							sync pulse for the
							specified stream. */
/* 0x200: PHY_CONFIG */
#define XDPRX_PHY_CONFIG_PHY_RESET_ENABLE_MASK \
				0x00000000	/**< Release reset. */
#define XDPRX_PHY_CONFIG_GTPLL_RESET_MASK \
				0x00000001	/**< Hold the GTPLL in reset. */
#define XDPRX_PHY_CONFIG_GTRX_RESET_MASK \
				0x00000002	/**< Hold GTRXRESET in reset. */
#define XDPRX_PHY_CONFIG_RX_PHY_PMA_RESET_MASK \
				0x00000100	/**< Hold RX_PHY_PMA reset. */
#define XDPRX_PHY_CONFIG_RX_PHY_PCS_RESET_MASK \
				0x00000200	/**< Hold RX_PHY_PCS reset. */
#define XDPRX_PHY_CONFIG_RX_PHY_BUF_RESET_MASK \
				0x00000400	/**< Hold RX_PHY_BUF reset. */
#define XDPRX_PHY_CONFIG_RX_PHY_DFE_LPM_RESET_MASK \
				0x00000800	/**< Hold RX_PHY_DFE_LPM
							reset. */
#define XDPRX_PHY_CONFIG_RX_PHY_POLARITY_MASK \
				0x00001000	/**< Set RX_PHY_POLARITY. */
#define XDPRX_PHY_CONFIG_RX_PHY_LOOPBACK_MASK \
				0x0000E000	/**< Set RX_PHY_LOOPBACK. */
#define XDPRX_PHY_CONFIG_RX_PHY_EYESCANRESET_MASK \
				0x00010000	/**< Set RX_PHY_EYESCANRESET. */
#define XDPRX_PHY_CONFIG_RX_PHY_EYESCANTRIGGER_MASK \
				0x00020000	/**< Set RX_PHY_
							EYESCANTRIGGER. */
#define XDPRX_PHY_CONFIG_RX_PHY_PRBSCNTRESET_MASK \
				0x00040000	/**< Set RX_PHY_PRBSCNTRESET. */
#define XDPRX_PHY_CONFIG_RX_PHY_RXLPMHFHOLD_MASK \
				0x00080000	/**< Set RX_PHY_RXLPMHFHOLD. */
#define XDPRX_PHY_CONFIG_RX_PHY_RXLPMLFHOLD_MASK \
				0x00100000	/**< Set RX_PHY_RXLPMLFHOLD. */
#define XDPRX_PHY_CONFIG_RX_PHY_RXLPMHFOVERDEN_MASK \
				0x00200000	/**< Set RX_PHY_
							RXLPMHFOVERDEN. */
#define XDPRX_PHY_CONFIG_RX_PHY_CDRHOLD_MASK \
				0x00400000	/**< Set RX_PHY_CDRHOLD. */
#define XDPRX_PHY_CONFIG_RESET_AT_TRAIN_ITER_MASK \
				0x00800000	/**< Issue reset at every
							training iteration. */
#define XDPRX_PHY_CONFIG_RESET_AT_LINK_RATE_CHANGE_MASK \
				0x01000000	/**< Issue reset at every link
							rate change. */
#define XDPRX_PHY_CONFIG_RESET_AT_TP1_START_MASK \
				0x02000000	/**< Issue reset at start of
							training pattern 1. */
#define XDPRX_PHY_CONFIG_EN_CFG_RX_PHY_POLARITY_MASK \
				0x04000000	/**< Enable the individual lane
							polarity. */
#define XDPRX_PHY_CONFIG_RX_PHY_POLARITY_LANE0_MASK \
				0x08000000	/**< Configure RX_PHY_POLARITY
							for lane 0. */
#define XDPRX_PHY_CONFIG_RX_PHY_POLARITY_LANE1_MASK \
				0x10000000	/**< Configure RX_PHY_POLARITY
							for lane 1. */
#define XDPRX_PHY_CONFIG_RX_PHY_POLARITY_LANE2_MASK \
				0x20000000	/**< Configure RX_PHY_POLARITY
							for lane 2. */
#define XDPRX_PHY_CONFIG_RX_PHY_POLARITY_LANE3_MASK \
				0x40000000	/**< Configure RX_PHY_POLARITY
							for lane 3. */
#define XDPRX_PHY_CONFIG_GT_ALL_RESET_MASK \
				0x00000003	/**< Rest GT and PHY. */













/* 0x208: PHY_STATUS */
#define XDPRX_PHY_STATUS_RESET_LANE_0_1_DONE_MASK \
				0x00000003	/**< Reset done for lanes
							0 and 1. */
#define XDPRX_PHY_STATUS_RESET_LANE_2_3_DONE_MASK \
				0x0000000C	/**< Reset done for lanes
							2 and 3. */
#define XDPRX_PHY_STATUS_RESET_LANE_2_3_DONE_SHIFT \
				2		/**< Shift bits for reset done
							for lanes 2 and 3. */
#define XDPRX_PHY_STATUS_PLL_LANE0_1_LOCK_MASK \
				0x00000010	/**< PLL locked for lanes
							0 and 1. */
#define XDPRX_PHY_STATUS_PLL_LANE2_3_LOCK_MASK \
				0x00000020	/**< PLL locked for lanes
							2 and 3. */
#define XDPRX_PHY_STATUS_PLL_FABRIC_LOCK_MASK \
				0x00000040	/**< FPGA fabric clock PLL
							locked. */
#define XDPRX_PHY_STATUS_RX_CLK_LOCK_MASK \
				0x00000080	/**< Receiver clock locked. */
#define XDPRX_PHY_STATUS_PRBSERR_LANE_0_MASK \
				0x00000100	/**< PRBS error on lane 0. */
#define XDPRX_PHY_STATUS_PRBSERR_LANE_1_MASK \
				0x00000200	/**< PRBS error on lane 1. */
#define XDPRX_PHY_STATUS_PRBSERR_LANE_2_MASK \
				0x00000400	/**< PRBS error on lane 2. */
#define XDPRX_PHY_STATUS_PRBSERR_LANE_3_MASK \
				0x00000800	/**< PRBS error on lane 3. */
#define XDPRX_PHY_STATUS_RX_VLOW_LANE_0_MASK \
				0x00001000	/**< RX voltage low on lane
							0. */
#define XDPRX_PHY_STATUS_RX_VLOW_LANE_1_MASK \
				0x00002000	/**< RX voltage low on lane
							1. */
#define XDPRX_PHY_STATUS_RX_VLOW_LANE_2_MASK \
				0x00004000	/**< RX voltage low on lane
							2. */
#define XDPRX_PHY_STATUS_RX_VLOW_LANE_3_MASK \
				0x00008000	/**< RX voltage low on lane
							3. */
#define XDPRX_PHY_STATUS_LANE_ALIGN_LANE_0_MASK \
				0x00010000	/**< Lane aligment status for
							lane 0. */
#define XDPRX_PHY_STATUS_LANE_ALIGN_LANE_1_MASK \
				0x00020000	/**< Lane aligment status for
							lane 1. */
#define XDPRX_PHY_STATUS_LANE_ALIGN_LANE_2_MASK \
				0x00040000	/**< Lane aligment status for
							lane 2. */
#define XDPRX_PHY_STATUS_LANE_ALIGN_LANE_3_MASK \
				0x00080000	/**< Lane aligment status for
							lane 3. */
#define XDPRX_PHY_STATUS_SYM_LOCK_LANE_0_MASK \
				0x00100000	/**< Symbol lock status for
							lane 0. */
#define XDPRX_PHY_STATUS_SYM_LOCK_LANE_1_MASK \
				0x00200000	/**< Symbol lock status for
							lane 1. */
#define XDPRX_PHY_STATUS_SYM_LOCK_LANE_2_MASK \
				0x00400000	/**< Symbol lock status for
							lane 2. */
#define XDPRX_PHY_STATUS_SYM_LOCK_LANE_3_MASK \
				0x00800000	/**< Symbol lock status for
							lane 3. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_0_MASK \
				0x03000000	/**< RX buffer status lane 0. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_0_SHIFT \
				24		/**< Shift bits for RX buffer
							status lane 0. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_1_MASK \
				0x0C000000	/**< RX buffer status lane 1. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUE_LANE_1_SHIFT \
				26		/**< Shift bits for RX buffer
							status lane 1. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_2_MASK \
				0x30000000	/**< RX buffer status lane 2. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_2_SHIFT \
				28		/**< Shift bits for RX buffer
							status lane 2. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_3_MASK \
				0xC0000000	/**< RX buffer status lane 3. */
#define XDPRX_PHY_STATUS_RX_BUFFER_STATUS_LANE_3_SHIFT \
				30		/**< Shift bits for RX buffer
							status lane 3. */
#define XDPRX_PHY_STATUS_LANES_0_1_READY_MASK \
				0x00000013	/**< Lanes 0 and 1 are ready. */
#define XDPRX_PHY_STATUS_ALL_LANES_READY_MASK \
				0x0000003F	/**< All lanes are ready. */
/* 0x210: PHY_POWER_DOWN */
#define XDPRX_PHY_POWER_DOWN_LANE_0_MASK 0x1	/**< Power down the PHY for lane
							0. */
#define XDPRX_PHY_POWER_DOWN_LANE_1_MASK 0x2	/**< Power down the PHY for lane
							1. */
#define XDPRX_PHY_POWER_DOWN_LANE_2_MASK 0x4	/**< Power down the PHY for lane
							2. */
#define XDPRX_PHY_POWER_DOWN_LANE_3_MASK 0x8	/**< Power down the PHY for lane
							3. */
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
