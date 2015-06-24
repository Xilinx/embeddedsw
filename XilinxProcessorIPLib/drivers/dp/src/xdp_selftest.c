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
 * @file xdp_selftest.c
 *
 * This file contains a diagnostic self-test function for the XDp driver. It
 * will check many of the DisplayPort core's register values against the default
 * reset values as a sanity-check that the core is ready to be used.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Prototypes *****************************/

static u32 XDp_TxSelfTest(XDp *InstancePtr);
static u32 XDp_RxSelfTest(XDp *InstancePtr);

/**************************** Variable Definitions ****************************/

/**
 * This table contains the default values for the DisplayPort TX core's general
 * usage registers.
 */
u32 TxResetValues[53][2] =
{
	{XDP_TX_LINK_BW_SET, 0},
	{XDP_TX_LANE_COUNT_SET, 0},
	{XDP_TX_ENHANCED_FRAME_EN, 0},
	{XDP_TX_TRAINING_PATTERN_SET, 0},
	{XDP_TX_LINK_QUAL_PATTERN_SET, 0},
	{XDP_TX_SCRAMBLING_DISABLE, 0},
	{XDP_TX_DOWNSPREAD_CTRL, 0},
	{XDP_TX_SOFT_RESET, 0},
	{XDP_TX_ENABLE, 0},
	{XDP_TX_ENABLE_MAIN_STREAM, 0},
	{XDP_TX_ENABLE_SEC_STREAM, 0},
	{XDP_TX_FORCE_SCRAMBLER_RESET, 0},
	{XDP_TX_MST_CONFIG, 0},
	{XDP_TX_AUX_CMD, 0},
	{XDP_TX_AUX_WRITE_FIFO, 0},
	{XDP_TX_AUX_ADDRESS, 0},
	{XDP_TX_AUX_CLK_DIVIDER, 0},
	{XDP_TX_USER_FIFO_OVERFLOW, 0},
	{XDP_TX_AUX_REPLY_DATA, 0},
	{XDP_TX_AUX_REPLY_CODE, 0},
	{XDP_TX_AUX_REPLY_COUNT, 0},
	{XDP_TX_INTERRUPT_MASK, 0x3F},
	{XDP_TX_REPLY_DATA_COUNT, 0},
	{XDP_TX_REPLY_STATUS, 0x10},
	{XDP_TX_STREAM1, 0},
	{XDP_TX_STREAM2, 0},
	{XDP_TX_STREAM3, 0},
	{XDP_TX_STREAM4, 0},
	{XDP_TX_PHY_CONFIG, 0x03},
	{XDP_TX_PHY_VOLTAGE_DIFF_LANE_0, 0},
	{XDP_TX_PHY_VOLTAGE_DIFF_LANE_1, 0},
	{XDP_TX_PHY_VOLTAGE_DIFF_LANE_2, 0},
	{XDP_TX_PHY_VOLTAGE_DIFF_LANE_3, 0},
	{XDP_TX_PHY_TRANSMIT_PRBS7, 0},
	{XDP_TX_PHY_CLOCK_SELECT, 0},
	{XDP_TX_PHY_POWER_DOWN, 0},
	{XDP_TX_PHY_PRECURSOR_LANE_0, 0},
	{XDP_TX_PHY_PRECURSOR_LANE_1, 0},
	{XDP_TX_PHY_PRECURSOR_LANE_2, 0},
	{XDP_TX_PHY_PRECURSOR_LANE_3, 0},
	{XDP_TX_PHY_POSTCURSOR_LANE_0, 0},
	{XDP_TX_PHY_POSTCURSOR_LANE_1, 0},
	{XDP_TX_PHY_POSTCURSOR_LANE_2, 0},
	{XDP_TX_PHY_POSTCURSOR_LANE_3, 0},
	{XDP_TX_GT_DRP_COMMAND, 0},
	{XDP_TX_GT_DRP_READ_DATA, 0},
	{XDP_TX_GT_DRP_CHANNEL_STATUS, 0},
	{XDP_TX_AUDIO_CONTROL, 0},
	{XDP_TX_AUDIO_CHANNELS, 0},
	{XDP_TX_AUDIO_INFO_DATA(1), 0},
	{XDP_TX_AUDIO_MAUD, 0},
	{XDP_TX_AUDIO_NAUD, 0},
	{XDP_TX_AUDIO_EXT_DATA(1), 0}
};

/**
 * This table contains the default values for the DisplayPort TX core's main
 * stream attribute (MSA) registers.
 */
u32 TxResetValuesMsa[20][2] =
{
	{XDP_TX_MAIN_STREAM_HTOTAL, 0},
	{XDP_TX_MAIN_STREAM_VTOTAL, 0},
	{XDP_TX_MAIN_STREAM_POLARITY, 0},
	{XDP_TX_MAIN_STREAM_HSWIDTH, 0},
	{XDP_TX_MAIN_STREAM_VSWIDTH, 0},
	{XDP_TX_MAIN_STREAM_HRES, 0},
	{XDP_TX_MAIN_STREAM_VRES, 0},
	{XDP_TX_MAIN_STREAM_HSTART, 0},
	{XDP_TX_MAIN_STREAM_VSTART, 0},
	{XDP_TX_MAIN_STREAM_MISC0, 0},
	{XDP_TX_MAIN_STREAM_MISC1, 0},
	{XDP_TX_M_VID, 0},
	{XDP_TX_TU_SIZE, 0},
	{XDP_TX_N_VID, 0},
	{XDP_TX_USER_PIXEL_WIDTH, 0},
	{XDP_TX_USER_DATA_COUNT_PER_LANE, 0},
	{XDP_TX_MAIN_STREAM_INTERLACED, 0},
	{XDP_TX_MIN_BYTES_PER_TU, 0},
	{XDP_TX_FRAC_BYTES_PER_TU, 0},
	{XDP_TX_INIT_WAIT, 32}
};

