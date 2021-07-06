/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps_selftest.c
* @addtogroup ttcps_v3_14
* @{
*
* This file contains the implementation of self test function for the
* XTtcPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
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


/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device.
*
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
*
* @return
*
*	- XST_SUCCESS if successful
*	- XST_FAILURE indicates a register did not read or write correctly
*
* @note		This test fails if it is not called right after initialization.
*
******************************************************************************/
s32 XTtcPs_SelfTest(XTtcPs *InstancePtr)
{
	s32 Status;
	u32 TempReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * All the TTC registers should be in their default state right now.
	 */
	TempReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				   XTTCPS_CNT_CNTRL_OFFSET);
	if (XTTCPS_CNT_CNTRL_RESET_VALUE != (u32)TempReg) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}
	return Status;
}
/** @} */
