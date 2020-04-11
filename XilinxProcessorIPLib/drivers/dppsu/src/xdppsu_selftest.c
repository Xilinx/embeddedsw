/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_selftest.c
 *
 * This file contains a diagnostic self-test function for the XDpPsu driver. It
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
 * 1.0   aad  01/17/17 Initial release.
 * 1.1   aad  10/04/17 Removed not applicable registers
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xstatus.h"

/**************************** Variable Definitions ****************************/

/**************************** Constant Definitions ****************************/
#define XDPPSU_NUM_RESET_VALUES		33
#define XDPPSU_NUM_MSA_RESET_VALUES	19
/**
 * This table contains the default values for the DisplayPort TX core's general
 * usage registers.
 */
u32 ResetValues[XDPPSU_NUM_RESET_VALUES][2] =
{
	{XDPPSU_LINK_BW_SET, 0},
	{XDPPSU_LANE_COUNT_SET, 0},
	{XDPPSU_ENHANCED_FRAME_EN, 0},
	{XDPPSU_TRAINING_PATTERN_SET, 0},
	{XDPPSU_LINK_QUAL_PATTERN_SET, 0},
	{XDPPSU_SCRAMBLING_DISABLE, 0},
	{XDPPSU_DOWNSPREAD_CTRL, 0},
	{XDPPSU_SOFT_RESET, 0},
	{XDPPSU_ENABLE, 0},
	{XDPPSU_ENABLE_MAIN_STREAM, 0},
	{XDPPSU_FORCE_SCRAMBLER_RESET, 0},
	{XDPPSU_AUX_CMD, 0},
	{XDPPSU_AUX_WRITE_FIFO, 0},
	{XDPPSU_AUX_ADDRESS, 0},
	{XDPPSU_AUX_CLK_DIVIDER, 0},
	{XDPPSU_TX_USER_FIFO_OVERFLOW, 0},
	{XDPPSU_AUX_REPLY_CODE, 0},
	{XDPPSU_AUX_REPLY_COUNT, 0},
	{XDPPSU_INTR_MASK, 0xFFFFF03F},
	{XDPPSU_REPLY_DATA_COUNT, 0},
	{XDPPSU_REPLY_STATUS, 0x10},
	{XDPPSU_PHY_CONFIG, 0x10001},
	{XDPPSU_PHY_TRANSMIT_PRBS7, 0},
	{XDPPSU_PHY_CLOCK_SELECT, 0},
	{XDPPSU_TX_PHY_POWER_DOWN, 0},
	{XDPPSU_PHY_PRECURSOR_LANE_0, 0},
	{XDPPSU_PHY_PRECURSOR_LANE_1, 0},
	{XDPPSU_TX_AUDIO_CONTROL, 0},
	{XDPPSU_TX_AUDIO_CHANNELS, 0},
	{XDPPSU_TX_AUDIO_INFO_DATA, 0},
	{XDPPSU_TX_AUDIO_MAUD, 0},
	{XDPPSU_TX_AUDIO_NAUD, 0},
	{XDPPSU_TX_AUDIO_EXT_DATA, 0}
};

/**
 * This table contains the default values for the DisplayPort TX core's main
 * stream attribute (MSA) registers.
 */
u32 ResetValuesMsa[XDPPSU_NUM_MSA_RESET_VALUES][2] =
{
	{XDPPSU_MAIN_STREAM_HTOTAL, 0},
	{XDPPSU_MAIN_STREAM_VTOTAL, 0},
	{XDPPSU_MAIN_STREAM_POLARITY, 0},
	{XDPPSU_MAIN_STREAM_HSWIDTH, 0},
	{XDPPSU_MAIN_STREAM_VSWIDTH, 0},
	{XDPPSU_MAIN_STREAM_HRES, 0},
	{XDPPSU_MAIN_STREAM_VRES, 0},
	{XDPPSU_MAIN_STREAM_HSTART, 0},
	{XDPPSU_MAIN_STREAM_VSTART, 0},
	{XDPPSU_MAIN_STREAM_MISC0, 0},
	{XDPPSU_MAIN_STREAM_MISC1, 0},
	{XDPPSU_M_VID, 0},
	{XDPPSU_TU_SIZE, 0x40},
	{XDPPSU_N_VID, 0},
	{XDPPSU_USER_PIXEL_WIDTH, 1},
	{XDPPSU_USER_DATA_COUNT_PER_LANE, 0},
	{XDPPSU_MIN_BYTES_PER_TU, 0},
	{XDPPSU_FRAC_BYTES_PER_TU, 0},
	{XDPPSU_INIT_WAIT, 0x20}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XDpPsu driver/device. The sanity test
 * checks whether or not all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDpPsu_SelfTest(XDpPsu *InstancePtr)
{
	u8 Index;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < XDPPSU_NUM_RESET_VALUES; Index++) {
		Val = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							ResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* Compare main stream attribute (MSA) registers with their default
	 * values. */
	for (Index = 0; Index < XDPPSU_NUM_MSA_RESET_VALUES; Index++) {
		Val = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
						ResetValuesMsa[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValuesMsa[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
