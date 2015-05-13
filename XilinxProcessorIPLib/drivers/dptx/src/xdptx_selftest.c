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
 * @file xdptx_selftest.c
 *
 * This file contains a diagnostic self-test function for the XDptx driver. It
 * will check many of the DisplayPort TX's register values against the default
 * reset values as a sanity-check that the core is ready to be used.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  05/17/14 Initial release.
 * 3.0   als  12/16/14 Stream naming now starts at 1 to follow IP.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xstatus.h"

/**************************** Variable Definitions ****************************/

/**
 * This table contains the default values for the DisplayPort TX core's general
 * usage registers.
 */
u32 ResetValues[53][2] =
{
	{XDPTX_LINK_BW_SET, 0},
	{XDPTX_LANE_COUNT_SET, 0},
	{XDPTX_ENHANCED_FRAME_EN, 0},
	{XDPTX_TRAINING_PATTERN_SET, 0},
	{XDPTX_LINK_QUAL_PATTERN_SET, 0},
	{XDPTX_SCRAMBLING_DISABLE, 0},
	{XDPTX_DOWNSPREAD_CTRL, 0},
	{XDPTX_SOFT_RESET, 0},
	{XDPTX_ENABLE, 0},
	{XDPTX_ENABLE_MAIN_STREAM, 0},
	{XDPTX_ENABLE_SEC_STREAM, 0},
	{XDPTX_FORCE_SCRAMBLER_RESET, 0},
	{XDPTX_TX_MST_CONFIG, 0},
	{XDPTX_AUX_CMD, 0},
	{XDPTX_AUX_WRITE_FIFO, 0},
	{XDPTX_AUX_ADDRESS, 0},
	{XDPTX_AUX_CLK_DIVIDER, 0},
	{XDPTX_TX_USER_FIFO_OVERFLOW, 0},
	{XDPTX_AUX_REPLY_DATA, 0},
	{XDPTX_AUX_REPLY_CODE, 0},
	{XDPTX_AUX_REPLY_COUNT, 0},
	{XDPTX_INTERRUPT_MASK, 0x3F},
	{XDPTX_REPLY_DATA_COUNT, 0},
	{XDPTX_REPLY_STATUS, 0x10},
	{XDPTX_STREAM1, 0},
	{XDPTX_STREAM2, 0},
	{XDPTX_STREAM3, 0},
	{XDPTX_STREAM4, 0},
	{XDPTX_PHY_CONFIG, 0x03},
	{XDPTX_PHY_VOLTAGE_DIFF_LANE_0, 0},
	{XDPTX_PHY_VOLTAGE_DIFF_LANE_1, 0},
	{XDPTX_PHY_VOLTAGE_DIFF_LANE_2, 0},
	{XDPTX_PHY_VOLTAGE_DIFF_LANE_3, 0},
	{XDPTX_PHY_TRANSMIT_PRBS7, 0},
	{XDPTX_PHY_CLOCK_SELECT, 0},
	{XDPTX_TX_PHY_POWER_DOWN, 0},
	{XDPTX_PHY_PRECURSOR_LANE_0, 0},
	{XDPTX_PHY_PRECURSOR_LANE_1, 0},
	{XDPTX_PHY_PRECURSOR_LANE_2, 0},
	{XDPTX_PHY_PRECURSOR_LANE_3, 0},
	{XDPTX_PHY_POSTCURSOR_LANE_0, 0},
	{XDPTX_PHY_POSTCURSOR_LANE_1, 0},
	{XDPTX_PHY_POSTCURSOR_LANE_2, 0},
	{XDPTX_PHY_POSTCURSOR_LANE_3, 0},
	{XDPTX_GT_DRP_COMMAND, 0},
	{XDPTX_GT_DRP_READ_DATA, 0},
	{XDPTX_GT_DRP_CHANNEL_STATUS, 0},
	{XDPTX_TX_AUDIO_CONTROL, 0},
	{XDPTX_TX_AUDIO_CHANNELS, 0},
	{XDPTX_TX_AUDIO_INFO_DATA, 0},
	{XDPTX_TX_AUDIO_MAUD, 0},
	{XDPTX_TX_AUDIO_NAUD, 0},
	{XDPTX_TX_AUDIO_EXT_DATA, 0}
};

/**
 * This table contains the default values for the DisplayPort TX core's main
 * stream attribute (MSA) registers.
 */
u32 ResetValuesMsa[20][2] =
{
	{XDPTX_MAIN_STREAM_HTOTAL, 0},
	{XDPTX_MAIN_STREAM_VTOTAL, 0},
	{XDPTX_MAIN_STREAM_POLARITY, 0},
	{XDPTX_MAIN_STREAM_HSWIDTH, 0},
	{XDPTX_MAIN_STREAM_VSWIDTH, 0},
	{XDPTX_MAIN_STREAM_HRES, 0},
	{XDPTX_MAIN_STREAM_VRES, 0},
	{XDPTX_MAIN_STREAM_HSTART, 0},
	{XDPTX_MAIN_STREAM_VSTART, 0},
	{XDPTX_MAIN_STREAM_MISC0, 0},
	{XDPTX_MAIN_STREAM_MISC1, 0},
	{XDPTX_M_VID, 0},
	{XDPTX_TU_SIZE, 0},
	{XDPTX_N_VID, 0},
	{XDPTX_USER_PIXEL_WIDTH, 0},
	{XDPTX_USER_DATA_COUNT_PER_LANE, 0},
	{XDPTX_MAIN_STREAM_INTERLACED, 0},
	{XDPTX_MIN_BYTES_PER_TU, 0},
	{XDPTX_FRAC_BYTES_PER_TU, 0},
	{XDPTX_INIT_WAIT, 32}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XDptx driver/device. The sanity test
 * checks whether or not all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_SelfTest(XDptx *InstancePtr)
{
	u8 Index;
	u8 StreamIndex;
	u32 StreamOffset;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 53; Index++) {
		Val = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							ResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* Compare main stream attribute (MSA) registers for all 4 streams with
	 * their default values. */
	for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
		/* Determine the MSA register offset for each stream. */
		if (StreamIndex == 0) {
			StreamOffset = 0;
		}
		else if (StreamIndex == 1) {
			StreamOffset = XDPTX_STREAM2_MSA_START_OFFSET;
		}
		else if (StreamIndex == 2) {
			StreamOffset = XDPTX_STREAM3_MSA_START_OFFSET;
		}
		else if (StreamIndex == 3) {
			StreamOffset = XDPTX_STREAM4_MSA_START_OFFSET;
		}

		for (Index = 0; Index < 20; Index++) {
			Val = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
				StreamOffset + ResetValuesMsa[Index][0]);
			/* Fail if register does not hold default value. */
			if (Val != ResetValuesMsa[Index][1]) {
				return XST_FAILURE;
			}
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
