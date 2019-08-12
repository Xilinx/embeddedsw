/******************************************************************************
*
* Copyright (C)2016 Xilinx, Inc. All rights reserved.
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
* @file xcsi2tx_selftest.c
* @addtogroup csi2tx_v1_0
* @{
*
* Contains diagnostic/self-test functions for the CSI Tx Controller core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/28/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xcsi2tx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

#define GSP_MASK		0x3F
#define GSP_DEFAULT		0x20
#define SPKTR_MASK		0xFFFF
#define SPKTR_DEFAULT		0x00
#define IER_MASK		0x3F
#define IER_DEFAULT		0x00
#define ISR_MASK		0x3F
#define ISR_DEFAULT		0x00
#define GIER_MASK		0x01
#define GIER_DEFAULT		0x00
#define ACT_LANES_MASK		0xE01B
#define CCR_MASK		0x0F
#define CCR_DEFAULT		0x04
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. This test checks if the LaneCount
* present in register matches the one from the generated file.
*
* @param	InstancePtr is a pointer to the XCsi instance.
*
* @return
*		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the read value was not equal to _g.c file
*
* @note		None
*
******************************************************************************/
u32 XCsi2Tx_SelfTest(XCsi2Tx *InstancePtr)
{
	u32 Result, RegValue, mask;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCsi2Tx_Reset(InstancePtr);

	if (Result == XST_FAILURE)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_CCR_OFFSET);
	if ((RegValue & CCR_MASK) != CCR_DEFAULT)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_PCR_OFFSET);

	mask = ((InstancePtr->Config.MaxLanesPresent - 1) << 3) |
						(InstancePtr->Config.MaxLanesPresent - 1);

	if ((RegValue & ACT_LANES_MASK) != mask)
		return XST_FAILURE;
	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_GIER_OFFSET);
	if ((RegValue & GIER_MASK) != GIER_DEFAULT)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_ISR_OFFSET);
	if ((RegValue & ISR_MASK) != ISR_DEFAULT)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_IER_OFFSET);
	if ((RegValue & IER_MASK) != IER_DEFAULT)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_SPKTR_OFFSET);
	if ((RegValue & SPKTR_MASK) != SPKTR_DEFAULT)
		return XST_FAILURE;

	RegValue = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_GSP_OFFSET);
	if ((RegValue & GSP_MASK) != GSP_DEFAULT)
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */
