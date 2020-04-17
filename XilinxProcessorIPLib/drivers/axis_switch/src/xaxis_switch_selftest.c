/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch_selftest.c
* @addtogroup axis_switch_v1_3
* @{
*
* This file contains self test function for the AXI4-Stream Source Control
* Router core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxis_switch.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on AXI4-Stream Switch core registers.
*
* @param	InstancePtr is a pointer to the XAxis_Switch core instance.
*
* @return
*		- TRUE if self test passed.
*		- FALSE if self test failed.
*
* @note		None.
*
******************************************************************************/
s32 XAxisScr_SelfTest(XAxis_Switch *InstancePtr)
{
	u8 MiIndex;
	u8 SiIndex;
	s32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Disable all MI ports */
	XAxisScr_MiPortDisableAll(InstancePtr);

	/* Source SI[1] to MI[0] */
	MiIndex = 0;
	SiIndex = 1;
	XAxisScr_MiPortEnable(InstancePtr, MiIndex, SiIndex);

	/* Check for MI port enable */
	Status = XAxisScr_IsMiPortEnabled(InstancePtr, MiIndex, SiIndex);
	if (Status) {
		xil_printf("MI[%d] is sourced from SI[%d].\r\n", MiIndex,
				SiIndex);
	}

	return Status;
}
/** @} */
