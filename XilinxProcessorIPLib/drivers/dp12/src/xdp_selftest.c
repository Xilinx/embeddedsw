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
 * @file xdp_selftest.c
 * @addtogroup dp_v6_0
 * @{
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
 * 5.0   als  07/27/16 8B10B enable is now set by default in PHY_CONFIG reg.
 * 6.0   tu   08/04/17 Changed Selftest regiter's default value.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Prototypes *****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
static u32 XDp_TxSelfTest(XDp *InstancePtr);
#endif
#if XPAR_XDPRXSS_NUM_INSTANCES
static u32 XDp_RxSelfTest(XDp *InstancePtr);
#endif

/**************************** Variable Definitions ****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
/**
 * This table contains the default values for the DisplayPort TX core's general
 * usage registers.
 */
u32 TxResetValues[2][2] =
{
	{XDP_TX_VERSION, 0x07000000},
	{XDP_TX_CORE_ID, 0x01020A00}
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
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/**
 * This table contains the default values for the DisplayPort RX core's general
 * usage registers.
 */
u32 RxResetValues[2][2] =
{
	{XDP_RX_VERSION, 0x07000000},
	{XDP_RX_CORE_ID, 0x01020A01}
};
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

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

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		Status = XDp_TxSelfTest(InstancePtr);
	} else
#endif
#if XPAR_XDPRXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_RX)	{
		Status = XDp_RxSelfTest(InstancePtr);
	} else
#endif
	{
		Status = XST_DEVICE_NOT_FOUND;
	}

	return Status;
}

#if XPAR_XDPTXSS_NUM_INSTANCES
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
	for (Index = 0; Index < 2; Index++) {
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
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
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
	for (Index = 0; Index < 2; Index++) {
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
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */
/** @} */
