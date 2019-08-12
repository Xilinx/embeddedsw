/******************************************************************************
*
* Copyright (C) 2016-2019 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xprc_selftest.c
* @addtogroup prc_v1_2
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
* 1.0   ms   07/18/2016    First release.
* 1.2   Nava 29/03/19      Updated the tcl logic to generated the
*                          XPrc_ConfigTable properly.
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