/**
 * This table contains the default values for the DisplayPort RX core's general
 * usage registers.
 */
u32 RxResetValues[46][2] =
{
	{XDP_RX_LINK_ENABLE, 0},
	{XDP_RX_AUX_CLK_DIVIDER, 0},
	{XDP_RX_DTG_ENABLE, 0},
	{XDP_RX_USER_PIXEL_WIDTH, 0},
	{XDP_RX_INTERRUPT_MASK, 0x7FFF},
	{XDP_RX_MISC_CTRL, 0},
	{XDP_RX_SOFT_RESET, 0},
	{XDP_RX_AUX_REQ_IN_PROGRESS, 0},
	{XDP_RX_REQ_ERROR_COUNT, 0},
	{XDP_RX_REQ_COUNT, 0},
	{XDP_RX_HPD_INTERRUPT, 0},
	{XDP_RX_REQ_CLK_WIDTH, 0},
	{XDP_RX_REQ_CMD, 0},
	{XDP_RX_REQ_ADDRESS, 0},
	{XDP_RX_REQ_LENGTH, 0},
	{XDP_RX_INTERRUPT_CAUSE, 0},
	{XDP_RX_INTERRUPT_MASK_1, 0},
	{XDP_RX_INTERRUPT_CAUSE_1, 0},
	{XDP_RX_HSYNC_WIDTH, 0xF0F},
	{XDP_RX_FAST_I2C_DIVIDER, 0},
	{XDP_RX_LOCAL_EDID_VIDEO, 0},
	{XDP_RX_LOCAL_EDID_AUDIO, 0},
	{XDP_RX_REMOTE_CMD, 0},
	{XDP_RX_DEVICE_SERVICE_IRQ, 0},
	{XDP_RX_VIDEO_UNSUPPORTED, 0},
	{XDP_RX_AUDIO_UNSUPPORTED, 0},
	{XDP_RX_OVER_LINK_BW_SET, 0},
	{XDP_RX_OVER_LANE_COUNT_SET, 0},
	{XDP_RX_OVER_TP_SET, 0},
	{XDP_RX_OVER_TRAINING_LANE0_SET, 0},
	{XDP_RX_OVER_TRAINING_LANE1_SET, 0},
	{XDP_RX_OVER_TRAINING_LANE2_SET, 0},
	{XDP_RX_OVER_TRAINING_LANE3_SET, 0},
	{XDP_RX_OVER_CTRL_DPCD, 0},
	{XDP_RX_OVER_DOWNSPREAD_CTRL, 0},
	{XDP_RX_OVER_LINK_QUAL_LANE0_SET, 0},
	{XDP_RX_OVER_LINK_QUAL_LANE1_SET, 0},
	{XDP_RX_OVER_LINK_QUAL_LANE2_SET, 0},
	{XDP_RX_OVER_LINK_QUAL_LANE3_SET, 0},
	{XDP_RX_MST_CAP, 0},
	{XDP_RX_SINK_COUNT, 0},
	{XDP_RX_GUID0, 0},
	{XDP_RX_GUID1, 0},
	{XDP_RX_GUID2, 0},
	{XDP_RX_GUID3, 0},
	{XDP_RX_OVER_GUID, 0}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XDp driver/device depending on whether
 * the core is operating in TX or RX mode. The sanity test checks whether or not
 * all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_SelfTest(XDp *InstancePtr)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		Status = XDp_TxSelfTest(InstancePtr);
	}
	else {
		Status = XDp_RxSelfTest(InstancePtr);
	}

	return Status;
}

/******************************************************************************/
/**
 * This function runs a self-test on the XDp driver/device. The sanity test
 * checks whether or not all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxSelfTest(XDp *InstancePtr)
{
	u8 Index;
	u8 StreamIndex;
	u32 StreamOffset;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 53; Index++) {
		Val = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						TxResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != TxResetValues[Index][1]) {
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
			StreamOffset = XDP_TX_STREAM2_MSA_START_OFFSET;
		}
		else if (StreamIndex == 2) {
			StreamOffset = XDP_TX_STREAM3_MSA_START_OFFSET;
		}
		else if (StreamIndex == 3) {
			StreamOffset = XDP_TX_STREAM4_MSA_START_OFFSET;
		}

		for (Index = 0; Index < 20; Index++) {
			Val = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				StreamOffset + TxResetValuesMsa[Index][0]);
			/* Fail if register does not hold default value. */
			if (Val != TxResetValuesMsa[Index][1]) {
				return XST_FAILURE;
			}
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function runs a self-test on the XDp driver/device running in RX mode.
 * The sanity test checks whether or not all tested registers hold their default
 * reset values.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxSelfTest(XDp *InstancePtr)
{
	u8 Index;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 46; Index++) {
		Val = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						RxResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != RxResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
