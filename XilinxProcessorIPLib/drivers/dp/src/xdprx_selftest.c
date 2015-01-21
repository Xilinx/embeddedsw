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
 * @file xdprx_selftest.c
 *
 * This file contains a diagnostic self-test function for the XDprx driver. It
 * will check many of the DisplayPort RX's register values against the default
 * reset values as a sanity-check that the core is ready to be used.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdprx.h"
#include "xstatus.h"

/**************************** Variable Definitions ****************************/

/**
 * This table contains the default values for the DisplayPort TX core's general
 * usage registers.
 */
u32 ResetValues[46][2] =
{
	{XDPRX_LINK_ENABLE, 0},
	{XDPRX_AUX_CLK_DIVIDER, 0},
	{XDPRX_DTG_ENABLE, 0},
	{XDPRX_USER_PIXEL_WIDTH, 0},
	{XDPRX_INTERRUPT_MASK, 0x7FFF},
	{XDPRX_MISC_CTRL, 0},
	{XDPRX_SOFT_RESET, 0},
	{XDPRX_AUX_REQ_IN_PROGRESS, 0},
	{XDPRX_REQ_ERROR_COUNT, 0},
	{XDPRX_REQ_COUNT, 0},
	{XDPRX_HPD_INTERRUPT, 0},
	{XDPRX_REQ_CLK_WIDTH, 0},
	{XDPRX_REQ_CMD, 0},
	{XDPRX_REQ_ADDRESS, 0},
	{XDPRX_REQ_LENGTH, 0},
	{XDPRX_INTERRUPT_CAUSE, 0},
	{XDPRX_INTERRUPT_MASK_1, 0},
	{XDPRX_INTERRUPT_CAUSE_1, 0},
	{XDPRX_HSYNC_WIDTH, 0xF0F},
	{XDPRX_FAST_I2C_DIVIDER, 0},
	{XDPRX_LOCAL_EDID_VIDEO, 0},
	{XDPRX_LOCAL_EDID_AUDIO, 0},
	{XDPRX_REMOTE_CMD, 0},
	{XDPRX_DEVICE_SERVICE_IRQ, 0},
	{XDPRX_VIDEO_UNSUPPORTED, 0},
	{XDPRX_AUDIO_UNSUPPORTED, 0},
	{XDPRX_OVER_LINK_BW_SET, 0},
	{XDPRX_OVER_LANE_COUNT_SET, 0},
	{XDPRX_OVER_TP_SET, 0},
	{XDPRX_OVER_TRAINING_LANE0_SET, 0},
	{XDPRX_OVER_TRAINING_LANE1_SET, 0},
	{XDPRX_OVER_TRAINING_LANE2_SET, 0},
	{XDPRX_OVER_TRAINING_LANE3_SET, 0},
	{XDPRX_OVER_CTRL_DPCD, 0},
	{XDPRX_OVER_DOWNSPREAD_CTRL, 0},
	{XDPRX_OVER_LINK_QUAL_LANE0_SET, 0},
	{XDPRX_OVER_LINK_QUAL_LANE1_SET, 0},
	{XDPRX_OVER_LINK_QUAL_LANE2_SET, 0},
	{XDPRX_OVER_LINK_QUAL_LANE3_SET, 0},
	{XDPRX_MST_CAP, 0},
	{XDPRX_SINK_COUNT, 0},
	{XDPRX_GUID0, 0},
	{XDPRX_GUID1, 0},
	{XDPRX_GUID2, 0},
	{XDPRX_GUID3, 0},
	{XDPRX_OVER_GUID, 0}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XDprx driver/device. The sanity test
 * checks whether or not all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDprx_SelfTest(XDprx *InstancePtr)
{
	u8 Index;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 46; Index++) {
		Val = XDprx_ReadReg(InstancePtr->Config.BaseAddr,
							ResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
