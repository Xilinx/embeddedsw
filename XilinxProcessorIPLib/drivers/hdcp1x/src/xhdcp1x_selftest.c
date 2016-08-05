/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xhdcp1x_selftest.c
* @addtogroup hdcp1x_v4_0
* @{
*
* This file contains self test function for the hdcp interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_debug.h"
#include "xhdcp1x_port.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Extern Declarations ******************************/

/************************** Global Declarations ******************************/

/*****************************************************************************/
/**
* This function self tests an HDCP interface.
*
* @param	InstancePtr is the interface to test.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_SelfTest(XHdcp1x *InstancePtr)
{
	const XHdcp1x_Config *CfgPtr = &InstancePtr->Config;
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Confirm that the version is reasonable. */
	RegVal = XHdcp1x_ReadReg(CfgPtr->BaseAddress,
			XHDCP1X_CIPHER_REG_VERSION);
	if (!RegVal || (RegVal == ((u32)(-1)))) {
		return (XST_FAILURE);
	}

	/* Confirm that the direction matches in both SW and HW. */
	if ((!CfgPtr->IsRx && XHdcp1x_IsRX(InstancePtr)) ||
			(CfgPtr->IsRx && XHdcp1x_IsTX(InstancePtr))) {
		return (XST_FAILURE);
	}

	/* Confirm that the protocol matches in both SW and HW. */
	if ((!CfgPtr->IsHDMI && XHdcp1x_IsHDMI(InstancePtr)) ||
			(CfgPtr->IsHDMI && XHdcp1x_IsDP(InstancePtr))) {
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);
}
/** @} */
