/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprc_selftest.c
* @addtogroup prc Overview
* @{
*
* This file contains the self-test functions for the XPrc driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date         Changes
* ---- ----- -----------  ------------------------------------------------
* 1.0   ms    07/18/16    First release.
* 1.2   Nava  29/03/19    Updated the tcl logic to generated the
*                         XPrc_ConfigTable properly.
* 2.2   Nava  07/04/23    Fixed code formatting issues.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
extern u32 XPrc_GetRegisterAddress(XPrc *InstancePtr, u32 VsmId,
				   u8 RegisterType, u16 TableRow);

/*****************************************************************************/
/**
*
* This function runs a self-test on the PRC driver. This self test reads from
* the status register.
*
* @param	InstancePtr is a pointer to the XPrc instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*
* @note		None
*
******************************************************************************/
s32 XPrc_SelfTest(XPrc *InstancePtr)
{
	u16 VsmId = 0;
	u32 Value;
	u32 Address;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read value from the Status register */
	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
					  XPRC_STATUS_REG, XPRC_REG_TABLE_ROW);
	Value = XPrc_ReadReg(Address);

	return XST_SUCCESS;
}
/** @} */
