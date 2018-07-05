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
 * @file xdppsu_serdes.c
 * Contains the set of functions used by the XDpPsu driver to configure SERDES.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	Who	Date	Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/27/17 Initial release.
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include "xdppsu_serdes.h"


/**
 *Voltage Swing Value table
 */
static const u32 vs[4][4] = {
		{ 0x2a, 0x27, 0x24, 0x20 },
		{ 0x27, 0x23, 0x20, 0xff },
		{ 0x24, 0x20, 0xff, 0xff },
		{ 0xff, 0xff, 0xff, 0xff },
};
/**
 * PreEmphasis Value table
 */
static const u32 pe[4][4] = {
		{ 0x02, 0x02, 0x02, 0x02 },
		{ 0x01, 0x01, 0x01, 0xff },
		{ 0x00, 0x00, 0xff, 0xff },
		{ 0xff, 0xff, 0xff, 0xff },
};

/******************************************************************************/
/**
 * This function sets current voltage swing and pre-emphasis level settings from
 * the LinkConfig structure to hardware.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	None.
*******************************************************************************/
static void XDpPsu_SetSerdesVswingPreemp(XDpPsu *InstancePtr)
{
	u8  VsLevelRx = InstancePtr->LinkConfig.VsLevel;
	u8  PeLevelRx = InstancePtr->LinkConfig.PeLevel;
	u8  Index;

	for (Index = 0; Index < InstancePtr->LinkConfig.LaneCount; Index++) {
		/* Write new voltage swing levels to the TX registers. */
		XDpPsu_WriteReg(SERDES_BASEADDR,
			SERDES_L0_TX_MARGININGF + Index * SERDES_LANE_OFFSET,
			vs[PeLevelRx][VsLevelRx]);

		/* Write new pre-emphasis levels to the TX registers. */
		XDpPsu_WriteReg(SERDES_BASEADDR,
			SERDES_L0_TX_DEEMPHASIS + Index * SERDES_LANE_OFFSET,
			pe[PeLevelRx][VsLevelRx]);
	}
}

/******************************************************************************/
/**
 * This function sets current voltage swing and pre-emphasis level settings from
 * the LinkConfig structure to hardware.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	AuxData is a pointer to the array used for preparing a burst
 *		write over the AUX channel.
 *
 * @return
 *		- XST_SUCCESS if writing the settings was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
*******************************************************************************/
void XDpPsu_SetVswingPreemp(XDpPsu *InstancePtr, u8 *AuxData)
{
	u8 Data = 0;
	u8 VsLevelRx = InstancePtr->LinkConfig.VsLevel;
	u8 PeLevelRx = InstancePtr->LinkConfig.PeLevel;

	/* The maximum voltage swing has been reached. */
	if (VsLevelRx >= XDPPSU_MAXIMUM_VS_LEVEL) {
		Data |= XDPPSU_DPCD_TRAINING_LANEX_SET_MAX_VS_MASK;
	}
	/* The maximum pre-emphasis level has been reached. */
	if (PeLevelRx >= XDPPSU_MAXIMUM_PE_LEVEL) {
		Data |= XDPPSU_DPCD_TRAINING_LANEX_SET_MAX_PE_MASK;
	}
	/* Set up the data buffer for writing to the RX device. */
	Data |= (PeLevelRx << XDPPSU_DPCD_TRAINING_LANEX_SET_PE_SHIFT) |
								VsLevelRx;
	memset(AuxData, Data, 4);

	XDpPsu_SetSerdesVswingPreemp(InstancePtr);
	return;
}

/******************************************************************************/
/**
 * This function sets the voltage swing level value in the DisplayPort TX that
 * will be used during link training for a given voltage swing training level.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Level is the voltage swing training level to set the DisplayPort
 *		TX level for.
 * @param	TxLevel is the DisplayPort TX voltage swing level value to be
 *		used during link training.
 *
 * @return	None.
 *
 * @note	There are 16 possible voltage swing levels in the DisplayPort TX
 *		core that map to 4 possible voltage swing training levels in the
 *		RX device.
 *
*******************************************************************************/
void XDpPsu_CfgTxVsLevel(XDpPsu *InstancePtr, u8 Level, u8 TxLevel)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Level < 4);
	Xil_AssertVoid(TxLevel < 16);

	InstancePtr->BoardChar.TxVsLevels[Level] = TxLevel;
}

/******************************************************************************/
/**
 * This function sets the pre-emphasis level value in the DisplayPort TX that
 * will be used during link training for a given pre-emphasis training level.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	Level is the pre-emphasis training level to set the DisplayPort
 *		TX level for.
 * @param	TxLevel is the DisplayPort TX pre-emphasis level value to be
 *		used during link training.
 *
 * @return	None.
 *
 * @note	There are 32 possible pre-emphasis levels in the DisplayPort TX
 *		core that map to 4 possible pre-emphasis training levels in the
 *		RX device.
 *
*******************************************************************************/
void XDpPsu_CfgTxPeLevel(XDpPsu *InstancePtr, u8 Level, u8 TxLevel)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Level < 4);
	Xil_AssertVoid(TxLevel < 32);

	InstancePtr->BoardChar.TxPeLevels[Level] = TxLevel;
}
