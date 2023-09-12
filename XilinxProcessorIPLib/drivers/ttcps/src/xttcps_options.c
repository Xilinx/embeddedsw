/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps_options.c
* @addtogroup ttcps Overview
* @{
*
* This file contains functions to get or set option features for the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 1.01a nm     03/05/2012 Removed break statement after return to remove
*                         compilation warnings.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.10  aru    05/16/19 Removed the redudant code from XTtcPs_SetOptions().
* 3.18  ml     09/08/23 Updated code to fix MISRA-C violation for Rule 14.3
* 3.18  ml     09/08/23 Added comments to fix HIS COMF violations.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xttcps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*
 * Create the table of options which are processed to get/set the device
 * options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
typedef struct {
	u32 Option;
	u32 Mask;
	u32 Register;
} OptionsMap;

static OptionsMap TmrCtrOptionsTable[] = {
	{
		XTTCPS_OPTION_EXTERNAL_CLK, XTTCPS_CLK_CNTRL_SRC_MASK,
		XTTCPS_CLK_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_CLK_EDGE_NEG, XTTCPS_CLK_CNTRL_EXT_EDGE_MASK,
		XTTCPS_CLK_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_INTERVAL_MODE, XTTCPS_CNT_CNTRL_INT_MASK,
		XTTCPS_CNT_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_DECREMENT, XTTCPS_CNT_CNTRL_DECR_MASK,
		XTTCPS_CNT_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_MATCH_MODE, XTTCPS_CNT_CNTRL_MATCH_MASK,
		XTTCPS_CNT_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_WAVE_DISABLE, XTTCPS_CNT_CNTRL_EN_WAVE_MASK,
		XTTCPS_CNT_CNTRL_OFFSET
	},
	{
		XTTCPS_OPTION_WAVE_POLARITY, XTTCPS_CNT_CNTRL_POL_WAVE_MASK,
		XTTCPS_CNT_CNTRL_OFFSET
	},
};

#define XTTCPS_NUM_TMRCTR_OPTIONS (sizeof(TmrCtrOptionsTable) / \
				   sizeof(OptionsMap))

/*****************************************************************************/
/**
*
* This function sets the options for the TTC device.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	Options contains the specified options to be set. This is a bit
*		mask where a 1 means to turn the option on, and a 0 means to
*		turn the option off. One or more bit values may be contained
*		in the mask. See the bit definitions named XTTCPS_*_OPTION in
*		the file xttcps.h.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_FAILURE if any of the options are unknown.
*
* @note		None
*
******************************************************************************/
s32 XTtcPs_SetOptions(XTtcPs *InstancePtr, u32 Options)
{
	u32 CountReg;
	u32 ClockReg;
	u32 Index;
	/*
	 * Validate input arguments and in case of error conditions assert.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	ClockReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XTTCPS_CLK_CNTRL_OFFSET);
	CountReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XTTCPS_CNT_CNTRL_OFFSET);
	/*
	 * Loop through the options table, turning the option on or off
	 * depending on whether the bit is set in the incoming options flag.
	 */
	for (Index = 0U; Index < XTTCPS_NUM_TMRCTR_OPTIONS; Index++) {
		if ((Options & TmrCtrOptionsTable[Index].Option) != (u32)0) {
			if (TmrCtrOptionsTable[Index].Register == XTTCPS_CLK_CNTRL_OFFSET) {
				ClockReg |= TmrCtrOptionsTable[Index].Mask;
			} else {
				CountReg |= TmrCtrOptionsTable[Index].Mask;
			}
		} else {
			if (TmrCtrOptionsTable[Index].Register == XTTCPS_CLK_CNTRL_OFFSET) {
				ClockReg &= ~TmrCtrOptionsTable[Index].Mask;
			} else {
				CountReg &= ~TmrCtrOptionsTable[Index].Mask;
			}
		}
	}
	/*
	 * Now write the registers. Leave it to the upper layers to restart the
	 * device.
	 */
	XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
			XTTCPS_CLK_CNTRL_OFFSET, ClockReg);
	XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
			XTTCPS_CNT_CNTRL_OFFSET, CountReg);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets the settings for the options for the TTC device.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
*
* @return
*
* The return u32 contains the specified options that are set. This is a bit
* mask where a '1' means the option is on, and a'0' means the option is off.
* One or more bit values may be contained in the mask. See the bit definitions
* named XTTCPS_*_OPTION in the file xttcps.h.
*
* @note		None.
*
******************************************************************************/
u32 XTtcPs_GetOptions(XTtcPs *InstancePtr)
{
	u32 OptionsFlag = 0U;
	u32 Register;
	u32 Index;

	/*
	 * Validate input arguments and in case of error conditions assert.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/*
	 * Loop through the options table to determine which options are set
	 */
	for (Index = 0U; Index < XTTCPS_NUM_TMRCTR_OPTIONS; Index++) {
		/*
		 * Get the control register to determine which options are
		 * currently set.
		 */
		Register = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
					  TmrCtrOptionsTable[Index].
					  Register);

		if ((Register & TmrCtrOptionsTable[Index].Mask) != (u32)0) {
			OptionsFlag |= TmrCtrOptionsTable[Index].Option;
		}
	}
	return OptionsFlag;
}
/** @} */
