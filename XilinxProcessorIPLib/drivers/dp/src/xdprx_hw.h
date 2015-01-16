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

/** @name DPTX core masks, shifts, and register values.
  * @{
  */
/* 0x014: INTERRUPT_MASK */
#define XDPRX_INTERRUPT_MASK_VIDEO_MODE_CHANGE \
					0x00001	/**< Mask the interrupt
							assertion for a
							resolution change, as
							detected from the MSA
							fields. */
#define XDPRX_INTERRUPT_MASK_POWER_STATE \
					0x00002 /**< Mask the interrupt
							assertion for a power
							state change. */
#define XDPRX_INTERRUPT_MASK_NO_VIDEO	0x00004 /**< Mask the interrupt
							assertion for the
							no-video condition being
							detected after active
							video received. */
#define XDPRX_INTERRUPT_MASK_VERTICAL_BLANKING \
					0x00008 /**< Mask the interrupt
							assertion for the start
							of the blanking
							interval. */
#define XDPRX_INTERRUPT_MASK_TRAINING_LOST \
					0x00010 /**< Mask the interrupt
							assertion for training
							loss on active lanes. */
#define XDPRX_INTERRUPT_MASK_VIDEO	0x00040 /**< Mask the interrupt
							assertion for a valid
							video frame being
							detected on the main
							link. Video interrupt is
							set after a delay of 8
							video frames following a
							valid scrambler reset
							character. */
#define XDPRX_INTERRUPT_MASK_INFO_PKT_RXD \
					0x00100 /**< Mask the interrupt
							assertion for an audio
							info packet being
							received. */
#define XDPRX_INTERRUPT_MASK_EXT_PKT_RXD \
					0x00200 /**< Mask the interrupt
							assertion for an audio
							extension packet being
							received. */
#define XDPRX_INTERRUPT_MASK_VCP_ALLOC	0x00400 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDPRX_INTERRUPT_MASK_VCP_DEALLOC \
					0x00800 /**< Mask the interrupt
							assertion for a virtual
							channel payload being
							allocated. */
#define XDPRX_INTERRUPT_MASK_DOWN_REPLY	0x01000 /**< Mask the interrupt
							assertion for a
							downstream reply being
							ready. */
#define XDPRX_INTERRUPT_MASK_DOWN_REQUEST \
					0x02000 /**< Mask the interrupt
							assertion for a
							downstream request being
							ready. */
#define XDPRX_INTERRUPT_MASK_TRAINING_DONE \
					0x04000 /**< Mask the interrupt
							assertion for link
							training completion. */
#define XDPRX_INTERRUPT_MASK_BW_CHANGE	0x08000 /**< Mask the interrupt
							assertion for a change
							in bandwidth. */
#define XDPRX_INTERRUPT_MASK_TP1	0x10000 /**< Mask the interrupt
							assertion for start of
							training pattern 1. */
#define XDPRX_INTERRUPT_MASK_TP2	0x20000 /**< Mask the interrupt
							assertion for start of
							training pattern 2. */
#define XDPRX_INTERRUPT_MASK_TP3	0x40000 /**< Mask the interrupt
							assertion for start of
							training pattern 3. */
#define XDPRX_INTERRUPT_MASK_ALL	0x7FFFF /**< Mask all interrupts. */
/* @} */

#endif /* XDPRX_HW_H_ */
