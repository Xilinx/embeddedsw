/******************************************************************************
*
* Copyright (C)2017 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_sditx_selftest.c
* @addtogroup xv_sditx_v1_1
* @{
*
* Contains diagnostic/self-test functions for the SDI Tx Controller core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xv_sditx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

#define SDITX_RST_CTRL_DEFAULT	0x000
#define SDITX_MDL_CTRL_DEFAULT	0x117000

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the mode control
* register valus have the default values.
*
* @param	InstancePtr is a pointer to the XV_SdiTx instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note		None
*
******************************************************************************/
u32 XV_SdiTx_SelfTest(XV_SdiTx *InstancePtr)
{
	u32 Result, RegValue;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XV_SdiTx_StopSdi(InstancePtr);

	if (Result == XST_FAILURE)
		return XST_FAILURE;

	RegValue = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_RST_CTRL_OFFSET);
	if (RegValue != SDITX_RST_CTRL_DEFAULT)
		return XST_FAILURE;

	RegValue = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_MDL_CTRL_OFFSET);
	if (RegValue != SDITX_MDL_CTRL_DEFAULT)
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */
