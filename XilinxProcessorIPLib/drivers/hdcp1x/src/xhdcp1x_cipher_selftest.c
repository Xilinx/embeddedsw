/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_selftest.c
*
* This file contains self test function for the Xilinx HDCP Cipher core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhdcp1x_cipher.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function self tests an hdcp cipher core.
*
* @param	InstancePtr is the cipher core to test.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_CipherSelfTest(XHdcp1x_Cipher *InstancePtr)
{
	u32 Base = 0;
	u32 Value = 0;
	int Status = XST_FAILURE;

	/* Verify parameters */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Base */
	Base = InstancePtr->CfgPtr->BaseAddress;

	/* Read the version */
	Value = XHdcp1x_CipherReadReg(Base, XHDCP1X_CIPHER_REG_VERSION);

	/* Confirm the version is reasonable */
	if ((Value != 0u) && (Value != ((u32)(-1)))) {
		const XHdcp1x_Config *CfgPtr = InstancePtr->CfgPtr;
		int IsRx = FALSE;
		int IsHdmi = FALSE;

		/* Determine isRx */
		Value = XHdcp1x_CipherReadReg(Base, XHDCP1X_CIPHER_REG_TYPE);
		Value &= XHDCP1X_CIPHER_BITMASK_TYPE_DIRECTION;
		if (Value == XHDCP1X_CIPHER_VALUE_TYPE_DIRECTION_RX) {
			IsRx = TRUE;
		}

		/* Determine isHdmi */
		Value = XHdcp1x_CipherReadReg(Base, XHDCP1X_CIPHER_REG_TYPE);
		Value &= XHDCP1X_CIPHER_BITMASK_TYPE_PROTOCOL;
		if (Value == XHDCP1X_CIPHER_VALUE_TYPE_PROTOCOL_HDMI) {
			IsHdmi = TRUE;
		}

		/* Confirm direction and protocol match */
		if (((IsRx == CfgPtr->IsRx)) && ((IsHdmi == CfgPtr->IsHDMI))) {
			Status = XST_SUCCESS;
		}
	}

	return (Status);
}
