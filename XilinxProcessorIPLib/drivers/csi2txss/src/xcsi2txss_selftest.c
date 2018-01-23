/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xcsi2txss_selftest.c
* @addtogroup csi2txss_v1_1
* @{
* This file contains self test function for the MIPI CSI Rx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- -----------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XCsi2TxSs_SelfTest(XCsi2TxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI CSI Tx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XCsi2TxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XCsi2TxSs_SelfTest(XCsi2TxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->CsiPtr) {
		Status = XCsi2Tx_SelfTest(InstancePtr->CsiPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "ERR::CSI2TXSS Self test"
			"failed\n\r");
			return Status;
		}
	}
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		Status = XDphy_SelfTest(InstancePtr->DphyPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,"ERR::Dphy Self test "
			"failed\n\r");
			return Status;
		}
	}
#endif
	return XST_SUCCESS;
}
/** @} */
